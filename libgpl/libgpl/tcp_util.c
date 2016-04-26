/* $Id: tcp_util.c,v 1.31 2011/07/18 17:39:32 alex Exp $ */
/*******************************************************************************

File:

    tcp_util.c

    TCP/IP Networking Utilities.


Author:    Alex Measday


Purpose:

    The TCP_UTIL functions allow you to easily establish and communicate
    over TCP/IP network connections between client and server processes,
    possibly residing on different hosts.  The TCP_UTIL functions follow
    a telephone-like model of networking: clients "call" servers and
    servers "answer" clients.  Once a network connection is established
    between a client and a server, the two can "talk" to each other by
    reading from and writing to the connection:

                  Client <----------------------> Server

    tcpListen(), tcpAnswer(), and tcpCall() are used to establish a
    communications link between a client and a server.  The server process
    calls tcpListen() to create a network endpoint at which the server
    listens for connection requests from clients.  When one is received,
    the server process calls tcpAnswer() to accept the client connection.
    A server may receive and accept connection requests from multiple
    clients; the operating system automatically creates a new connection
    (data endpoint) for each client.  The server can then multiplex the
    servicing of the clients or, if the operating system supports it,
    fork separate subprocesses to service each client.

    Client processes call tcpCall() to submit connection requests to
    the server process (which may be running on another host).  Note
    that a process can be both a client and a server with respect to
    other processes.

    tcpRead() and tcpWrite() are used to send and receive data over a
    network connection.  Because there is no concept of record boundaries
    in a TCP/IP network stream, communicating processes must follow their
    own protocol in order to determine how long a message is.  In the
    example presented below, the protocol is very simple: every message
    is exactly 64 bytes long, even if the text of the message doesn't
    fill the transmitted "record".  More sophisticated protocols (e.g.,
    the XDR record marking standard) are available.

    The following is a very simple server process that listens for and
    answers a client "call".  It then reads and displays 64-byte messages
    sent by the client.  If the client "hangs up", the server loops back
    and waits for another client.

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.

        main (int argc, char *argv[])
        {
            char  buffer[128] ;
            TcpEndpoint client, server ;

            tcpListen ("<name>", -1, &server) ;	-- Create listening endpoint.

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                for ( ; ; ) {			-- Service connected client.
                    if (tcpRead (client, -1.0, 64, buffer, NULL))  break ;
                    printf ("Message from client: %s\n", buffer) ;
                }
                tcpDestroy (client) ;		-- Lost client.
            }

        }

    The following client process "calls" the server and, once the connection
    is established, sends it 16 messages:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.

        main (int argc, char *argv[])
        {
            char  buffer[128] ;
            int  i ;
            TcpEndpoint  server ;

            tcpCall ("<name>", 0, &server) ;	-- Call server.

            for (i = 0 ;  i < 16 ;  i++) {	-- Send messages.
                sprintf (buffer, "Hello for the %dth time!", i) ;
                tcpWrite (server, -1.0, 64, buffer, NULL) ;
            }

            tcpDestroy (server) ;		-- Hang up.

        }


Notes:

    These functions are reentrant under VxWorks (except for the global
    debug flag).


Ancient History (C to C++ and back again):

    This package borrowed 95% of its code and comments from a C++ version
    of my NET_UTIL package, so I hope the comments are all up to date.

    I'm not crazy about the term "endpoint", but I wanted something other
    than "socket" that was common to TCP listening sockets and TCP and
    UDP data sockets.  I tried "port", but TCP server sockets all have
    their listening socket's port number.  I've seen "endpoint" used in
    a couple of networking books and in some articles, so there it is.


Public Procedures (for listening endpoints):

    tcpAnswer() - answers a client connection request.
    tcpListen() - creates a listening endpoint.
    tcpRequestPending() - checks if a client is trying to connect.

Public Procedures (for data endpoints, * defined as macros):

    tcpCall() - establishes a client-side connection with a server.
    tcpComplete() - completes a no-wait call.
    tcpIsReadable() - checks if data is waiting to be read on a connection.
    tcpIsUp() - checks if a connection is up.
    tcpIsWriteable() - checks if data can be written to a connection.
    tcpRead() - reads data from a connection.
  * tcpSetBuf() - changes the sizes of a connection's receive and send buffers.
    tcpWrite() - writes data to a connection.

Public Procedures (general):

    tcpDestroy() - closes an endpoint.
    tcpFd() - returns the file descriptor for an endpoint's socket.
    tcpName() - returns the name of an endpoint.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */

#if defined(VMS)
#    include  <netdb.h>			/* Network database definitions. */
#    include  <socket.h>		/* Socket-related definitions. */
#    ifndef UCX$C_TCP
#        include  <ucx$inetdef.h>	/* VMS/Ultrix Connection definitions. */
#    endif
#    include  "fd.h"			/* File descriptor set definitions. */
#    include  "ioctl.h"			/* UCX ioctl() implementation. */
#elif defined(VXWORKS)
#    include  <ioLib.h>			/* I/O library definitions. */
#    include  <selectLib.h>		/* SELECT(2) definitions. */
#    include  <socket.h>		/* Socket-related definitions. */
#    include  <sockLib.h>		/* Socket library definitions. */
#elif !defined(_WIN32) && !defined(__palmos__)
#    include  <netdb.h>			/* Network database definitions. */
#    include  <sys/socket.h>		/* Socket-related definitions. */
#    include  <sys/types.h>		/* System type definitions. */
#endif

#include  "meo_util.h"			/* Memory operations. */
#include  "net_util.h"			/* Networking utilities. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */

#ifndef MAXHOSTNAMELEN
#    define  MAXHOSTNAMELEN  64
#endif
					/* Delete an endpoint without
					   overwriting the value of ERRNO. */
#define  CLEAN_UP(endpoint)	\
        PUSH_ERRNO ;		\
        tcpDestroy (endpoint) ;	\
        endpoint = NULL ;	\
	POP_ERRNO ;


/*******************************************************************************
    TCP Endpoint - contains information about a server's listening endpoint
        or about a client's or server's data endpoint.
*******************************************************************************/

typedef  enum  TcpEndpointType {
    TcpNone, TcpListeningPoint, TcpDataPoint
}  TcpEndpointType ;

typedef  struct  _TcpEndpoint {
    char  *name ;			/* "<port>[@<host>]" */
    TcpEndpointType  type ;		/* Listening or data endpoint? */
    IoFd  fd ;				/* Listening or data socket. */
}  _TcpEndpoint ;


int  tcp_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  tcp_util_debug

/*!*****************************************************************************

Procedure:

    tcpAnswer ()

    Answer a Client's Connection Request.


Purpose:

    The tcpAnswer() function waits for and answers a client's request for a
    network connection.  When a request is accepted, the system automatically
    creates a new socket (the "data" socket) through which the server can talk
    to the client.


    Invocation:

        status = tcpAnswer (listeningPoint, timeout, &dataPoint) ;

    where

        <listeningPoint>	- I
            is the listening endpoint handle returned by tcpListen().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for a connection to be established.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) causes an immediate return if no
            connection request is pending.
        <dataPoint>		- O
            returns a handle for the data endpoint created by the acceptance
            of a connection request from another net location (i.e., a client).
        <status>		- O
            returns the status of answering a network connection request:
            zero if there were no errors, EWOULDBLOCK if the timeout interval
            expired before a connection was established, and ERRNO otherwise.

*******************************************************************************/


errno_t  tcpAnswer (

#    if PROTOTYPES
        TcpEndpoint  listeningPoint,
        double  timeout,
        TcpEndpoint  *dataPoint)
#    else
        listeningPoint, timeout, dataPoint)

        TcpEndpoint  listeningPoint ;
        double  timeout ;
        TcpEndpoint  *dataPoint ;
#    endif

{    /* Local variables. */
    char  *hostName, serverName[PATH_MAX+1] ;
    fd_set  readMask ;
    int  numActive ;
    IoctlArg  optval ;
    struct  sockaddr  clientAddress ;
    socklen_t  length ;
    struct  timeval  deltaTime, expirationTime ;




/*******************************************************************************
    Wait for the next connection request from a client.
*******************************************************************************/

/* If a timeout interval was specified, then compute the expiration time
   of the interval as the current time plus the interval. */

    if (timeout >= 0.0)
        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;

/* Wait for a connection request from a client. */

    for ( ; ; ) {
        if (timeout >= 0.0)
            deltaTime = tvSubtract (expirationTime, tvTOD ()) ;
        FD_ZERO (&readMask) ;  FD_SET (listeningPoint->fd, &readMask) ;
        numActive = select (listeningPoint->fd+1, &readMask, NULL, NULL,
                            (timeout < 0.0) ? NULL : &deltaTime) ;
        if (numActive >= 0)  break ;
        SET_ERRNO (GET_NETERRNO ()) ;
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(tcpAnswer) Error waiting for connection request on %s.\nselect: ",
            listeningPoint->name) ;
        return (errno) ;
    }

    if (numActive == 0) {
        SET_ERRNO (EWOULDBLOCK) ;
        LGE "(tcpAnswer) Timeout while waiting for connection request on %s.\n",
            listeningPoint->name) ;
        return (errno) ;
    }


/*******************************************************************************
    Answer the connection request.
*******************************************************************************/

/* Create an endpoint structure for the pending connection request. */

    *dataPoint = (TcpEndpoint) malloc (sizeof (_TcpEndpoint)) ;
    if (*dataPoint == NULL) {
        LGE "(tcpAnswer) Error allocating endpoint structure for %s.\nmalloc: ",
            listeningPoint->name) ;
        return (errno) ;
    }

    (*dataPoint)->name = NULL ;
    (*dataPoint)->type = TcpDataPoint ;
    (*dataPoint)->fd = INVALID_SOCKET ;

/* Accept the connection request. */

    do {				/* Retry interrupted system calls. */
        length = sizeof clientAddress ;
        (*dataPoint)->fd = accept (listeningPoint->fd,
                                   &clientAddress, &length) ;
        SET_ERRNO (GET_NETERRNO ()) ;
    } while (!VALID_FD ((*dataPoint)->fd) && (errno == EINTR)) ;

    if (!VALID_FD ((*dataPoint)->fd)) {
        if (errno != EWOULDBLOCK)
            LGE "(tcpAnswer) Error accepting connection request on %s.\naccept: ",
                listeningPoint->name) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }

/* Configure the socket so that the operating system periodically verifies
   that the connection is still alive by "pinging" the client. */

    optval = 1 ;			/* Enable keep-alive transmissions. */
    if (setsockopt ((*dataPoint)->fd, SOL_SOCKET, SO_KEEPALIVE,
                    (char *) &optval, sizeof optval) == -1) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpAnswer) Error enabling keep-alive mode for %s's client connection.\nsetsocketopt: ",
            listeningPoint->name) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }

/* Construct the connection's name. */

    hostName = (char *) sktPeer ((*dataPoint)->fd) ;
    if (hostName == NULL)  hostName = "localhost" ;
    sprintf (serverName, "%u", sktPort ((*dataPoint)->fd)) ;
    (*dataPoint)->name = malloc (strlen (serverName) + 1 +
                                 strlen (hostName) + 1) ;
    if ((*dataPoint)->name == NULL) {
        LGE "(tcpAnswer) Error duplicating server name: %s#%s\nmalloc: ",
            serverName, hostName) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }
    sprintf ((*dataPoint)->name, "%s#%s", serverName, hostName) ;


/* Return the data connection to the caller. */

    LGI "(tcpAnswer) Accepted connection %s, socket %d.\n",
        (*dataPoint)->name, (*dataPoint)->fd) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    tcpListen ()

    Listen for Network Connection Requests from Clients.


Purpose:

    Function tcpListen() creates a "listening" endpoint on which a network
    server can listen for connection requests from clients.  The server
    then calls tcpAnswer() to "answer" incoming requests.


    Invocation:

        status = tcpListen (serverName, backlog, &listeningPoint) ;

    where

        <serverName>		- I
            is the server's name.  This is used for determining the port
            associated with the server (via the system's name/port mappings).
            You can side-step the system maps and explicitly specify a
            particular port by passing in a decimal number encoded in ASCII
            (e.g., "1234" for port 1234).
        <backlog>		- I
            is the number of connection requests that can be outstanding
            for the server.  UNIX systems typically allow a maximum of 5.
        <listeningPoint>	- O
            returns a handle for the new listening endpoint.
        <status>		- O
            returns the status of creating the endpoint: zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  tcpListen (

#    if PROTOTYPES
        const  char  *serverName,
        int  backlog,
        TcpEndpoint  *listeningPoint)
#    else
        serverName, backlog, listeningPoint)

        char  *serverName ;
        int  backlog ;
        TcpEndpoint  *listeningPoint ;
#    endif

{    /* Local variables. */
    IoctlArg  optval ;
    struct  sockaddr_in  socketName ;
    unsigned  int  portNumber ;




    if (serverName == NULL)  serverName = "0" ;
    if ((backlog < 0) || (MAX_LISTEN_BACKLOG < backlog))
        backlog = MAX_LISTEN_BACKLOG ;

/* Create an endpoint structure. */

    *listeningPoint = (TcpEndpoint) malloc (sizeof (_TcpEndpoint)) ;
    if (*listeningPoint == NULL) {
        LGE "(tcpListen) Error allocating endpoint structure for %s.\nmalloc: ",
            serverName) ;
        return (errno) ;
    }

    (*listeningPoint)->type = TcpListeningPoint ;
    (*listeningPoint)->fd = INVALID_SOCKET ;

    (*listeningPoint)->name = strdup (serverName) ;
    if ((*listeningPoint)->name == NULL) {
        LGE "(tcpListen) Error duplicating server name: %s\nstrdup: ",
            serverName) ;
        CLEAN_UP (*listeningPoint) ;
        return (errno) ;
    }

/* Lookup the port number bound to the server name. */

    portNumber = netPortOf (serverName, "tcp") ;
    if (portNumber == -1) {
        LGE "(tcpListen) Error getting server entry for %s.\nnetPortOf: ",
            serverName) ;
        CLEAN_UP (*listeningPoint) ;
        return (errno) ;
    }

/* Set up the network address for the connection. */

    memset (&socketName, '\0', sizeof socketName) ;
    socketName.sin_family = AF_INET ;
    socketName.sin_port = htons (portNumber) ;
    socketName.sin_addr.s_addr = INADDR_ANY ;

/* If a port number of 0 was specified, a random, unused port number is chosen.
   This is done automatically on most operating systems, but, if necessary,
   use a random-number generator and hope a conflicting port is not chosen!
   NOTE that HAVE_NETWORK_PORT_0 should be set to 1 (true) if the host OS
   does NOT pick a random port itself. */

#if defined(HAVE_NETWORK_PORT_0) && HAVE_NETWORK_PORT_0
    if (portNumber == 0) {
#    define  MIN_PORT_NUMBER  29000
        portNumber = MIN_PORT_NUMBER + (rand () & 0x03F) ;
        socketName.sin_port = htons (portNumber) ;
        portNumber = 0 ;
    }
#endif

/* Create a socket for the connection. */

    (*listeningPoint)->fd = socket (AF_INET, SOCK_STREAM, 0) ;
    if (!VALID_FD ((*listeningPoint)->fd)) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpListen) Error creating listening socket for endpoint %s.\nsocket: ",
            serverName) ;
        CLEAN_UP (*listeningPoint) ;
        return (errno) ;
    }

/* Configure the socket so it can be discarded quickly when no longer needed.
   If the SO_REUSEADDR option is not enabled and server A goes down without
   client B closing its half of the broken connection, server A can't come
   back up again.  Server A keeps getting an "address in use" error, even
   though A is the "owner" of the port and B can't really do anything with
   its broken connection!  The reuse-address option allows A to come right
   back up again and create a new listening socket. */

#ifdef __palmos__
	/* No errors in POSE emulator, but my M105 returns a not-implemented
	   error in tcpCall(); not yet sure about tcpListen(). */
#else
    optval = 1 ;			/* Enable address reuse. */
    if (setsockopt ((*listeningPoint)->fd, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &optval, sizeof optval) == -1) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpListen) Error setting %s endpoint's listening socket for re-use.\nsetsocketopt: ",
            serverName) ;
        CLEAN_UP (*listeningPoint) ;
        return (errno) ;
    }
#endif

/* Bind the network address to the socket and enable it to listen for
   connection requests. */

    if (bind ((*listeningPoint)->fd,
              (struct sockaddr *) &socketName, sizeof socketName)) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpListen) Error binding %s endpoint's socket name.\nbind: ",
            serverName) ;
        CLEAN_UP (*listeningPoint) ;
        return (errno) ;
    }

    if (listen ((*listeningPoint)->fd, backlog)) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpListen) Error enabling acceptance of connection requests on %s endpoint.\nlisten: ",
            (*listeningPoint)->name) ;
        CLEAN_UP (*listeningPoint) ;
        return (errno) ;
    }

    if (portNumber == 0) {
        portNumber = sktPort ((*listeningPoint)->fd) ;
        free ((*listeningPoint)->name) ;
        (*listeningPoint)->name = (char *) malloc (16) ;
        if ((*listeningPoint)->name == NULL) {
            LGE "(tcpListen) Error duplicating port name: %u\nmalloc: ",
                portNumber) ;
            CLEAN_UP (*listeningPoint) ;
        }
        sprintf ((*listeningPoint)->name, "%u", portNumber) ;
        LGI "(tcpListen) Random port: %u\n", portNumber) ;
    }

    LGI "(tcpListen) Listening on %s, port %u, socket %d.\n",
        (*listeningPoint)->name, portNumber, (*listeningPoint)->fd) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    tcpRequestPending ()

    Check a Listening Port for Pending Connection Requests.


Purpose:

    The tcpRequestPending() function checks to see if any connection requests
    from potential clients are waiting to be answered.


    Invocation:

        isPending = tcpRequestPending (listeningPoint) ;

    where

        <listeningPoint>	- I
            is the endpoint handle returned by tcpListen().
        <isPending>		- O
            returns true (a non-zero value) if connection requests are
            pending and false (zero) otherwise.

*******************************************************************************/


bool  tcpRequestPending (

#    if PROTOTYPES
        TcpEndpoint  listeningPoint)
#    else
        listeningPoint)

        TcpEndpoint  listeningPoint ;
#    endif

{    /* Local variables. */
    fd_set  readMask ;
    struct  timeval  timeout ;



    if (listeningPoint == NULL)  return (false) ;

/* Poll the listening socket for input. */

    for ( ; ; ) {
        FD_ZERO (&readMask) ;  FD_SET (listeningPoint->fd, &readMask) ;
        timeout.tv_sec = timeout.tv_usec = 0 ;	/* No wait. */
        if (select (listeningPoint->fd+1, &readMask, NULL, NULL, &timeout) >= 0)
            break ;
        SET_ERRNO (GET_NETERRNO ()) ;
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(tcpRequestPending) Error polling endpoint %s, socket %d.\nselect: ",
            listeningPoint->name, listeningPoint->fd) ;
        return (false) ;
    }

    return (FD_ISSET (listeningPoint->fd, &readMask)) ;	/* Request pending? */

}

/*!*****************************************************************************

Procedure:

    tcpCall ()

    Request a Network Connection to a Server.


Purpose:

    Function tcpCall() is used by a client task to "call" a server task
    and request a network connection to the server.  If its no-wait
    argument is false (zero), tcpCall() waits until the connection is
    established (or refused) before returning to the invoking function:

        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        TcpEndpoint  connection ;
        ...
        if (tcpCall ("<server>[@<host>]", 0, &connection)) {
            ... error establishing connection ...
        } else {
            ... connection is established ...
        }

    If the no-wait argument is true (non-zero), tcpCall() initiates the
    connection attempt and immediately returns; the application must then
    invoke tcpComplete() to complete the connection attempt:

        if (tcpCall ("<server>[@<host>]", 1, &connection)) {
            ... error initiating connection attempt ...
        } else {
            ... do something else ...
            if (tcpComplete (connection, -1.0, 1)) {
                ... error establishing connection ...
            } else {
                ... connection is established ...
            }
        }

    No-wait mode is useful in applications which monitor multiple I/O sources
    using select(2).  After tcpCall() returns, a to-be-established connection's
    file descriptor can be retrieved with tcpFd() and placed in select(2)'s
    write mask.  When select(2) detects that the connection is writeable,
    the application can call tcpComplete() to complete the connection.

    IMPLEMENTATION NOTE:  Connecting to a server is usually accomplished
    by creating an unbound socket and calling connect(), a system call, to
    establish the connection.  In order to implement the timeout capability,
    the socket is configured for non-blocking I/O before connect() is called.
    If the connection cannot be established right away, connect() returns
    immediately and select() is called to wait for the timeout interval to
    expire or for the socket to become writeable (indicating the connection
    is complete).  Once the connection is established, the socket is
    reconfigured for blocking I/O.

    The basic idea for implementing a timeout capability was outlined in
    a posting from W. Richard Stevens (*the* network guru) I retrieved
    from one of the WWW news archives.  VxWorks has a non-portable
    connectWithTimeout() function which, to save a few "#ifdef"s, I don't
    use.


    Invocation:

        status = tcpCall (serverName, noWait, &dataPoint) ;

    where

        <serverName>	- I
            is the server's name: "<server>[@<host>]".  The server can be
            specified as a name or as a port number.  The host, if given,
            can be specified as a name or as a dotted Internet address.
        <noWait>	- I
            specifies if tcpCall() should wait for a connection to be
            established.  If noWait is false (zero), tcpCall() waits
            until a connection is established or refused before returning
            control to the caller.  If noWait is true (a non-zero value),
            tcpCall() initiates a connection attempt and returns to the
            caller immediately; the caller is responsible for eventually
            calling tcpComplete() to complete the connection.
        <dataPoint>	- O
            returns a handle for the endpoint.
        <status>	- O
            returns the status of establishing the network connection:
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  tcpCall (

#    if PROTOTYPES
        const  char  *serverName,
        bool  noWait,
        TcpEndpoint  *dataPoint)
#    else
        serverName, noWait, dataPoint)

        char  *serverName ;
        bool  noWait ;
        TcpEndpoint  *dataPoint ;
#    endif

{    /* Local variables. */
    char  *s, hostName[MAXHOSTNAMELEN+1], server_name[MAXHOSTNAMELEN+1] ;
    int  portNumber ;
    IoctlArg  optval ;
    struct  sockaddr_in  socketName ;
    socklen_t  length ;




/*******************************************************************************
    Determine the host and server information needed to make the connection.
*******************************************************************************/


/* Parse the host and server names.  If the host name is not defined
   explicitly, it defaults to the local host. */

    s = (char *) netHostOf (netAddrOf (NULL), false) ;
    if (s == NULL) {
        LGE "(tcpCall) Error getting local host name.\nnetHostOf: ") ;
        return (errno) ;
    }
    strcpy (hostName, s) ;

    s = strchr (serverName, '@') ;
    if (s == NULL) {				/* "<server>" */
        strcpy (server_name, serverName) ;
    } else {					/* "<server>@<host>" */
        length = s - serverName ;
        strncpym (server_name, serverName, length, sizeof server_name) ;
        strcpy (hostName, ++s) ;
    }


/* Create an endpoint structure. */

    *dataPoint = (TcpEndpoint) malloc (sizeof (_TcpEndpoint)) ;
    if (*dataPoint == NULL) {
        LGE "(tcpCall) Error allocating connection structure for %s.\nmalloc: ",
            serverName) ;
        return (errno) ;
    }

    (*dataPoint)->type = TcpDataPoint ;
    (*dataPoint)->fd = INVALID_SOCKET ;

    (*dataPoint)->name = malloc (strlen (server_name) + 1 +
                                 strlen (hostName) + 1) ;
    if ((*dataPoint)->name == NULL) {
        LGE "(tcpCall) Error duplicating server name: %s@%s\nmalloc: ",
            server_name, hostName) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }
    sprintf ((*dataPoint)->name, "%s@%s", server_name, hostName) ;


/* Lookup the port number bound to the server name. */

    portNumber = netPortOf (server_name, "tcp") ;
    if (portNumber == -1) {
        LGE "(tcpCall) Error getting server entry for %s.\nnetPortOf: ",
            serverName) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }


/* Set up the network address for the connection. */

    memset (&socketName, '\0', sizeof socketName) ;
    socketName.sin_family = AF_INET ;
    socketName.sin_port = htons (portNumber) ;

    socketName.sin_addr.s_addr = netAddrOf (hostName) ;
    if (socketName.sin_addr.s_addr == 0) {
        LGE "(tcpCall) Error getting host entry for %s.\nnetAddrOf: ",
            hostName) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }

/*******************************************************************************
    Establish a connection with the server.
*******************************************************************************/


/* Create a socket for the connection. */

    (*dataPoint)->fd = socket (AF_INET, SOCK_STREAM, 0) ;
    if (!VALID_FD ((*dataPoint)->fd)) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpCall) Error creating socket for %s.\nsocket: ",
            (*dataPoint)->name) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }


/* Configure the socket so it can be discarded quickly when no longer needed.
   (See the description of the SO_REUSEADDR option under NET_ANSWER, above.) */

#ifdef __palmos__
	/* No errors in POSE emulator, but my M105 returns a not-implemented
	   error in tcpCall(); not yet sure about tcpListen(). */
#else
    optval = 1 ;			/* Enable address reuse. */
    if (setsockopt ((*dataPoint)->fd, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &optval, sizeof optval) == -1) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpCall) Error setting %s's socket for re-use.\nsetsocketopt: ",
            (*dataPoint)->name) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }
#endif


/* If caller does not wish to wait for the connection to be established, then
   configure the socket for non-blocking I/O.  This causes the connect(2) call
   to only initiate the attempt to connect; tcpComplete() must be called to
   complete the connection. */

    if (noWait && sktBlock ((*dataPoint)->fd, false)) {
        LGE "(tcpCall) Error configuring %s's socket for non-blocking I/O.\nsktBlock: ",
            (*dataPoint)->name) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }


/* Attempt to establish the connection. */

    if (connect ((*dataPoint)->fd,
                 (struct sockaddr *) &socketName, sizeof socketName)) {
        SET_ERRNO (GET_NETERRNO ()) ;	/* EWOULDBLOCK returned on PalmOS. */
        if (!noWait || ((errno != EINPROGRESS) && (errno != EWOULDBLOCK))) {
            LGE "(tcpCall) Error attempting to connect to %s.\nconnect: ",
                (*dataPoint)->name) ;
            CLEAN_UP (*dataPoint) ;
            return (errno) ;
        }
    }


/* If caller does not wish to wait for the connection to be established,
   then return immediately; the caller is responsible for subsequently
   calling tcpComplete(). */

    if (noWait)  return (0) ;


/* The connection has been established.  Configure the socket so that the
   operating system periodically verifies that the connection is still alive
   by "pinging" the server. */

    optval = 1 ;			/* Enable keep-alive transmissions. */
    if (setsockopt ((*dataPoint)->fd, SOL_SOCKET, SO_KEEPALIVE,
                    (char *) &optval, sizeof optval) == -1) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpCall) Error enabling keep-alive mode for connection to %s.\nsetsockopt: ",
            (*dataPoint)->name) ;
        CLEAN_UP (*dataPoint) ;
        return (errno) ;
    }

    LGI "(tcpCall) Connected to %s, port %u, socket %d.\n",
        (*dataPoint)->name, sktPort ((*dataPoint)->fd), (*dataPoint)->fd) ;


    return (0) ;	/* Successful completion. */

}

/*!*****************************************************************************

Procedure:

    tcpComplete ()

    Complete a Call to a Server.


Purpose:

    Function tcpComplete() waits for an asynchronous, network connection
    attempt to complete.  Invoking tcpCall() in no-wait mode initiates
    an attempt to connect to a network server.  At some later time, the
    application must call tcpComplete() to complete the connection attempt
    (if it is fated to complete).  See tcpCall() for an example of using
    tcpComplete().

    The default behavior of tcpComplete() is to NOT destroy the endpoint
    created by tcpCall(), regardless of any errors.  Since the endpoint
    SHOULD be destroyed in the event of an error, the destroyOnError flag
    provides a short-hand means of waiting for a connection without
    explicitly destroying the endpoint if the connection attempt fails:

        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        TcpEndpoint  connection ;
        ...
        if (tcpCall ("<server>[@<host>]", 1, &connection) ||
            tcpComplete (connection, 30.0, 1)) {
            ... timeout or error establishing connection;
                connection is destroyed ...
        } else {
            ... connection is established ...
        }

    Some applications, however, may not wish to destroy the connection right
    away.  For example, the following code fragment periodically displays an
    in-progress message until a connection is established or refused:

        if (tcpCall ("<server>[@<host>]", 1, &connection)) {
            ... error initiating connection attempt ...
        } else {
            for ( ; ; ) {
                printf ("Waiting for %s ...\n", tcpName (connection)) ;
                if (!tcpComplete (connection, 5.0, 0)) {
                    ... connection is established ...
                    break ;
                } else if (errno != EWOULDBLOCK) {
                    ... connection attempt failed ...
                    tcpDestroy (connection) ;  connection = NULL ;
                    break ;
                }
            }
        }


    Invocation:

        status = tcpComplete (dataPoint, timeout, destroyOnError) ;

    where

        <dataPoint>		- I
            is the endpoint handle returned by tcpCall().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the call to complete.  A fractional
            time can be specified; e.g., 2.5 seconds.  A negative timeout
            (e.g., -1.0) causes an infinite wait; a zero timeout (0.0)
            causes an immediate return if the connection is not yet
            established.
        <destroyOnError>	- I
            specifies if tcpComplete() should destroy the endpoint in the
            event of an error.  If this argument is true (a non-zero value),
            tcpComplete() calls tcpDestroy() to destroy the endpoint.  If
            this argument is false (zero), the calling routine itself is
            responsible for destroying the endpoint.
        <status>		- O
            returns the status of completing the network connection:
            zero if there were no errors, EWOULDBLOCK if the timeout
            interval expired before the connection was established,
            and ERRNO otherwise.

*******************************************************************************/


errno_t  tcpComplete (

#    if PROTOTYPES
        TcpEndpoint  dataPoint,
        double  timeout,
        bool  destroyOnError)
#    else
        dataPoint, timeout, destroyOnError)

        TcpEndpoint  dataPoint ;
        double  timeout ;
        bool  destroyOnError ;
#    endif

{    /* Local variables. */
    fd_set  writeMask ;
    int  numActive ;
    IoctlArg  optval ;
    socklen_t  length ;
    struct  timeval  deltaTime, expirationTime ;




/* If a timeout interval was specified, then compute the expiration time
   of the interval as the current time plus the interval. */

    if (timeout >= 0.0)
        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;


/* Wait for the call to complete, which is indicated by the connection
   being writeable.  */

    for ( ; ; ) {
        if (timeout >= 0.0)  deltaTime = tvSubtract (expirationTime, tvTOD ()) ;
        FD_ZERO (&writeMask) ;  FD_SET (dataPoint->fd, &writeMask) ;
        numActive = select (dataPoint->fd+1, NULL, &writeMask, NULL,
                            (timeout < 0.0) ? NULL : &deltaTime) ;
        if (numActive > 0)  break ;
        SET_ERRNO (GET_NETERRNO ()) ;
        if (numActive == 0)  SET_ERRNO (EWOULDBLOCK) ;
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(tcpComplete) Error waiting to connect to %s.\nselect: ",
            dataPoint->name) ;
        if (destroyOnError)  CLEAN_UP (dataPoint) ;
        return (errno) ;
    }


/* Check the connection's error status. */

    length = sizeof optval ;
    if (getsockopt (dataPoint->fd, SOL_SOCKET, SO_ERROR,
                    (char *) &optval, &length) == -1) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpComplete) Error checking error status of connection to %s.\ngetsockopt: ",
            dataPoint->name) ;
        if (destroyOnError)  CLEAN_UP (dataPoint) ;
        return (errno) ;
    }

    if (optval) {
        SET_ERRNO (optval) ;
        LGE "(tcpComplete) Error connecting to %s.\nconnect: ",
            dataPoint->name) ;
        if (destroyOnError)  CLEAN_UP (dataPoint) ;
        return (errno) ;
    }


/* The connection has been established.  Configure the socket so that the
   operating system periodically verifies that the connection is still alive
   by "pinging" the server. */

    optval = 1 ;			/* Enable keep-alive transmissions. */
    if (setsockopt (dataPoint->fd, SOL_SOCKET, SO_KEEPALIVE,
                    (char *) &optval, sizeof optval) == -1) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(tcpComplete) Error enabling keep-alive mode for connection to %s.\nsetsockopt: ",
            dataPoint->name) ;
        if (destroyOnError)  CLEAN_UP (dataPoint) ;
        return (errno) ;
    }


/* Reconfigure the socket for blocking I/O. */

    if (sktBlock (dataPoint->fd, true)) {
        LGE "(tcpComplete) Error reconfiguring %s's socket for blocking I/O.\nsktBlock: ",
            dataPoint->name) ;
        if (destroyOnError)  CLEAN_UP (dataPoint) ;
        return (errno) ;
    }

    LGI "(tcpComplete) Connected to %s, port %u, socket %d.\n",
        dataPoint->name, sktPort (dataPoint->fd), dataPoint->fd) ;


    return (0) ;			/* Successful completion. */

}

/*!*****************************************************************************

Procedure:

    tcpIsReadable ()

    Check if Data is Waiting to be Read.


Purpose:

    The tcpIsReadable() function checks to see if data is waiting to
    be read from a connection.


    Invocation:

        isReadable = tcpIsReadable (dataPoint) ;

    where

        <dataPoint>	- I
            is the endpoint handle returned by tcpAnswer() or tcpCall().
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  tcpIsReadable (

#    if PROTOTYPES
        TcpEndpoint  dataPoint)
#    else
        dataPoint)

        TcpEndpoint  dataPoint ;
#    endif

{
    if (dataPoint == NULL)
        return (false) ;
    else
        return (sktIsReadable (dataPoint->fd)) ;
}

/*!*****************************************************************************

Procedure:

    tcpIsUp ()

    Check if a Connection is Up.


Purpose:

    The tcpIsUp() function checks to see if a network connection is still up.


    Invocation:

        isUp = tcpIsUp (dataPoint) ;

    where

        <dataPoint>	- I
            is the endpoint handle returned by tcpAnswer() or tcpCall().
        <isUp>		- O
            returns true (a non-zero value) if the network connection is
            up and false (zero) otherwise.

*******************************************************************************/


bool  tcpIsUp (

#    if PROTOTYPES
        TcpEndpoint  dataPoint)
#    else
        dataPoint)

        TcpEndpoint  dataPoint ;
#    endif

{
    if (dataPoint == NULL)
        return (false) ;
    else
        return (sktIsUp (dataPoint->fd)) ;
}

/*!*****************************************************************************

Procedure:

    tcpIsWriteable ()

    Check if Data can be Written.


Purpose:

    The tcpIsWriteable() function checks to see if data can be written
    to a connection.


    Invocation:

        isWriteable = tcpIsWriteable (dataPoint) ;

    where

        <dataPoint>	- I
            is the endpoint handle returned by tcpAnswer() or tcpCall().
        <isWriteable>	- O
            returns true (a non-zero value) if data connection is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  tcpIsWriteable (

#    if PROTOTYPES
        TcpEndpoint  dataPoint)
#    else
        dataPoint)

        TcpEndpoint  dataPoint ;
#    endif

{
    if (dataPoint == NULL)
        return (false) ;
    else
        return (sktIsWriteable (dataPoint->fd)) ;
}

/*!*****************************************************************************

Procedure:

    tcpRead ()

    Read Data from a Network Connection.


Purpose:

    Function tcpRead() reads data from a network connection.  Because of
    the way network I/O works, a single record written to a connection
    by one task may be read in multiple "chunks" by the task at the other
    end of the connection.  For example, a 100-byte record written by a
    client may be read by the server in two chunks, one of 43 bytes and
    the other of 57 bytes.  tcpRead() takes this into account and, if
    you ask it for 100 bytes, it will automatically perform however many
    network reads are necessary to collect the 100 bytes.  You can also
    ask tcpRead() to return the first chunk received, whatever its length;
    see the numBytesToRead argument.

    A timeout can be specified that limits how long tcpRead() waits for the
    first data to arrive.  If a complete record must be read (see above),
    tcpRead() will then wait as long as necessary for the remainder of the
    record to be received.  This ensures that a partial record will NOT be
    returned at the end of the timeout interval.


    Invocation:

        status = tcpRead (dataPoint, timeout, numBytesToRead,
                          buffer, &numBytesRead) ;

    where

        <dataPoint>		- I
            is the endpoint handle returned by tcpAnswer() or tcpCall().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the caller
            wishes to wait for the first data to arrive.  A fractional time can
            be specified; e.g., 2.5 seconds.  A negative timeout (e.g., -1.0)
            causes an infinite wait; a zero timeout (0.0) allows a read only
            if input is immediately available.
        <numBytesToRead>	- I
            has two different meanings depending on its sign:
                (1) If the number of bytes to read is positive, tcpRead()
                    will continue to read input until it has accumulated
                    the exact number of bytes requested.  If the timeout
                    interval expires before ANY data has been read, then
                    tcpRead() returns with an EWOULDBLOCK status.  If some
                    of the data is read before the timeout interval expires,
                    tcpRead() will wait as long as necessary to read the
                    remaining data.  This ensures that a partial record is
                    not returned to the caller at the end of the timeout
                    interval.
                (2) If the number of bytes to read is negative, tcpRead()
                    returns after reading the first "chunk" of input received;
                    the number of bytes read from that first "chunk" is limited
                    by the absolute value of numBytesToRead.  A normal status
                    (0) is returned if the first "chunk" of input is received
                    before the timeout interval expires; EWOULDBLOCK is
                    returned if no input is received within that interval.
        <buffer>		- O
            receives the input data.  This buffer should be at least
            numBytesToRead in size.
        <numBytesRead>	- O
            returns the actual number of bytes read.
        <status>		- O
            returns the status of reading from the network connection:
            zero if no errors occurred, EWOULDBLOCK if the timeout
            interval expired before the requested amount of data was
            input, and ERRNO otherwise.  (See the numBytesToRead
            argument for a description of how that argument affects
            the returned status code.)

*******************************************************************************/


errno_t  tcpRead (

#    if PROTOTYPES
        TcpEndpoint  dataPoint,
        double  timeout,
        ssize_t  numBytesToRead,
        char  *buffer,
        size_t  *numBytesRead)
#    else
        dataPoint, timeout, numBytesToRead, buffer, numBytesRead)

        TcpEndpoint  dataPoint ;
        double  timeout ;
        ssize_t  numBytesToRead ;
        char  *buffer ;
        size_t  *numBytesRead ;
#    endif

{    /* Local variables. */
    bool  firstInputOnly ;
    fd_set  readMask ;
    int  length, numActive ;
    struct  timeval  deltaTime, expirationTime ;




    if (dataPoint == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(tcpRead) NULL endpoint handle: ") ;
        return (errno) ;
    }

    if (!VALID_FD (dataPoint->fd)) {
        SET_ERRNO (EBADF) ;
        LGE "(tcpRead) %d file descriptor: ", dataPoint->fd) ;
        return (errno) ;
    }

    if (numBytesRead != NULL)  *numBytesRead = 0 ;


/*******************************************************************************
    If a timeout interval was specified, then wait until the expiration of
    the interval for data to be received.
*******************************************************************************/

    if (timeout >= 0.0) {

/* Compute the expiration time as the current time plus the interval. */

        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;

/* Wait for data to arrive. */

        for ( ; ; ) {
            deltaTime = tvSubtract (expirationTime, tvTOD ()) ;
            FD_ZERO (&readMask) ;  FD_SET (dataPoint->fd, &readMask) ;
            numActive = select (dataPoint->fd+1, &readMask, NULL, NULL,
                                &deltaTime) ;
            if (numActive >= 0)  break ;
            SET_ERRNO (GET_NETERRNO ()) ;
            if (errno == EINTR)  continue ;	/* Retry on signal interrupt. */
            LGE "(tcpRead) Error waiting for input on %s, socket %d.\nselect: ",
                dataPoint->name, dataPoint->fd) ;
            return (errno) ;
        }

        if (numActive == 0) {
            SET_ERRNO (EWOULDBLOCK) ;
#ifdef REPORT_TIMEOUT
            LGE "(tcpRead) Timeout while waiting for input on %s, socket %d.\n",
                dataPoint->name, dataPoint->fd) ;
#endif
            return (errno) ;
        }

    }


/*******************************************************************************
    Read the requested amount of data from the network connection.
*******************************************************************************/

    firstInputOnly = (numBytesToRead < 0) ;
    if (firstInputOnly)  numBytesToRead = - numBytesToRead ;

    while (numBytesToRead > 0) {

        length = recv (dataPoint->fd, buffer, numBytesToRead, 0) ;
        if (length < 0) {
            SET_ERRNO (GET_NETERRNO ()) ;
            LGE "(tcpRead) Error reading from connection %d.\nrecv: ",
                dataPoint->fd) ;
            return (errno) ;
        } else if (length == 0) {
            SET_ERRNO (EPIPE) ;
            LGE "(tcpRead) Broken connection on %s, socket %d.\nread: ",
                dataPoint->name, dataPoint->fd) ;
            return (errno) ;
        }

        LGI "(tcpRead) Read %d bytes from %s, socket %d.\n",
            length, dataPoint->name, dataPoint->fd) ;
        if (I_DEFAULT_GUARD) {
            meoDumpX (NULL, "    ", 0, buffer, length) ;
        }

        if (numBytesRead != NULL)  *numBytesRead += length ;
        numBytesToRead -= length ;
        buffer += length ;

        if (firstInputOnly)  break ;

    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    tcpWrite ()

    Write Data to a Network Connection.


Purpose:

    Function tcpWrite() writes data to a network connection.  Because
    of the way network I/O works, attempting to output a given amount
    of data to a network connection may require multiple system
    WRITE(2)s.  For example, when called to output 100 bytes of data,
    WRITE(2) may return after only outputting 64 bytes of data; the
    application is responsible for re-calling WRITE(2) to output the
    other 36 bytes.  tcpWrite() takes this into account and, if you
    ask it to output 100 bytes, it will call WRITE(2) as many times
    as necessary to output the full 100 bytes of data to the connection.

    A timeout interval can be specified that limits how long tcpWrite()
    waits to output the desired amount of data.  If the timeout interval
    expires before all the data has been written, tcpWrite() returns the
    number of bytes actually output in the numBytesWritten argument.


    Invocation:

        status = tcpWrite (dataPoint, timeout, numBytesToWrite, buffer,
                           &numBytesWritten) ;

    where

        <dataPoint>		- I
            is the endpoint handle returned by tcpAnswer() or tcpCall().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the data to be output.  A fractional
            time can be specified; e.g., 2.5 seconds.   A negative timeout
            (e.g., -1.0) causes an infinite wait; tcpWrite() will wait as
            long as necessary to output all of the data.  A zero timeout
            (0.0) specifies no wait: if the socket is not ready for writing,
            tcpWrite() returns immediately; if the socket is ready for
            writing, tcpWrite() returns after outputting whatever it can.
        <numBytesToWrite>	- I
            is the number of bytes to write.  If the timeout argument
            indicates an infinite wait, tcpWrite() won't return until
            it has output the entire buffer of data.  If the timeout
            argument is greater than or equal to zero, tcpWrite() will
            output as much as it can in the specified time interval,
            up to a maximum of numBytesToWrite.
        <buffer>		- O
            is the data to be output.
        <numBytesWritten>	- O
            returns the actual number of bytes written.  If an infinite wait
            was specified (TIMEOUT < 0.0), then this number should equal
            numBytesToWrite.  If a finite wait was specified, the number
            of bytes written may be less than the number requested.
        <status>		- O
            returns the status of writing to the network connection:
            zero if no errors occurred, EWOULDBLOCK if the timeout
            interval expired before the entire buffer of data was
            output, and ERRNO otherwise.

*******************************************************************************/


errno_t  tcpWrite (

#    if PROTOTYPES
        TcpEndpoint  dataPoint,
        double  timeout,
        size_t  numBytesToWrite,
        const  char  *buffer,
        size_t  *numBytesWritten)
#    else
        dataPoint, timeout, numBytesToWrite, buffer, numBytesWritten)

        TcpEndpoint  dataPoint ;
        double  timeout ;
        size_t  numBytesToWrite ;
        const  char  *buffer ;
        size_t  *numBytesWritten ;
#    endif

{    /* Local variables. */
    fd_set  writeMask ;
    int  length, numActive ;
    struct  timeval  deltaTime, expirationTime ;




    if (dataPoint == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(tcpWrite) NULL endpoint handle: ") ;
        return (errno) ;
    }

    if (!VALID_FD (dataPoint->fd)) {
        SET_ERRNO (EBADF) ;
        LGE "(tcpWrite) %d file descriptor: ", dataPoint->fd) ;
        return (errno) ;
    }


/* If a timeout interval was specified, then compute the expiration time
   of the interval as the current time plus the interval. */

    if (timeout >= 0.0)
        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;


/*******************************************************************************
    While the timeout interval has not expired, attempt to write the entire
    buffer of data to the network connection.
*******************************************************************************/

    if (numBytesWritten != NULL)  *numBytesWritten = 0 ;

    while (numBytesToWrite > 0) {

/* Wait for the connection to be ready for writing. */

        for ( ; ; ) {
            if (timeout >= 0.0)
                deltaTime = tvSubtract (expirationTime, tvTOD ()) ;
            FD_ZERO (&writeMask) ;  FD_SET (dataPoint->fd, &writeMask) ;
            numActive = select (dataPoint->fd+1, NULL, &writeMask, NULL,
                                (timeout < 0.0) ? NULL : &deltaTime) ;
            if (numActive >= 0)  break ;
            SET_ERRNO (GET_NETERRNO ()) ;
            if (errno == EINTR)  continue ;	/* Retry on signal interrupt. */
            LGE "(tcpWrite) Error waiting to write data to %s.\nselect: ",
                dataPoint->name) ;
            return (errno) ;
        }

        if (numActive == 0) {
            SET_ERRNO (EWOULDBLOCK) ;
            LGE "(tcpWrite) Timeout while waiting to write data to %s.\n",
                dataPoint->name) ;
            return (errno) ;
        }

/* Write the next chunk of data to the network. */

        length = send (dataPoint->fd, (char *) buffer, numBytesToWrite, 0) ;
        if (length < 0) {
            SET_ERRNO (GET_NETERRNO ()) ;
            LGE "(tcpWrite) Error writing to %s.\nsend: ", dataPoint->name) ;
            return (errno) ;
        }

        LGI "(tcpWrite) Wrote %d bytes to %s, socket %d.\n",
            length, dataPoint->name, dataPoint->fd) ;
        if (I_DEFAULT_GUARD) {
            meoDumpX (NULL, "    ", 0, buffer, length) ;
        }

        if (numBytesWritten != NULL)  *numBytesWritten += length ;
        numBytesToWrite -= length ;
        buffer += length ;

    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    tcpDestroy ()

    Close a Network Endpoint.


Purpose:

    Function tcpDestroy() deletes listening endpoints created by tcpListen()
    and data endpoints created by tcpAnswer() or tcpCall().


    Invocation:

        status = tcpDestroy (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by tcpListen(), tcpAnswer(),
            or tcpCall().
        <status>	- O
            returns the status of deleting the endpoint, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  tcpDestroy (

#    if PROTOTYPES
        TcpEndpoint  endpoint)
#    else
        endpoint)

        TcpEndpoint  endpoint ;
#    endif

{

    if (endpoint == NULL)  return (0) ;

    LGI "(tcpDestroy) Closing %s, socket %d.\n", endpoint->name, endpoint->fd) ;

/* Close the endpoint's socket. */

    if (VALID_FD (endpoint->fd)) {
#ifdef NEEDED
        shutdown (endpoint->fd, SHUT_RDWR) ;
#endif
        CLOSESOCKET (endpoint->fd) ;
        endpoint->fd = INVALID_SOCKET ;
    }

/* Deallocate the endpoint structure. */

    if (endpoint->name != NULL)  free (endpoint->name) ;
    endpoint->name = NULL ;
    free (endpoint) ;
    endpoint = NULL ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    tcpFd ()

    Get an Endpoint's Socket.


Purpose:

    Function tcpFd() returns a listening or data endpoint's socket.


    Invocation:

        fd = tcpFd (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by tcpListen(), tcpAnswer(),
            or tcpCall().
        <fd>		- O
            returns the UNIX file descriptor for the endpoint's socket.

*******************************************************************************/


IoFd  tcpFd (

#    if PROTOTYPES
        TcpEndpoint  endpoint)
#    else
        endpoint)

        TcpEndpoint  endpoint ;
#    endif

{
    return ((endpoint == NULL) ? INVALID_SOCKET : endpoint->fd) ;
}

/*!*****************************************************************************

Procedure:

    tcpName ()

    Get an Endpoint's Name.


Purpose:

    Function tcpName() returns a listening or data endpoint's name.


    Invocation:

        name = tcpName (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by tcpListen(), tcpAnswer(),
            or tcpCall().
        <name>		- O
            returns the endpoint's name.  The name is stored in memory local
            to the TCP utilities and it should not be modified or freed
            by the caller.

*******************************************************************************/


const  char  *tcpName (

#    if PROTOTYPES
        TcpEndpoint  endpoint)
#    else
        endpoint)

        TcpEndpoint  endpoint ;
#    endif

{
    if (endpoint == NULL)  return ("") ;
    if (endpoint->name == NULL)  return ("") ;
    return (endpoint->name) ;
}
