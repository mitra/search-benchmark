/* $Id: scanet.c,v 1.3 2011/03/31 22:27:24 alex Exp $ */
/*******************************************************************************

Process:

    scanet

    Network Scanning Utility.


Author:    Alex Measday


Purpose:

    SCANET scans the network ports on a computer, looking for active
    listening ports.


    Invocation:

        % scanet [-debug] [-from <lower>] [-to <upper>] <host>

    where:

        "-debug"
            enables debug output (written to STDOUT).
        "-from <lower>"
        "-to <upper>"
            specify the range of port numbers to scan.  The default lower
            bound is port 0; the default upper bound is something really
            high.
        "<host>"
            is the name of the computer being scanned.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  "net_util.h"			/* Networking utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */

/*******************************************************************************
    SCANET's Main Program.
*******************************************************************************/

extern  int  main P_((int argc, char *argv[]))  OCD ("sio_appl") ;

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
    char  *argument, *host, serverName[256] ;
    int  errflg, option ;
    long  i, lower, upper ;
    OptContext  scan ;
    TcpEndpoint  connection ;

    const  char  *optionList[] = {	/* Command line options. */
        "{debug}", "{from:}", "{to:}", NULL
    } ;




/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    if (sktStartup ()) {		/* Initialize WINSOCK on Windows. */
        exit (errno) ;
    }

    host = NULL ;  lower = 0 ;  upper = 128L * 1024L ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            tcp_util_debug = 1 ;
            break ;
        case 2:			/* "-from <lower>" */
            lower = atoi (argument) ;
            break ;
        case 3:			/* "-to <upper>" */
            upper = atoi (argument) ;
            break ;
        case NONOPT:
            host = argument ;
            if (netAddrOf (host) == 0) {
                fprintf (stderr, "[%s] Invalid host: %s\n", argv[0], host) ;
                errflg++ ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg || (host == NULL)) {
        fprintf (stderr, "Usage:  scanet [-debug] [-from <lower>] [-to <upper>] <host>\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    Scan the target computer for active listening ports.
*******************************************************************************/

    for (i = lower ;  i <= upper ;  i++) {
        sprintf (serverName, "%ld@%s", i, host) ;
        if (tcpCall (serverName, 0, &connection))  continue ;
        printf ("%s\n", serverName) ;
        tcpDestroy (connection) ;
    }


    exit (0) ;

}
