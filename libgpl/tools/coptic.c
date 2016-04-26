/* $Id$ */
/*******************************************************************************

Program:

    coptic.c

    CORBA Protocol Test Interface.


Author:    Alex Measday


Purpose:

    COPTIC can be invoked as a client or server that speaks the
    CORBA Internet Inter-ORB Protocol (IIOP).


    Invocation:

        % coptic [-debug] [-listen] <server>[@<host>]

    where:

        "-debug"
            enables debug output (written to STDOUT).
        "-listen"
            puts COPTIC in server mode, i.e., COPTIC listens for a connection
            request from a client, answers it, and then communicates with the
            client.
        "<server>[@<host>]"
            specifies the name or number of the port at which the server is
            listening for connection requests and, optionally, the name of
            the host on which the server is running.  If the "-listen" option
            was specified on the command line, <server> is the port at which
            COPTIC listens for a connection request from a client; no <host>
            should be specified.  If the "-listen" option was NOT specified,
            COPTIC attempts to connect as a client to the <server> port on
            <host>; if no host is specified, the local host is assumed.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "comx_util.h"			/* CORBA marshalling utilities. */
#include  "iiop_util.h"			/* Internet Inter-ORB Protocol streams. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "aperror.h"			/* APERROR() definitions. */


extern  void  dumpRequest (
    IiopStream  stream,
    IiopHeader  *header,
    const  octet  *body
) ;

/*******************************************************************************
    COPTIC's Main Program.
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
    char  *argument, *serverName ;
    IiopHeader  header ;
    IiopStream  stream ;
    int  errflg, isServer, option ;
    octet  *body ;
    OptContext  scan ;
    TcpEndpoint  connection, listener ;

    const  char  *optionList[] = {	/* Command line options. */
        "{debug}", "{listen}", NULL
    } ;





#if HAVE_SIGNAL && defined(SIGPIPE)
    signal (SIGPIPE, SIG_IGN) ;
#endif
    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    isServer = 0 ;  serverName = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            iiop_util_debug = 1 ;  tcp_util_debug = 1 ;
            break ;
        case 2:			/* "-listen" */
            isServer = 1 ;
            break ;
        case NONOPT:
            if (serverName == NULL)
                serverName = argument ;
            else
                errflg++ ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg || (serverName == NULL)) {
        fprintf (stderr, "Usage:  coptic [-debug] [-listen] <server>[@<host>]\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    Establish a connection with the host/server.
*******************************************************************************/

    if (isServer) {

        fprintf (stderr, "... \"%s\" waiting for connection request ...\n",
                 serverName) ;

        if (tcpListen (serverName, -1, &listener)) {
            aperror ("[%s] Error listening for connection requests.\ntcpListen: ",
                     argv[0]) ;
            exit (errno) ;
        }

        if (tcpAnswer (listener, -1.0, &connection)) {
            aperror ("[%s] Error answering connection request.\ntcpAnswer: ",
                     argv[0]) ;
            exit (errno) ;
        }

        tcpDestroy (listener) ;

    } else {

        if (tcpCall (serverName, 0, &connection)) {
            aperror ("[%s] Error establishing connection.\ntcpCall: ",
                     argv[0]) ;
            exit (errno) ;
        }

    }


/* Create an IIOP stream on the TCP/IP connection. */

    iiopCreate (connection, &stream) ;


    printf ("==>/ %s /==>    %s\n", argv[0], serverName) ;


/*******************************************************************************
    Communicate over the established connection with the network peer.
*******************************************************************************/

    for ( ; ; ) {

        if (iiopRead (stream, -1.0, &header, &body))  break ;

        printf ("[%s] %s message (%lu bytes)\n",
                argv[0],
                iiopToName (MsgTypeLookup, (int) header.message_type),
                header.message_size) ;

        switch (header.message_type) {
        case Request:
            dumpRequest (stream, &header, body) ;
            break ;
        case Reply:
        case CancelRequest:
        case LocateRequest:
        case LocateReply:
        case CloseConnection:
        case MessageError:
        case Fragment:
        default:
            break ;
        }

    }


/* Close the connection. */

    iiopDestroy (stream) ;

    exit (0) ;

}

/*******************************************************************************
    dumpRequest() - dumps the contents of a Request message.
*******************************************************************************/


void  dumpRequest (

    IiopStream  stream,
    IiopHeader  *header,
    const  octet  *body)

{    /* Local variables. */
    char  *argument ;
    ComxChannel  channel ;
    int  i ;
    octet  boolean ;
    ServiceContext  *list ;
    unsigned  long  request_id ;




    if (body == NULL)  return ;

    if (comxCreate (header->GIOP_version, header->flags & ENDIAN_MASK, 12,
                    (octet *) body, header->message_size, &channel)) {
        aperror ("(dumpRequest) Error creating marshalling channel.\ncomxCreate: ") ;
        return ;
    }


/*******************************************************************************
    Decode and dump a GIOP 1.0/1.1 Request Header.
*******************************************************************************/

    if (header->GIOP_version.minor < 2) {

        RequestHeader_1_1  rqhdr ;

        comxRequestHeader_1_1 (channel, &rqhdr) ;
        request_id = rqhdr.request_id ;

        printf ("Request Header (GIOP %d.%d)  ID: 0x%08lX  Response: %s  Operation: %s\n",
                header->GIOP_version.major, header->GIOP_version.minor,
                rqhdr.request_id, rqhdr.response_expected ? "YES" : "NO",
                rqhdr.operation) ;

        list = rqhdr.service_context.elements ;
        for (i = 0 ;  i < rqhdr.service_context.count ;  i++) {
            printf ("    service_context[%d]  ID: 0x%08lX  %lu bytes of data\n",
                    i, list[i].context_id, list[i].context_data.count) ;
        }

        printf ("    requesting_principal  %lu bytes of data\n",
                rqhdr.requesting_principal.count) ;

        if (strcmp (rqhdr.operation, "_is_a") == 0)
            boolean = 1 ;
        else if (strcmp (rqhdr.operation, "_non_existent") == 0)
            boolean = 0 ;
        else
            boolean = 1 ;

    }


/*******************************************************************************
    Decode and dump a GIOP 1.2 Request Header.
*******************************************************************************/

    else {

        RequestHeader  rqhdr ;

        comxRequestHeader (channel, &rqhdr) ;
        request_id = rqhdr.request_id ;

        printf ("Request Header (GIOP %d.%d)  ID: 0x%08lX  Response: %s  Operation: %s\n",
                header->GIOP_version.major, header->GIOP_version.minor,
                rqhdr.request_id,
                iiopToName (SyncScopeLookup, rqhdr.response_flags),
                rqhdr.operation) ;

        printf ("    target  Disposition: %s\n",
                iiopToName (AddressingDispositionLookup,
                            rqhdr.target.disposition)) ;

        list = rqhdr.service_context.elements ;
        for (i = 0 ;  i < rqhdr.service_context.count ;  i++) {
            printf ("    service_context[%d]  ID: 0x%08lX  %lu bytes of data\n",
                    i, list[i].context_id, list[i].context_data.count) ;
        }

        if (strcmp (rqhdr.operation, "_is_a") == 0)
            boolean = 1 ;
        else if (strcmp (rqhdr.operation, "_non_existent") == 0)
            boolean = 0 ;
        else
            boolean = 1 ;

    }


/*******************************************************************************
    Decode and dump the request body.
*******************************************************************************/

/* Assume a single string for right now. */

    if (comxSkip (channel, 0) < header->message_size) {
        comxString (channel, &argument) ;
        printf ("    arguments  \"%s\"\n", argument) ;
        comxSetOp (channel, MxERASE) ;
        comxString (channel, &argument) ;
        printf ("argument = %p\n", argument) ;
    }

    comxDestroy (channel) ;


/*******************************************************************************
    Send a reply.
*******************************************************************************/

    if (comxCreate (header->GIOP_version, 0, 12, NULL, 1024, &channel)) {
        aperror ("(dumpRequest) Error creating marshalling channel.\ncomxCreate: ") ;
        return ;
    }

    comxSetOp (channel, MxENCODE) ;

    if (header->GIOP_version.minor < 2) {
        ReplyHeader_1_1  rphdr ;
        rphdr.service_context.count = 0 ;
        rphdr.service_context.elements = NULL ;
        rphdr.request_id = request_id ;
        rphdr.reply_status = NO_EXCEPTION ;
        if (comxReplyHeader_1_1 (channel, &rphdr)) {
            aperror ("(dumpRequest) Error encoding reply header.\ncomxReplyHeader_1_1: ") ;
        }
    } else {
        ReplyHeader  rphdr ;
        rphdr.service_context.count = 0 ;
        rphdr.service_context.elements = NULL ;
        rphdr.request_id = request_id ;
        rphdr.reply_status = NO_EXCEPTION ;
        if (comxReplyHeader (channel, &rphdr)) {
            aperror ("(dumpRequest) Error encoding reply header.\ncomxReplyHeader: ") ;
        }
    }

    if (comxBoolean (channel, &boolean)) {
        aperror ("(dumpRequest) Error encoding reply body.\ncomxBoolean: ") ;
    }

    header->GIOP_version.minor = 2 ;
    header->message_type = Reply ;
    header->message_size = comxSkip (channel, 0) ;

    if (iiopWrite (stream, -1.0, header, comxBuffer (channel))) {
        aperror ("(dumpRequest) Error sending reply to %s.\niiopWrite: ",
                 iiopName (stream)) ;
    }

    comxDestroy (channel) ;

    return ;

}
