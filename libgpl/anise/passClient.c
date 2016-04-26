/* $Id: passClient.c,v 1.5 2012/05/06 22:30:32 alex Exp $ */
/*******************************************************************************

File:

    passClient.c

    Pass-Through Client Utilities.


Author:    Alex Measday


Purpose:

    This package implements pass-through client objects.  Pass-through
    objects function as intermediaries between network clients and
    target servers:

            Client  <----->  Pass-Through  <----->  Target
                               Object


Public Procedures:

    passClientCreate() - creates a pass-through client.
    passClientDestroy() - destroys a pass-through client.

Private Procedures:

    passClientInputCB() - reads input from a pass-through client or target.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "anise.h"			/* ANISE definitions. */
#include  "passClient.h"		/* Pass-through client utilities. */


#define  MAXBUF  8192


/*******************************************************************************
    Client Object - represents a pass-through client.
*******************************************************************************/

typedef  struct  _PassClient {
    TcpEndpoint  client ;		/* Network connection to client. */
    IoxCallback  clientCB ;		/* Client input callback. */
    TcpEndpoint  target ;		/* Network connection to target. */
    IoxCallback  targetCB ;		/* Target input callback. */
}  _PassClient ;


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  passClientInputCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    passClientCreate ()

    Create a Pass-Through Client Object.


Purpose:

    Function passClientCreate() creates a pass-through client object.  A
    network connection is established to the target server.  Then, both the
    connection to the client and the connection to the target are registered
    with the IOX dispatcher as input sources.  When input is received on a
    connection, the IOX Dispatcher invokes passClientInputCB(), which simply
    inputs the data and outputs it to the other connection.


    Invocation:

        status = passClientCreate (connection, dispatcher, targetName,
                                   &client) ;

    where:

        <connection>	- I
            is the client's network connection.
        <dispatcher>	- I
            is the I/O event dispatcher with which the network connection
            will be registered.
        <targetName>	- I
            is the target's name: "<server>[@<host>]".  The server can be
            specified as a name or as a port number.  The host, if given,
            can be specified as a name or as a dotted Internet address.
        <client>	- O
            returns a handle for the client.  This handle is used in calls
            to the other PASSCLIENT functions.
        <status>	- O
            returns the status of creating the client object, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  passClientCreate (

#    if PROTOTYPES
        TcpEndpoint  connection,
        IoxDispatcher  dispatcher,
        const  char  *targetName,
        PassClient  *client)
#    else
        connection, context, targetName, client)

        TcpEndpoint  connection ;
        IoxDispatcher  dispatcher ;
        char  *targetName ;
        PassClient  *client ;
#    endif

{    /* Local variables. */
    TcpEndpoint  target ;




/* Establish a network connection with the target server. */

    if (tcpCall (targetName, 0, &target)) {
        LGE "(passClientCreate) Error connecting to %s.\ntcpCall: ",
            targetName) ;
        PUSH_ERRNO ;  tcpDestroy (connection) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Increase the sizes of the network connections' I/O buffers. */

    if (sktSetBuf (tcpFd (connection), 32*1024, 32*1024)) {
        LGE "(passClientCreate) Error setting %s's system buffer sizes.\nsktSetBuf: ",
            tcpName (connection)) ;
    }

    if (sktSetBuf (tcpFd (target), 32*1024, 32*1024)) {
        LGE "(passClientCreate) Error setting %s's system buffer sizes.\nsktSetBuf: ",
            tcpName (target)) ;
    }

/* Create and initialize a client object. */

    *client = (PassClient) malloc (sizeof (_PassClient)) ;
    if (*client == NULL) {
        LGE "(passClientCreate) Error allocating a client object for %s.\nmalloc: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;
        tcpDestroy (connection) ;  tcpDestroy (target) ;
        POP_ERRNO ;
        return (errno) ;
    }

    (*client)->client = connection ;
    (*client)->clientCB = NULL ;
    (*client)->target = target ;
    (*client)->targetCB = NULL ;

/* Register the client and target connections with the IOX dispatcher.  When
   input is received on either connection, the callback function is invoked
   to input the data. */

    (*client)->clientCB = ioxOnIO (dispatcher, passClientInputCB, *client,
                                   IoxRead, tcpFd (connection)) ;

    (*client)->targetCB = ioxOnIO (dispatcher, passClientInputCB, *client,
                                   IoxRead, tcpFd (target)) ;

    LGI "(passClientCreate) Created %s client.\n", tcpName (connection)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    passClientDestroy ()

    Destroy a Pass-Through Client Object.


Purpose:

    Function passClientDestroy() destroys a pass-through client object.


    Invocation:

        status = passClientDestroy (client) ;

    where:

        <client>	- I
            is the client handle returned by passClientCreate().
        <status>	- O
            returns the status of destroying the client, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  passClientDestroy (

#    if PROTOTYPES
        PassClient  client)
#    else
        client)

        PassClient  client ;
#    endif

{

    if (client == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(passClientDestroy) NULL client handle: ") ;
        return (errno) ;
    }

    LGI "(passClientDestroy) Destroying %s client.\n",
        (client->client == NULL) ? "null" : tcpName (client->client)) ;

/* Remove the client and target network connections from the group of input
   sources monitored by the IOX dispatcher. */

    if (client->clientCB != NULL) {
        ioxCancel (client->clientCB) ;
        client->clientCB = NULL ;
    }

    if (client->targetCB != NULL) {
        ioxCancel (client->targetCB) ;
        client->targetCB = NULL ;
    }

/* Close the client and target network connections. */

    if (client->client != NULL) {
        tcpDestroy (client->client) ;
        client->client = NULL ;
    }

    if (client->target != NULL) {
        tcpDestroy (client->target) ;
        client->target = NULL ;
    }

/* Deallocate the client object. */

    free (client) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    passClientInputCB ()

    Read Input from a Pass-Through Client or Target.


Purpose:

    Function passClientInputCB() is the IOX callback assigned to a pass-through
    client or target network connection by passClientCreate().  When input is
    received on the connection, the IOX dispatcher automatically invokes this
    callback function to read the input and output it to the other connection.


    Invocation:

        status = passClientInputCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the address of the client object created by passClientCreate().
        <status>	- O
            returns the status of reading/processing the input, zero if
            there were no errors and ERRNO otherwise.  The status value
            is ignored by the IOX dispatcher, but it may be useful if
            the application calls passClientInputCB() directly.

*******************************************************************************/


static  errno_t  passClientInputCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        dispatcher, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    char  *buffer ;
    PassClient  client ;
    size_t  numBytes ;




    client = (PassClient) userData ;

    if ((client->client == NULL) || (client->target == NULL)) {
        SET_ERRNO (EPIPE) ;
        return (errno) ;
    }

    buffer = malloc (MAXBUF) ;
    if (buffer == NULL) {
        LGE "(passClientInputCB) Error allocating %d-byte buffer.\nmalloc: ",
            MAXBUF) ;
        return (errno) ;
    }


/*******************************************************************************
    While data from the client is waiting to be read, input the data and
    output it to the target.
*******************************************************************************/

    while ((callback == client->clientCB) && tcpIsReadable (client->client)) {

        if (tcpRead (client->client, -1.0, -MAXBUF, buffer, &numBytes)) {
            LGE "(passClientInputCB) Error reading from %s.\ntcpRead: ",
                tcpName (client->client)) ;
            break ;
        }

        if (tcpWrite (client->target, -1.0, numBytes, buffer, NULL)) {
            LGE "(passClientInputCB) Error writing to %s.\ntcpWrite: ",
                tcpName (client->target)) ;
            break ;
        }

    }


/*******************************************************************************
    While data from the target is waiting to be read, input the data and
    output it to the client.
*******************************************************************************/

    while ((callback == client->targetCB) && tcpIsReadable (client->target)) {

        if (tcpRead (client->target, -1.0, -MAXBUF, buffer, &numBytes)) {
            LGE "(passClientInputCB) Error reading from %s.\ntcpRead: ",
                tcpName (client->target)) ;
            break ;
        }

        if (tcpWrite (client->client, -1.0, numBytes, buffer, NULL)) {
            LGE "(passClientInputCB) Error writing to %s.\ntcpWrite: ",
                tcpName (client->client)) ;
            break ;
        }

    }

    free (buffer) ;


/*******************************************************************************
    If either connection has gone down, then destroy the client.
*******************************************************************************/

    if ((callback == client->clientCB) && !tcpIsUp (client->client)) {
        LGE "(passClientInputCB) Broken connection to %s.\ntcpIsUp: ",
            tcpName (client->client)) ;
        passClientDestroy (client) ;
    } else if ((callback == client->targetCB) && !tcpIsUp (client->target)) {
        LGE "(passClientInputCB) Broken connection to %s.\ntcpIsUp: ",
            tcpName (client->target)) ;
        passClientDestroy (client) ;
    }


    return (0) ;

}
