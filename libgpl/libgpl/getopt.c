/* $Id: getopt.c,v 1.4 2003/03/10 19:49:51 alex Exp $ */
/*******************************************************************************

Procedure:

    getopt ()

    Get Option Letter from Argument Vector.


Author:    Alex Measday (c.a.measday@ieee.org)


Purpose:

    Function GETOPT gets the next option letter from the command line.  GETOPT
    is an enhanced version of the C Library function, GETOPT(3).

    The AT&T Command Line Standard specifies the following style for invoking
    a program and passing it parameters:

                    <program>  <options>  <other_args>

    GETOPT(3) handles the options portion of the command line; the program is
    responsible for scanning and processing the other arguments.  Options are
    specified by preceding them with a hyphen.  Single-letter options can be
    grouped after a single hyphen; e.g., "-alex" is identical to "-a -l -e -x".

    Argument options consist of a single-letter option followed by exactly one
    string argument; e.g., the output option for the C compiler specifies a
    file name, "-o <filename>".  The argument may be positioned flush up
    against the option letter ("-o<filename>") or they may be separated by
    white space.

    Single-letter options and argument options may be mixed together, as in
    the following example:

            <program>  -abc -o <filename> -xy -z  <other_args>

    GETOPT(3) knows it has reached the end of the options when it encounters
    an argument that is not prefixed by a hyphen and that is not part of an
    argument option; alternatively, you can explicitly signal the end of the
    options with the explicit "--" option.

    The enhanced GETOPT allows the user to mix options and arguments:

        <program> <options> <other_args> <options> <other_args> ...

    More specifically, GETOPT(3) does NOT allow you to alter its argument
    scan by modifying its options index, OPTIND.  Once you reach the end of
    a first set of options, GETOPT(3) doesn't let you increment OPTIND past
    the non-options arguments to reach a second set of options.  The enhanced
    GETOPT lets you do this.

    The enhanced GETOPT is fully-compatible with GETOPT(3).  The traditional
    GETOPT(3) approach to command line processing is to use GETOPT to scan
    the options; OPTIND is then manually incremented through the non-option
    arguments remaining on the command line:

            while ((option = getopt (argc, argv, optstring)) != -1) {
                switch (option) {
                case 'a': ...
                case 'b': ...
                ...
                case '?': ... error ...
                default:  break ;
                }
            }
            while (optind < argc) {
                ... process argv[optind++] ...
            }

    The new way of using the enhanced GETOPT is to let GETOPT scan the entire
    command line; OPTIND is no longer really needed:

            while ((option = getopt (argc, argv, optstring)) != NONOPT) ||
                   (optarg != NULL)) {
                switch (option) {
                case 'a': ...
                case 'b': ...
                ...
                case '?': ... error ...
                case NONOPT: ... process optarg ...
                default:  break ;
                }
            }

    The option returned by GETOPT and the value of OPTARG define the current
    state of the command line scan and are interpreted as follows:

            Option    OPTARG    Meaning
            ------    ------    -------
            letter     NULL     Single-letter option
            letter    string    Option plus its argument
             '?'                Illegal option or missing option argument
            NONOPT    string    Non-option argument
            NONOPT     NULL     Command line scan completed

    In the case of the question mark ('?') option, OPTARG returns the
    trailing portion of the command line argument that contains the offending
    option; e.g., if illegal option "Q" is detected in "program -abQcde ...",
    GETOPT returns '?' with OPTARG set to "Qcde".


    Invocation:

        option = getopt (argc, argv, optstring) ;

    where

        <argc>
            is the number of arguments in the argument value array.
        <argv>
            is the argument value array, i.e., an array of pointers to the
            "words" extracted from the command line.
        <optstring>
            is the set of recognized options.  Each character in the string
            is a legal option; any other character encountered as an option
            in the command line is an illegal option and an error message is
            displayed.  If a character is followed by a colon in OPTSTRING,
            the option expects an argument.  For example, an OPTSTRING of
            "aelo:x" will result in the recognition of "-alex -o <filename>".
        <option>
            returns the next option letter from the command line.  If
            the option expects an argument, OPTARG is set to point to
            the argument.  '?' is returned in the cases of an illegal
            option letter or a missing option argument.  If a non-option
            argument is encountered, constant NONOPT is returned and
            OPTIND is set to index the non-option argument.


    Public Variables:

        OPTARG - returns the text of an option's argument or a non-option
            argument.  NULL is returned if an option has no argument or if
            the command line scan is complete.
        OPTERR - controls whether (OPTERR != 0) or not (OPTERR == 0) GETOPT
            prints out an error message upon detecting an illegal option or
            a missing option argument.
        OPTIND - is the index in ARGV of the command line argument that GETOPT
            will examine next.  GETOPT recognizes changes to this variable.
            Arguments can be skipped by incrementing OPTIND outside of GETOPT
            and the command line scan can be restarted by resetting OPTIND to
            either 0 or 1.

*******************************************************************************/


#ifdef  VXWORKS
#    include  <vxWorks.h>
#    include  <stdioLib.h>		/* Standard I/O definitions. */
#    include  <strLib.h>		/* C Library string functions. */
#    define  strchr  index
#else
#    include  <stdio.h>			/* Standard I/O definitions. */
#    ifdef  BSD				/* Define for BSD variants of UNIX. */
#        include  <strings.h>		/* C Library string functions. */
#        define  strchr  index
#    else
#        include  <string.h>		/* C Library string functions. */
#    endif
#endif

#include  "getopt.h"			/* GETOPT(3) definitions. */

					/* Public variables. */
char  *optarg = NULL ;
int  opterr = -1 ;
int  optind = 0 ;
					/* Private variables. */
static  int  end_optind = 0 ;
static  int  last_optind = 0 ;
static  int  offset_in_group = 1 ;





int  getopt (

#    ifdef  __STDC__
        int  argc,
        char  **argv,
        char  *optstring)
#    else
        argc, argv, optstring)

        int  argc ;
        char  **argv ;
        char  *optstring ;
#    endif

{    /* Local variables. */

    char  *group, option, *s ;





/* Check if the caller restarted or advanced the scan by modifying OPTIND. */

    if (optind <= 0) {
        end_optind = 0 ;  last_optind = 0 ;  optind = 1 ;
    }
    if (optind != last_optind)  offset_in_group = 1 ;


/*******************************************************************************

    Scan the command line and return the next option or, if none, the
    next non-option argument.  At the start of each loop iteration, OPTIND
    is the index of the command line argument currently under examination
    and OFFSET_IN_GROUP is the offset within the current ARGV string of the
    next option (i.e., to be examined in this iteration).

*******************************************************************************/


    for (option = ' ', optarg = NULL ;
         optind < argc ;
         optind++, offset_in_group = 1, option = ' ', optarg = NULL) {

        group = argv[optind] ;

/* Check for I/O redirection, indicated by "<" (input) or ">" (output)
   characters. */

        if (strchr (SPECIAL_OPTIONS, group[0]) != NULL) {
            if (optind == last_optind)  continue ;
            offset_in_group = 0 ;
        }

/* Is this a non-option argument?  If it is and it's the same one GETOPT
   returned on the last call, then loop and try the next command line
   argument.  If it's a new, non-option argument, then return the argument
   to the calling routine. */

        else if ((group[0] != '-') ||
                 ((end_optind > 0) && (optind > end_optind))) {
            if (optind == last_optind)  continue ;
            optarg = group ;		/* Return NONOPT and argument. */
            break ;
        }

/* Are we at the end of the current options group?  If so, loop and try the
   next command line argument. */

        if (offset_in_group >= strlen (group))  continue ;

/* If the current option is the end-of-options indicator, remember its
   position and move on to the next command line argument. */

        option = group[offset_in_group++] ;
        if (option == '-') {
            end_optind = optind ;	/* Mark end-of-options position. */
            continue ;
        }

/* If the current option is an illegal option, print an error message and
   return '?' to the calling routine. */

        s = strchr (optstring, option) ;
        if ((s == NULL) && (option != '<') && (option != '>')) {
            if (opterr)
                (void) fprintf (stderr, "%s: illegal option -- %c\n",
                                argv[0], option) ;
            option = '?' ;  optarg = &group[offset_in_group-1] ;
            break ;
        }
        if (s == NULL)  s = strchr (SPECIAL_OPTIONS, option) ;

/* Does the option expect an argument?  If yes, return the option and its
   argument to the calling routine.  The option's argument may be flush up
   against the option (i.e., the argument is the remainder of the current
   ARGV) or it may be separated from the option by white space (i.e., the
   argument is the whole of the next ARGV). */

        if (*++s == ':') {

            if (offset_in_group < strlen (group)) {
                optarg = &group[offset_in_group] ;
                offset_in_group = strlen (group) ;
            } else {
                if ((++optind < argc) && (*argv[optind] != '-')) {
                    optarg = argv[optind] ;
                } else {
                    if (opterr)
                        (void) fprintf (stderr,
                                   "%s: option requires an argument -- %c\n",
                                   argv[0], option) ;
                    option = '?' ;  optarg = &group[offset_in_group-1] ;
                    offset_in_group = 1 ;
                }
            }

        }     /* Option with argument. */

/* If I/O redirection was specified and the calling routine does not
   explicitly handle it, then perform the redirection in GETOPT(). */

        if ((strchr (optstring, option) == NULL) && (*s == ':')) {

            switch (option) {
            case '<':
                if (freopen (optarg, "r", stdin) == NULL)  option = '?' ;
                if ((option == '?') && opterr) {
                    fprintf (stderr, "%s: unable to redirect input from %s\n",
                             argv[0], optarg) ;
                    perror ("freopen") ;
                }
                break ;
            case '>':
                if (freopen (optarg, "w", stdout) == NULL)  option = '?' ;
                if ((option == '?') && opterr) {
                    fprintf (stderr, "%s: unable to redirect output to %s\n",
                             argv[0], optarg) ;
                    perror ("freopen") ;
                }
                break ;
            default:
                break ;
            }

            if (option != '?')  continue ;	/* Loop for next option. */

        }

/* It must be a single-letter option without an argument. */

        break ;

    }


/* Return the option and (optionally) its argument. */

    last_optind = optind ;

    return ((option == ' ') ? NONOPT : (int) option) ;


}
