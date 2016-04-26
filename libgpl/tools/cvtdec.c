/* $Id: cvtdec.c,v 1.2 2011/03/31 22:36:12 alex Exp alex $ */
/*******************************************************************************

    cvtdec.c

    Convert DEC Variable Record Length Files to Unix Files.


    Program CVTDEC converts a Digital Equipment Corporation (DEC) variable
    record length sequential file to a normal Unix text file.


    Invocation:

        % cvtdec [-binary] [-debug] [-newline]
                 <inputFile> [-output <file>]

    where:

        "-binary"
            specifies that the record length is a 2-byte binary count rather
            than an ASCII-encoded number.
        "-debug"
            turns debug on.
        "-newline"
            inhibits the program from appending a newline character to the end
            of a record on output.
        "<inputFile>"
            is the DEC file to be input and converted.  If this argument
            is not specified, input is taken from standard input.
        "-output <file>"
            specifies a file to which the converted text is to be output.  If
            this argument is not specified, the output is written to standard
            output.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Character classification definitions. */
#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "aperror.h"			/* APERROR() definitions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */


#define  MAXRECLEN  1024





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
    char  *argument, field[4+1], *inputFile, *outputFile ;
    int  binaryCount, c, debug, errflg, i, length, newlineMode, option ;
    FILE  *infile, *outfile ;
    OptContext  scan ;

    const  char  *optionList[] = {	/* Command line options. */
        "{binary}", "{debug}", "{newline}", "{output:}", NULL
    } ;




/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    binaryCount = 0 ;  debug = 0 ;  newlineMode = 1 ;
    inputFile = outputFile = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-binary" */
            binaryCount = 1 ;
            break ;
        case 2:			/* "-debug" */
            debug = 1 ;
            break ;
        case 3:			/* "-newline" */
            newlineMode = 1 ;
            break ;
        case 4:			/* "-output <file> " */
            outputFile = argument;
            break ;
        case NONOPT:		/* "<inputFile>" */
            if (inputFile == NULL)
                inputFile = argument ;
            else
                errflg++ ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default:  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  cvtdec [-binary] [-debug] [-newline] [<inputFile>] [-output <file>]\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************

    Read and convert the input file.

*******************************************************************************/

    outfile = stdout ;
    infile = fopen (inputFile, "r") ;
    if (infile == NULL) {
        aperror ("[%s] Error opening input file: %s\nfopen: ", argv[0], inputFile) ;
        exit (errno) ;
    }

    if (binaryCount) {
        while (fread (&field, 1, 2, infile) == 2) {
            length = (field[1] * 256) + field[0] ;
            if (length % 2)  length++ ;
            if (debug)  fprintf (stderr, "(binary) length = %d\n", length) ;
            while ((length-- > 0) && ((c = getc (infile)) != EOF)) {
                if ((c != '\0') && (putc (c, outfile) == EOF))  exit (errno) ;
            }
            if (newlineMode)  putc ('\n', outfile) ;
        }
        exit (0) ;
    }


    while ((c = getc (infile)) != EOF) {

/* Look for the start of the record length field. */

        if (!isascii (c) || !isdigit (c)) {
            if (debug)  fprintf (stderr, "... skipping '%c' ...\n", c) ;
            continue ;
        }

/* Extract the record length field ; the record length includes this field. */

        i = 0 ;  field[i++] = c ;
        while ((i < 4) && ((c = getc (infile)) != EOF))
            field[i++] = c ;
        if (i < 4) {
            fprintf (stderr, "cvtdec: Error reading record length field.\n") ;
            perror ("getc") ;  exit (errno) ;
        }
        field[i] = '\0' ;

        length = atoi (field) ;
        if (debug)  fprintf (stderr, "length = %d\n", length) ;
        if (length < 4)  continue ;	/* End-of-tape-record indicator? */
        if (length > MAXRECLEN) {	/* Artificial Intelligence!  Reasonable record length? */
            fprintf (stderr, "cvtdec: Invalid record length %d\n", length) ;
            continue ;
        }

        length = length - 4 ;

/* Read the text portion of the record and write it to the output file. */

        while ((length > 0) && ((c = getc (infile)) != EOF)) {
            if (putc (c, outfile) == EOF) {
                fprintf (stderr, "cvtdec: Error writing to output file.\n") ;
                perror ("putc") ;  exit (errno) ;
            }
            length-- ;
        }

/* Append a newline character to the record. */

        if (newlineMode) {
            if (putc ('\n', outfile) == EOF) {
                fprintf (stderr, "cvtdec: Error writing newline to output file.\n") ;
                perror ("putc") ;  exit (errno) ;
            }
        }

    }

    exit (0) ;

}
