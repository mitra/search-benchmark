#include  <stdio.h>			/* Standard I/O definitions. */

#define  MAXBUF  (16*1024)




int  main (argc, argv)

    int  argc ;
    char  *argv[] ;

{    /* Local variables. */
    char  buffer[MAXBUF], fileName[64], *inputFile, *outputFile ;
    FILE  *infile, *outfile ;
    int  numFiles ;
    long  inputFileSize, outputFileSize ;
    long  maxBytesPerFile = (128 * 1024) ;
    unsigned  int  numBytesRead ;




/* Scan the command line arguments. */

    switch (argc) {
    case 4:
        maxBytesPerFile = atol (argv[3]) ;
    case 3:
        inputFile = argv[2] ;
        outputFile = argv[1] ;
        break ;
    default:
        fprintf (stderr, "split <inputFile> <outputFile> [<fileSize>]\n") ;
        exit (0) ;
    }

/* Open the input file. */

    if (tstfile (inputFile, &inputFileSize))
        _error ("\n%s: %s does not exist.\n", argv[0], inputFile) ;

    infile = fopen (inputFile, "r") ;
    if (infile == NULL)
        _error ("\n%s: Error opening %s.\n", argv[0], inputFile) ;

    printf ("Input File: %s  (%ld bytes)\n", inputFile, inputFileSize) ;

/* Split the file into parts. */

    numFiles = 0 ;

    while (inputFileSize > 0) {

/* Open the next output file. */

        sprintf (fileName, "%s.p%02d/0", outputFile, ++numFiles) ;
        outfile = fopen (fileName, "w") ;
        if (outfile == NULL)
            _error ("\n%s: Error opening %s.\n", argv[0], fileName) ;
        printf ("Output File: %s", fileName) ;  fflush (stdout) ;

/* Read up to the maximum file size number of bytes from the input file
   and write it to the output file. */

        outputFileSize = 0 ;
        while (outputFileSize < maxBytesPerFile) {

            numBytesRead = fread (buffer, 1, sizeof buffer, infile) ;
            if (numBytesRead <= 0)  break ;

            if (fwrite (buffer, 1, numBytesRead, outfile) < 0)
                _error ("%s: Error writing %ld bytes to %s.\n",
                        argv[0], numBytesRead, fileName) ;

            outputFileSize += numBytesRead ;
            inputFileSize -= numBytesRead ;

        }

        printf ("  (%ld bytes)\n", outputFileSize) ;
        fclose (outfile) ;

    }

    fclose (infile) ;

    exit (0) ;

}
