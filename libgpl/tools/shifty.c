/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

    shifty.c

    Shift File Left/Right N Characters.


    SHIFTY is a filter that reads its input, shifts each line of input left
    or right a specified number of character positions, and outputs the
    shifted line.  Shifting a line of text right N positions is equivalent
    to inserting N blanks at the beginning of the line.  Shifting a line of
    text left N positions is equivalent to deleting the first N characters
    of the line.  All input is run through the UNIX EXPAND filter to expand
    tabs to the equivalent number of spaces before performing the shift.
    Form feeds that appear in column 1 of a line are retained in column 1.

    (The name SHIFTY is not entirely for fun - SHIFT is a built-in C-Shell
     command.)


    Invocation:

        % shifty [-l nchars] [-r nchars]  [input_file(s)]

    where:

        "-l <nchars>"
            specifies that each line in the file is to be shifted to the
            left NCHARS characters (effectively deleting the first NCHARS
            characters of each line).
        "-r <nchars>"
            specifies that each line in the file is to be shifted to the
            right NCHARS characters (effectively inserting NCHARS spaces
            at the beginning of each line).
        <input_file(s)>
		are the files to be shifted the specified number of characters.
                If no files are specified, input is taken from standard input.

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  "getopt.h"			/* GETOPT(3) definitions. */

#define  FORM_FEED  12



main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    int  errflg, num_input_files, option, shift_size ;





/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    num_input_files = 0 ;  shift_size = 5 ;

    opterr = -1 ;  errflg = 0 ;

    while (((option = getopt (argc, argv, "l:r:")) != NONOPT) ||
           (optarg != NULL)) {

        switch (option) {
        case 'l':  shift_size = - atoi (optarg) ;  break ;
        case 'r':  shift_size = atoi (optarg) ;  break ;
        case '?':  fprintf (stderr, "shifty: Invalid option - '%c'\n", option) ;
                   errflg++ ;  break ;
        case NONOPT:
            num_input_files++ ;
            shift_file (optarg, shift_size) ;
            break ;
        default :  break ;
        }

    }

    if (errflg) {
        fprintf (stderr,
                 "Usage:  shifty  [-l nchars] [-r nchars]  [input_file(s)]\n") ;
        exit (-1) ;
    }


/* If no files were specified, filter standard input. */

    if (num_input_files == 0)  shift_file (NULL, shift_size) ;

}

/*******************************************************************************

    shift_file ()

    Shift a File N Columns.

    Invocation:

        status = shift_file (file_name, shift_size) ;

    where

        <file_name>
            is the name of the file to be shifted.  A NULL argument indicates
            standard input.
        <shift_size>
            is the number of character positions to shift each line of input.
            A negative shift size indicates a left shift; a positive shift
            size indicates a right shift.
        <status>
            returns zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  shift_file (file_name, shift_size)

    char  *file_name ;
    int  shift_size ;

{    /* Local variables. */
    char  inbuf[1024], outbuf[1024] ;
    int  length ;
    FILE  *infile ;



    strcpy (inbuf, "expand ") ;	/* Expand tabs to spaces. */
    if (file_name != NULL)  strcat (inbuf, file_name) ;
    infile = popen (inbuf, "r") ;
    if (infile == NULL) {
        perror ("popen (\"expand <file>\"") ;  return (errno) ;
    }


    while (!feof (infile)) {

        if (fgets (inbuf, (sizeof inbuf), infile) == NULL)  break ;
        length = strlen (inbuf) ;
        if (inbuf[length-1] == '\n')  inbuf[--length] = '\0' ;

        if (inbuf[0] == FORM_FEED)
            sprintf (outbuf, "%s", inbuf) ;
        else {
            if (shift_size < 0) {
                if (length > abs(shift_size))
                    sprintf (outbuf, "%s", &inbuf[abs(shift_size)]) ;
                else
                    sprintf (outbuf, " ") ;
            } else if (shift_size == 0) {
                sprintf (outbuf, "%s", inbuf) ;
            } else {
                sprintf (outbuf, "%*s%s", shift_size, " ", inbuf) ;
            }
        }

        printf ("%*s\n", str_trim (outbuf, -1), outbuf) ;

    }


    pclose (infile) ;

    return (0) ;

}
