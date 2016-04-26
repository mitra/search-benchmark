/*
@(#) File name: cvtpe.c Release: 1.1  Date: 5/9/90, 13:38:43
*/
/*******************************************************************************

    cvtpe.c

    Convert Perkin-Elmer Tape Files to Unix Files.


    Program CVTPE converts a Perkin-Elmer (PE) sequential file to a normal
    UNIX text file.  CVTPE assumes that the PE file header occupies the
    first magnetic tape block (see the "-b <size>" option below) and that
    the remainder of the file (blocks 2-N) consists of fixed-length text
    records (see the "-r <length>" option below).


    Invocation:

        % cvtpe [-b size] [-d] [-o output_file] [-r length]  [input_file]

    where:

        "-b <size>"
            specifies the block size (in bytes) in the input file; the default
            is 12288 (12K) bytes.
        "-d"
            turns debug on.
        "-o <output_file>"
            specifies a file to which the converted text is to be output.  If
            this argument is not specified, the output is written to standard
            output.
        "-r <length>"
            specifies the length of the fixed-length records in the file; the
            default is 132 characters.
        "<input_file>"
            is the PE file to be input and converted.  If this argument
            is not specified, input is taken from standard input.

*******************************************************************************/


#include  <ctype.h>			/* Character classification definitions. */
#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  "getopt.h"			/* GETOPT(3) definitions. */

					/* External functions. */
extern  int  str_trim () ;

#define  MAXBLOCKSIZE  (32*1024)





main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    char  buffer[MAXBLOCKSIZE], *file_spec, *input_file, *output_file ;
    int  block_size, debug, errflg, i, length ;
    int  num_records_per_block, option, record_length ;
    FILE  *infile, *outfile ;





/*******************************************************************************

  Scan the command line options.

*******************************************************************************/

    debug = 0 ;
    block_size = 12*1024 ;  record_length = 132 ;
    input_file = output_file = NULL ;

    errflg = 0 ;

    while (((option = getopt (argc, argv, "b:do:r:")) != NONOPT) ||
           (optarg != NULL)) {

        switch (option) {
        case 'b':  block_size = atoi (optarg) ;  break ;
        case 'd':  debug = -1 ;  break ;
        case 'o':  output_file = optarg ;  break ;
        case 'r':  record_length = atoi (optarg) ;  break ;
        case '?':  errflg++ ;  break ;
        case NONOPT:
            if (input_file == NULL)  input_file = argv[optind] ;
            break ;
        default :  break ;
        }

    }

    if (errflg) {
        fprintf (stderr, "Usage:  cvtpe [-b block_size] [-d] [-o output_file]\n") ;
        fprintf (stderr, "              [-r record_length]  [input_file]\n") ;
        exit (-1) ;
    }

    if (block_size > sizeof buffer) {
        fprintf (stderr, "(cvtpe) %d-byte block size is too large.\n", block_size) ;
        exit (-1) ;
    }
    if ((record_length+1) > sizeof buffer) {
        fprintf (stderr, "(cvtpe) %d-byte record length is too large.\n", record_length) ;
        exit (-1) ;
    }

/*******************************************************************************

    Read and convert the input file.

*******************************************************************************/


    if (open_output_file (output_file, &outfile, &file_spec))  exit (errno) ;
    if (open_input_file (input_file, &infile, &file_spec))  exit (errno) ;
    fprintf (stderr, "Converting \"%s\" ...\n", file_spec) ;


/* Skip the header record. */

    length = fread (buffer, 1, block_size, infile) ;
    if (debug)  fprintf (stderr, "length = %d (header)\n", length) ;
    if (length < block_size) {
        vperror ("(cvtpe) Error reading %d-byte file header.\nfread: ", block_size) ;
        exit (errno) ;
    }


/* For each of the remaining blocks in the input file, read the block and
   output each of the records in the block. */

    num_records_per_block = block_size / record_length ;

    for ( ; ; ) {

        length = fread (buffer, 1, block_size, infile) ;
        if (debug)  fprintf (stderr, "length = %d\n", length) ;
        if (length == 0)  break ;	/* End-of-file. */
        if (length < block_size) {
            vperror ("(cvtpe) Error reading %d-byte text block.\nfread: ", block_size) ;
            exit (errno) ;
        }

        for (i = 0 ;  i < num_records_per_block ;  i++) {
            length = str_trim (&buffer[i*record_length], record_length) ;
            fwrite (&buffer[i*record_length], 1, length, outfile) ;
            fprintf (outfile, "\n") ;
        }

    }


/* Close the files.*/

    fclose (infile) ;
    fclose (outfile) ;

}
