/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

Program:

    netmet

    Network Metrics Tool.


Author:    Alex Measday, ISI


Purpose:

    Program NETMET measures the speed of a network connection.


    Invocation (Client):

        % netmet [-debug] [-records <numRecords>] [-size <numBytes>] [-udp]
                 <port>[@<host>]

    Invocation (Server):

        % netmet [-debug] [-records <numRecords>] [-size <numBytes>] [-udp]
                 -listen <port>

    where

        "-debug"
            enables debug output (written to STDOUT).
        "-records <numRecords>"
            specifies the number of records to send when NETMET is
            functioning as a client.
        "-size <numBytesPerRecord>"
            specifies the size of records sent/received.
        "-udp"
            specifies UDP/IP communications; the default is TCP/IP.
        "<port>[@<host>]"
            causes NETMET to function as a client and specifies the network
            port (i.e., service name or port number) of the server and,
            optionally, the name of the host on which the server is running.
            If no host is specified, the local host is assumed.
        "-listen <port>"
            causes NETMET to function as a server and specifies the network
            server port (i.e., service name or port number) at which NETMET
            will listen for connection requests from clients.

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <signal.h>			/* Signal definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#if defined(VMS)
#    include  <socket.h>		/* Socket-related definitions. */
#elif defined(VXWORKS)
#    include  <socket.h>		/* Socket-related definitions. */
#    include  <sockLib.h>		/* Socket library definitions. */
#    define  exit  return
#else
#    include  <sys/types.h>		/* System type definitions. */
#    include  <sys/socket.h>		/* Socket-related definitions. */
#endif
#include  "bmw_util.h"			/* Benchmarking functions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "udp_util.h"			/* UDP/IP networking utilities. */
#include  "vperror.h"			/* VPERROR() definitions. */

/*******************************************************************************
    NETMET's Main Program.
*******************************************************************************/

#ifdef VXWORKS

int  netmet (

#    if __STDC__
        char  *commandLine)
#    else
        commandLine)

        char  *commandLine ;
#    endif

#else

int  main (

#    if __STDC__
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

#endif

{  /* Local variables. */
    BmwClock  clock ;
    char  *argument, *buffer, *serverName ;
    int  bufferSize, errflg, isClient, option, useTCP ;
    long  i, numRecords, recordSize ;
    unsigned  long  numBytes, totalNumBytes ;

    static  char  *optionList[] = {	/* Command line options. */
        "{buffer:}", "{debug}", "{listen}",
        "{records:}", "{size:}", "{udp}",
        NULL
    } ;





#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("netmet", commandLine, &argc, &argv) ;
#endif

    vperror_print = 1 ;

/* Ignore SIGPIPE signals generated when attempting to write to a broken
   connection. */

    signal (SIGPIPE, SIG_IGN) ;

/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    bufferSize = -1 ;  isClient = 1 ;  numRecords = 1000 ;
    recordSize = 1024 ;  serverName = NULL ;  useTCP = 1 ;

    opt_init (argc, argv, 1, optionList, NULL) ;
    errflg = 0 ;

    while ((option = opt_get (NULL, &argument))) {

        switch (option) {
        case 1:			/* "-buffer <numBytesPerBuffer>" */
            bufferSize = atoi (argument) ;
            break ;
        case 2:			/* "-debug" */
            tcp_util_debug = 1 ;
            udp_util_debug = 1 ;
            break ;
        case 3:			/* "-listen" */
            isClient = 0 ;
            break ;
        case 4:			/* "-records <numRecords>" */
            numRecords = atol (argument) ;
            break ;
        case 5:			/* "-size <numBytesPerRecord>" */
            recordSize = atol (argument) ;
            break ;
        case 6:			/* "-udp" */
            useTCP = 0 ;
            break ;
        case NONOPT:		/* "<port>[@<host>]" */
            serverName = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    if (errflg || (serverName == NULL)) {
        fprintf (stderr, "Usage (Client):  netmet [-debug] [-records <numRecords>]\n") ;
        fprintf (stderr, "                        [-size <numBytes>] [-udp] <port>[@<host>]\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "Usage (Server):  netmet [-debug] [-records <numRecords>]\n") ;
        fprintf (stderr, "                        [-size <numBytes>] [-udp] -listen <port>\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    Allocate a buffer for I/O.
*******************************************************************************/

    buffer = malloc (recordSize) ;
    if (buffer == NULL) {
        vperror ("[NetMet] Error allocating %ld-byte buffer.\nmalloc: ",
                 recordSize) ;
        exit (errno) ;
    }
    memset (buffer, '\0', recordSize) ;


/*******************************************************************************
    NETMET as a TCP/IP Client - connect to the server and send the
        user-specified amount of data to the server.
*******************************************************************************/

    if (useTCP && isClient) {

        TcpEndpoint  connection ;


        if (tcpCall (serverName, 0, &connection)) {
            vperror ("[NetMet] Error establishing connection.\ntcpCall: ") ;
            exit (errno) ;
        }

        tcpSetBuf (connection, 0, bufferSize) ;

        printf ("[NetMet] %s: ", tcpName (connection)) ;  fflush (stdout) ;

        totalNumBytes = 0 ;  vperror_print = 0 ;
        bmwStart (&clock) ;	/* Output the specified amount of data. */
        for (i = 0 ;  i < numRecords ;  i++) {
            if (tcpWrite (connection, -1.0, recordSize, buffer, &numBytes))
                break ;
            totalNumBytes += numBytes ;
        }
        bmwStop (&clock) ;  vperror_print = 1 ;

        tcpDestroy (connection) ;

        printf ("%ld bytes at %g KBytes per second\n",
                totalNumBytes, bmwRate (&clock, totalNumBytes) / 1024.0) ;

    }


/*******************************************************************************
    NETMET as a TCP/IP Server - listen for connection requests from clients.
        When a connection request is received, answer it, read input from the
        client until the connection is closed, and display the transfer speed.
        Then, go back and listen for connection requests from new clients.
*******************************************************************************/

    else if (useTCP && !isClient) {

        TcpEndpoint  connection, listeningPoint ;


        if (tcpListen (serverName, 99, &listeningPoint)) {
            vperror ("[NetMet] Error listening for connection requests.\ntcpListen: ") ;
            exit (errno) ;
        }

        for ( ; ; ) {		/* Listen for and service the next client. */

            if (tcpAnswer (listeningPoint, -1.0, &connection)) {
                vperror ("[NetMet] Error answering connection request.\ntcpAnswer: ") ;
                exit (errno) ;
            }

            tcpSetBuf (connection, bufferSize, 0) ;

            printf ("[NetMet] %s: ", tcpName (connection)) ;  fflush (stdout) ;

            totalNumBytes = 0 ;  vperror_print = 0 ;
            bmwStart (&clock) ;
            for ( ; ; ) {	/* Read input until the connection is closed. */
                if (tcpRead (connection, -1.0, -recordSize, buffer, &numBytes))
                    break ;
                totalNumBytes += numBytes ;
            }
            bmwStop (&clock) ;  vperror_print = 1 ;

            printf ("%ld bytes at %g KBytes per second\n",
                    totalNumBytes, bmwRate (&clock, totalNumBytes) / 1024.0) ;

            tcpDestroy (connection) ;

        }

    }

/*******************************************************************************
    NETMET as a UDP/IP Client - connect to the server and send the
        user-specified amount of data to the server.
*******************************************************************************/

    else if (!useTCP && isClient) {

        UdpEndpoint  client, server ;


        if (udpCreate (NULL, NULL, &client)) {
            vperror ("[NetMet] Error creating client endpoint.\nudpCreate: ") ;
            exit (errno) ;
        }

        udpSetBuf (client, 0, bufferSize) ;

        if (udpCreate (serverName, client, &server)) {
            vperror ("[NetMet] Error creating server endpoint.\nudpCreate: ") ;
            exit (errno) ;
        }

        printf ("[NetMet] %s: ", udpName (server)) ;  fflush (stdout) ;

        totalNumBytes = 0 ;  vperror_print = 0 ;
        bmwStart (&clock) ;	/* Output the specified amount of data. */
        for (i = 0 ;  i < numRecords ;  i++) {
            if (udpWrite (server, -1.0, recordSize, buffer))  break ;
            totalNumBytes += recordSize ;
        }
        bmwStop (&clock) ;  vperror_print = 1 ;

        udpDestroy (client) ;

        printf ("%ld bytes at %g KBytes per second (%d records)\n",
                totalNumBytes, bmwRate (&clock, totalNumBytes) / 1024.0, i) ;

    }


/*******************************************************************************
    NETMET as a UDP/IP Server - read the user-specified amount of data from
        the client.
*******************************************************************************/

    else if (!useTCP && !isClient) {

        UdpEndpoint  client, server ;


        if (udpCreate (serverName, NULL, &server)) {
            vperror ("[NetMet] Error creating server endpoint.\nudpCreate: ") ;
            exit (errno) ;
        }

        udpSetBuf (server, bufferSize, 0) ;

        printf ("[NetMet] %s: ", udpName (server)) ;  fflush (stdout) ;

        totalNumBytes = 0 ;  vperror_print = 0 ;
        bmwStart (&clock) ;	/* Input the specified amount of data. */
        for (i = 0 ;  i < numRecords ;  i++) {
            if (udpRead (server, -1.0, recordSize, buffer, &numBytes, &client))
                break ;
            if (i == 0)  bmwStart (&clock) ;
            totalNumBytes += numBytes ;
        }
        bmwStop (&clock) ;  vperror_print = 1 ;

        udpDestroy (server) ;

        printf ("%ld bytes at %g KBytes per second (%d records)\n",
                totalNumBytes, bmwRate (&clock, totalNumBytes) / 1024.0, i) ;

    }


    exit (0) ;

}
