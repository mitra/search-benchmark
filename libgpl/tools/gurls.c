/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

Program:

    gurls

    Gather URLs.


Author:    Alex Measday, ISI


Purpose:

    Program GURLS accesses a WWW search engine and retrieves the URLs
    of pages containing the desired key.


    Invocation:

        % gurls [-debug] [-engine <name>] [-news] [-proxy <server>[@<host>]]
                [-summary] [-url] [-verbose] <query>

    where

        "-debug"
            enables debug output (written to STDOUT).
        "-engine <name>"
            specifies the name of the search engine; the default is "alta"
            for the AltaVista search engine.
        "<query>"
            is the query expression to be passed to the search engine.

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

					/* Search parameters. */
char  *proxy = NULL ;			/* Proxy server. */
int  onlyURL = 0 ;			/* Report URL or complete HTML line? */
int  searchNews = 0 ;			/* Search news archive or WWW? */
int  summariesToo = 0 ;			/* Retrieve summaries too? */
int  verbose = 0 ;			/* Track progress on stderr? */


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  int  retrieveAltaVista (
#    if __STDC__
        const  char  *query
#    endif
    ) ;

static  int  retrieveExcite (
#    if __STDC__
        const  char  *query
#    endif
    ) ;

static  int  retrieveWebCrawler (
#    if __STDC__
        const  char  *query
#    endif
    ) ;

static  int  retrieveYahoo (
#    if __STDC__
        const  char  *query
#    endif
    ) ;

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
    char  *argument, *engine, *query ;
    int  errflg, option ;

    static  char  *optionList[] = {	/* Command line options. */
        "{debug}", "{engine:}", "{news}", "{proxy:}",
        "{summary}", "{url}", "{verbose}",
        NULL
    } ;





    vperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    engine = "altaVista" ;  query = NULL ;

    opt_init (argc, argv, 1, optionList, NULL) ;
    errflg = 0 ;

    while ((option = opt_get (NULL, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            debug = 1 ;
            lfn_util_debug = 1 ;
            vperror_print = 1 ;
            break ;
        case 2:			/* "-engine <name>" */
            if (strMatch (engine, "altaVista") ||
                strMatch (engine, "excite") ||
                strMatch (engine, "webCrawler") ||
                strMatch (engine, "yahoo")) {
                engine = argument ;
            } else {
                errflg++ ;
            }
            break ;
        case 3:			/* "-news" */
            searchNews = 1 ;
            break ;
        case 4:			/* "-proxy <server>[@<host>]" */
            proxy = argument ;
            break ;
        case 5:			/* "-summary" */
            summariesToo = 1 ;  onlyURL = 0 ;
            break ;
        case 6:			/* "-url" */
            onlyURL = 1 ;  summariesToo = 0 ;
            break ;
        case 7:			/* "-verbose" */
            verbose = 1 ;
            break ;
        case NONOPT:		/* "<query>" */
            query = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    if (errflg) {
        fprintf (stderr, "Usage:  gurls [-debug] [-engine <name>] [-news] [-proxy <server>[@<host>]]\n") ;
        fprintf (stderr, "              [-summary] [-url] [-verbose] <query>\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    Retrieve the desired information from the specified search engine.
*******************************************************************************/

    if (strMatch (engine, "altaVista")) {
        retrieveAltaVista (query) ;
    } else if (strMatch (engine, "excite")) {
/*        retrieveExcite (query) ; */
    } else if (strMatch (engine, "webCrawler")) {
        retrieveWebCrawler (query) ;
    } else if (strMatch (engine, "yahoo")) {
/*        retrieveYahoo (query) ; */
    }

    exit (0) ;

}

/*******************************************************************************

Procedure:

    retrieveAltaVista ()

    Retrieve Information from the AltaVista Search Engine.


Purpose:

    Function retrieveAltaVista() queries the AltaVista search engine for the
    desired information and prints a list of the matching URLs to standard
    output.


    Invocation:

        status = retrieveAltaVista (query) ;

    where:

        <query>		- I
            is the search-engine-specific query for the desired
            information.
        <status>	- O
            returns the status of retrieving the information, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


static  int  retrieveAltaVista (

#    if  __STDC__
        const  char  *query)
#    else
        query)

        char  *query ;
#    endif

{    /* Local variables. */
    char  command[1024], *s, *string ;
    int  firstItem, lastItem, totalItems ;
    LfnStream  stream ;
    TcpEndpoint  connection ;




/*******************************************************************************
    Query the search engine for the desired information and extract the
    matching URLs from each page returned by the search engine.
*******************************************************************************/

    if (!onlyURL)  printf ("<PRE>\n") ;
    lastItem = 0 ;

    do {

/* Establish a network connection with the search engine. */

        if (verbose)
            fprintf (stderr, "[gurls] Connecting: www.altavista.digital.com\n") ;

        if (tcpCall ((proxy == NULL) ? "80@www.altavista.digital.com" : proxy,
                     0, &connection)) {
            vperror ("(retrieveAltaVista) Error connecting to search engine.\ntcpCall: ") ;
            return (errno) ;
        }

        if (sktSetBuf (tcpName (connection), tcpFd (connection), 8192, 8192)) {
            vperror ("(retrieveAltaVista) Error setting sizes of receive/send buffers.\nsktSetBuf: ") ;
            return (errno) ;
        }

        if (lfnCreate (connection, NULL, &stream)) {
            vperror ("(retrieveAltaVista) Error creating LF-terminated stream for search engine.\nlfnCreate: ") ;
            return (errno) ;
        }

/* Format the query for the next batch of items and send the query to the
   search engine. */

        sprintf (command,
                 "%s/cgi-bin/query?pg=aq&what=%s&stq=%d&fmt=%s&text=yes&q=%s",
                 (proxy == NULL) ? "" : "http://www.altavista.digital.com",
                 searchNews ? "news" : "web", lastItem,
                 summariesToo ? "d" : "c", query) ;

        if (verbose)  fprintf (stderr, "[gurls] %s\n", command) ;

        if (lfnPutLine (stream, -1.0, "GET %s HTTP/1.0\n\n", command)) {
            vperror ("(retrieveAltaVista) Error sending query to search engine.\nlfnWrite: ") ;
            return (errno) ;
        }

/* Read the header information returned by the search engine. */

        if (lfnGetLine (stream, -1.0, &string)) {
            vperror ("(retrieveAltaVista) Error reading header from %s.\nlfnGetLine: ",
                     lfnName (stream)) ;
            return (errno) ;
        }

        if (verbose)  fprintf (stderr, "[gurls] %s\n", string) ;

        while (strcmp (string, "") != 0) {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveAltaVista) Error reading header from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
        }

/* Continue reading the page from the search engine and look for the start
   of the search results. */

        do {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveAltaVista) Error reading page from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
            if (debug)  printf ("(retrieveAltaVista) Leading text: \"%s\"\n", string) ;
        } while ((s = strstr (string, "documents match your query")) == NULL) ;

        while ((s != string) && (strncmp (s, "<b>", 3) != 0))
            --s ;
        totalItems = atoi (s+3) ;

/* Read and display this batch of items. */

        if (verbose)  fprintf (stderr, "[gurls] Next Item: %d of %d\n",
                               lastItem + 1, totalItems) ;

        for ( ; ; ) {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveAltaVista) Error reading search results from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
            if (strcmp (string, "</pre>") == 0)  break ;
            lastItem = atoi (string) ;
            if (onlyURL) {
                if (strstr (string, "<a href=") == NULL)  continue ;
                string = strchr (string, '"') + 1 ;
                *(strchr (string, '"')) = '\0' ;
            }
            printf ("%s\n", string) ;
        }

/* Skip the rest of the page and close the connection to the search engine. */

        if (verbose)  fprintf (stderr,
                               "Disconnecting: www.altavista.digital.com\n") ;

        lfnDestroy (stream) ;

    } while (lastItem < totalItems) ;	/* For each batch of results. */


    if (!onlyURL)  printf ("</PRE>\n") ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    retrieveWebCrawler ()

    Retrieve Information from the WebCrawler Search Engine.


Purpose:

    Function retrieveWebCrawler() queries the WebCrawler search engine for the
    desired information and prints a list of the matching URLs to standard
    output.


    Invocation:

        status = retrieveWebCrawler (query) ;

    where:

        <query>		- I
            is the search-engine-specific query for the desired
            information.
        <status>	- O
            returns the status of retrieving the information, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


static  int  retrieveWebCrawler (

#    if  __STDC__
        const  char  *query)
#    else
        query)

        char  *query ;
#    endif

{    /* Local variables. */
    char  command[1024], *string ;
    int  firstItem, lastItem, totalItems ;
    LfnStream  stream ;
    TcpEndpoint  connection ;




/*******************************************************************************
    Query the search engine for the desired information and extract the
    matching URLs from each page returned by the search engine.
*******************************************************************************/

    lastItem = 0 ;

    do {

/* Establish a network connection with the search engine. */

        if (verbose)
            fprintf (stderr, "[gurls] Connecting: www.webcrawler.com\n") ;

        if (tcpCall ((proxy == NULL) ? "80@192.216.46.52" : proxy,
                     0, &connection)) {
            vperror ("(retrieveWebCrawler) Error connecting to search engine.\ntcpCall: ") ;
            return (errno) ;
        }

        if (sktSetBuf (tcpName (connection), tcpFd (connection), 8192, 8192)) {
            vperror ("(retrieveWebCrawler) Error setting sizes of receive/send buffers.\nsktSetBuf: ") ;
            return (errno) ;
        }

        if (lfnCreate (connection, NULL, &stream)) {
            vperror ("(retrieveWebCrawler) Error creating LF-terminated stream for search engine.\nlfnCreate: ") ;
            return (errno) ;
        }

/* Format the query for the next batch of items and send the query to the
   search engine. */

        sprintf (command,
                 "%s/cgi-bin/WebQuery?summaries=%s;offset=%d;text=%s",
                 (proxy == NULL) ? "" : "http://192.216.46.52",
                 summariesToo ? "yes" : "no", lastItem, query) ;

        if (verbose)  fprintf (stderr, "[gurls] %s\n", command) ;

        if (lfnPutLine (stream, -1.0, "GET %s HTTP/1.0\n\n", command)) {
            vperror ("(retrieveWebCrawler) Error sending query to search engine.\nlfnWrite: ") ;
            return (errno) ;
        }

/* Read the header information returned by the search engine. */

        if (lfnGetLine (stream, -1.0, &string)) {
            vperror ("(retrieveWebCrawler) Error reading header from %s.\nlfnGetLine: ",
                     lfnName (stream)) ;
            return (errno) ;
        }

        if (verbose)  fprintf (stderr, "[gurls] %s\n", string) ;

        while (strcmp (string, "") != 0) {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveWebCrawler) Error reading header from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
        }

/* Continue reading the page from the search engine and look for the start
   of the search results. */

        do {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveWebCrawler) Error reading page from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
            if (debug)  printf ("(retrieveWebCrawler) Leading text: \"%s\"\n", string) ;
        } while (!strMatch ("Documents ", string)) ;

        if (sscanf (string, "%*[^0-9]%d-%d%*[^0-9]%d",
                    &firstItem, &lastItem, &totalItems) != 3) {
            vperror ("(retrieveWebCrawler) Error decoding: \"%s\"\nsscanf: ",
                     string) ;
            return (errno) ;
        }

        do {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveWebCrawler) Error reading page from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
            if (debug)  printf ("(retrieveWebCrawler) Leading text: \"%s\"\n", string) ;
        } while (!strMatch ("<p>", string)) ;

/* Read and display this batch of items. */

        if (verbose)  fprintf (stderr, "[gurls] Items: %d-%d of %d\n",
                               firstItem, lastItem, totalItems) ;

        for ( ; ; ) {
            if (lfnGetLine (stream, -1.0, &string)) {
                vperror ("(retrieveWebCrawler) Error reading search results from %s.\nlfnGetLine: ",
                         lfnName (stream)) ;
                return (errno) ;
            }
            if (strMatch ("<form", string) ||
                strMatch ("<center>", string))  break ;
            if (onlyURL) {
                if (!strMatch ("<a href=", string))  continue ;
                string = strchr (string, '"') + 1 ;
                *(strchr (string, '"')) = '\0' ;
            }
            printf ("%s\n", string) ;
        }

/* Skip the rest of the page and close the connection to the search engine. */

        if (verbose)  fprintf (stderr,
                               "Disconnecting: www.webcrawler.com\n") ;

        lfnDestroy (stream) ;

    } while (lastItem < totalItems) ;	/* For each batch of results. */


    return (0) ;

}
