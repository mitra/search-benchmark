/* $Id: info_util.c,v 1.6 2004/02/16 20:52:07 alex Exp alex $ */
/*******************************************************************************

File:

    info_util.c

    Information Repository Utilities.


Author:    Alex Measday


Purpose:

    The INFO_UTIL package implements an information repository that stores
    key/value mappings.  A "key" is a string consisting of one or more names
    concatenated and separated by periods:

        thisStream.abc.xyz.packetID

    The final name ("packetID") is known as the key "name"; the preceding
    portion ("thisStream.abc.xyz") is called the key "path".

    Key/value mappings are defined in X Resource Database-like files:

        thisStream.abc.xyz.packetID:	123
        *packetID:			0
        ...

    Wildcards ("*"s) are allowed in key paths (but not key names) when
    a key/value mapping is defined.  Given the definitions above, the
    value retrieved for "thisStream.abc.uvw.packetID" would be 0; the
    value retrieved for "thisStream.abc.xyz.packetID" would be 123.

    An information repository is created as follows:

        #include  "info_util.h"		-- Information repository utilities.
        Repository  dictionary ;
        ...
        infoCreate ("<definitionFile>", "<options>", &dictionary) ;

    infoCreate() creates an empty repository and loads an initial set of
    definitions from a file.  infoMerge() can be called to add definitions
    from other files.  An application can also add definitions on the fly:

        ...
        infoSet (dictionary, "thisStream.abc.xyz", "packetID", "123") ;
        infoSet (dictionary, "*", "packetID", "0") ;
        ...

    Looking up a key in a repository produces the string value to which
    that key maps:

        ...
        value = infoGet (dictionary, "thisStream.abc.xyz", "packetID", 0) ;
        printf ("%s", value) ;			-- Displays "123"
        value = infoGet (dictionary, "thisStream.abc.uvw", "packetID", 0) ;
        printf ("%s", value) ;			-- Displays "0"
        ...

    Note that looking up "thisStream.abc.xyz"'s packet ID returns "123";
    looking up "thisStream.abc.uvw.packetID", on the other hand, matches
    its key to the "*packetID" definition and returns "0".

    Unneeded definitions can be deleted from a repository:

        infoDelete (dictionary, "thisStream.abc.uvw", "packetID", 0) ;

    and an unneeded repository can be deleted from an application:

        infoDestroy (dictionary) ;


Public Procedures:

    infoCreate() - creates an information repository.
    infoDelete() - deletes a definition from a repository.
    infoDestroy() - destroys an information repository.
    infoGet() - retrieves a definition from a repository.
    infoMerge() - loads definitions from a file and adds them to a repository.
    infoSave() - saves a repository to disk.
    infoSet() - adds a definition to a repository.

Private Procedures:

    dfnCreate() - creates a definition.
    dfnDestroy() - destroys a definition.
    infoMatch() - matches a target key against a definition's key.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "hash_util.h"			/* Hash table utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "rex_util.h"			/* Regular expression definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "info_util.h"			/* Information repository utilities. */


/*******************************************************************************
    Information Repository - in which the information reposes.
*******************************************************************************/

typedef  struct  _Repository {
    FileName  definitionFile ;		/* Name of initial definition file. */
    bool  useCPP ;			/* Filter files through CPP(1)? */
    HashTable  table ;			/* Provides fast lookup of keys. */
}  _Repository ;


/*******************************************************************************
    Definition - defines a key/value mapping.
*******************************************************************************/

typedef  struct  _Definition {
    char  *name ;			/* Final component of key. */
    char  *path ;			/* Preceding components of key. */
    CompiledRE  pattern ;		/* Regular expression for path. */
    char  *value ;			/* Key's value. */
    struct  _Definition  *next ;
}  _Definition, *Definition ;


int  info_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  info_util_debug


#if defined(VXWORKS)
#elif defined(VMS)
#    define  CPP_COMMAND  "CC/PREPROCESS=SYS$OUTPUT:"
#else
#    define  CPP_COMMAND  "/lib/cpp"
#endif
#define  DESTROY(dictionary) \
    (infoDestroy ((dictionary)), (dictionary) = NULL)
#define  MAX_DEFINITIONS  128
#define  MAX_LINE  512


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  bool  infoMatch (
#    if PROTOTYPES
        const  char  *name,
        const  char  *path,
        Definition  definition
#    endif
    ) ;

static  errno_t  dfnCreate (
#    if PROTOTYPES
        const  char  *text,
        Definition  *definition
#    endif
    ) ;

static  errno_t  dfnDestroy (
#    if PROTOTYPES
        Definition  definition
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    infoCreate ()

    Create an Information Repository.


Purpose:

    Function infoCreate() creates an empty information repository and
    loads its initial set of definitions from a file.

    The options argument passed into this function is a string containing
    zero or more of the following UNIX command line-style options:

        "-cpp"
            causes the definition file to be filtered through the C
            Preprocessor, cpp(1).
        "-max <items>"
            specifies the estimated number of definitions that will be
            entered in the repository; the default is 128.  This number
            is used to size the hash table.  There is no harm in exceeding
            the "maximum", except that collisions between keys may slow
            lookups.


    Invocation:

        status = infoCreate (definitionFile, options, &dictionary) ;

    where:

        <definitionFile>	- I
            is the name of the file from which the initial set of definitions
            will be loaded.  If this argument is NULL, an empty repository is
            created.
        <options>		- I
            is a string containing zero or more of the UNIX command line-style
            options described above.
        <dictionary>		- O
            returns a handle for the repository.  This handle is used in calls
            to the other INFO_UTIL functions.
        <status>		- O
            returns the status of creating the repository, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  infoCreate (

#    if PROTOTYPES
        const  char  *definitionFile,
        const  char  *options,
        Repository  *dictionary)
#    else
        definitionFile, options, dictionary)

        char  *definitionFile ;
        char  *options ;
        Repository  *dictionary ;
#    endif

{    /* Local variables. */
    bool  useCPP ;
    char  *argument, **argv ;
    int  argc, errflg, maxDefinitions, option ;
    OptContext  context ;

    static  const  char  *optionList[] = {
        "{cpp}", "{maximum:}", NULL
    } ;




    *dictionary = NULL ;

/*******************************************************************************
    Convert the options string into an ARGC/ARGV array and scan the arguments.
*******************************************************************************/

    maxDefinitions = MAX_DEFINITIONS ;
    useCPP = false ;

    if (options != NULL) {

        opt_create_argv ("infoCreate", options, &argc, &argv) ;
        opt_init (argc, argv, NULL, optionList, &context) ;
        opt_errors (context, false) ;

        errflg = 0 ;
        while ((option = opt_get (context, &argument))) {
            switch (option) {
            case 1:			/* "-cpp" */
                useCPP = true ;
                break ;
            case 2:			/* "-maximum <definitions>" */
                maxDefinitions = atoi (argument) ;
                break ;
            case NONOPT:
            case OPTERR:
            default:
                errflg++ ;  break ;
            }
        }

        opt_term (context) ;
        opt_delete_argv (argc, argv) ;

        if (errflg) {
            SET_ERRNO (EINVAL) ;
            LGE "(infoCreate) Invalid option/argument: \"%s\"\n", options) ;
            return (errno) ;
        }

    }


/* Create and initialize a repository structure. */

    *dictionary = (_Repository *) malloc (sizeof (_Repository)) ;
    if (*dictionary == NULL) {
        LGE "(infoCreate) Error allocating repository structure.\nmalloc: ") ;
        return (errno) ;
    }

    (*dictionary)->definitionFile = NULL ;
    (*dictionary)->useCPP = useCPP ;
    (*dictionary)->table = NULL ;

/* Create a FileName object for the initial definition file. */

    if (definitionFile != NULL) {
        (*dictionary)->definitionFile = fnmCreate (definitionFile, NULL) ;
        if ((*dictionary)->definitionFile == NULL) {
            LGE "(infoCreate) Error duplicating initial definition file name: %s\nfnmCreate: ",
                definitionFile) ;
            PUSH_ERRNO ;  DESTROY (*dictionary) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

/* Create an empty hash table that will provide fast key lookups. */

    if (hashCreate (maxDefinitions, &(*dictionary)->table)) {
        LGE "(infoCreate) Error creating %d-item hash table.\nhashCreate: ",
            maxDefinitions) ;
        PUSH_ERRNO ;  DESTROY (*dictionary) ;  POP_ERRNO ;
        return (errno) ;
    }

/* If an initial definition file was specified, then load it. */

    if ((definitionFile != NULL) &&
        infoMerge (*dictionary, fnmPath ((*dictionary)->definitionFile))) {
        LGE "(infoCreate) Error loading initial definition file, %s.\ninfoMerge: ",
            fnmPath ((*dictionary)->definitionFile)) ;
        PUSH_ERRNO ;  DESTROY (*dictionary) ;  POP_ERRNO ;
        return (errno) ;
    }

    if (definitionFile == NULL) {
        LGI "(infoCreate) New repository %p.\n", (void *) *dictionary) ;
    } else {
        LGI "(infoCreate) New repository %p.  (%s)\n",
            (void *) *dictionary, fnmPath ((*dictionary)->definitionFile)) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    infoDestroy ()

    Destroy an Information Repository.


Purpose:

    The infoDestroy() function destroys an information repository.


    Invocation:

        status = infoDestroy (dictionary) ;

    where

        <dictionary>	- I
            is the repository handle returned by infoCreate().
        <status>	- O
            returns the status of destroying the repository, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  infoDestroy (

#    if PROTOTYPES
        Repository  dictionary)
#    else
        dictionary)

        Repository  dictionary ;
#    endif

{

    if (dictionary == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(infoDestroy) NULL repository handle: ") ;
        return (errno) ;
    }

    free (dictionary) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    infoMerge ()

    Load Definitions from a File into a Repository.


Purpose:

    Function infoMerge() reads definitions from a file and adds the
    definitions to a repository.


    Invocation:

        status = infoMerge (dictionary, definitionFile) ;

    where:

        <dictionary>		- I
            is the repository handle returned by infoCreate().
        <definitionFile>	- I
            is the name of the file being loaded.
        <status>		- O
            returns the status of loading the definitions, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  infoMerge (

#    if PROTOTYPES
        Repository  dictionary,
        const  char  *definitionFile)
#    else
        dictionary, definitionFile)

        Repository  dictionary ;
        char  *definitionFile ;
#    endif

{    /* Local variables. */
    bool  isContinued ;
    char  buffer[MAX_LINE], cppFileName[PATH_MAX+2], *inputLine, *s ;
    Definition  definition, next ;
    FILE  *file ;
    int  cppLineNumber, length ;




    if (dictionary == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(infoMerge) NULL repository handle: ") ;
        return (errno) ;
    }


/*******************************************************************************
    Open the definition file.
*******************************************************************************/

#ifdef CPP_COMMAND
    if (dictionary->useCPP) {		/* Pipe the input file through CPP(1)? */

        char  command[PATH_MAX] ;

        sprintf (command, "%s %s", CPP_COMMAND, definitionFile) ;
        file = popen (command, "r") ;
        if (file == NULL) {
            LGE "(infoMerge) Error opening %s.\npopen: ", definitionFile) ;
            return (errno) ;
        }
#else
    if (0) {
#endif

    } else {				/* No. */

        file = fopen (definitionFile, "r") ;
        if (file == NULL) {
            LGE "(infoMerge) Error opening %s.\nfopen: ", definitionFile) ;
            return (errno) ;
        }

    }


/*******************************************************************************
    Read the definitions and add them to the repository.
*******************************************************************************/

    strcpy (cppFileName, definitionFile) ;  cppLineNumber = 0 ;
    inputLine = NULL ;

/* Read each line from the file. */

    while (fgets (buffer, sizeof buffer, file) != NULL) {

        cppLineNumber++ ;

        length = strlen (buffer) ;
        if ((length > 0) && (buffer[length-1] == '\n'))
            buffer[--length] = '\0' ;
        if ((length == 0) && (inputLine == NULL))  continue ;

/* Check for a cpp(1) tag specifying the current file and line number. */

        if ((buffer[0] == '#') && dictionary->useCPP) {
            sscanf (buffer, "# %d %s", &cppLineNumber, cppFileName) ;
            cppLineNumber-- ;
            continue ;
        }

/* Skip blank lines and comments. */

        s = buffer + strspn (buffer, " \t\f") ;
        if ((*s == '\0') || (*s == '!'))  continue ;

/* Check if the current line is continued on the next line. */

        if ((length > 0) && (buffer[length-1] == '\\')) {
            buffer[--length] = '\0' ;
            isContinued = true ;
        } else {
            isContinued = false ;
        }

/* Make a copy of the current line, appending it to the previous line if
   it is a continuation line. */

        if ((inputLine == NULL) && !(inputLine = strdup (""))) {
            LGE "(infoMerge) Error initializing input line in %s.\nstrdup: ",
                definitionFile) ;
            return (errno) ;
        }

        inputLine = realloc (inputLine, strlen (inputLine) + length + 1) ;
        if (inputLine == NULL) {
            LGE "(infoMerge) Error enlarging input line in %s.\nrealloc: ",
                definitionFile) ;
            return (errno) ;
        }

        strcat (inputLine, buffer) ;

/* If the current line is continued on the next line, then read the next line. */

        if (isContinued)  continue ;

/* Parse the input line into its components. */

        if (dfnCreate (inputLine, &definition)) {
            LGE "(infoMerge) Error at line %d in %s.\ndfnCreate: ",
                cppLineNumber, cppFileName) ;
            return (errno) ;
        }

/* Add the new key/value definition to the dictionary. */

        next = hashFind (dictionary->table, definition->name) ;
        definition->next = next ;
        if (hashAdd (dictionary->table,
                     definition->name, (void *) definition)) {
            LGE "(infoMerge) Error adding definition at line %d in %s.\nhashAdd: ",
                cppLineNumber, cppFileName) ;
            return (errno) ;
        }

        free (inputLine) ;  inputLine = NULL ;

    }


/*******************************************************************************
    Close the file.
*******************************************************************************/

    if (dictionary->useCPP)
        pclose (file) ;
    else
        fclose (file) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    infoSave ()

    Save Definitions in a Repository to a File.


Purpose:

    Function infoSave() saves the definitions in a repository to a file.


    Invocation:

        status = infoSave (dictionary, definitionFile) ;

    where:

        <dictionary>		- I
            is the repository handle returned by infoCreate().
        <definitionFile>	- I
            is the name of the file to which the definitions will be written.
        <status>		- O
            returns the status of saving the definitions, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  infoSave (

#    if PROTOTYPES
        Repository  dictionary,
        const  char  *definitionFile)
#    else
        dictionary, definitionFile)

        Repository  dictionary ;
        char  *definitionFile ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    Definition  definition ;
    FILE  *file ;
    size_t  i ;




    if (dictionary == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(infoSave) NULL repository handle: ") ;
        return (errno) ;
    }

/* Open the output file. */

    fileName = (char *) fnmBuild (FnmPath, definitionFile, NULL) ;
    file = fopen (fileName, "w") ;
    if (file == NULL) {
        LGE "(infoSave) Error opening %s.\nfopen: ",
            fileName) ;
        return (errno) ;
    }


/*******************************************************************************
    Write the definitions to the definition file.
*******************************************************************************/

    for (i = 0 ;  i < hashCount (dictionary->table) ;  i++) {

        hashGet (dictionary->table, i, (void **) &definition) ;

        while (definition != NULL) {

            if (0 > fprintf (file, "%s%s%s: %s\n",
                             definition->path,
                             (definition->path == NULL) ? "" : ".",
                             definition->name,
                             definition->value)) {
                LGE "(infoSave) Error saving definition %ld to %s.\nfprintf: ",
                    (long) i, fileName) ;
                break ;
            }

            definition = definition->next ;

        }

        if (definition != NULL)  break ;	/* Error writing to file? */

    }


/* Close the output file. */

    fclose (file) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    dfnCreate ()

    Create a Definition.


Purpose:

    Function dfnCreate() creates a definition from its textual specification.


    Invocation:

        status = dfnCreate (specification, &definition) ;

    where:

        <specification>	- I
            is the textual specification of the definition in the following
            basic format: "<key>: <value>".
        <definition>	- O
            returns a handle for the definition.  This handle is used in
            other dfnXXXX() calls.
        <status>	- O
            returns the status of creating the definition, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


static  errno_t  dfnCreate (

#    if PROTOTYPES
        const  char  *specification,
        Definition  *definition)
#    else
        specification, definition)

        const  char  *specification ;
        Definition  *definition ;
#    endif

{    /* Local variables. */
    char  *nameStart, *pathStart, *s, *valueStart ;
    int  nameLength, pathLength, valueLength ;
    static  CompiledRE  specPattern = NULL ;
#define  SPEC_PATTERN  \
    "^[ \t]*(([:alnum:_\\-*?.]*[*?.])?)$0([:alnum:_\\-]+)$1[ \t]*:[ \t]*(.*)$2$"




/* Compile the regular expression used to parse the specification. */

    if ((specPattern == NULL) && rex_compile (SPEC_PATTERN, &specPattern)) {
        LGE "(dfnCreate) Error compiling the pattern for matching keys.\nrex_compile: ",
            rex_error_text) ;
        return (errno) ;
    }


/* Parse the definition's specification, determining the path portion of the
   key, the name portion of the key, and the value bound to the key. */

    if (!rex_match (specification, specPattern, NULL, NULL, 3,
                    &pathStart, &pathLength,
                    &nameStart, &nameLength,
                    &valueStart, &valueLength)) {
        SET_ERRNO (EINVAL) ;
        LGE "(dfnCreate) Invalid definition: %s\n", specification) ;
        return (errno) ;
    }


/* Create a definition structure. */

    *definition = (_Definition *) malloc (sizeof (_Definition)) ;
    if (*definition == NULL) {
        LGE "(dfnCreate) Error allocating definition structure.\nmalloc: ") ;
        return (errno) ;
    }

    (*definition)->name = NULL ;
    (*definition)->path = NULL ;
    (*definition)->pattern = NULL ;
    (*definition)->value = NULL ;
    (*definition)->next = NULL ;

    (*definition)->name = strndup (nameStart, nameLength) ;
    if ((*definition)->name == NULL) {
        LGE "(dfnCreate) Error duplicating key name: %*$\nstrndup: ",
            nameLength, nameStart) ;
        PUSH_ERRNO ;  dfnDestroy (*definition) ;  POP_ERRNO ;
        return (errno) ;
    }

    (*definition)->path = strndup (pathStart, pathLength) ;
    if ((*definition)->path == NULL) {
        LGE "(dfnCreate) Error duplicating key path: %*$\nstrndup: ",
            pathLength, pathStart) ;
        PUSH_ERRNO ;  dfnDestroy (*definition) ;  POP_ERRNO ;
        return (errno) ;
    }
    if (pathLength > 0) {	/* Trim trailing field separator, ".". */
        s = (*definition)->path + pathLength - 1 ;
        if (*s == '.')  *s = '\0' ;
    }

    (*definition)->value = strndup (valueStart, valueLength) ;
    if ((*definition)->value == NULL) {
        LGE "(dfnCreate) Error duplicating value: %*$\nstrndup: ",
            valueLength, valueStart) ;
        PUSH_ERRNO ;  dfnDestroy (*definition) ;  POP_ERRNO ;
        return (errno) ;
    }
    strConvert ((*definition)->value) ;

    LGI "(dfnCreate) Path: %s\tName: %s\tValue: %s\n",
        (*definition)->path,
        (*definition)->name,
        (*definition)->value) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    dfnDestroy ()

    Destroy a Definition.


Purpose:

    The dfnDestroy() function destroys a definition.


    Invocation:

        status = dfnDestroy (definition) ;

    where

        <definition>	- I
            is the definition handle returned by dfnCreate().
        <status>	- O
            returns the status of destroying the definition, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


static  errno_t  dfnDestroy (

#    if PROTOTYPES
        Definition  definition)
#    else
        definition)

        Definition  definition ;
#    endif

{

    if (definition == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(dfnDestroy) NULL definition handle: ") ;
        return (errno) ;
    }

    if (definition->name != NULL)  free (definition->name) ;
    if (definition->path != NULL)  free (definition->path) ;
    if (definition->pattern != NULL)  rex_delete (definition->pattern) ;
    if (definition->value != NULL)  free (definition->value) ;

    free (definition) ;

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the INFO_UTIL() functions.

    Under UNIX,
        compile and link as follows:
            % cc -g -DTEST info_util.c -I<... includes ...> <libraries ...>
        and run with the following command line:
            % a.out <resourceFile>

    Under VxWorks,
        compile and link as follows:
            % cc -g -c -DTEST -DVXWORKS info_util.c -I<... includes ...> \
                       -o test_info.o
            % ld -r test_info.o <libraries ...> -o test_info.vx.o
        load as follows:
            -> ld <test_info.vx.o
        and run with the following command line:
            -> test_info.vx.o "<resourceFile>"

*******************************************************************************/

#ifdef VXWORKS

test_info (fs)
    char  *fs ;
{    /* Local variables. */

#else

main (argc, argv)
    int  argc ;
    char  *argv[] ;
{    /* Local variables. */
    char  *fs = argv[1] ;

#endif

    Repository  dictionary ;


    info_util_debug = 1 ;

    if (infoCreate (fs, "-cpp", &dictionary))  exit (errno) ;

/*    infoSave (dictionary, "dictionary.txt") ; */

    infoDestroy (dictionary) ;

}
#endif
