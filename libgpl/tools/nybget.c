/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

Program:

    nybget

    Get NY Review of Books Article.


Author:    Alex Measday, ISI


Purpose:

    Program NYBGET retrieves the HTML pages for a NY Review of Books article.


    Invocation:

        % nybget [-debug] [-proxy <server>[@<host>]] <article>

    where

        "-debug"
            enables debug output (written to STDOUT).
        "-proxy <server>[@<host>]"
            specifies a proxy web server.
        "<article>"
            specifies the article, e.g., "19991104032F"; NYBGET automatically
            appends the page specification ("@pN").

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <signal.h>			/* Signal definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "vperror.h"			/* VPERROR() definitions. */


int  debug = 0 ;			/* Global debug switch (1/0 = yes/no). */
char  *proxy = NULL ;			/* Proxy server. */

/*******************************************************************************
    The Main Program.
*******************************************************************************/

int  main (

#    if __STDC__
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{  /* Local variables. */
    char  *argument, *article, request[1024], *string ;
    int  errflg, option, page ;
    LfnStream  stream ;
    TcpEndpoint  connection ;

    static  char  *optionList[] = {	/* Command line options. */
        "{debug}", "{proxy:}",
        NULL
    } ;





    vperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    article = NULL ;

    opt_init (argc, argv, 1, optionList, NULL) ;
    errflg = 0 ;

    while ((option = opt_get (NULL, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            debug = 1 ;
            lfn_util_debug = 1 ;
            vperror_print = 1 ;
            break ;
        case 2:			/* "-proxy <server>[@<host>]" */
            proxy = argument ;
            break ;
        case NONOPT:		/* "<article>" */
            article = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    if (errflg || (article == NULL)) {
        fprintf (stderr, "Usage:  nybget [-debug] [-proxy <server>[@<host>]] <article>\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    Retrieve each page of the article.
*******************************************************************************/

    for (page = 1 ;  ;  page ++) {

/* Establish a network connection with the search engine. */

        char  *server = (proxy == NULL) ? "80@www.nybooks.com" : proxy ;

        if (tcpCall (server, 0, &connection)) {
            vperror ("[%s] Error connecting to %s.\ntcpCall: ",
                     argv[0], server) ;
            exit (errno) ;
        }

        if (sktSetBuf (tcpName (connection), tcpFd (connection), 8192, 8192)) {
            vperror ("[%s] Error setting sizes of receive/send buffers for %s.\nsktSetBuf: ",
                     argv[0], server) ;
            return (errno) ;
        }

        if (lfnCreate (connection, NULL, &stream)) {
            vperror ("[%s] Error creating LF-terminated stream for %s.\nlfnCreate: ",
                     argv[0], server) ;
            return (errno) ;
        }

/* Format the request for the page and send it to the server. */

        sprintf (request,
                 "%s/nyrev/WWWfeatdisplay.cgi?%s@p%d",
                 (proxy == NULL) ? "" : "http://www.nybooks.com",
                 article, page) ;

        fprintf (stderr, "%s", request) ;

        if (lfnPutLine (stream, -1.0, "GET %s HTTP/1.0\n\n", request)) {
            vperror ("[%s] Error sending request to %s.\nlfnWrite: ",
                     argv[0], lfnName (stream)) ;
            return (errno) ;
        }

/* Read the header information returned by the server. */

        if (lfnGetLine (stream, -1.0, &string)) {
            vperror ("[%s] Error reading header from %s.\nlfnGetLine: ",
                     argv[0], lfnName (stream)) ;
            return (errno) ;
        }

        while (strcmp (string, "") != 0) {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("[%s] Error reading header from %s.\nlfnGetLine: ",
                         argv[0], lfnName (stream)) ;
                return (errno) ;
            }
        }

/* Read the article. */

        for ( ; ; ) {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("[%s] Error reading search results from %s.\nlfnGetLine: ",
                         argv[0], lfnName (stream)) ;
                break ;
            }
            printf ("%s\n", string) ;
        }

/* Close the connection to the server. */

        lfnDestroy (stream) ;

    }

    exit (0) ;

}
