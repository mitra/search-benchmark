/* $Id: port_util.c,v 1.8 2011/07/18 17:51:24 alex Exp $ */
/*******************************************************************************

File:

    port_util.c

    Listening Port Utilities.


Author:    Alex Measday


Purpose:

    This package implements network listening ports for network servers
    based on the IOX I/O dispatcher (see "iox_util.c").  The server must
    first create the listening port and then turn control over to the
    IOX dispatcher:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  "iox_util.h"		-- I/O event dispatcher definitions.
        #include  "port_util.h"		-- Listening port utilities.
        IoxDispatcher  dispatcher ;
        ListeningPort  port ;
        ...
        ioxCreate (&dispatcher) ;	-- Create I/O event dispatcher.
        ...
        portCreate ("<service>", dispatcher, createF, NULL, &port) ;
        ...
        ioxMonitor (dispatcher, -1.0) ;	-- Dispatcher takes over.

    When the dispatcher detects a client attempting to connect to the server,
    it calls an internal PORT_UTIL callback.  This callback, portAnswer(),
    establishes the connection with the client and then calls the client
    creation function specified in the portCreate() call.  The client
    creation function typically creates some data structure representing
    the client and registers the new connection with the IOX dispatcher
    as an input source.

    Using the PORT_UTIL package, a simple date/time server requires only
    a few lines of code:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  <string.h>		-- Standard C string functions.
        #include  <time.h>		-- Time definitions.
        #include  "port_util.h"		-- Listening port utilities.

        static  int  sendTime (TcpEndpoint connection, IoxDispatcher dispatcher,
                               void *parameter, void **client)
        {   char  *asciiTOD ;
            time_t  binaryTOD = time (NULL) ;

            asciiTOD = ctime (&binaryTOD) ;
            tcpWrite (connection, -1.0, strlen (asciiTOD), asciiTOD, NULL) ;
            tcpDestroy (connection) ;
        }

        int  main (int argc, char *argv[])
        {   IoxDispatcher  dispatcher ;
            ListeningPort  port ;

            ioxCreate (&dispatcher) ;
            portCreate (argv[1], dispatcher, sendTime, NULL, &port) ;
            ioxMonitor (dispatcher, -1.0) ;
        }

    Function main() creates the network listening port (whose service name
    or port number is specified on the command line) and relinquishes control
    to the IOX I/O dispatcher.  When a client requests a connection to the
    server, the connection is established and sendTime() is called to "create"
    the client, which, in this case, consists solely of outputting the current
    time-of-day on the data connection and then closing the connection.


Public Procedures:

    portCreate() - creates a listening port.
    portDestroy() - destroys a listening port.

Private Procedures:

    portAnswer() - answers a connection request.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */
#include  "port_util.h"			/* Listening port utilities. */


/*******************************************************************************
    Listening Port - represents a network port at which a server listens
        for connection requests from clients.
*******************************************************************************/

typedef  struct  _ListeningPort {
    char  *name ;			/* Server name. */
    ClientCreateFunc  createF ;		/* Function to create a client. */
    void  *parameter ;			/* Arbitrary parameter passed to above. */
    TcpEndpoint  endpoint ;		/* Listening socket. */
    IoxDispatcher  dispatcher ;		/* IOX dispatcher. */
    IoxCallback  sourceID ;		/* ID of registered listening port. */
}  _ListeningPort ;


int  port_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  port_util_debug


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  portAnswer (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    portCreate ()

    Create a Listening Port.


Purpose:

    Function portCreate() creates a network listening port at which a
    server listens for connection requests from clients.  The port's
    listening socket is registered with the IOX Dispatcher; when a
    connection request is received, the callback function assigned
    to the socket answers the connection request.


    Invocation:

        status = portCreate (name, dispatcher, clientCreateF, clientParameter,
                             &port) ;

    where:

        <name>			- I
            is the port's server name.
        <dispatcher>		- I
            is the IOX dispatcher with which the listening socket will be
            registered.
        <clientCreateF>		- I
            is a function that will be called by portAnswer() when a
            connection request is accepted and a new data connection has
            been established.  The function should be declared as follows:
                    int  createClient (TcpEndpoint connection,
                                       IoxDispatcher dispatcher,
                                       void *parameter,
                                       void **clientObject) ;
            and is responsible for creating the new client object.  The
            client object and status returned by the function are ignored
            by portAnswer().  Instead, the creation function is responsible
            for saving a reference to the client object somewhere and the
            function must arrange for the connection to be destroyed when
            it is no longer needed.
        <clientParameter>	- I
            is an arbitrary (VOID *) parameter passed to the client creation
            function.
        <port>			- O
            returns a handle for the listening port.  This handle is used
            in calls to the other PORT functions.
        <status>		- O
            returns the status of creating the listening port, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  portCreate (

#    if PROTOTYPES
        const  char  *name,
        IoxDispatcher  dispatcher,
        ClientCreateFunc  clientCreateF,
        void  *clientParameter,
        ListeningPort  *port)
#    else
        name, dispatcher, clientCreateF, clientParameter, port)

        char  *name ;
        IoxDispatcher  dispatcher ;
        ClientCreateFunc  clientCreateF ;
        void  *clientParameter ;
        ListeningPort  *port ;
#    endif

{

/* Create and initialize a listening port structure. */

    *port = (ListeningPort) malloc (sizeof (_ListeningPort)) ;
    if (*port == NULL) {
        LGE "(portCreate) Error allocating a port structure for %s.\nmalloc: ",
            name) ;
        return (errno) ;
    }

    (*port)->name = strdup (name) ;
    if ((*port)->name == NULL) {
        LGE "(portCreate) Error duplicating server name for %s.\nstrdup: ",
            name) ;
        return (errno) ;
    }

    (*port)->dispatcher = dispatcher ;
    (*port)->sourceID = NULL ;
    (*port)->createF = clientCreateF ;
    (*port)->parameter = clientParameter ;
    (*port)->endpoint = NULL ;

/* Create a listening socket on which connection requests for this server
   will be received. */

    if (tcpListen (name, -1, &(*port)->endpoint)) {
        LGE "(portCreate) Error creating listening socket for %s.\ntcpListen: ",
            name) ;
        return (errno) ;
    }

/* Register the listening socket with the IOX Dispatcher.  When a connection
   request is received, the "answer" callback is invoked to accept the request. */

    (*port)->sourceID = ioxOnIO (dispatcher, portAnswer, *port, IoxRead,
                                 tcpFd ((*port)->endpoint)) ;

    LGI "(portCreate) Created %s listening port.\n", name) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    portDestroy ()

    Destroy a Listening Port.


Purpose:

    Function portDestroy() destroys a listening port object.


    Invocation:

        status = portDestroy (port) ;

    where:

        <port>		- I
            is the listening port handle returned by portCreate().
        <status>	- O
            returns the status of destroying the listening port, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  portDestroy (

#    if PROTOTYPES
        ListeningPort  port)
#    else
        port)

        ListeningPort  port ;
#    endif

{

    if (port == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(portDestroy) NULL port handle: ") ;
        return (errno) ;
    }

    LGI "(portDestroy) Closing %s.\n", port->name) ;

/* Remove the listening port from the group of input sources monitored by
   the IOX Dispatcher. */

    if (port->sourceID != NULL)  ioxCancel (port->sourceID) ;

/* Close the listening port's socket. */

    tcpDestroy (port->endpoint) ;

/* Deallocate the listening port structure. */

    if (port->name != NULL)  free (port->name) ;
    free (port) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    portAnswer ()

    Answer a Network Connection Request.


Purpose:

    Function portAnswer() is the IOX callback assigned to a listening
    socket by portCreate().  When a network connection request is
    received on the socket, the IOX Dispatcher automatically invokes
    this callback function to answer the connection request.  After
    accepting the request, portAnswer() creates a client object for
    the new client.


    Invocation:

        status = portAnswer (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason (i.e., IoxRead) the callback is being invoked.
        <userData>	- I
            is the address of the listening port structure created for
            the network connection by portCreate().
        <status>	- O
            returns the status of answering a connection request, zero if
            there were no errors and ERRNO otherwise.  The status value
            is ignored by the IOX dispatcher.

*******************************************************************************/


static  errno_t  portAnswer (

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
    ListeningPort  port ;
    TcpEndpoint  connection ;
    void  *client ;



    port = (ListeningPort) userData ;

/* Accept the connection request from a new client. */

    if (tcpAnswer (port->endpoint, -1.0, &connection)) {
        LGE "(portAnswer) Error answering connection request for %s.\ntcpAnswer: ",
            port->name) ;
        return (errno) ;
    }

    LGI "(portAnswer) Answered connection %s, socket %d.\n",
        tcpName (connection), tcpFd (connection)) ;

/* Create a client object for the new client. */

    if (port->createF == NULL) {
        tcpDestroy (connection) ;
    } else if (port->createF (connection, ioxDispatcher (callback),
                              port->parameter, &client)) {
        LGE "(portAnswer) Error creating client object for %s: ",
            tcpName (connection)) ;
        return (errno) ;
    }

    return (0) ;

}
