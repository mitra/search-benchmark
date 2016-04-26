/* $Id: wwwClient.c,v 1.6 2012/05/06 22:28:55 alex Exp alex $ */
/*******************************************************************************

File:

    wwwClient.c

    WWW Client Utilities.


Author:    Alex Measday


Purpose:

    This package implements WWW client objects.


Public Procedures:

    wwwClientCreate() - creates a WWW client.
    wwwClientDestroy() - destroys a WWW client.
    wwwClientIsReadable() - polls a WWW client for input.
    wwwClientIsUp() - checks if a WWW client is still connected.

Private Procedures:

    wwwClientInputCB() - reads a message from a WWW client.
    wwwClientTimeoutCB() - closes a WWW client connection.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */
#include  "anise.h"			/* ANISE definitions. */
#include  "http_util.h"			/* HTTP utilities. */
#include  "wwwClient.h"			/* WWW client utilities. */


/*******************************************************************************
    Client Object - represents a WWW client.  Each client has its own
        network connection.
*******************************************************************************/

typedef  struct  _WwwClient {
    LfnStream  stream ;			/* Network connection for this client. */
    LogFile  logFile ;			/* Log of HTTP transactions. */
    IoxCallback  inputCB ;		/* Client input callback. */
    IoxCallback  timeoutCB ;		/* Client timeout callback. */
}  _WwwClient ;


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  wwwClientInputCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

static  errno_t  wwwClientTimeoutCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    wwwClientCreate ()

    Create a WWW Client Object.


Purpose:

    Function wwwClientCreate() creates a WWW client object.  The client's
    network connection is registered with the IOX dispatcher as an input
    source.  When a message is received on the client's connection, the
    IOX dispatcher invokes wwwClientInputCB() to read the message;
    wwwClientInputCB() then passes the message to executeHTTP() for
    evaluation.


    Invocation:

        status = wwwClientCreate (connection, dispatcher, logFile, &client) ;

    where:

        <connection>	- I
            is the client's network connection.
        <dispatcher>	- I
            is the I/O event dispatcher with which the network connection
            will be registered.
        <logFile>	- I
            is the LogFile handle for the file to which HTTP transactions are
            to be logged; NULL can be specified if transactions are not to be
            logged.
        <client>	- O
            returns a handle for the client.  This handle is used in calls
            to the other WWWCLIENT functions.
        <status>	- O
            returns the status of creating the client object, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  wwwClientCreate (

#    if PROTOTYPES
        TcpEndpoint  connection,
        IoxDispatcher  dispatcher,
        LogFile  logFile,
        WwwClient  *client)
#    else
        connection, context, logFile, client)

        TcpEndpoint  connection ;
        IoxDispatcher  dispatcher ;
        LogFile  logFile ;
        WwwClient  *client ;
#    endif

{    /* Local variables. */
    LfnStream  stream ;




/* Increase the sizes of the network connection's I/O buffers. */

    if (sktSetBuf (tcpFd (connection), 32*1024, 32*1024)) {
        LGE "(wwwClientCreate) Error setting %s's system buffer sizes.\nsktSetBuf: ",
            tcpName (connection)) ;
    }

/* Create a line feed-terminated stream for the client. */

    if (lfnCreate (connection, NULL, &stream)) {
        LGE "(wwwClientCreate) Error creating LF-terminated stream for client %s.\nlfnCreate: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;  tcpDestroy (connection) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Create and initialize a client object. */

    *client = (WwwClient) malloc (sizeof (_WwwClient)) ;
    if (*client == NULL) {
        LGE "(wwwClientCreate) Error allocating a client object for %s.\nmalloc: ",
            lfnName (stream)) ;
        PUSH_ERRNO ;  lfnDestroy (stream) ;  POP_ERRNO ;
        return (errno) ;
    }

    (*client)->stream = stream ;
    (*client)->logFile = logFile ;
    (*client)->inputCB = NULL ;
    (*client)->timeoutCB = NULL ;

/* Register the I/O stream with the IOX dispatcher.  When an input message
   is received, the IOX dispatcher automatically invokes wwwClientInputCB()
   to read and process the message. */

    if (stream != NULL) {
        (*client)->inputCB = ioxOnIO (dispatcher, wwwClientInputCB, *client,
                                      IoxRead, lfnFd (stream)) ;
    }

    LGI "(wwwClientCreate) Created %s client.\n", lfnName (stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    wwwClientDestroy ()

    Destroy a WWW Client Object.


Purpose:

    Function wwwClientDestroy() destroys a WWW client object.


    Invocation:

        status = wwwClientDestroy (client) ;

    where:

        <client>	- I
            is the client handle returned by wwwClientCreate().
        <status>	- O
            returns the status of destroying the client, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  wwwClientDestroy (

#    if PROTOTYPES
        WwwClient  client)
#    else
        client)

        WwwClient  client ;
#    endif

{

    if (client == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(wwwClientDestroy) NULL client handle: ") ;
        return (errno) ;
    }

    LGI "(wwwClientDestroy) Destroying %s client.\n",
        (client->stream == NULL) ? "null" : lfnName (client->stream)) ;

/* Clear the client's timeout callback, if set. */

    if (client->timeoutCB != NULL) {
        ioxCancel (client->timeoutCB) ;
        client->timeoutCB = NULL ;
    }

/* Remove the client's I/O stream from the group of input sources monitored
   by the IOX dispatcher. */

    if (client->inputCB != NULL) {
        ioxCancel (client->inputCB) ;
        client->inputCB = NULL ;
    }

/* Close the client's I/O stream. */

    if (client->stream != NULL) {
        lfnDestroy (client->stream) ;
        client->stream = NULL ;
    }

/* Deallocate the client object. */

    free (client) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    wwwClientIsReadable ()

    Poll a WWW Client for Input.


Purpose:

    Function wwwClientIsReadable() checks to see if any messages from
    a WWW client are waiting to be read.


    Invocation:

        isReadable = wwwClientIsReadable (client) ;

    where:

        <client>	- I
            is the client handle returned by wwwClientCreate().
        <isReadable>	- O
            returns true (a non-zero number) if messages are waiting to be
            read from the client and false (zero) otherwise.

*******************************************************************************/


bool  wwwClientIsReadable (

#    if PROTOTYPES
        WwwClient  client)
#    else
        client)

        WwwClient  client ;
#    endif

{

    if ((client == NULL) || (client->stream == NULL))
        return (false) ;
    else
        return (lfnIsReadable (client->stream)) ;

}

/*!*****************************************************************************

Procedure:

    wwwClientIsUp ()

    Check if a WWW Client is Up.


Purpose:

    Function wwwClientIsUp() checks to see if a WWW client is still up.


    Invocation:

        isUp = wwwClientIsUp (client) ;

    where:

        <client>	- I
            is the client handle returned by wwwClientCreate().
        <isUp>		- O
            returns true (a non-zero number) if the client is still up
            and false (zero) otherwise.

*******************************************************************************/


bool  wwwClientIsUp (

#    if PROTOTYPES
        WwwClient  client)
#    else
        client)

        WwwClient  client ;
#    endif

{

    if ((client == NULL) || (client->stream == NULL))
        return (false) ;
    else
        return (lfnIsUp (client->stream)) ;

}

/*!*****************************************************************************

Procedure:

    wwwClientInputCB ()

    Read a Message from a WWW Client.


Purpose:

    Function wwwClientInputCB() is the IOX callback assigned to a client's
    I/O stream by wwwClientCreate().  When a message (i.e., an HTTP command)
    is received on the stream, the IOX dispatcher automatically invokes this
    callback function to read and execute the command.


    Invocation:

        status = wwwClientInputCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the address of the client object created by wwwClientCreate().
        <status>	- O
            returns the status of reading/processing the input, zero if
            there were no errors and ERRNO otherwise.  The status value
            is ignored by the IOX dispatcher, but it may be useful if
            the application calls wwwClientInputCB() directly.

*******************************************************************************/


static  errno_t  wwwClientInputCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        callback, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    bool  keepAlive = false ;
    char  *command[64], *string ;
    WwwClient  client ;
    int  numLines = 0 ;
    ResponseInfo  response ;




    client = (WwwClient) userData ;

    if (client->stream == NULL) {
        SET_ERRNO (EPIPE) ;
        return (errno) ;
    }

/* Clear the client's pending I/O timeout, if any. */

    if (client->timeoutCB != NULL) {
        ioxCancel (client->timeoutCB) ;
        client->timeoutCB = NULL ;
    }


/*******************************************************************************
    While lines of input are available for reading, input and assemble the
    HTTP command.
*******************************************************************************/

    while (lfnIsReadable (client->stream)) {


/* Read input until a complete HTTP command has been assembled. */

        numLines = 0 ;

        for ( ; ; ) {

            if (lfnGetLine (client->stream, -1.0, &string)) {
                LGE "(wwwClientInputCB) Error reading line from %s.\nlfnGetLine: ",
                    lfnName (client->stream)) ;
                break ;
            }
            LGI "(wwwClientInputCB) From %s: %s\n",
                lfnName (client->stream), string) ;

/* Accumulate lines of input until a blank line (indicating the end of the
   HTTP header) is received. */

            if (strlen (string) == 0) {
                break ;
            } else {
                httpConvert (string) ;		/* Convert escape sequences. */
                command[numLines++] = strdup (string) ;
            }

        }     /* Until the HTTP header is complete */


/* Check if an error occurred before a complete command was assembled. */

        if (numLines == 0)  break ;


/* Evaluate the complete command. */

        LGI "(wwwClientInputCB) Evaluating: %s\n", command[0]) ;
        if (httpEvaluate (client->stream, numLines, command, NULL,
                          &keepAlive, &response)) {
            LGE "(wwwClientInputCB) Error executing input command from %s: %s\nhttpEvaluate: ",
                lfnName (client->stream), command[0]) ;
        }
        if (client->logFile != NULL)
            httpLog (client->logFile, client->stream, numLines, command,
                     &response) ;
        while (numLines > 0)
            free (command[--numLines]) ;


    }


/* Free any accumulated command lines. */

    while (numLines > 0)
        free (command[--numLines]) ;


/* If the client signalled "keep-alive", then keep the network connection
   open for future requests.  Otherwise, destroy the client. */

    if (keepAlive)
        client->timeoutCB = ioxAfter (ioxDispatcher (callback),
                                      wwwClientTimeoutCB, client, 60.0) ;
    else
        wwwClientDestroy (client) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    wwwClientTimeoutCB ()

    Close a Keep-Alive Client.


Purpose:

    Function wwwClientTimeoutCB() is an IOX timer callback created after the
    program responds to a keep-alive client's HTTP request.  If new input is
    received before the timer expires, the timer is cancelled and the incoming
    request is processed.  If no new input is received before the timer expires,
    the timeout callback closes the client connection.


    Invocation:

        status = wwwClientTimeoutCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxAfter().
        <reason>	- I
            is the reason, IoxFire, the callback is being invoked.
        <userData>	- I
            is the address of the client object created by wwwClientCreate().
        <status>	- O
            returns the status of destroying the client connection, zero if
            there were no errors and ERRNO otherwise.  The status value is
            ignored by the IOX dispatcher, but it may be useful if the
            application calls wwwClientTimeoutCB() directly.

*******************************************************************************/


static  errno_t  wwwClientTimeoutCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        callback, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    WwwClient  client ;



    client = (WwwClient) userData ;

    client->timeoutCB = NULL ;

    return (wwwClientDestroy (client)) ;

}
