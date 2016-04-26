/* $Id: chafn.c,v 1.2 2011/03/31 22:24:36 alex Exp alex $ */
/*******************************************************************************

Program:

    chafn.c

    Change File Names.


Author:    Alex Measday


Purpose:

    CHAFN renames files using regular-expression-based pattern substitutions
    applied to the file names.


    Invocation (regular-expression matching and substitution):

        % chafn [-debug] [-all] [-max <substitutions>]
                <pattern> <replacement>
                <file(s)>

    Invocation (case conversion applied to the whole file name):

        % chafn [-debug]
                [-capitalize] [-lower] [-upper]
                [-max <substitutions>]
                <file(s)>

    Invocation (case conversion applied only to matched text in the file name):

        % chafn [-debug]
                <pattern>
                [-capitalize] [-lower] [-upper]
                [-max <substitutions>]
                <file(s)>

    Invocation (insert prefix):

        % chafn [-debug]
                -prefix <text>
                <file(s)>

    Invocation (insert track number, "%02d - "):

        % chafn [-debug]
                -track <number>
                <file(s)>

    where

        "-debug"
            puts the program in debug mode.  In this mode, the program
            generates its usual output but does NOT rename the files.
            This is useful if you want to check beforehand that your
            invocation works the way you expect.
        "-help"
            displays help information.
        "-all"
        "-max <substitutions>"
            specify the maximum number of substitutions to perform.
            The default maximum is 1 for the usual pattern/replacement
            substitutions.  The "-all" option causes global replacement.
            For case conversions, the default is to apply the conversion
            globally to the file name; this can be altered by specifying
            the "-max" option *after* the case-conversion option.
        "-capitalize"
        "-lower"
        "-upper"
            causes the matched text to be capitalized, converted to lower-case,
            or converted to upper-case, respectively.  The replacement text is
            automatically supplied internally and should not be specified on
            the command line.  To perform case conversion on the whole file
            name, don't specify <pattern>.  To perform case conversion only on
            matched text in the file name, specify <pattern> *before* one of
            these options.
        "-change"
            begins the new specification of a file name change.  This option
            allows you to specify a file name change for one set of files and
            then a different file name change for another set of files.  The
            option is followed by a name change specification (e.g., pattern
            and replacement text or other options) and the files to which the
            name change is to be applied.  (There is an implied "-change" at
            the beginning of the command line.)
        "-dot"
            deletes dotted artist/album information from MP3 file names of
            the form, "<track>. <text> - <song>.mp3".  The ". <text>" part
            is removed, leaving "<track> - <song>.mp3".
        "-prefix <text>"
            inserts the text at the beginning of each file name.  (This is just
            shorthand for the pattern, "^", and the replacement text, "<text>".)
        "-swap <extension>"
            swaps the second and third hyphen-separated fields in a file name.
            The file extension must include the period (".mp3" instead of just
            "mp3").  I frequently use this for music file names of the form:
                "<track> - <artist> - <title>.mp3"
            The result is:
                "<track> - <title> - <artist> .mp3"
            Notice the extra blank inserted before the file extension.
        "-track <number>"
            inserts a track number, separated by a hyphen, at the beginning of
            each file name.  A leading zero is added for numbers less than ten.
            Examples: "01 - ", "02 - ", ..., "10 - ", "11 - ", etc.
        "-underscores"
            replaces all underscores ("_") in the file name with spaces.
        "-yphen"
            replaces the first space in a file name by " - " (space, 'yphen,
            space).
        "<pattern>"
            is a regular expression used to match the text to be replaced.
        "<replacement>"
            is the replacement text.
        "<file(s)>"
            specifies the file(s) to be renamed.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <sys/types.h>			/* System type definitions. */
#include  <sys/stat.h>			/* File status definitions. */
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "rex_util.h"			/* Regular expression definitions. */
#include  "str_util.h"			/* String manipulation functions. */


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  errno_t  renameFiles (
#    if PROTOTYPES
        CompiledRE  pattern,
        const  char  *replacement,
        int  maxSubstitutions,
        bool  debug,
        const  char  *fileSpec
#    endif
    ) ;

/*******************************************************************************
    Main Program.
*******************************************************************************/

int  main (

#    if PROTOTYPES
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{    /* Local variables. */
    bool  debug, help ;
    char  *argument, buffer[128], number[32], *replacement ;
    int  errflg, maxSubstitutions, option ;
    CompiledRE  pattern ;
    OptContext  context ;

    const  char  *optionList[] = {	/* Command line options. */
        "{Debug}", "{debug}", "{help}",
        "{all}", "{capitalize}", "{change}", "{dot}",
        "{lower}", "{max:}", "{prefix:}", "{swap:}", "{track:}",
        "{underscores}", "{upper}", "{yphen}", "{Yphen}",
        NULL
    } ;




    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    debug = false ;  help = false ;
    maxSubstitutions = 1 ;
    pattern = NULL ;  replacement = NULL ;

    opt_init (argc, argv, NULL, optionList, &context) ;
    errflg = 0 ;

    while ((option = opt_get (context, &argument))) {

        switch (option) {
        case 1:			/* "-Debug" */
            drs_util_debug = 1 ;
            rex_util_debug = 1 ;
        case 2:			/* "-debug" */
            debug = true ;
            break ;
        case 3:			/* "-help" */
            help = true ;
            break ;
        case 4:			/* "-all" */
            maxSubstitutions = -1 ;
            break ;
        case 5:			/* "-capitalize" */
            if ((pattern == NULL) &&
                rex_compile ("(['.]|[:alpha:])$1([:alpha:]*)$2", &pattern)) {
                errflg++ ;
            }
            replacement = "$u1$l2" ;
            maxSubstitutions = -1 ;
            break ;
        case 6:			/* "-change" */
            pattern = NULL ;  replacement = NULL ;
            break ;
        case 7:			/* "-dot" */
            if ((pattern == NULL) && rex_compile ("\\. [^\\-]", &pattern)) {
                errflg++ ;
            }
            replacement = " " ;
            break ;
        case 8:			/* "-lower" */
            if ((pattern == NULL) && rex_compile ("^.*$", &pattern))  errflg++ ;
            replacement = "$l&" ;
            break ;
        case 9:			/* "-max <substitutions>" */
            maxSubstitutions = atoi (argument) ;
            break ;
        case 10:		/* "-prefix <text>" */
            if ((pattern == NULL) && rex_compile ("^", &pattern))  errflg++ ;
            replacement = argument ;
            break ;
        case 11:		/* "-swap <extension>" */
            strlcpy (buffer, " \\- ([^\\-]*)$1\\- (.*)$2(",
                     sizeof buffer) ;
            strlcat (buffer, argument, sizeof buffer) ;
            strlcat (buffer, ")$3", sizeof buffer) ;
            if ((pattern == NULL) && rex_compile (buffer, &pattern))  errflg++ ;
            replacement = " - $2 - $1$3" ;
            break ;
        case 12:		/* "-track <number>" */
            if ((pattern == NULL) && rex_compile ("^", &pattern))  errflg++ ;
            sprintf (number, "%02d - ", atoi (argument)) ;
            replacement = number ;
            break ;
        case 13:		/* "-underscores" */
            if ((pattern == NULL) && rex_compile ("_", &pattern))  errflg++ ;
            replacement = " " ;
            maxSubstitutions = -1 ;
            break ;
        case 14:		/* "-upper" */
            if ((pattern == NULL) && rex_compile ("^.*$", &pattern))  errflg++ ;
            replacement = "$u&" ;
            break ;
        case 15:		/* "-yphen" */
            if ((pattern == NULL) && rex_compile (" ", &pattern))  errflg++ ;
            replacement = " - " ;
            break ;
        case 16:		/* "-Yphen" */
            if ((pattern == NULL) && rex_compile ("\\-", &pattern))  errflg++ ;
            replacement = " - " ;
            break ;
        case NONOPT:		/* "<pattern> <replacement> <file(s)>" */
            if (pattern == NULL) {
                strConvert (argument) ;
                if (rex_compile (argument, &pattern))  errflg++ ;
            } else if (replacement == NULL) {
                replacement = argument ;
            } else if (renameFiles (pattern, replacement, maxSubstitutions,
                                    debug, argument)) {
                errflg++ ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (context) ;

#ifdef NDS
    gets (number) ;
#endif

    if (errflg || help || (pattern == NULL)) {
        fprintf (stderr, "Usage:  chafn [-debug] [-Debug] [-help]\n") ;
        fprintf (stderr, "              [-change]\n") ;
        fprintf (stderr, "              [-capitalize] [-dot] [-lower] [-upper]\n") ;
        fprintf (stderr, "              [-max <substitutions>] [-prefix <text>]\n") ;
        fprintf (stderr, "              [-swap <extension] [-track <number>]\n") ;
        fprintf (stderr, "              [-underscores] [-yphen] [-Yphen]\n") ;
        fprintf (stderr, "              [[<pattern>] <replacement>] <file(s)>\n") ;
        if (!help) {
            fprintf (stderr, "Type \"chafn -help\" for more detailed information.\n") ;
            exit (EINVAL) ;
        }
        fprintf (stderr, "\n") ;
        fprintf (stderr, "NOTES - To verify your changes before making them, first run CHAFN\n") ;
        fprintf (stderr, "        with an initial \"-debug\" option to show the results without\n") ;
        fprintf (stderr, "        actually renaming the files:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "            %% chafn -debug ...\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Usage (regular-expression matching and substitution):\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    %% chafn [-max <substitutions>] <pattern> <replacement> <file(s)>\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Usage (case conversion applied to the whole file name):\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    %% chafn -capitalize <file(s)>\n") ;
        fprintf (stderr, "    %% chafn -lower <file(s)>\n") ;
        fprintf (stderr, "    %% chafn -upper <file(s)>\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Usage (case conversion applied only to matched text in the file name):\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    %% chafn <pattern> -capitalize <file(s)>\n") ;
        fprintf (stderr, "    %% chafn <pattern> -lower <file(s)>\n") ;
        fprintf (stderr, "    %% chafn <pattern> -upper <file(s)>\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Usage (insert text at the beginning of the file name):\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    %% chafn -prefix <text> <file(s)>\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Regular expression constructs:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    Basic RE symbols:     . ^ $\n") ;
        fprintf (stderr, "    Character classes:    [] ^ -\n") ;
        fprintf (stderr, "    [:class:]    match or ...\n") ;
        fprintf (stderr, "    [^:class:]   ... doesn't match a character in the specified class,\n") ;
        fprintf (stderr, "                 where the possible classes are \"alpha\", \"upper\", \"lower\",\n") ;
        fprintf (stderr, "                 \"digit\", \"xdigit\", \"alnum\", \"space\", \"punct\", \"print\",\n") ;
        fprintf (stderr, "                 \"cntrl\", and \"graph\" (see CTYPE(3) for descriptions).\n") ;
        fprintf (stderr, "    RE*          matches zero or more instances of the RE.\n") ;
        fprintf (stderr, "    RE+          matches one or more instances of the RE.\n") ;
        fprintf (stderr, "    RE?          matches zero or one instance of the RE.\n") ;
        fprintf (stderr, "    RE1RE2       matches RE1 followed immediately by RE2 (no intervening\n") ;
        fprintf (stderr, "                 spaces in the RE pattern or in the target string).\n") ;
        fprintf (stderr, "    RE1|RE2      matches RE1 or RE2.\n") ;
        fprintf (stderr, "    (RE)         parentheses allow grouping of RE's.\n") ;
        fprintf (stderr, "    (RE)$n       returns a pointer to the text matched by the RE in the\n") ;
        fprintf (stderr, "                 N-th return argument.  N is a single digit between zero\n") ;
        fprintf (stderr, "                 and 9, inclusive.\n") ;
        fprintf (stderr, "    RE{[m][,[n]]}\n") ;
        fprintf (stderr, "                 matches M through N instances of the RE.  If not\n") ;
        fprintf (stderr, "                 specified, M defaults to 0.  Depending on whether or\n") ;
        fprintf (stderr, "                 not the comma is present, N defaults to M (\"RE{m}\")\n") ;
        fprintf (stderr, "                 or a very large number (\"RE{m,}\").  \"RE*\" is equivalent\n") ;
        fprintf (stderr, "                 to \"RE{0,}\".  \"RE+\" is equivalent to \"RE{1,}\".  \"RE?\" is\n") ;
        fprintf (stderr, "                 equivalent to \"RE{0,1}\".\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Special character sequences in replacement text:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    $0 - $9    Insert subexpression (0..9) matched by RE.\n") ;
        fprintf (stderr, "    $&         Insert entire text matched by RE.\n") ;
        fprintf (stderr, "    $l0 - $l9  Insert subexpression (0..9) matched by RE, converted to\n") ;
        fprintf (stderr, "               lower case.\n") ;
        fprintf (stderr, "    $l&        Insert entire text matched by RE, converted to lower case.\n") ;
        fprintf (stderr, "    $u0 - $u9  Insert subexpression (0..9) matched by RE, converted to\n") ;
        fprintf (stderr, "               upper case.\n") ;
        fprintf (stderr, "    $u&        Insert entire text matched by RE, converted to upper case.\n") ;
        fprintf (stderr, "    \\c         Insert character 'c' (e.g., \"\\$\" gives \"$\").\n") ;
        exit (0) ;
    }


    exit (0) ;

}

/*!*****************************************************************************

Procedure:

    renameFiles ()


Purpose:

    Function renameFiles() renames the specified set of files.


    Invocation:

        status = renameFiles (pattern, replacement, maxSubstitutions,
                              debug, fileSpec) ;

    where

        <pattern>		- I
            is the compiled regular expression specifying the text to be matched
            in the file names.
        <replacement>		- I
            is the text to replace the matched patterns in the file names.
        <maxSubstitutions>	- I
            specifies the maximum number (0..N) of substitutions that are
            to be made in the source string.  A value of -1 causes global
            substitutions in the source string.  Substitutions are not
            recursive; the search for the next occurrence of the search
            string in the source string begins following the end of the
            last match.
        <debug>			- I
            specifies whether or not the renaming is in debug mode.  In debug
            mode, debug output is generated, but the files are NOT renamed.
            This is useful to test if the renaming is as desired before actually
            renaming the files.
        <fileSpec>		- I
            is the (possibly wildcard) specification of the file(s) to be
            renamed.
        <status>
            returns the status of renaming the file(s), zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  renameFiles (

#    if PROTOTYPES
        CompiledRE  pattern,
        const  char  *replacement,
        int  maxSubstitutions,
        bool  debug,
        const  char  *fileSpec)
#    else
        pattern, replacement, maxSubstitutions, debug, fileSpec)

        CompiledRE  pattern ;
        char  *replacement ;
        int  maxSubstitutions ;
        bool  debug ;
        char  *fileSpec ;
#    endif

{    /* Local variables. */
    char  *newName, *oldName, *s ;
    DirectoryScan  scan ;
    int  numSubstitutions ;




/*******************************************************************************
    For each file matched by the old file specification, rename the file using
    the template provided by the new file specification.
*******************************************************************************/

    if (drsCreate (fileSpec, &scan))  return (errno) ;

    oldName = (char *) drsFirst (scan) ;

    while (oldName != NULL) {

        if (rex_replace (fnmBuild (FnmFile, oldName, NULL),
                         pattern, replacement, maxSubstitutions,
                         &newName, &numSubstitutions)) {
            return (errno) ;
        }

        if (numSubstitutions > 0) {

            s = (char *) fnmBuild (FnmPath,
                                   newName,
                                   fnmBuild (FnmDirectory, oldName, NULL),
                                   NULL) ;

            printf ("RENAME %s\n    TO %s\n", oldName, s) ;

            if (!debug && rename (oldName, s)) {
                LGE "(renameFiles) Error renaming \"%s\" to \"%s\"\nrename: ",
                    oldName, s) ;
                return (errno) ;
            }

        }

        free (newName) ;

        oldName = (char *) drsNext (scan) ;

    }

    drsDestroy (scan) ;

    return (0) ;

}
