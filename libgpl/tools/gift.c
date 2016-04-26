/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

  gift.c

  Generate Intermediate File from FORTRAN Source Code.


    Invocation:

        % gift [-o output_file] [input_file(s)]

    where:

        "-o <output_file>"
                specifies the name of the output file; if not specified, the
                output is directed to the standard output.
        <input_file(s)>
		is/are the FORTRAN source code files to be scanned.
*******************************************************************************/


#include  <ctype.h>			/* Character functions. */
#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <string.h>			/* String functions. */
#include  <sys/param.h>			/* System parameters. */
#include  "getopt.h"			/* GETOPT(3) definitions. */
#include  "fparse.h"			/* Filename parsing definitions. */

					/* External functions. */
extern  char  *getword (), *str_trim (), *str_upcase () ;

#define  MAX_FILES  1024
#define  MAX_STRING  256
#define  FORM_FEED  12

					/* List of input file names. */
static  char  *file_table[MAX_FILES] ;
static  int  num_input_files = 0 ;

					/* Miscellaneous non-locals. */
static  char  input_file_spec[MAXPATHLEN] ;
static  int  debug = 0 ;


main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    char  inbuf[MAX_STRING], input_file[MAXPATHLEN], *output_file ;
    char  *file_spec, *s ;
    int  errflg, i, length, module_found, option, status ;
    FILE  *infile, *outfile ;




/*******************************************************************************

  Scan the command line options.

*******************************************************************************/

    output_file = NULL ;

    opterr = -1 ;  errflg = 0 ;

    while (((option = getopt (argc, argv, "do:")) != NONOPT) ||
           (optarg != NULL)) {

        switch (option) {
        case 'd':  debug = -1 ;  break ;
        case 'o':  output_file = optarg ;  break ;
        case '?':  errflg++ ;  break ;
        case NONOPT:
            if (num_input_files < MAX_FILES) {
                file_table[num_input_files++] = optarg ;
            }
            break ;
        default :  break ;
        }

    }

    if (errflg) {
        fprintf (stderr,
                 "Usage:  gift [-d] [-o output_file] [-u] input_file(s)]\n") ;
        exit (-1) ;
    }


    if (num_input_files == 0)	/* If no input file, use standard input. */
        file_table[num_input_files++] = NULL ;

    if (open_output_file (output_file, &outfile, &file_spec))  exit (errno) ;

/*******************************************************************************

  For each input file, scan the file for subprogram call information.

*******************************************************************************/

    for (i = 0 ;  i < num_input_files ;  i++) {

        if (open_input_file (file_table[i], &infile, &file_spec))  continue ;
        fprintf (stderr, "%s\n", file_spec) ;

        module_found = 0 ;

        while (fgets (inbuf, (sizeof inbuf), infile) != NULL) {

            if ((s = strchr (inbuf, '\f')) != NULL)  *s = ' ' ;		/* Replace form-feed with space. */
            if ((s = strchr (inbuf, '\n')) != NULL)  *s = '\0' ;	/* Trim trailing newline. */

            if (!isdigit (inbuf[0]) &&		/* Skip comment lines. */
                (inbuf[0] != ' ') && (inbuf[0] != '\t')) {
                continue ;
            }
						/* Trim in-line comments. */
            if ((s = strchr (inbuf, '!')) != NULL)  *s = '\0' ;

            str_trim (inbuf, -1) ;
            if (debug)  fprintf (stderr, "\"%s\"\n", inbuf) ;
            str_upcase (inbuf, -1) ;

            for (s = inbuf, length = 0 ; ; ) {

                s = getword (s, " \t()", &length) ;
                if (length == 0)  break ;

                if ((length == 8) && (strncmp (s, "FUNCTION", length) == 0)) {

                    s = getword (s, " \t()", &length) ;
                    if (length == 0)  break ;
                    fprintf (outfile, "Function %.*s Calls\n", length, s) ;
                    module_found = -1 ;

                } else if ((length == 10) && (strncmp (s, "SUBROUTINE", length) == 0)) {

                    s = getword (s, " \t()", &length) ;
                    if (length == 0)  break ;
                    fprintf (outfile, "Routine %.*s Calls\n", length, s) ;
                    module_found = -1 ;

                } else if ((length == 4) &&
                           (strncmp (s, "CALL", length) == 0)) {

                    s = getword (s, " \t()", &length) ;
                    if (length == 0)  break ;
                    if (!module_found) {
                        fprintf (outfile, "Routine %s Calls\n",
                                 str_upcase (
                                     fparse (file_table[i], NULL, FILENAME),
                                             0)) ;
                        module_found = -1 ;
                    }
                    fprintf (outfile, "    %.*s\n", length, s) ;

                }

            }     /* For each word in line. */

        }     /* For each line in file. */

        fclose (infile) ;	/* Close the file. */

    }    /* For each file. */

}
