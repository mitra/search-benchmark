/* $Id: dump.c,v 1.4 2011/03/31 22:26:41 alex Exp alex $ */
/*******************************************************************************

    dump.c

    File Dump Utility.


    Alex Measday


    DUMP dumps the contents of a file to the standard output.  The data is
    represented in two ways in the output, as numerical data dumped in a
    user-specified format and as character data dumped in a string:

        <address>: <data in user-specified format> "<data as a string>"


    Invocation:

        % dump [-d] [-e] [-E] [-n numToDump] [-o] [-s offset]
               [-t] [-w num] [-x] [inputFile]

    where:

        "-decimal"
            dumps each byte as a decimal number.
        "-ebcdic"
            specifies that the input data is EBCDIC.  The numeric portion of
            the dump is unconverted data and the ASCII portion of the dump is
            converted data.
        "-hexadecimal"
            dumps every 4 bytes as a hexadecimal number.
        "-number <bytesToDump>"
            specifies the number of bytes to dump.
        "-octal"
            dumps each byte as an octal number.
        "-skip <offset>"
            specifies the number of bytes (0..N-1) in the input file to skip
            before starting to dump the data.
        "-text"
            dumps each byte as text, i.e., the data is not dumped in numeric
            form.  Each output line will appear as:
                      <address>: "<data as a string>"
        "-width <num>"
            changes the number of bytes (width) of input data dumped on each
            line of output.
        <inputFile>
            is the file to be dumped.  If this argument is a wildcard file
            specification, only the first file in the expanded list is
            dumped.  Files with a ".Z" extension are automatically piped
            through the "uncompress" filter.  If this argument is not
            specified, input is taken from standard input.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "meo_util.h"			/* Memory operations. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */

#define  MAX_LENGTH (128*1024)
#define  MAX_STRING  256

#define  ASCII  'A'			/* Input data modes. */
#define  EBCDIC  'E'

/*******************************************************************************
    Main Program
*******************************************************************************/


#ifdef VXWORKS

int  dump (

#    if PROTOTYPES
        char  *commandLine)
#    else
        commandLine)

        char  *commandLine ;
#    endif

#else

int  main (

#    if PROTOTYPES
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

#endif

{    /* Local variables. */
    char  *argument, buffer[MAX_STRING] ;
    char  *inputFile, *outputFile ;
    int  dumpMode, errflg, inputMode, length ;
    int  numBytesToDump, numBytesPerLine, offset, option ;
    FILE  *infile, *outfile ;
    long  address ;
    OptContext  scan ;

    const  char  *optionList[] = {	/* Command line options. */
        "{decimal}", "{ebcdic}", "{hexadecimal}", "{number:}",
        "{octal}", "{skip:}", "{text}", "{width:}",
        NULL
    } ;





#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("dump", commandLine, &argc, &argv) ;
#endif


    aperror_print = 1 ;


/*******************************************************************************
  Scan the command line options.
*******************************************************************************/

    dumpMode = MeoHexadecimal ;  inputMode = ASCII ;
    numBytesPerLine = 0 ;  numBytesToDump = -1 ;  offset = 0 ;
    inputFile = NULL ;  outputFile = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-decimal" */
            dumpMode = MeoDecimal ;
            break ;
        case 2:			/* "-ebcdic" */
            inputMode = EBCDIC ;
            break ;
        case 3:			/* "-hexadecimal" */
            dumpMode = MeoHexadecimal ;
            break ;
        case 4:			/* "-number <bytesToDump>" */
            numBytesToDump = atoi (argument) ;
            break ;
        case 5:			/* "-octal" */
            dumpMode = MeoOctal ;
            break ;
        case 6:			/* "-skip <offset>" */
            offset = atoi (argument) ;
            break ;
        case 7:			/* "-text" */
            dumpMode = MeoText ;
            break ;
        case 8:			/* "-width <num>" */
            numBytesPerLine = atoi (argument) ;
            break ;
        case NONOPT:
            if (inputFile == NULL)  inputFile = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  dump  [-decimal] [-ebcdic] [-hexadecimal] [-number <bytes>]\n") ;
        fprintf (stderr, "              [-octal] [-skip <offset>] [-text] [-width <numBytes>\n") ;
        fprintf (stderr, "              <inputFile>\n") ;
        exit (-1) ;
    }

/* Set the number of bytes dumped on each line of output. */

    if (numBytesPerLine <= 0) {
        switch (dumpMode) {
        case MeoDecimal:      numBytesPerLine = 8 ;  break ;
        case MeoHexadecimal:  numBytesPerLine = 16 ;  break ;
        case MeoOctal:        numBytesPerLine = 8 ;  break ;
        case MeoText:         numBytesPerLine = 40 ;  break ;
        default:              break ;
        }
    }

/*******************************************************************************

  Read and dump the input file.

*******************************************************************************/

    if (outputFile == NULL) {
        outputFile = "<stdout>" ;
        outfile = stdout ;
    } else if (NULL == (outfile = fopen (outputFile, "w"))) {
        aperror ("(dump) Error opening output file: %s\nfopen: ", outputFile) ;
        exit (errno) ;
    }

    if (inputFile == NULL) {
        inputFile = "<stdin>" ;
        infile = stdin ;
    } else if (NULL == (infile = fopen (inputFile, "rb"))) {
        aperror ("(dump) Error opening input file: %s\nfopen: ", inputFile) ;
        exit (errno) ;
    }

    fprintf (outfile, "Input File: \"%s\"\n\n", inputFile) ;

    if (offset > 0) {
        if (fseek (infile, offset, 0) != 0) {
            aperror ("(dump) Error positioning to byte %d of the input file.\nfseek: ",
                     offset) ;
            exit (errno) ;
        }
        address = ftell (infile) ;
    } else {
        address = 0 ;
    }

    memset (buffer, '\0', sizeof buffer) ;
    while (0 < (length = fread (buffer, 1, numBytesPerLine, infile))) {
        if (inputMode == EBCDIC)  strEtoA (buffer, length) ;
        meoDump (outfile, NULL, dumpMode, numBytesPerLine,
                 (void *) address, buffer, length) ;
        address = address + length ;
        if (numBytesToDump != -1) {
            numBytesToDump = numBytesToDump - length ;
            if (numBytesToDump <= 0)  break ;	/* Dumped enough bytes? */
        }
        memset (buffer, '\0', sizeof buffer) ;
    }


    exit (0) ;

}
