/* $Id: rname.c,v 1.3 2005/02/12 00:44:18 alex Exp alex $ */
/*******************************************************************************

  rename.c

  Rename Files.


    Invocation:

        % rename  [-debug] [-lower] [-upper] <old_name> [<new_name>]

    where

        "-debug"
            puts the program in debug mode.  In this mode, the program
            generates its usual output but does NOT rename the files.
            This is useful if you want to check beforehand that your
            invocation works the way you expect.
        "-lower"
            forces new file names to be all lower-case.
        "-upper"
            forces new file names to be all upper-case.
        "<old_name>"
            is the old UNIX file name of the file(s) being renamed.  If
            this is a wildcard file specification, it should be quoted
            so that the Shell doesn't expand it.
        "<new_name>"
            is the new UNIX file name for the file(s) being renamed.  If
            this is a wildcard file specification, it should be quoted
            so that the Shell doesn't expand it.  If a new name is not
            specified, the lower- and upper-case flags ("-lower" and
            "-upper") determine the new names.

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
#include  "str_util.h"			/* String manipulation functions. */

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

{  /* Local variables. */
    char  *argument, *newFile, *newName, *oldFile, *oldName, *s ;
    char  fileName[PATH_MAX] ;
    int  debug, errflg, lowerCase, option, upperCase ;
    struct  stat  fileStatus ;
    DirectoryScan  scan ;
    OptContext  context ;




    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    debug = 0 ;  newName = oldName = NULL ;
    lowerCase = upperCase = 0 ;
    opt_init (argc, argv, "{debug}{lower}{upper}", NULL, &context) ;
    errflg = 0 ;

    while ((option = opt_get (context, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            debug = 1 ;  break ;
        case 2:			/* "-lower" */
            lowerCase = 1 ;  break ;
        case 3:			/* "-upper" */
            upperCase = 1 ;  break ;
        case NONOPT:		/* File names. */
            if (oldName == NULL) {
                oldName = argument ;
            } else if (newName == NULL) {
                newName = argument ;
            } else {
                fprintf (stderr, "[%s] Too many file specifications: %s\n",
                         argv[0], argument) ;
                errflg++ ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (context) ;

    if (errflg || (oldName == NULL)) {
        fprintf (stderr,
                 "Usage:  rename [-debug] [-lower] [-upper] <oldName> [<newName>]\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    For each file matched by the old file specification, rename the file using
    the template provided by the new file specification.
*******************************************************************************/

    drsCreate (oldName, &scan) ;
    oldFile = (char *) drsFirst (scan) ;
    while (oldFile != NULL) {

        s = (char *) fnmBuild (FnmFile, oldFile, NULL) ;

        if (lowerCase) {
            strcpy (fileName, s) ;
            newName = strToLower (fileName, -1) ;
        } else if (upperCase) {
            strcpy (fileName, s) ;
            newName = strToUpper (fileName, -1) ;
        }

#ifdef HAVE_FPARSE
        newFile = fparse (newName, oldFile, NULL, NULL) ;
#else
        newFile = newName ;
#endif

        if (stat (newFile, &fileStatus) && (errno == ENOENT)) {

            printf ("%s\t-\t%s ...\n", oldFile, newFile) ;
            if (!debug && rename (oldFile, newFile)) {
                aperror ("[%s] Error renaming %s to %s.\nrename: ",
                         argv[0], oldFile, newFile) ;
            }

        } else {

            errno = EEXIST ;
            aperror ("[%s] %s not renamed;\n[%s] %s already exists.\n",
                     argv[0], oldFile, argv[0], newFile) ;

        }

        oldFile = (char *) drsNext (scan) ;

    }


    exit (0) ;

}
