/*
@(#) File name: fifo.c Release: 1.1  Date: 5/9/90, 13:38:55
*/
/*******************************************************************************

    fifo.c

    FIFO Utility.


    Program FIFO acts as either a reader from or a writer to a UNIX FIFO.
    In reader mode, the FIFO program creates a FIFO and then continuously
    reads messages from the FIFO; incoming messages are written to standard
    output.  In writer mode, the FIFO program opens an existing FIFO and
    then writes whatever the user types in to the FIFO.


    Invocation:

        % fifo [-d] [-r] [-w] [FIFO_name]

    where:

        "-d"    turns debug on.
        "-r"    puts the FIFO program in reader mode.
        "-w"    puts the FIFO program in writer mode.
        <FIFO_name>
                is the UNIX pathname for the FIFO.

*******************************************************************************/

#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <string.h>			/* String functions. */
#include  <sys/types.h>			/* System type definitions. */
#ifdef  PRE_SUN_OS_40
#    include  "fd.h"			/* File descriptor set definitions. */
#endif
#include  "getopt.h"			/* GETOPT(3) definitions. */

extern  int  msx_util_debug ;		/* Global debug switch (-1/0 = yes/no). */

					/* External functions. */
extern  char  *set_handler () ;
extern  int  convert_string (), dump_data () ;
extern  int  msx_create (), msx_open (), msx_read (), msx_write (), select () ;
extern  void  generic_handler () ;


#define  MAX_STRING  8192
#define  READER  0
#define  WRITER  1


main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    char  buffer[MAX_STRING], *fifo_name ;
    int  address, channel, errflg, length, mode ;
    int  num_bytes_per_line, option, status ;
    fd_set  read_mask ;





/*******************************************************************************

  Scan the command line options.

*******************************************************************************/

    errflg = 0 ;  fifo_name = NULL ;  mode = WRITER ;

    while (((option = getopt (argc, argv, "drw")) != NONOPT) ||
           (optarg != NULL)) {
        switch (option) {
        case 'd':  msx_util_debug = -1 ;  break ;
        case 'r':  mode = READER ;  break ;
        case 'w':  mode = WRITER ;  break ;
        case '?':  errflg++ ;  break ;
        case NONOPT:
            fifo_name = optarg ;  break ;
        default :  break ;
        }
    }

    if (errflg) {
        fprintf (stderr, "Usage:  fifo [-d] [-r] [-w]  [fifo_name]\n") ;
        exit (-1) ;
    }

/*******************************************************************************

    Reader mode - create the FIFO and then loop, reading data from the FIFO
    and writing it out to standard output.

*******************************************************************************/

    if (mode == READER) {

        if (msx_create (fifo_name, &channel)) {
            fprintf (stderr, "[FIFO] Unable to create reader channel.\n") ;
            exit (-1) ;
        }

        for ( ; ; ) {

            FD_ZERO(&read_mask) ;  FD_SET(channel, &read_mask) ;
            for ( ; ; ) {
                if (select (FD_SETSIZE, &read_mask, NULL, NULL, NULL) < 0) {
                    if (errno == EINTR)  continue ;	/* SELECT interrupted by signal - try again. */
                    fprintf (stderr, "[FIFO] Error selecting input.\n") ;
                    perror ("select") ;  exit (errno) ;
                } else {
                    break ;
                }
            }

            if (msx_read (channel, sizeof buffer, buffer, &length)) {
                fprintf (stderr, "[FIFO] Error reading from channel.\n") ;
                exit (-1) ;
            }

            printf ("FIFO %s:\n", fifo_name) ;
            address = 0 ;  num_bytes_per_line = 16 ;
            while (length > 0) {
                dump_data (stdout, address, &buffer[address], length,
                           'X', 0, num_bytes_per_line) ;
                address = address + num_bytes_per_line ;
                length = length - num_bytes_per_line ;
            }

        }

    }

/*******************************************************************************

    Writer mode - open the FIFO and then loop, reading data from standard
    input and writing it to the FIFO.

*******************************************************************************/

    else {

        if (msx_open (fifo_name, &channel)) {
            fprintf (stderr, "[FIFO] Unable to open writer channel.\n") ;
            exit (-1) ;
        }

        for ( ; ; ) {

            if (gets (buffer) == NULL) {
                if (feof (stdin)) {
                    fprintf (stderr, "[FIFO] EOF on standard input.\n") ;
                    exit (0) ;
                } else {
                    fprintf (stderr, "[FIFO] Error reading user input.\n") ;
                    perror ("fgets") ;  exit (errno) ;
                }
            }

            length = strlen (buffer) ;
            convert_string (buffer, &length) ;
            if (msx_write (channel, buffer, length)) {
                fprintf (stderr, "[FIFO] Error writing to channel.\n") ;
                exit (-1) ;
            }

        }

    }

}
