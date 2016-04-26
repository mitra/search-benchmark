/* $Id: frob.c,v 1.5 2012/08/30 20:02:50 alex Exp alex $ */
/*******************************************************************************

Program:

    frob.c

    Frobnicate Files.


Author:    Alex Measday


Purpose:

    FROB "frobnicate"s (encrypts) one or more files.  Encryption consists of
    simply exclusive-OR'ing each byte in the file with the key.  Since XOR is
    reversible, FROBing the file once encrypts the file; FROBing it a second
    time decrypts the file.

        NOTE: On the NDS and similar platforms, be sure to create
              a top-level "/tmp" directory for use by tmpfile(3).


    Invocation:

        % frob [-key <number>] [-stdout] <file(s)>

    where:

        "-key <number>"
            specifies a number to be exclusive-ORed with each character;
            the default is 42.
        "-stdout"
            causes the encrypted/unencrypted version of the input file(s) to
            be written to standard output.  The input files are NOT modified.
        "<file(s)>"
            are the files to be encrypted.  If the "-stdout" option is NOT
            specified, the files' contents are overwritten by FROB.  If no
            files are specified, FROB reads from standard input and writes
            to standard output (useful in a pipeline).

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

errno_t  processFile (

#    if PROTOTYPES
        unsigned  char  key,
        const  char  *fileName,
        bool  toStdout)
#    else
        key, fileName, toStdout)

        unsigned  char  key ;
        char  *fileName ;
        bool  toStdout ;
#    endif

{    /* Local variables. */
    FILE  *inFile, *outFile ;
    size_t  i, length ;
    unsigned  char  buffer[8192] ;



/* Open the input file. */

    if (fileName == NULL) {
        inFile = stdin ;
    } else {
        inFile = fopen (fileName, "rb") ;
        if (inFile == NULL) {
            perror (fileName) ;
            exit (errno) ;
        }
    }

/* Open a temporary output file. */

    if ((fileName == NULL) || toStdout) {
        outFile = stdout ;
    } else {
        outFile = tmpfile () ;
        if (outFile == NULL) {
            perror ("tmpfile") ;
            exit (errno) ;
        }
    }

/* Copy the input file to the temporary output file, encrypting its contents
   in the process. */

    while (!feof (inFile)) {
        length = fread (buffer, 1, sizeof buffer, inFile) ;
        if (length == 0) {
            if (feof (inFile))  break ;
            if (ferror (inFile)) {
                perror ("fread/in") ;
                exit (errno) ;
            }
        }
        for (i = 0 ;  i < length ;  i++) {
            buffer[i] ^= key ;
        }
        if (fwrite (buffer, 1, length, outFile) != length) {
            perror ("fwrite/out") ;
            exit (errno) ;
        }
    }

    if (fileName == NULL) {
        return (0) ;
    } else if (toStdout) {
        fclose (inFile) ;
        return (0) ;
    }

/* Copy the encrypted temporary file back to the input file. */

    inFile = freopen (fileName, "wb", inFile) ;
    if (inFile == NULL) {
        perror ("freopen/in") ;
        exit (errno) ;
    }

    rewind (outFile) ;

    while (!feof (outFile)) {
        length = fread (buffer, 1, sizeof buffer, outFile) ;
        if (length == 0) {
            if (feof (outFile))  break ;
            if (ferror (outFile)) {
                perror ("fread/out") ;
                exit (errno) ;
            }
        }
        if (fwrite (buffer, 1, length, inFile) != length) {
            perror ("fwrite/in") ;
            exit (errno) ;
        }
    }

    fclose (inFile) ;
    fclose (outFile) ;

    return (0) ;

}

/*******************************************************************************
    FROB's Main Program.
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
    bool  toStdout ;
    char  *argument ;
    int  errflg, numFiles, option ;
    OptContext  scan ;
    unsigned  char  key ;

    const  char  *optionList[] = {	/* Command line options. */
        "{key:}", "{stdout}", NULL
    } ;




    aperror_print = 1 ;

/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    key = 42 ;  numFiles = 0 ;  toStdout = false ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:				/* "-key <number>" */
            key = (unsigned char) atoi (argument) ;
            break ;
        case 2:				/* "-stdout" */
            toStdout = true ;
            break ;
        case NONOPT:
            numFiles++ ;
            fprintf (stderr, "%s\n", argument) ;
            processFile (key, argument, toStdout) ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  frob [-key <number>] [-stdout] <file(s)>\n") ;
        exit (EINVAL) ;
    }

    if (numFiles == 0) {	/* No files?  Read from standard input. */
        processFile (key, NULL, toStdout) ;
    }

    exit (0) ;

}
