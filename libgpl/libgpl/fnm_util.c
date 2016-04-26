/* $Id: fnm_util.c,v 1.20 2011/07/18 17:49:45 alex Exp $ */
/*******************************************************************************

File:

    fnm_util.c

    Filename Utilities.


Author:    Alex Measday


Purpose:

    The FNM_UTIL package provides a filename parsing capability inspired
    by the VAX/VMS lexical function, F$PARSE().  File specifications have
    the following structure:

                 node:/directory(s)/name.extension.version

    Any field is optional.  NODE is a host name; DIRECTORY is one or more
    names separated by "/"s; NAME follows the last "/" in the pathname.
    VERSION is a 3-digit number (e.g., "002") and EXTENSION follows the
    last dot before the VERSION dot.

    A filename is created as follows:

        #include  "fnm_util.h"			-- Filename utilities.
        FileName  fname ;
        ...
        fname = fnmCreate ("<file_spec>", NULL) ;

    fnmCreate() expands the file specification, translating environment
    variable references and filling in defaults for missing fields.

    fnmCreate() can be passed multiple file specifications, which are
    then processed from left to right in the calling sequence:

        fname = fnmCreate ("<spec1>", ..., "<specN>", NULL) ;

    First, the leftmost file specification is examined and any references
    to environment variables are translated.  The next file specification
    is then examined.  Environment variables are translated and fields
    missing in the first file specification are supplied from the new
    file specification.  Subsequent file specifications are examined,
    in turn, and "applied" to the results of the processing of the
    previous file specifications.  Finally, system defaults (e.g., the
    user's home directory) are supplied for missing fields that remain.

    Is that clear?  I used to have a diagram that showed the file names
    stacked up, one on top of another:

                          ... System Defaults ...
                               File_Spec_#N    |
                                    ...        |
                               File_Spec_#2    |
                               File_Spec_#1    V
                              --------------
                                  Result

    File name components would drop down through holes in lower-level
    specifications to fill in missing fields in the result.

    Specifying multiple file specifications is useful for replacing
    extensions, concatenating directories, etc.:

        #include  "fnm_util.h"			-- Filename utilities.
        FileName  fname ;
        ...					-- "/usr/me" (current directory)
        fname = fnmCreate (NULL) ;
						-- "/usr/me/prog.lis"
        fname = fnmCreate (".lis", "prog.c", NULL) ;
						-- "/usr/you/tools/dump.o"
        fname = fnmCreate (".o", "tools/dump.c", "/usr/you/", NULL) ;

    What can you do with a file name once it is created?  You call
    fnmParse() to get the whole file name or parts of the file name
    as a string:

        #include  "fnm_util.h"			-- Filename utilities.
        char  *s ;
        FileName  fname ;
        ...
        fname = fnmCreate ("host:/usr/who/myprog.c.001", NULL) ;
        s = fnmParse (fname, FnmPath) ;		-- "host:/usr/who/myprog.c.001"
        s = fnmParse (fname, FnmNode) ;		-- "host:"
        s = fnmParse (fname, FnmDirectory) ;	-- "/usr/who"
        s = fnmParse (fname, FnmFile) ;		-- "myprog.c.001"
        s = fnmParse (fname, FnmName) ;		-- "myprog"
        s = fnmParse (fname, FnmExtension) ;	-- ".c"
        s = fnmParse (fname, FnmVersion) ;	-- ".001"
        fnmDestroy (fname) ;

    Shorthand macros - fnmPath(), fnmNode(), etc. - are defined for each
    of the fnmParse() calls above.


Origins:

    The FNM_UTIL package is a repackaging of my FPARSE package, which was
    inspired by the VAX/VMS lexical function, F$PARSE().


Public Procedures:

    fnmBuild() - builds a pathname.
    fnmCreate() - creates a filename.
    fnmDestroy() - destroys a filename.
    fnmExists() - checks if a file exists.
    fnmFind() - looks for a file under different names or directories.
    fnmParse() - parses a filename.

Macro Definitions:

    fnmPath() - returns a filename's full pathname.
    fnmNode() - returns the node from a filename.
    fnmDirectory() - returns the directory from a filename.
    fnmFile() - returns the file, extension, and version from a filename.
    fnmName() - returns the file from a filename.
    fnmExtension() - returns the extension from a filename.
    fnmVersion() - returns the version number from a filename.

Private Procedures:

    fnmFillParts() - fill in missing parts of a filename with defaults.
    fnmLocateParts() - locate the parts of a filename.
    fnmNew() - allocates and initializes a filename structure.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#if defined(VMS)
#    include  <fab.h>			/* RMS file access block definitions. */
#    include  <nam.h>			/* RMS name block definitions. */
#    include  <unixio.h>		/* UNIX I/O definitions. */
#    include  <unixlib.h>		/* UNIX library definitions. */
#elif defined(_WIN32)
#    include  <direct.h>		/* Directory control functions. */
#    include  <io.h>			/* Low-level I/O definitions. */
#elif !defined(HAVE_GETCWD) || HAVE_GETCWD
#    include  <unistd.h>		/* UNIX I/O definitions. */
#else
#    define  getcwd(buf,size)  (((buf)[0] = '\0'), (buf))
#endif
#if !defined(HAVE_STAT_H) || HAVE_STAT_H
#    include  <sys/stat.h>		/* File status definitions. */
#endif

#include  "get_util.h"			/* "Get Next" functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "fnm_util.h"			/* Filename utilities. */


/*******************************************************************************
    File Name - contains the fully-expanded file specification as well as
        the individual components of the file specification.
*******************************************************************************/

typedef  struct  _FileName {
    char  *path ;			/* Fully-expanded file specification. */
    char  *node ;			/* "node:" */
    char  *directory ;			/* "/directory(ies)/" */
    char  *file ;			/* "name.extension.version" */
    char  *name ;			/* "name" */
    char  *extension ;			/* ".extension" */
    char  *version ;			/* ".version" */
}  _FileName ;


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  FileName  fnmFillParts (
#    if PROTOTYPES
        const  FileName  name1,
        const  FileName  name2
#    endif
    )  OCD ("fnm_util") ;

static  errno_t  fnmLocateParts (
#    if PROTOTYPES
        FileName  fileSpec
#    endif
    )  OCD ("fnm_util") ;

static  FileName  fnmNew (
#    if PROTOTYPES
        const  char  *pathname
#    endif
    )  OCD ("fnm_util") ;

/*!*****************************************************************************

Procedure:

    fnmBuild ()

    Build a Pathname.


Purpose:

    The fnmBuild() function builds a pathname from one or more file
    specifications.  fnmBuild() is essentially an encapsulation of
    the following code fragment:

        char  pathname[PATH_MAX] ;
        FileName  fname = fnmCreate (fileSpec1, ..., fileSpecN, NULL) ;
        ...
        strcpy (pathname, fnmParse (fname, FnmPath)) ;
        fnmDestroy (fname) ;

    I got tired of coding up variations of this every time I needed to
    build a full pathname, so I made a copy of fnmCreate() and modified
    it to return a character-string pathname instead of a FileName handle.


    Invocation:

        pathname = fnmBuild (part, [fileSpec1, ..., fileSpecN,] NULL) ;

    where

        <part>		- I
            specifies which part of the file name you want returned:
                FnmPath - "node:/directory(ies)/name.extension.version"
                FnmNode - "node:"
                FnmDirectory - "/directory(ies)/"
                FnmFile - "name[.extension[.version]]"
                FnmName - "name"
                FnmExtension - ".extension"
                FnmVersion - ".version"
            (These enumerated values are defined in "fnm_util.h".)
        <fileSpec1>	- I
        <fileSpecN>	- I
            are the file specfications used to construct the resulting file
            name.  Each file specification is a UNIX pathname containing
            one or more of the components of a pathname (e.g., the directory,
            the extension, the version number, etc.).  Missing components in
            the result are filled in from the file specifications as they are
            examined in left-to-right order.  The NULL argument marks the end
            of the file specification list.
        <pathname>	- O
            returns the pathname constructed from the file specifications;
            "" is returned in the event of an error.  The returned string
            is private to this routine and it should be used or duplicated
            before calling fnmBuild() again.

*******************************************************************************/


const  char  *fnmBuild (

#    if PROTOTYPES
        FnmPart  part,
        const  char  *fileSpec,
        ...)
#    else
        part, fileSpec, va_alist)

        FnmPart  part ;
        char  *fileSpec ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    FileName  defaults, newResult, result ;
    static  char  pathname[PATH_MAX] ;




/* Process each file specification in the argument list. */

#if HAVE_STDARG_H
    va_start (ap, fileSpec) ;
#else
    va_start (ap) ;
#endif

    result = NULL ;

    while (fileSpec != NULL) {

        defaults = fnmNew (fileSpec) ;
        if (defaults == NULL) {
            LGE "(fnmBuild) Error creating defaults: %s\nfnmNew: ", fileSpec) ;
            return ("") ;
        }
        fnmLocateParts (defaults) ;

        newResult = fnmFillParts (result, defaults) ;
        PUSH_ERRNO ;
        if (result != NULL)  fnmDestroy (result) ;
        fnmDestroy (defaults) ;
        POP_ERRNO ;
        if (newResult == NULL) {
            LGE "(fnmBuild) Error creating intermediate result.\nfnmFillParts: ") ;
            return ("") ;
        }
        result = newResult ;

        fileSpec = va_arg (ap, const char *) ;

    }

    va_end (ap) ;


/* Fill in missing fields with the system defaults. */

    getcwd (pathname, sizeof pathname) ;
#ifndef VMS
    strcat (pathname, "/") ;
#endif
    defaults = fnmNew (pathname) ;
    if (defaults == NULL) {
        LGE "(fnmBuild) Error creating system defaults: %s\nfnmNew: ",
            pathname) ;
        return ("") ;
    }
    fnmLocateParts (defaults) ;

    newResult = fnmFillParts (result, defaults) ;
    PUSH_ERRNO ;
    if (result != NULL)  fnmDestroy (result) ;
    fnmDestroy (defaults) ;
    POP_ERRNO ;
    if (newResult == NULL) {
        LGE "(fnmBuild) Error creating final result.\nfnmFillParts: ") ;
        return ("") ;
    }
    result = newResult ;


/* Return the full pathname to the caller. */

    strcpy (pathname, fnmParse (result, part)) ;
    fnmDestroy (result) ;

    return (pathname) ;

}

/*!*****************************************************************************

Procedure:

    fnmCreate ()

    Create a File Name.


Purpose:

    The fnmCreate() function creates a file name.


    Invocation:

        fileName = fnmCreate ([fileSpec1, ..., fileSpecN,] NULL) ;

    where

        <fileSpec1>	- I
        <fileSpecN>	- I
            are the file specfications used to construct the resulting file
            name.  Each file specification is a UNIX pathname containing
            one or more of the components of a pathname (e.g., the directory,
            the extension, the version number, etc.).  Missing components in
            the result are filled in from the file specifications as they are
            examined in left-to-right order.  The NULL argument marks the end
            of the file specification list.
        <fileName>	- O
            returns a handle that can be used in other FNM_UTIL calls.  NULL
            is returned in the event of an error.

*******************************************************************************/


FileName  fnmCreate (

#    if PROTOTYPES
        const  char  *fileSpec,
        ...)
#    else
        fileSpec, va_alist)

        char  *fileSpec ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    char  pathname[PATH_MAX] ;
    FileName  defaults, newResult, result ;




/* Process each file specification in the argument list. */

#if HAVE_STDARG_H
    va_start (ap, fileSpec) ;
#else
    va_start (ap) ;
#endif

    result = NULL ;

    while (fileSpec != NULL) {

        defaults = fnmNew (fileSpec) ;
        if (defaults == NULL) {
            LGE "(fnmCreate) Error creating defaults: %s\nfnmNew: ", fileSpec) ;
            return (NULL) ;
        }
        fnmLocateParts (defaults) ;

        newResult = fnmFillParts (result, defaults) ;
        PUSH_ERRNO ;
        if (result != NULL)  fnmDestroy (result) ;
        fnmDestroy (defaults) ;
        POP_ERRNO ;
        if (newResult == NULL) {
            LGE "(fnmCreate) Error creating intermediate result.\nfnmFillParts: ") ;
            return (NULL) ;
        }
        result = newResult ;

        fileSpec = va_arg (ap, const char *) ;

    }

    va_end (ap) ;


/* Fill in missing fields with the system defaults. */

    getcwd (pathname, sizeof pathname) ;
#ifndef VMS
    strcat (pathname, "/") ;
#endif
    defaults = fnmNew (pathname) ;
    if (defaults == NULL) {
        LGE "(fnmCreate) Error creating system defaults: %s\nfnmNew: ",
            pathname) ;
        return (NULL) ;
    }
    fnmLocateParts (defaults) ;

    newResult = fnmFillParts (result, defaults) ;
    PUSH_ERRNO ;
    if (result != NULL)  fnmDestroy (result) ;
    fnmDestroy (defaults) ;
    POP_ERRNO ;
    if (newResult == NULL) {
        LGE "(fnmCreate) Error creating final result.\nfnmFillParts: ") ;
        return (NULL) ;
    }
    result = newResult ;


    return (result) ;

}

/*!*****************************************************************************

Procedure:

    fnmDestroy ()

    Destroy a File Name.


Purpose:

    The fnmDestroy() function destroys a file name.


    Invocation:

        status = fnmDestroy (fileName) ;

    where

        <fileName>	- I
            is the file name handle returned by fnmCreate().
        <status>	- O
            returns the status of destroying the file name, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  fnmDestroy (

#    if PROTOTYPES
        FileName  fileName)
#    else
        fileName)

        FileName  fileName ;
#    endif

{

    if (fileName == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(fnmDestroy) NULL file handle: ") ;
        return (errno) ;
    }

    if (fileName->path != NULL)  free (fileName->path) ;
    if (fileName->node != NULL)  free (fileName->node) ;
    if (fileName->directory != NULL)  free (fileName->directory) ;
    if (fileName->file != NULL)  free (fileName->file) ;
    if (fileName->name != NULL)  free (fileName->name) ;
    if (fileName->extension != NULL)  free (fileName->extension) ;
    if (fileName->version != NULL)  free (fileName->version) ;

    free (fileName) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    fnmExists ()

    Check If a File Exists.


Purpose:

    The fnmExists() function checks to see if the file referenced by a file
    name actually exists.


    Invocation:

        exists = fnmExists (fileName) ;

    where

        <fileName>	- I
            is the file name handle returned by fnmCreate().
        <exists>	- O
            returns true (1) if the referenced file exists and false (0)
            if it doesn't exist.

*******************************************************************************/


bool  fnmExists (

#    if PROTOTYPES
        const  FileName  fileName)
#    else
        fileName)

        FileName  fileName ;
#    endif

{    /* Local variables. */
#if !defined(HAVE_STAT_H) || HAVE_STAT_H
    struct  stat  fileInfo ;
#endif



    if (fileName == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(fnmExists) NULL file handle: ") ;
        return (false) ;
    }

#if !defined(HAVE_STAT_H) || HAVE_STAT_H
    if (stat (fileName->path, &fileInfo)) {
        switch (errno) {
        case EACCES:				/* Expected errors. */
        case ENOENT:
        case ENOTDIR:
            break ;
        default:				/* Unexpected errors. */
            LGE "(fnmExists) Error getting information for %s.\nstat: ",
                fileName->path) ;
        }
        return (false) ;			/* Not found. */
    }
#endif

    return (true) ;				/* Found. */

}

/*!*****************************************************************************

Procedure:

    fnmFind ()

    Find a File under Any of Several Names.


Purpose:

    Function fnmFind() locates a file under any of several file names.
    fnmFind() returns a pointer to the name under which the file was
    found, or NULL if the file was not found.  Since the name is stored
    in a string local to fnmFind(), the calling routine should immediately
    use it or make a copy of it.

    fnmFind() uses the FNM_UTIL functions, whose very powerful substitution
    capabilities can be used to advantage when calling fnmFind().  The
    following examples illustrate the use of some of these capabilities:


    FINDING A FILE BY ANY OF SEVERAL NAMES:

        found_filename = fnmFind ("top_priority.dat",
                                  "not_as_important.dat",
                                  "who_cares.dat", NULL) ;

    SEARCHING FOR A FILE IN MULTIPLE DIRECTORIES:

        found_filename = fnmFind ("my_appraisal.dat",	<= searches current
                                  "/supervisor/",	   directory first.
                                  "/boss/",
                                  "/CEO/", NULL) ;

        found_filename =
            fnmFind ("my_appraisal.dat",
                     "/supervisor/,/boss/,/CEO/",	<= multiple paths in
                     NULL) ;				   one specification.

    FINDING A FILE BY ANY OF SEVERAL EXTENSIONS:

        found_filename = fnmFind ("display_page",	<= try no extension 1st.
                                  ".pdl",
                                  ".tdl", NULL) ;


    Invocation:

        pathname = fnmFind (fileSpec1 [, ..., fileSpecN], NULL) ;

    where

        <fileSpec1>	- I
        <fileSpecN>	- I
            are the file specfications used to look for the desired file.
            The NULL argument marks the end of the file specification list.
        <pathname>	- O
            returns the full pathname of the file, if found, and NULL otherwise.
            The returned string is private to this routine and it should be used
            or duplicated before calling fnmFind() again.

*******************************************************************************/


const  char  *fnmFind (

#    if PROTOTYPES
        const  char  *path,
        ...)
#    else
        path, va_alist)

        char  *path ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    bool  found ;
    const  char  *s ;
    FileName  fname ;
    ssize_t  length ;
    static  char  result[PATH_MAX+1] ;



/* Search for each file specification in the argument list. */

#if HAVE_STDARG_H
    va_start (ap, path) ;
#else
    va_start (ap) ;
#endif

    found = false ;
    strcpy (result, "") ;

    for (s = path ;  !found && (s != NULL) ;
         s = va_arg (ap, const char *)) {
        length = -1 ;
        while (!found && (s = getarg (s, &length))) {
            fname = fnmCreate (s, result, NULL) ;
            strlcpy (result, fnmPath (fname), sizeof result) ;
            found = fnmExists (fname) ;
            fnmDestroy (fname) ;
        }
    }

    va_end (ap) ;

    return (found ? result : NULL) ;

}

/*!*****************************************************************************

Procedure:

    fnmParse ()

    Parse a File Name.


Purpose:

    The fnmParse() function returns the requested part of a file name, e.g.,
    the directory, the name, the extension, etc.


    Invocation:

        value = fnmParse (fileName, part) ;

    where

        <fileName>	- I
            is the file name handle returned by fnmCreate().
        <part>		- I
            specifies which part of the file name you want returned:
                FnmPath - "node:/directory(ies)/name.extension.version"
                FnmNode - "node:"
                FnmDirectory - "/directory(ies)/"
                FnmFile - "name[.extension[.version]]"
                FnmName - "name"
                FnmExtension - ".extension"
                FnmVersion - ".version"
            (These enumerated values are defined in "fnm_util.h".)
        <value>		- O
            returns the requested part of the file name; "" is returned
            in the event of an error or if the requested part is missing.
            The returned string is private to the file name and it should
            not be modified or deleted; it should not be used after the
            file name is deleted.

*******************************************************************************/


const  char  *fnmParse (

#    if PROTOTYPES
        const  FileName  fileName,
        FnmPart  part)
#    else
        fileName, part)

        FileName  fileName ;
        FnmPart  part ;
#    endif

{

    if (fileName == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(fnmParse) NULL file handle: ") ;
        return ("") ;
    }

    switch (part) {
    case FnmPath:
        return ((fileName->path == NULL) ? "" : fileName->path) ;
    case FnmNode:
        return ((fileName->node == NULL) ? "" : fileName->node) ;
    case FnmDirectory:
        return ((fileName->directory == NULL) ? "" : fileName->directory) ;
    case FnmFile:
        return ((fileName->file == NULL) ? "" : fileName->file) ;
    case FnmName:
        return ((fileName->name == NULL) ? "" : fileName->name) ;
    case FnmExtension:
        return ((fileName->extension == NULL) ? "" : fileName->extension) ;
    case FnmVersion:
        return ((fileName->version == NULL) ? "" : fileName->version) ;
    default:
        return ("") ;
    }

}

/*!*****************************************************************************

Procedure:

    fnmFillParts ()

    Fill the Missing Parts of a File Name with Defaults.


Purpose:

    Function fnmFillParts() fills the missing parts of a file name with the
    corresponding parts from a defaults file name.


    Invocation:

        result = fnmFillParts (fileName, defaults) ;

    where

        <fileName>	- I
            is the handle returned by fnmCreate() for the file name in question.
        <defaults>	- I
            is the handle returned by fnmCreate() for the file name containing
            the defaults.
        <result>	- O
            returns a handle for a new file name consisting of the old file
            name with missing parts supplied by the defaults file name.  NULL
            is returned in the event of an error.

*******************************************************************************/


static  FileName  fnmFillParts (

#    if PROTOTYPES
        const  FileName  fileName,
        const  FileName  defaults)
#    else
        fileName, defaults)

        FileName  fileName ;
        FileName  defaults ;
#    endif

{    /* Local variables. */
    char  pathname[PATH_MAX+1] ;
    FileName  result ;
#ifdef VMS
    int  status ;
    struct  FAB  fab ;			/* RMS file access block. */
    struct  NAM  nam ;			/* RMS name block. */
#else
    char  *ddef, *dnew ;
    int  ldef, lnew ;
#endif




#ifdef VMS

    fab = cc$rms_fab ;				/* Initialize the FAB. */
    fab.fab$l_fop = FAB$M_NAM ;			/* File processing option. */
    fab.fab$l_nam = &nam ;			/* Name block (address). */
    if ((fileName != NULL) && (fileName->path != NULL)) {
        fab.fab$l_fna = fileName->path ;	/* Primary file specification. */
        fab.fab$b_fns = strlen (fileName->path) ;
    }
    if ((defaults != NULL) && (defaults->path != NULL)) {
        fab.fab$l_dna = defaults->path ;	/* Default file specification. */
        fab.fab$b_dns = strlen (defaults->path) ;
    }

    nam = cc$rms_nam ;				/* Initialize the name block. */    nam.nam$b_nop = NAM$M_SYNCHK ;              /* Name block options. */
    nam.nam$b_nop = NAM$M_SYNCHK ;		/* Name block options. */
    nam.nam$l_esa = pathname ;			/* Expanded string. */
    nam.nam$b_ess = PATH_MAX ;

    status = SYS$PARSE (&fab) ;			/* Parse the file specifications. */

    if (!(status & STS$M_SUCCESS)) {
        SET_ERRNO (EVMSERR) ;  vaxc$errno = status ;
        LGE "(fnmFillParts) Error parsing file specification: %s\nSYS$PARSE: ",
            ((fileName == NULL) || (fileName->path == NULL))
            ? "<nil>" : fileName->path) ;
        return (NULL) ;
    }

    pathname[nam.nam$b_esl] = '\0' ;		/* NUL-terminate the pathname. */

    if ((nam.nam$b_esl > 2) &&			/* If directory ... */
        (strcmp (&pathname[nam.nam$b_esl-3], "].;") == 0)) {
        pathname[nam.nam$b_esl - 2] = '\0' ;	/* ... then strip ".;". */
    }

    printf ("(fnmFillParts) fileName = \"%s\"\n",
            ((fileName == NULL) || (fileName->path == NULL))
            ? "<nil>" : fileName->path) ;
    printf ("(fnmFillParts) defaults = \"%s\"\n",
            ((defaults == NULL) || (defaults->path == NULL))
            ? "<nil>" : defaults->path) ;
    printf ("(fnmFillParts) pathname = \"%s\"\n", pathname) ;

#else

    strcpy (pathname, "") ;

/* Substitute the node name. */

    if ((fileName == NULL) || (fileName->node == NULL)) {
        if (defaults->node != NULL)  strcat (pathname, defaults->node) ;
    } else {
        strcat (pathname, fileName->node) ;
    }

/* Substitute the directory.  First, process dot directories in the
   new file specification ("fileName").  Single dots (current directory)
   are replaced by the current working directory; double dots (parent
   directory) remove successive child directories from the default
   file specfication ("defaults").  Dot directories in the default
   FS have no effect, unless the new FS has no directory yet. */

    dnew = ((fileName == NULL) || (fileName->directory == NULL))
           ? "" : fileName->directory ;
    lnew = strlen (dnew) ;
    ddef = (defaults->directory == NULL) ? "" : defaults->directory ;
    ldef = strlen (ddef) ;
    ddef = defaults->directory + ldef ;		/* DDEF points at '\0'. */

/* Prior to loop:  DDEF points to end of (N+1)-th component of directory.
   Be careful making changes to this code - it's not very straightforward.
   It should handle cases like the "/" directory, no dot directories, and
   so on. */

    while ((lnew > 0) && (ldef > 0)) {
        if (strcmp (dnew, ".") == 0) {			/* Current directory. */
            ldef = 0 ;
        } else if (strncmp (dnew, "./", 2) == 0) {	/* Current directory. */
            ldef = 0 ;
        } else if (strcmp (dnew, "..") == 0) {		/* Up one directory. */
            dnew = dnew + 2 ;  lnew = lnew - 2 ;
            do { ldef-- ; } while ((ldef > 0) && (*--ddef != '/')) ;
        } else if (strncmp (dnew, "../", 3) == 0) {	/* Up one directory. */
            dnew = dnew + 3 ;  lnew = lnew - 3 ;
            do { ldef-- ; } while ((ldef > 0) && (*--ddef != '/')) ;
        } else {					/* No dot directory. */
            break ;
        }
    }

/* After loop:  DDEF points to end of (N+1-M)-th component of directory,
   where M is the number of ".." (parent) directories processed.  Get rid
   of the "+1"-th component. */

    while ((ldef > 0) && (*--ddef != '/'))  ldef-- ;

    ddef = (defaults->directory == NULL) ? "" : defaults->directory ;

/* After processing the dot directories, perform the actual directory
   substitutions.  This procedure is complicated by the two types of
   directories, absolute and relative.  If the new directory and the default
   directory are both absolute or both relative, use the new directory.
   If one directory is relative and the other absolute, append the relative
   directory to the absolute directory. */

    if (lnew == 0) {			/* No previous directory spec. */
        strncat (pathname, ddef, ldef) ;
    }
    else if ((strcmp  (dnew, ".") == 0) ||
             (strncmp (dnew, "./", 2) == 0)) {
	/* Dot directories in default FS won't have any effect; use new FS. */
        getcwd (&pathname[strlen (pathname)],
                sizeof pathname - strlen (pathname)) ;
        strcat (pathname, "/") ;
    }
    else if ((strcmp  (dnew, "..") == 0) ||
             (strncmp (dnew, "../", 3) == 0)) {
	/* Dot directories in default FS won't have any effect; use new FS. */
        strncat (pathname, dnew, lnew) ;
    }
    else if (*ddef == '/') {
        if (*dnew == '/')		/* Two absolute directory specs. */
            strncat (pathname, dnew, lnew) ;
        else {				/* Append relative to absolute. */
            strncat (pathname, ddef, ldef) ;
            strncat (pathname, dnew, lnew) ;
        }
    }
    else {
        if (*dnew == '/') {		/* Append relative to absolute. */
            strncat (pathname, dnew, lnew) ;
            strncat (pathname, ddef, ldef) ;
        }
        else				/* Two relative directory specs. */
            strncat (pathname, dnew, lnew) ;
    }

/* Substitute the file name. */

    if ((fileName == NULL) || (fileName->name == NULL)) {
        if (defaults->name != NULL)  strcat (pathname, defaults->name) ;
    } else {
        strcat (pathname, fileName->name) ;
    }

/* Substitute the extension. */

    if ((fileName == NULL) || (fileName->extension == NULL)) {
        if (defaults->extension != NULL)
            strcat (pathname, defaults->extension) ;
    } else {
        strcat (pathname, fileName->extension) ;
    }

/* Substitute the version number. */

    if ((fileName == NULL) || (fileName->version == NULL)) {
        if (defaults->version != NULL)  strcat (pathname, defaults->version) ;
    } else {
        strcat (pathname, fileName->version) ;
    }

#endif


/* Construct a file name structure for the resulting file name. */

    result = fnmNew (pathname) ;
    if (result == NULL) {
        LGE "(fnmFillParts) Error creating result: %s\nfnmNew: ", pathname) ;
        return (NULL) ;
    }
    fnmLocateParts (result) ;


    return (result) ;

}

/*!*****************************************************************************

Procedure:

    fnmLocateParts ()

    Locate the Parts of a File Name.


Purpose:

    The fnmLocateParts() function determines the locations of the different
    parts of a file name, e.g., the directory, the name, the extension, etc.


    Invocation:

        status = fnmLocateParts (fileName) ;

    where

        <fileName>	- I
            is the file name handle returned by fnmCreate().
        <status>	- O
            returns the status of dissecting the file name, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


static  errno_t  fnmLocateParts (

#    if PROTOTYPES
        FileName  fileName)
#    else
        fileName)

        FileName  fileName ;
#    endif

{    /* Local variables. */
    char  pathname[PATH_MAX] ;
#if defined(VMS)
    int  status ;
    struct  FAB  fab ;			/* RMS file access block. */
    struct  NAM  nam ;			/* RMS name block. */
#elif defined(_WIN32)
    char  dir[_MAX_DIR+1] ;
    char  drive[_MAX_DRIVE+1] ;
    char  ext[_MAX_EXT+1] ;
    char  fname[_MAX_FNAME+1] ;
#else
    char  *fs, *s ;
#endif




#if defined(VMS)

/* Ask the operating system to parse the file name into it constituent parts. */

    fab = cc$rms_fab ;				/* Initialize the FAB. */
    fab.fab$l_fop = FAB$M_NAM ;			/* File processing option. */
    fab.fab$l_nam = &nam ;			/* Name block (address). */
    fab.fab$l_fna = fileName->path ;		/* Primary file specification. */
    fab.fab$b_fns = strlen (fileName->path) ;

    nam = cc$rms_nam ;				/* Initialize the name block. */    nam.nam$b_nop = NAM$M_SYNCHK ;              /* Name block options. */
    nam.nam$b_nop = NAM$M_SYNCHK ;		/* Name block options. */
    nam.nam$l_esa = pathname ;			/* Expanded string. */
    nam.nam$b_ess = PATH_MAX ;

    status = SYS$PARSE (&fab) ;			/* Parse the file specifications. */

    if (!(status & STS$M_SUCCESS)) {
        SET_ERRNO (EVMSERR) ;  vaxc$errno = status ;
        LGE "(fnmLocateParts) Error parsing file specification: %s\nSYS$PARSE: ",
            fileName->path) ;
        return (errno) ;
    }

/* Make copies of the pathname components. */

    if (nam.nam$b_node > 0) {
        fileName->node = strndup (nam.nam$l_node, nam.nam$b_node) ;
        if (fileName->node == NULL) {
            LGE "(fnmLocateParts) Error duplicating node of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if ((nam.nam$b_dev + nam.nam$b_dir) > 0) {
        fileName->directory = strndup (nam.nam$l_dev,
                                       nam.nam$b_dev + nam.nam$b_dir) ;
        if (fileName->directory == NULL) {
            LGE "(fnmLocateParts) Error duplicating directory of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if ((nam.nam$b_name + nam.nam$b_type + nam.nam$b_ver) > 0) {
        fileName->file = strndup (nam.nam$l_name, nam.nam$b_name +
                                  nam.nam$b_type + nam.nam$b_ver) ;
        if (fileName->file == NULL) {
            LGE "(fnmLocateParts) Error duplicating file of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if (nam.nam$b_name > 0) {
        fileName->name = strndup (nam.nam$l_name, nam.nam$b_name) ;
        if (fileName->name == NULL) {
            LGE "(fnmLocateParts) Error duplicating name of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if (nam.nam$b_type > 0) {
        fileName->extension = strndup (nam.nam$l_type, nam.nam$b_type) ;
        if (fileName->extension == NULL) {
            LGE "(fnmLocateParts) Error duplicating extension of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if (nam.nam$b_ver > 0) {
        fileName->version = strndup (nam.nam$l_ver, nam.nam$b_ver) ;
        if (fileName->version == NULL) {
            LGE "(fnmLocateParts) Error duplicating version of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

#elif defined(_WIN32)

/* Ask the operating system to parse the file name into it constituent parts. */

    _splitpath (fileName->path, drive, dir, fname, ext) ;

/* Make copies of the pathname components. */

    strcpy (pathname, drive) ;
    strcat (pathname, dir) ;
    if (strlen (pathname) > 0) {
        fileName->directory = strdup (pathname) ;
        if (fileName->directory == NULL) {
            LGE "(fnmLocateParts) Error duplicating directory of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    strcpy (pathname, fname) ;
    strcat (pathname, ext) ;

    if (strlen (pathname) > 0) {
        fileName->file = strdup (pathname) ;
        if (fileName->file == NULL) {
            LGE "(fnmLocateParts) Error duplicating file of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if (strlen (fname) > 0) {
        fileName->name = strdup (fname) ;
        if (fileName->name == NULL) {
            LGE "(fnmLocateParts) Error duplicating name of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

    if (strlen (ext) > 0) {
        fileName->extension = strdup (ext) ;
        if (fileName->extension == NULL) {
            LGE "(fnmLocateParts) Error duplicating extension of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

#else

/* Advance the "fs" pointer as you scan the file specification. */

    strcpy (pathname, fileName->path) ;
    fs = pathname ;

/* First, if the file specification contains multiple pathnames, separated
   by commas or spaces, discard the trailing pathnames. */

#ifdef DISALLOW_SPACES_AND_COMMAS
    if ((s = strchr (fs, ' ')) != NULL)  *s = '\0' ;
    if ((s = strchr (fs, ',')) != NULL)  *s = '\0' ;
#endif

/* Locate the node.  The node name is separated from the rest of the file
   name by a colon (":"). */

    if ((s = strchr (fs, ':')) != NULL) {
        fileName->node = strndup (fs, (int) (s - fs + 1)) ;
        if (fileName->node == NULL) {
            LGE "(fnmLocateParts) Error duplicating node of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
        fs = ++s ;
    }

/* Locate the directory.  The directory extends through the last "/" in the
   file name. */

    if ((s = strrchr (fs, '/')) != NULL) {
        fileName->directory = strndup (fs, (int) (s - fs + 1)) ;
        if (fileName->directory == NULL) {
            LGE "(fnmLocateParts) Error duplicating directory of %s.\nstrndup: ",
                fileName->path) ;
            return (errno) ;
        }
        fs = ++s ;
    }

/* The remainder of the pathname is the combined name, extension, and
   version number. */

    if (*fs != '\0') {
        fileName->file = strdup (fs) ;
        if (fileName->file == NULL) {
            LGE "(fnmLocateParts) Error duplicating file of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

/* Locate the version number.  Since version numbers are not part of
   UNIX, these version numbers are a user convention.  Any file extension
   that can be converted to an integer is considered a version number;
   e.g., ".007", etc.  (So we can make this test, a version number of zero
   is not allowed. */

    if (((s = strrchr (fs, '.')) != NULL) && (atoi (++s) != 0)) {
        fileName->version = strdup (--s) ;
        if (fileName->version == NULL) {
            LGE "(fnmLocateParts) Error duplicating version of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
        *s = '\0' ;			/* Exclude version temporarily. */
    }

/* Locate the extension.  The extension is the last part of the file name
   preceded by a "." (not including the version number, though). */

    if ((s = strrchr (fs, '.')) != NULL) {
        fileName->extension = strdup (s) ;
        if (fileName->extension == NULL) {
            LGE "(fnmLocateParts) Error duplicating extension of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
        *s = '\0' ;			/* Exclude extension temporarily. */
    }

/* Locate the name.  The name is the rest of the file name, excluding the
   last extension and the version number, if any. */

    if (*fs != '\0') {
        fileName->name = strdup (fs) ;
        if (fileName->name == NULL) {
            LGE "(fnmLocateParts) Error duplicating name of %s.\nstrdup: ",
                fileName->path) ;
            return (errno) ;
        }
    }

/* Restore extension and version. */

    if (fileName->extension != NULL)  fs[strlen (fs)] = '.' ;
    if (fileName->version != NULL)  fs[strlen (fs)] = '.' ;

#endif


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    fnmNew ()

    Allocates a File Name Structure.


Purpose:

    The fnmNew() function allocates and initializes a file name structure.


    Invocation:

        fileName = fnmNew (pathname) ;

    where

        <pathname>	- I
            is the initial pathname.
        <fileName>	- O
            returns a pointer to the allocated file name structure.  The
            caller is responible for FREE(3)ing the structure when it is
            no longer needed.  NULL is returned in the event of an error.

*******************************************************************************/


static  FileName  fnmNew (

#    if PROTOTYPES
        const  char  *pathname)
#    else
        pathname)

        char  *pathname ;
#    endif

{    /* Local variables. */
    char  expandedPathname[PATH_MAX] ;
    FileName  fileName ;



/* Allocate the file name structure. */

    fileName = (FileName) malloc (sizeof (_FileName)) ;
    if (fileName == NULL) {
        LGE "(fnmNew) Error allocating structure for %s.\n", pathname) ;
        return (NULL) ;
    }

/* Initialize the structure. */

    if (pathname == NULL) {
        fileName->path = NULL ;
    } else {
        strEnv (pathname, -1, expandedPathname, sizeof expandedPathname) ;
        fileName->path = strdup (expandedPathname) ;
        if (fileName->path == NULL) {
            LGE "(fnmNew) Error duplicating pathname: %s\n", expandedPathname) ;
            PUSH_ERRNO ;  free (fileName) ;  POP_ERRNO ;
            return (NULL) ;
        }
    }

    fileName->node = NULL ;
    fileName->directory = NULL ;
    fileName->file = NULL ;
    fileName->name = NULL ;
    fileName->extension = NULL ;
    fileName->version = NULL ;

    return (fileName) ;

}
