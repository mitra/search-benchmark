/* $Id: pjoin.c,v 1.3 2010/10/21 20:32:40 alex Exp $ */
/*******************************************************************************

Program:

    pjoin.c

    Paragraph Join.


Author:    Alex Measday


Purpose:

    PJOIN joins lines of a paragraph into a single long line, useful when
    a word processor treats the line breaks within the paragraph as "hard"
    line breaks.


    Invocation:

        % pjoin <file(s)>

    where:

        "<file(s)>"
            are the input files; output is written to standard output.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */

/*******************************************************************************
    processFile() - process an input file.
*******************************************************************************/

int  processFile (

#    if PROTOTYPES
        FILE  *file)
#    else
        file)

        FILE  *file ;
#    endif

{  /* Local variables. */
    bool  inParagraph ;
    char  buffer[32*1024], last, *s ;
    int  numBlankLines ;
    size_t  indent ;


    inParagraph = false ;
    last = ' ' ;
    numBlankLines = 0 ;

    while (!feof (file)) {
        if (fgets (buffer, sizeof buffer, file) == NULL)  break ;
        s = strrchr (buffer, '\r') ;
        if (s != NULL)  *s = ' ' ;
        s = buffer + strspn (buffer, "\f\r\n") ;
        indent = strspn (s, " \t") ;
        s += indent ;
        if (strTrim (s, -1) > 0) {
            if (numBlankLines >= 3) {
                printf ("\f\n") ;  numBlankLines = 3 ;
            }
            while (numBlankLines > 0) {
                printf ("\n") ;
                numBlankLines-- ;
            }
#ifdef NOT_NOW
            if (indent > 0) {
                printf ("\t%s\n", s) ;
                inParagraph = false ;
                last = ' ' ;
            } else
#endif
            if (inParagraph && (last != '-')) {
                printf (" %s", s) ;
                last = buffer[strlen (buffer) - 1] ;
            } else {
                printf ("%s", s) ;
                inParagraph = true ;
                last = buffer[strlen (buffer) - 1] ;
            }
        } else {
            if (numBlankLines++ == 0)  printf ("\n") ;
            inParagraph = false ;
            last = ' ' ;
        }
    }

    return (0) ;

}

/*******************************************************************************
    PJOIN's Main Program.
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
    char  *argument ;
    FILE  *file ;
    int  errflg, numFiles, option ;
    OptContext  scan ;

    const  char  *optionList[] = {	/* Command line options. */
        NULL
    } ;




    aperror_print = 1 ;

/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    numFiles = 0 ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case NONOPT:
            numFiles++ ;
            file = fopen (argument, "r") ;
            if (file == NULL) {
                aperror ("[%s] Error opening \"%s\"\nfopen: ", argv[0], argument) ;
                errflg++ ;
            } else {
                processFile (file) ;
                fclose (file) ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  pjoin <file(s)>\n") ;
        exit (EINVAL) ;
    }

    if (numFiles == 0) {	/* No files?  Read from standard input. */
        processFile (stdin) ;
    }

    exit (0) ;

}
