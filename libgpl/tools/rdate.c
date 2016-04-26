/* $Id: rdate.c,v 1.2 2011/03/31 22:30:18 alex Exp $ */
/**************************************************************************

Process:

    rdate

    Remote Date Setter.


Author:    Alex Measday


Purpose:

    RDATE retrieves the current date and time from a remote host and
    (under VxWorks) sets the system clock.

    RDATE establishes a TCP/IP network connection with the "time" server
    (port 37) on the remote host and reads the time sent by the "time"
    server.  This time is the number of seconds since January 1, 1900;
    RDATE converts this time to a UNIX-compatible time (i.e., the number
    of seconds since January 1, 1970).  Under VxWorks, RDATE then sets
    the system clock to the new time.

    A UNIX version of RDATE from Dartmouth College provided the information
    needed to interpret the time from the "time" server.  Apparently, the
    protocol is defined by RFC 868.  The Dartmouth RDATE sends a null packet
    (i.e., zero bytes in length) to the "time" server and measures how long
    it takes for the "time" server to respond; this round-trip time is then
    used to estimate a latency correction for the "time" server's time.
    Since our Sun "time" servers send the time to you whether or not you
    send a null packet (thus making the round-trip time somewhat suspect)
    and since the one-second resolution of the "time" server's time seems
    to dwarf our network transmission times, my version of RDATE doesn't
    send the null packet or correct for network delays.


    Invocation (under UNIX):

        % rdate [-debug] <host>

    Invocation (under VxWorks):

        -> rdate "[-debug] <host>"

    where

        "-debug"
            enables level debug output (to standard output).
        "<host>"
            is the name of the host from which the time is to be retrieved.

**************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */

#define  OFFSET_1900  2208988800UL

/*******************************************************************************
    RDATE's Main Program.
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
    int  errflg, option, status ;
    OptContext  scan ;
    struct  timeval  localTime ;
    unsigned  long  remoteTime ;
    TcpEndpoint  server ;

    const  char  *optionList[] = {	/* Command line options. */
        "{debug}", NULL
    } ;




    aperror_print = 1 ;			/* Enable APERROR output. */

    if (sktStartup ()) {		/* Initialize networking. */
        exit (errno) ;
    }


/*******************************************************************************
  Scan the command line options.
*******************************************************************************/

    host = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            tcp_util_debug = 1 ;  break ;
        case NONOPT:
            host = argument ;  break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg || (host == NULL)) {
        fprintf (stderr, "Usage:  rdate [-debug] <host>\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    Connect to the remote host and retrieve the time.
*******************************************************************************/

/* Construct the time server`s name. */

#if defined(HAVE_ETC_SERVICES) && !HAVE_ETC_SERVICES
    strcpy (serverName, "37") ;
#else
    strcpy (serverName, "time") ;
#endif
    if (host != NULL) {
        strcat (serverName, "@") ;
        strcat (serverName, host) ;
    }

/* Connect to the time server. */

    if (tcpCall (serverName, true, &server) ||
        tcpComplete (server, 30.0, true)) {
        LGE "[RDATE] Error connecting to %s.\ntcpCall: ", serverName) ;
        exit (errno) ;
    }

/* Read the time from the time server. */

    if (tcpRead (server, 15.0, sizeof remoteTime, (char *) &remoteTime, NULL)) {
        LGE "[RDATE] Error reading time from %s.\ntcpRead: ", serverName) ;
        status = errno ;  tcpDestroy (server) ;  errno = status ;
        exit (errno) ;
    }
    remoteTime = ntohl (remoteTime) ;

/* The time retrieved from the time server is the number of seconds since
   January 1, 1900.  Convert it to a UNIX-compatible number of seconds
   since January 1, 1970.  (That very large constant was borrowed from
   Dartmouth's RDATE program.) */

    localTime.tv_sec = remoteTime - OFFSET_1900 ;
    localTime.tv_usec = 0 ;

    printf (" Local Time: %s\n", tvShow (tvTOD (), 0, "%c")) ;
    printf ("Remote Time: %s\n", tvShow (localTime, 0, "%c")) ;

/* Close the connection with the time server. */

    tcpDestroy (server) ;


    if (sktCleanup ()) {		/* Terminate networking. */
        exit (errno) ;
    }


/*******************************************************************************
    Set the system time.
*******************************************************************************/

#ifdef VXWORKS
    if (clock_settime (CLOCK_REALTIME, &localTime) != 0) {
        aperror ("[RDATE] Error setting the system clock to %s.\nclock_settime: ",
                 tsShow (localTime, 0, "%c")) ;
        exit (errno) ;
    }
    printf ("   New Time: %s\n", tsShow (tsTOD (), 0, "%c")) ;
#endif


    exit (0) ;

}
