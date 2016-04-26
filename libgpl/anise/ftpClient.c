/* $Id: ftpClient.c,v 1.6 2012/05/06 22:28:55 alex Exp $ */
/*******************************************************************************

File:

    ftpClient.c

    FTP Client Utilities.


Author:    Alex Measday


Purpose:

    This package implements FTP client objects.


Public Procedures:

    ftpClientCreate() - creates an FTP client.
    ftpClientDestroy() - destroys an FTP client.

Private Procedures:

    ftpClientInputCB() - reads a message from a client.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "nft_util.h"			/* FTP utilties. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */
#include  "anise.h"			/* ANISE definitions. */
#include  "ftpClient.h"			/* FTP client utilities. */


/*******************************************************************************
    Client Object - represents an FTP client.  Each client has its own
        FTP session.
*******************************************************************************/

typedef  struct  _FtpClient {
    NftSession  session ;		/* FTP session for this client. */
    IoxCallback  inputCB ;		/* Client input callback. */
}  _FtpClient ;


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  ftpClientInputCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    ftpClientCreate ()

    Create an FTP Client Object.


Purpose:

    Function ftpClientCreate() creates an FTP client object.  A new FTP
    session is created for the client and the client's control connection
    is registered with the IOX dispatcher as an input source.  When a
    command is received on the client's connection, the IOX dispatcher
    calls ftpClientInputCB() to read and process the command.


    Invocation:

        status = ftpClientCreate (connection, dispatcher, parameter, &client) ;

    where:

        <connection>	- I
            is the client's control connection.
        <dispatcher>	- I
            is the I/O event dispatcher with which the network connection
            will be registered.
        <parameter>	- I
            is an arbitrary (VOID *) parameter.
        <client>	- O
            returns a handle for the client.  This handle is used in calls
            to the other FTPCLIENT functions.
        <status>	- O
            returns the status of creating the client object, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  ftpClientCreate (

#    if PROTOTYPES
        TcpEndpoint  connection,
        IoxDispatcher  dispatcher,
        void  *parameter,
        FtpClient  *client)
#    else
        connection, dispatcher, parameter, client)

        TcpEndpoint  connection ;
        IoxDispatcher  dispatcher ;
        void  *parameter ;
        FtpClient  *client ;
#    endif

{    /* Local variables. */
    NftSession  session ;




/* Create an FTP session for the client. */

    if (nftCreate (connection, NULL, NULL, NULL, &session)) {
        LGE "(ftpClientCreate) Error creating FTP session for %s.\nnftCreate: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;  tcpDestroy (connection) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Create and initialize a client object. */

    *client = (FtpClient) malloc (sizeof (_FtpClient)) ;
    if (*client == NULL) {
        LGE "(ftpClientCreate) Error allocating a client object for %s.\nmalloc: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;  nftDestroy (session) ;  POP_ERRNO ;
        return (errno) ;
    }

    (*client)->session = session ;
    (*client)->inputCB = NULL ;

/* Register the client's connection with the IOX dispatcher.  When input is
   detected on the connection, the IOX dispatcher automatically invokes
   ftpClientInputCB() to read and process the input. */

    (*client)->inputCB = ioxOnIO (dispatcher, ftpClientInputCB, *client,
                                  IoxRead, tcpFd (connection)) ;

    nftPutLine ((*client)->session, "220 ANISE (%s) is looking good.\r\n",
                tcpName (connection)) ;

    LGI "(ftpClientCreate) Created %s client.\n", tcpName (connection)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    ftpClientDestroy ()

    Destroy an FTP Client Object.


Purpose:

    Function ftpClientDestroy() destroys an FTP client object.


    Invocation:

        status = ftpClientDestroy (client) ;

    where:

        <client>	- I
            is the client handle returned by ftpClientCreate().
        <status>	- O
            returns the status of destroying the client, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  ftpClientDestroy (

#    if PROTOTYPES
        FtpClient  client)
#    else
        client)

        FtpClient  client ;
#    endif

{

    if (client == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ftpClientDestroy) NULL client handle: ") ;
        return (errno) ;
    }

    LGI "(ftpClientDestroy) Destroying %s client.\n",
        nftName (client->session, 0)) ;

/* Remove the client's connection from the group of input sources monitored
   by the IOX dispatcher. */

    if (client->inputCB != NULL) {
        ioxCancel (client->inputCB) ;
        client->inputCB = NULL ;
    }

/* Close the client's FTP session. */

    if (client->session != NULL) {
        nftDestroy (client->session) ;
        client->session = NULL ;
    }

/* Deallocate the client object. */

    free (client) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    ftpClientInputCB ()

    Read a Message from a Client.


Purpose:

    Function ftpClientInputCB() is the IOX callback assigned to a client's
    connection by ftpClientCreate().  When a message (i.e., an FTP command)
    is received on the connection, the IOX dispatcher automatically invokes
    this callback function to read and execute the command.


    Invocation:

        status = ftpClientInputCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the address of the client object created by ftpClientCreate().
        <status>	- O
            returns the status of reading/processing the input, zero if
            there were no errors and ERRNO otherwise.  The status value
            is ignored by the IOX dispatcher, but it may be useful if
            the application calls ftpClientInputCB() directly.

*******************************************************************************/


static  errno_t  ftpClientInputCB (

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
    char  *command ;
    FtpClient  client = (FtpClient) userData ;




/* While input is pending, read and evaluate each command. */

    while (nftIsReadable (client->session, 0)) {

        if (nftGetLine (client->session, &command)) {
            LGE "(ftpClientInputCB) Error reading command from %s.\nnftGetLine: ",
                nftName (client->session, 0)) ;
            break ;
        }

        LGI "(ftpClientInputCB) From %s: %s\n",
            nftName (client->session, 0), command) ;

        if (nftEvaluate (client->session, command)) {
            LGE "(ftpClientInputCB) Error evaluating command from %s.\nnftEvaluate: ",
                nftName (client->session, 0)) ;
            break ;
        }

    }


/* If the connection has gone down, then destroy the client. */

    if (!nftIsUp (client->session, 0)) {
        LGE "(ftpClientInputCB) Broken connection to %s.\nnftIsUp: ",
            nftName (client->session, 0)) ;
        ftpClientDestroy (client) ;
    }


    return (0) ;

}
