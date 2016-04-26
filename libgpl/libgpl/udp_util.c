/* $Id: udp_util.c,v 1.18 2011/03/31 22:19:18 alex Exp $ */
/*******************************************************************************

File:

    udp_util.c

    UDP Networking Utilities.


Author:    Alex Measday


Purpose:

    The UDP_UTIL package implements a high-level interface to UDP/IP
    network communications.  Under the UDP protocol, processes send
    "datagrams" to each other.  Unlike TCP, UDP has no concept of a
    "connection"; messages are individually routed to one or more
    destination endpoints through a single socket.  Also unlike TCP,
    UDP does not guarantee reliable transmission; messages may be
    lost or sent out of order.

    In the UDP_UTIL package, the endpoints of a connection-less UDP
    "connection" are called, well, "endpoints".  A given program may
    create an anonymous UDP endpoint bound to a system-assigned network
    port:

        udpCreate (NULL, NULL, &endpoint) ;

    or it may create a UDP endpoint bound to a predetermined network
    port (specified by name or port number):

        udpCreate ("<name>", NULL, &endpoint) ;

    Client processes generally create anonymous UDP endpoints for the
    purpose of sending messages to a server process at a predetermined
    network port.  With an anonymous endpoint, a client must let the
    server(s) know its port number before the client can receive messages
    from the server(s).  The act of sending a datagram to the server(s)
    automatically supplies the server(s) with the client's port number
    and IP address.

    By creating a UDP endpoint bound to a predetermined network port, a
    server is immediately ready to receive datagrams sent by clients to
    that port; the clients already know the port number, so there is no
    need for the server to send messages first.

    To send a datagram from one endpoint to another, you must specify the
    network address of the destination endpoint.  Since the destination
    endpoint probably belongs to another process, possibly on a remote host,
    the UDP_UTIL package requires you to create a "proxy" endpoint for the
    destination endpoint:

        udpCreate ("<name>[@<host>]", source, &destination) ;

    A proxy endpoint simply specifies the network address of the destination
    endpoint; the proxy endpoint is not bound to a network port and it has
    no operating system socket associated with it.  The proxy endpoint is
    internally linked with its source endpoint, so, when a datagram is sent
    to the proxy, udpWrite() automatically sends the datagram through the
    source endpoint to the destination.  A source endpoint may have many
    proxy endpoints, but a given proxy endpoint is only linked to a single
    source.  (If you have multiple source endpoints, you can create multiple
    proxy endpoints for the same destination.)

    When a datagram is read from an anonymous or predetermined endpoint,
    udpRead() returns the text of the datagram and a proxy endpoint for
    the sender of the datagram.  The proxy endpoint can be used to return
    a response to the sender:

        char  message[64], response[32] ;
        UdpEndpoint  me, you ;
        ...
						-- Read message.
        udpRead (me, -1.0, sizeof message, message, NULL, &you) ;
						-- Send response.
        udpWrite (you, -1.0, sizeof response, response) ;

    Although there is no harm in doing so, there is no need to delete proxy
    endpoints; they are automatically garbage-collected when their source
    endpoint is deleted.

    The following is a very simple server process that creates a UDP
    endpoint bound to a predetermined network port and then reads and
    displays messages received from clients:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "udp_util.h"			-- UDP utilities.

        main (int argc, char *argv[])
        {
            char  buffer[128] ;
            UdpEndpoint  client, server ;
						-- Create UDP endpoint.
            udpCreate ("<name>", NULL, &server) ;

            for ( ; ; ) {			-- Read and display messages.
                udpRead (server, -1.0, 128, buffer, NULL, &client) ;
                printf ("From %s: %s\n", udpName (client), buffer) ;
            }

        }

    The following client process creates an anonymous UDP endpoint and
    sends 16 messages through that endpoint to the server:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "udp_util.h"			-- UDP utilities.

        main (int argc, char *argv[])
        {
            char  buffer[128] ;
            int  i ;
            UdpEndpoint  client, server ;

            udpCreate (NULL, NULL, &client) ;	-- Create client and target.
            udpCreate ("<name>[@<host>]", client, &server) ;

            for (i = 0 ;  i < 16 ;  i++) {	-- Send messages.
                sprintf (buffer, "Hello for the %dth time!", i) ;
                udpWrite (server, -1.0, strlen (buffer) + 1, buffer) ;
            }

            udpDestroy (client) ;		-- Deletes client and target.

        }

    Note that "client" is the anonymous endpoint and "server" is a proxy
    for the destination endpoint.


Notes:

    These functions are reentrant under VxWorks (except for the global
    debug flag).


Public Procedures (* defined as macros):

    udpCreate() - creates a UDP endpoint.
    udpDestroy() - destroys a UDP endpoint.
    udpFd() - returns the file descriptor for an endpoint's socket.
    udpIsReadable() - checks if a datagram is waiting to be read.
    udpIsUp() - checks if an endpoint is up.
    udpIsWriteable() - checks if a datagram can be written.
    udpName() - returns the name of an endpoint.
    udpRead() - reads a datagram.
  * udpSetBuf() - changes the sizes of an endpoint's receive and send buffers.
    udpWrite() - sends a datagram.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */

#if defined(VMS)
#    include  <netdb.h>			/* Network database definitions. */
#    include  <socket.h>		/* Socket-related definitions. */
#    ifndef UCX$C_UDP
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
#include  "udp_util.h"			/* UDP networking utilities. */

#ifndef MAXHOSTNAMELEN
#    define  MAXHOSTNAMELEN  64
#endif


/*******************************************************************************
    UDP Endpoints - contain information about local and remote UDP sockets.
        A "local" (to the application) endpoint represents a UDP socket on
        which the application can receive UDP datagrams.  A "remote" (to
        the application) endpoint has no socket and simply specifies the
        source address of a datagram received by the application or the
        destination address of a datagram being sent by the application.
        Socket-less remote endpoints are linked to their "parent" local
        endpoint; when the parent is deleted, the children are automatically
        deleted.
*******************************************************************************/

typedef  struct  _UdpEndpoint {
    char  *name ;			/* "<port>[@<host>]" */
    struct  sockaddr  address ;		/* Network address and port number. */
    int  addressLength ;		/* Length (in bytes) of address. */
    IoFd  fd ;				/* UDP socket. */
    struct  _UdpEndpoint  *parent ;	/* Local parent of remote endpoint. */
    struct  _UdpEndpoint  *next ;	/* Link to first child or next sibling. */
}  _UdpEndpoint ;


int  udp_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  udp_util_debug

/*!*****************************************************************************

Procedure:

    udpCreate ()

    Create a UDP Endpoint.


Purpose:

    The udpCreate() function creates any one of three types of UDP endpoints:

        (1) a UDP socket bound to a system-chosen network port,

        (2) a UDP socket bound to a predetermined network port
            (identified by port name or number), and

        (3) a socket-less UDP endpoint used to specify the target
            of a datagram being sent.

    The anonymous and predetermined endpoints (#'s 1 and 2 above) should
    be closed by a call to udpDestroy() when they are no longer needed.
    Proxy endpoints (#3 above) are linked upon creation to an existing
    anonymous or predetermined endpoint (i.e., its "parent").  When a
    parent endpoint is closed, all of its "children" (i.e., the associated
    proxy endpoints) are automatically deleted.


    Invocation (anonymous endpoint):

        status = udpCreate (NULL, NULL, &endpoint) ;

    Invocation (predetermined endpoint):

        status = udpCreate (serverName, NULL, &endpoint) ;

    Invocation (proxy endpoint):

        status = udpCreate (targetName, parent, &endpoint) ;

    where

        <serverName>	- I
            is the server's name.  This is used for determining the port
            associated with the server (via the system's name/port mappings).
            You can side-step the system maps and explicitly specify a
            particular port by passing in a decimal number encoded in ASCII
            (e.g., "1234" for port 1234).
        <targetName>	- I
            is the target's name: "<server>[@<host>]".  The server can be
            specified as a name or as a port number; see the "serverName"
            argument description above.  The host, if given, can be
            specified as a name or as a dotted Internet address.
        <endpoint>	- O
            returns a handle for the UDP endpoint.
        <status>	- O
            returns the status of creating the UDP endpoint, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  udpCreate (

#    if  PROTOTYPES
        const  char  *name,
        UdpEndpoint  parent,
        UdpEndpoint  *endpoint)
#    else
        name, parent, endpoint)

        char  *name ;
        UdpEndpoint  parent ;
        UdpEndpoint  *endpoint ;
#    endif

{    /* Local variables. */
    char  *s, hostName[MAXHOSTNAMELEN+1], serverName[MAXHOSTNAMELEN+1] ;
    int  length, portNumber ;
    struct  sockaddr_in  socketName ;





/*******************************************************************************
    Construct the endpoint's network address.
*******************************************************************************/

/* Parse the host and server names.  If the host name is not defined
   explicitly, it defaults to the local host. */

    s = (char *) netHostOf (netAddrOf (NULL), false) ;
    if (s == NULL) {
        LGE "(udpCreate) Error getting local host name.\nnetHostOf: ") ;
        return (errno) ;
    }
    strcpy (hostName, s) ;

    if (name == NULL) {				/* Let system choose port #? */
        strcpy (serverName, "0") ;
    } else {					/* User-specified port #. */
        s = strchr (name, '@') ;
        if (s == NULL) {			/* "<server>" */
            strcpy (serverName, name) ;
        } else {				/* "<server>@<host>" */
            length = s - name ;
            strncpym (serverName, name, length, sizeof serverName) ;
            strcpy (hostName, ++s) ;
        }
    }

/* Lookup the port number bound to the server name. */

    portNumber = netPortOf (serverName, "udp") ;
    if (portNumber == -1) {
        LGE "(udpCreate) Error getting server entry for %s.\nnetPortOf: ",
            serverName) ;
        return (errno) ;
    }

/* Set up the network address for the endpoint. */

    memset (&socketName, '\0', sizeof socketName) ;
    socketName.sin_family = AF_INET ;
    socketName.sin_port = htons (portNumber) ;

    socketName.sin_addr.s_addr = netAddrOf (hostName) ;
    if (socketName.sin_addr.s_addr == 0) {
        LGE "(udpCreate) Error getting host entry for %s.\nnetAddrOf: ",
            hostName) ;
        return (errno) ;
    }


/*******************************************************************************
    Create a UDP endpoint structure.
*******************************************************************************/

    *endpoint = (UdpEndpoint) malloc (sizeof (_UdpEndpoint)) ;
    if (*endpoint == NULL) {
        LGE "(udpCreate) Error allocating endpoint structure for %s.\nmalloc: ",
            serverName) ;
        return (errno) ;
    }

    (*endpoint)->name = NULL ;
    *((struct sockaddr_in *) &(*endpoint)->address) = socketName ;
    (*endpoint)->addressLength = sizeof socketName ;
    (*endpoint)->fd = INVALID_SOCKET ;
    (*endpoint)->parent = parent ;
    (*endpoint)->next = NULL ;


/*******************************************************************************
    If a UDP socket is to be created, then create the socket and bind it to
    the specified port number.
*******************************************************************************/

    if (parent == NULL) {

/* Create a socket for the endpoint. */

        (*endpoint)->fd = socket (AF_INET, SOCK_DGRAM, 0) ;
        if (!VALID_FD ((*endpoint)->fd)) {
            SET_ERRNO (GET_NETERRNO ()) ;
            LGE "(udpCreate) Error creating socket for %s.\nsocket: ",
                serverName) ;
            PUSH_ERRNO ;
            udpDestroy (*endpoint) ;  *endpoint = NULL ;
            POP_ERRNO ;
            return (errno) ;
        }

/* Bind the network address to the socket. */

        if (bind ((*endpoint)->fd,
                  &(*endpoint)->address, (*endpoint)->addressLength)) {
            SET_ERRNO (GET_NETERRNO ()) ;
            LGE "(udpCreate) Error binding %s's socket name.\nbind: ",
                serverName) ;
            PUSH_ERRNO ;
            udpDestroy (*endpoint) ;  *endpoint = NULL ;
            POP_ERRNO ;
            return (errno) ;
        }

        sprintf (serverName, "%u", sktPort ((*endpoint)->fd)) ;

    }


/*******************************************************************************
    Otherwise, if a socket-less, proxy endpoint is being created, then
    simply link it to its parent endpoint.
*******************************************************************************/

    else {

        (*endpoint)->next = parent->next ;
        parent->next = *endpoint ;

    }


/*******************************************************************************
    Construct the endpoint's name.
*******************************************************************************/

    (*endpoint)->name = malloc (strlen (serverName) + 1 +
                                strlen (hostName) + 1) ;
    if ((*endpoint)->name == NULL) {
        LGE "(udpCreate) Error duplicating server name: %s%c%s\nmalloc: ",
            serverName, (parent == NULL) ? '#' : '@', hostName) ;
        PUSH_ERRNO ;
        udpDestroy (*endpoint) ;  *endpoint = NULL ;
        POP_ERRNO ;
        return (errno) ;
    }
    sprintf ((*endpoint)->name, "%s%c%s",
             serverName, (parent == NULL) ? '#' : '@', hostName) ;


    LGI "(udpCreate) Created %s, socket %d.\n",
        (*endpoint)->name, (*endpoint)->fd) ;

    return (0) ;	/* Successful completion. */

}

/*!*****************************************************************************

Procedure:

    udpDestroy ()

    Destroy a UDP Endpoint.


Purpose:

    The udpDestroy() function destroys a UDP endpoint.  If the endpoint
    is a socket bound to a network port, the socket is closed.  If the
    endpoint is a socket-less, proxy endpoint, the endpoint is simply
    unlinked from the socket endpoint (i.e., its "parent") with which
    it is associated.


    Invocation:

        status = udpDestroy (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by udpCreate() or udpRead().
        <status>	- O
            returns the status of destroying the endpoint, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  udpDestroy (

#    if  PROTOTYPES
        UdpEndpoint  endpoint)
#    else
        endpoint)

        UdpEndpoint  endpoint ;
#    endif

{    /* Local variables. */
    UdpEndpoint  prev ;



    if (endpoint == NULL)  return (0) ;

    LGI "(udpDestroy) Destroying %s, socket %d ...\n",
        endpoint->name, endpoint->fd) ;

/* If this is a socket endpoint, then delete all of the socket-less proxy
   endpoints associated with the endpoint. */

    if (endpoint->parent == NULL) {
        while (endpoint->next != NULL)
            udpDestroy (endpoint->next) ;
        CLOSESOCKET (endpoint->fd) ;
        endpoint->fd = INVALID_SOCKET ;
    }

/* Otherwise, if this is a socket-less proxy, then unlink the proxy endpoint
   from its socket endpoint (i.e., its "parent"). */

    else {
        for (prev = endpoint->parent ;  prev != NULL ;  prev = prev->next)
            if (prev->next == endpoint)  break ;
        if (prev != NULL)  prev->next = endpoint->next ;
    }

/* Deallocate the endpoint structure. */

    if (endpoint->name != NULL) {
        free (endpoint->name) ;  endpoint->name = NULL ;
    }
    free (endpoint) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    udpFd ()

    Get a UDP Endpoint's Socket.


Purpose:

    Function udpFd() returns a UDP endpoint's socket.


    Invocation:

        fd = udpFd (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by udpCreate() or udpRead().
        <fd>		- O
            returns the UNIX file descriptor for the endpoint's socket.

*******************************************************************************/


IoFd  udpFd (

#    if PROTOTYPES
        UdpEndpoint  endpoint)
#    else
        endpoint)

        UdpEndpoint  endpoint ;
#    endif

{
    return ((endpoint == NULL) ? INVALID_SOCKET : endpoint->fd) ;
}

/*!*****************************************************************************

Procedure:

    udpIsReadable ()

    Check if Data is Waiting to be Read.


Purpose:

    The udpIsReadable() function checks to see if data is waiting to
    be read from a UDP endpoint.


    Invocation:

        isReadable = udpIsReadable (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by udpCreate().
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  udpIsReadable (

#    if PROTOTYPES
        UdpEndpoint  endpoint)
#    else
        endpoint)

        UdpEndpoint  endpoint ;
#    endif

{

    if (endpoint == NULL)
        return (false) ;
    else if (endpoint->parent == NULL)
        return (sktIsReadable (endpoint->fd)) ;
    else
        return (sktIsReadable ((endpoint->parent)->fd)) ;

}

/*!*****************************************************************************

Procedure:

    udpIsUp ()

    Check if a UDP Endpoint is Up.


Purpose:

    The udpIsUp() function checks to see if a UDP endpoint is still up.


    Invocation:

        isUp = udpIsUp (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by udpCreate().
        <isUp>		- O
            returns true (a non-zero value) if the endpoint is up and
            false (zero) otherwise.

*******************************************************************************/


bool  udpIsUp (

#    if PROTOTYPES
        UdpEndpoint  endpoint)
#    else
        endpoint)

        UdpEndpoint  endpoint ;
#    endif

{

    if (endpoint == NULL)
        return (false) ;
    else if (endpoint->parent == NULL)
        return (sktIsUp (endpoint->fd)) ;
    else
        return (sktIsUp ((endpoint->parent)->fd)) ;

}

/*!*****************************************************************************

Procedure:

    udpIsWriteable ()

    Check if Data can be Written.


Purpose:

    The udpIsWriteable() function checks to see if data can be written
    to a UDP endpoint.


    Invocation:

        isWriteable = udpIsWriteable (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by udpCreate().
        <isWriteable>	- O
            returns true (a non-zero value) if the endpoint is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  udpIsWriteable (

#    if PROTOTYPES
        UdpEndpoint  endpoint)
#    else
        endpoint)

        UdpEndpoint  endpoint ;
#    endif

{

    if (endpoint == NULL)
        return (false) ;
    else if (endpoint->parent == NULL)
        return (sktIsWriteable (endpoint->fd)) ;
    else
        return (sktIsWriteable ((endpoint->parent)->fd)) ;

}

/*!*****************************************************************************

Procedure:

    udpName ()

    Get a UDP Endpoint's Name.


Purpose:

    Function udpName() returns a UDP endpoint's name.


    Invocation:

        name = udpName (endpoint) ;

    where

        <endpoint>	- I
            is the endpoint handle returned by udpCreate() or udpRead().
        <name>	- O
            returns the endpoint's name.  The name is stored in memory
            local to the UDP utilities and it should not be modified or
            freed by the caller.

*******************************************************************************/


const  char  *udpName (

#    if PROTOTYPES
        UdpEndpoint  endpoint)
#    else
        endpoint)

        UdpEndpoint  endpoint ;
#    endif

{
    if (endpoint == NULL)  return ("") ;
    if (endpoint->name == NULL)  return ("") ;
    return (endpoint->name) ;
}

/*!*****************************************************************************

Procedure:

    udpRead ()

    Read Data from a UDP Endpoint.


Purpose:

    Function udpRead() reads the next message on a UDP endpoint.  A timeout
    can be specified that limits how long udpRead() waits for the message
    to be received.


    Invocation:

        status = udpRead (endpoint, timeout, maxBytesToRead,
                          buffer, &numBytesRead, &source) ;

    where

        <endpoint>		- I
            is the endpoint handle returned by udpCreate().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the caller
            wishes to wait for the next message to be received.  A fractional
            time can be specified; e.g., 2.5 seconds.  A negative timeout
            (e.g., -1.0) causes an infinite wait; a zero timeout (0.0) allows
            a read only if message is immediately available.
        <maxBytesToRead>	- I
            specifies the maximum number of bytes to read; i.e., the size of
            the message buffer.
        <buffer>		- O
            receives the input data.  This buffer should be at least
            maxBytesToRead in size.
        <numBytesRead>		- O
            returns the actual number of bytes read.
        <source>		- O
            returns an endpoint handle for the source of the message.
        <status>		- O
            returns the status of reading from the endpoint: zero if no
            errors occurred, EWOULDBLOCK if the timeout interval expired
            before a message was received, and ERRNO otherwise.

*******************************************************************************/


errno_t  udpRead (

#    if PROTOTYPES
        UdpEndpoint  endpoint,
        double  timeout,
        size_t  maxBytesToRead,
        char  *buffer,
        size_t  *numBytesRead,
        UdpEndpoint  *source)
#    else
        endpoint, timeout, maxBytesToRead, buffer, numBytesRead, source)

        UdpEndpoint  endpoint ;
        double  timeout ;
        size_t  maxBytesToRead ;
        char  *buffer ;
        size_t  *numBytesRead ;
        UdpEndpoint  *source ;
#    endif

{    /* Local variables. */
    char  *hostName ;
    char  sourceName[PATH_MAX+1] ;
    fd_set  readMask ;
    int  addressLength, length, numActive ;
    IoFd  fd ;
    struct  sockaddr  address ;
    struct  timeval  deltaTime, expirationTime ;
    UdpEndpoint  ep, sourcePoint ;





    if (endpoint == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(udpRead) NULL endpoint handle: ") ;
        return (errno) ;
    }

    fd = endpoint->fd ;
    if (!VALID_FD (fd)) {
        SET_ERRNO (EBADF) ;
        LGE "(udpRead) %d file descriptor: ", fd) ;
        return (errno) ;
    }


/*******************************************************************************
    If a timeout interval was specified, then wait until the expiration of
    the interval for a message to be received.
*******************************************************************************/

    if (timeout >= 0.0) {

/* Compute the expiration time as the current time plus the interval. */

        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;

/* Wait for the next message to arrive. */

        for ( ; ; ) {
            deltaTime = tvSubtract (expirationTime, tvTOD ()) ;
            FD_ZERO (&readMask) ;  FD_SET (fd, &readMask) ;
            numActive = select (fd+1, &readMask, NULL, NULL, &deltaTime) ;
            if (numActive >= 0)  break ;
            SET_ERRNO (GET_NETERRNO ()) ;
            if (errno == EINTR)  continue ;
            LGE "(udpRead) Error waiting for input on %s.\nselect: ",
                endpoint->name) ;
            return (errno) ;
        }

        if (numActive == 0) {
            SET_ERRNO (EWOULDBLOCK) ;
            LGE "(udpRead) Timeout while waiting for input on %s.\n",
                endpoint->name) ;
            return (errno) ;
        }

    }


/*******************************************************************************
    Read the message.
*******************************************************************************/

    addressLength = sizeof address ;
    length = recvfrom (fd, buffer, maxBytesToRead, 0,
                       &address, &addressLength) ;
    if (length < 0) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(udpRead) Error reading from %s.\nrecvfrom: ", endpoint->name) ;
        return (errno) ;
    } else if (length == 0) {
        SET_ERRNO (EPIPE) ;
        LGE "(udpRead) Broken connection on %s.\nrecvfrom: ", endpoint->name) ;
        return (errno) ;
    }

    if (numBytesRead != NULL)  *numBytesRead = length ;


/*******************************************************************************
    Create a UDP endpoint for the source of the message.
*******************************************************************************/

/* Check to see if an endpoint already exists for this particular source. */

    for (ep = endpoint->next ;  ep != NULL ;  ep = ep->next) {
        if ((addressLength == ep->addressLength) &&
            (memcmp (&address, &ep->address, addressLength) == 0))  break ;
    }

/* If not, create a brand new endpoint.  If so, then use the existing one. */

    if (ep == NULL) {
        hostName = (char *) netHostOf (
                    ((struct sockaddr_in *) &address)->sin_addr.s_addr, false) ;
        sprintf (sourceName, "%d@%s",
                 ntohs (((struct sockaddr_in *) &address)->sin_port),
                 hostName) ;
        if (udpCreate (sourceName, endpoint, &sourcePoint)) {
            LGE "(udpRead) Error creating source endpoint: %s\nudpCreate: ",
                sourceName) ;
            return (errno) ;
        }
    } else {
        sourcePoint = ep ;
    }

    if (source != NULL)  *source = sourcePoint ;


    LGI "(udpRead) Read %d bytes from %s on %s, socket %d.\n",
        length, sourcePoint->name, endpoint->name, endpoint->fd) ;
    if (udp_util_debug) {
        meoDumpX (stdout, "    ", 0, buffer, length) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    udpWrite ()

    Write Data to a UDP Endpoint.


Purpose:

    Function udpWrite() writes a message to a destination UDP endpoint.
    A timeout interval can be specified that limits how long udpWrite()
    waits to output the message.

    Note that a message is written through a local source endpoint to the
    remote destination endpoint.  Only the destination endpoint is passed
    to udpWrite(); udpWrite() will use the destination's "parent" as the
    source endpoint.


    Invocation:

        status = udpWrite (destination, timeout, numBytesToWrite, buffer) ;

    where

        <destination>		- I
            is the destination endpoint handle returned by udpCreate()
            or udpRead().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the data to be output.  A fractional
            time can be specified; e.g., 2.5 seconds.   A negative timeout
            (e.g., -1.0) causes an infinite wait; udpWrite() will wait as
            long as necessary to output all of the data.  A zero timeout
            (0.0) specifies no wait: if the socket is not ready for writing,
            udpWrite() returns immediately.
        <numBytesToWrite>	- I
            is the number of bytes to write.
        <buffer>		- O
            is the data to be output.
        <status>		- O
            returns the status of sending the message: zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired before
            the message could be sent, and ERRNO otherwise.

*******************************************************************************/


errno_t  udpWrite (

#    if PROTOTYPES
        UdpEndpoint  destination,
        double  timeout,
        size_t  numBytesToWrite,
        const  char  *buffer)
#    else
        destination, timeout, numBytesToWrite, buffer)

        UdpEndpoint  destination ;
        double  timeout ;
        size_t  numBytesToWrite ;
        const  char  *buffer ;
#    endif

{    /* Local variables. */
    fd_set  writeMask ;
    int  length, numActive ;
    IoFd  fd ;
    struct  timeval  deltaTime, expirationTime ;





    if (destination == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(udpWrite) NULL destination handle: ") ;
        return (errno) ;
    }

    fd = (destination->parent == NULL) ? destination->fd
                                       : (destination->parent)->fd ;
    if (!VALID_FD (fd)) {
        SET_ERRNO (EBADF) ;
        LGE "(udpWrite) %d file descriptor: ", fd) ;
        return (errno) ;
    }


/*******************************************************************************
    If a timeout interval was specified, then wait until the expiration
    of the interval for the endpoint's socket to be ready for writing.
*******************************************************************************/

    if (timeout >= 0.0) {

/* Compute the expiration time as the current time plus the interval. */

        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;

/* Wait for the endpoint to be ready for writing. */

        for ( ; ; ) {
            deltaTime = tvSubtract (expirationTime, tvTOD ()) ;
            FD_ZERO (&writeMask) ;  FD_SET (fd, &writeMask) ;
            numActive = select (fd+1, NULL, &writeMask, NULL, &deltaTime) ;
            if (numActive >= 0)  break ;
            SET_ERRNO (GET_NETERRNO ()) ;
            if (errno == EINTR)  continue ;
            LGE "(udpWrite) Error waiting to write to %s.\nselect: ",
                destination->name) ;
            return (errno) ;
        }

        if (numActive == 0) {
            SET_ERRNO (EWOULDBLOCK) ;
            LGE "(udpWrite) Timeout while waiting to write data to %s.\n",
                destination->name) ;
            return (errno) ;
        }

    }


/*******************************************************************************
    Send the message to the destination endpoint.
*******************************************************************************/

    length = sendto (fd, (char *) buffer, numBytesToWrite, 0,
                     &destination->address, destination->addressLength) ;
    if (length < 0) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(udpWrite) Error sending %d-byte message to %s.\nsendto: ",
            numBytesToWrite, destination->name) ;
        return (errno) ;
    }

    LGI "(udpWrite) Wrote %d bytes to %s, socket %d.\n",
        length, destination->name, fd) ;
    if (udp_util_debug) {
        meoDumpX (stdout, "    ", 0, buffer, length) ;
    }


    return (0) ;

}
