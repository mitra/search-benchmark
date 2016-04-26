/* $Id: skt_util.c,v 1.21 2011/07/18 17:41:30 alex Exp alex $ */
/*******************************************************************************

File:

    skt_util.c

    Socket Utilities.


Author:    Alex Measday


Purpose:

    The SKT_UTIL package contains a number of common functions used by
    my other networking packages: TCP_UTIL, UDP_UTIL, ...


Public Procedures:

    sktBlock() - configures a socket for blocking or non-blocking I/O.
    sktCleanup() - shuts down the socket library.
    sktIsReadable() - checks if data is waiting to be read on a socket.
    sktIsUp() - checks if a socket is up.
    sktIsWriteable() - checks if data can be written to a socket.
    sktPeek() - peeks at a socket's pending input data.
    sktPeer() - returns the name of the host at the other end of a socket.
    sktPort() - returns the port number of a socket.
    sktSetBuf() - changes the sizes of a socket's receive and send buffers.
    sktStartup() - starts up the socket library.

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
#elif defined(__palmos__)
#    include  <SystemMgr.h>		/* System Manager API. */
#elif !defined(_WIN32)
#    include  <netdb.h>			/* Network database definitions. */
#    include  <sys/types.h>		/* System type definitions. */
#    if !defined(HAVE_IOCTL_H) || HAVE_IOCTL_H
#        include  <sys/ioctl.h>		/* I/O control definitions. */
#    endif
#    include  <sys/socket.h>		/* Socket-related definitions. */
#    include  <sys/time.h>		/* System time definitions. */
#endif

#include  "net_util.h"			/* Networking utilities. */
#include  "skt_util.h"			/* TCP/IP networking utilities. */

/*!*****************************************************************************

Procedure:

    sktBlock ()

    Configure a Socket for Blocking or Non-Blocking I/O.


Purpose:

    Function sktBlock() configures a socket for blocking or non-blocking I/O.


    Invocation:

        status = sktBlock (fd, blocking) ;

    where

        <fd>		- I
            is the UNIX file descriptor for the socket.
        <blocking>	- I
            specifies whether the socket is to be configured for blocking I/O
            (true) or non-blocking I/O (false).
        <status>	- O
            returns the status of configuring the socket, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  sktBlock (

#    if PROTOTYPES
        IoFd  fd,
        bool  blocking)
#    else
        fd, blocking)

        IoFd  fd ;
        bool  blocking ;
#    endif

{    /* Local variables. */
    IoctlArg  optval ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (errno) ;
    }

/* Configure the socket with the desired I/O mode. */

    optval = blocking ? 0 : 1 ;
#ifdef __palmos__
    if (setsockopt (fd, SOL_SOCKET, netSocketOptSockNonBlocking,
                    &optval, sizeof optval)) {
#else
    if (IOCTLSOCKET (fd, FIONBIO, &optval) == -1) {
#endif
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(sktBlock) Error configuring socket %d for %s I/O.\nioctl: ",
            fd, blocking ? "blocking" : "non-blocking") ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sktCleanup ()

    Shuts Down the Socket Library.


Purpose:

    The sktCleanup() function shuts down the socket library on platforms
    that require it (e.g., Windows).


    Invocation:

        status = sktCleanup () ;

    where

        <status>	- O
            returns the status of shutting down the library, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  sktCleanup (

#    if PROTOTYPES
        void)
#    else
        )
#    endif

{
#if defined (_WIN32)
    if (WSACleanup ()) {				/* WINSOCK shutdown. */
        SET_ERRNO (GET_NETERRNO ()) ;
        return (errno) ;
    }
#elif defined(__palmos__)
    SET_ERRNO (NetLibClose (AppNetRefnum, false)) ;
    LGE "(sktCleanup) NetLibClose() = %d\n", (int) errno) ;
    if (errno)  return (errno) ;
#endif
    return (0) ;
}

/*!*****************************************************************************

Procedure:

    sktIsReadable ()

    Check if Data is Waiting to be Read from a Socket.


Purpose:

    The sktIsReadable() function checks to see if data is waiting to
    be read from a socket.


    Invocation:

        isReadable = sktIsReadable (fd) ;

    where

        <fd>		- I
            is the UNIX file descriptor for the socket.
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  sktIsReadable (

#    if PROTOTYPES
        IoFd  fd)
#    else
        fd)

        IoFd  fd ;
#    endif

{    /* Local variables. */
    fd_set  readMask ;
    size_t  length ;
    struct  timeval  timeout ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (false) ;
    }

/* Poll the data socket for input. */

    for ( ; ; ) {
        FD_ZERO (&readMask) ;  FD_SET (fd, &readMask) ;
        timeout.tv_sec = timeout.tv_usec = 0 ;	/* No wait. */
        if (select (fd+1, &readMask, NULL, NULL, &timeout) >= 0)  break ;
        SET_ERRNO (GET_NETERRNO ()) ;
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(sktIsReadable) Error polling socket %d.\nselect: ", fd) ;
        return (false) ;
    }
						/* No input pending? */
    if (!FD_ISSET (fd, &readMask))  return (false) ;

/* Input is pending.  Find out how many bytes of data are actually available
   for input.  If SELECT(2) indicates pending input, but IOCTL(2) indicates
   zero bytes of pending input, the connection is broken. */

    if (sktPeek (fd, 0, NULL, &length)) {
        LGE "(sktIsReadable) Error polling socket %d.\nsktPeek: ", fd) ;
        return (false) ;
    }

    if (length > 0) {
        return (true) ;				/* Pending input. */
    } else {
        SET_ERRNO (EPIPE) ;
        LGE "(sktIsReadable) Broken connection to socket %d.\n", fd) ;
        return (false) ;			/* EOF. */
    }

}

/*!*****************************************************************************

Procedure:

    sktIsUp ()

    Check if a Connection is Up.


Purpose:

    The sktIsUp() function checks to see if a network connection is still up.


    Invocation:

        isUp = sktIsUp (fd) ;

    where

        <fd>		- I
            is the UNIX file descriptor for the socket.
        <isUp>		- O
            returns true (a non-zero value) if the network connection is
            up and false (zero) otherwise.

*******************************************************************************/


bool  sktIsUp (

#    if PROTOTYPES
        IoFd  fd)
#    else
        fd)

        IoFd  fd ;
#    endif

{    /* Local variables. */
    fd_set  readMask ;
    size_t  length ;
    struct  timeval  timeout ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (false) ;
    }

/* Poll the data socket for input. */

    for ( ; ; ) {
        FD_ZERO (&readMask) ;  FD_SET (fd, &readMask) ;
        timeout.tv_sec = timeout.tv_usec = 0 ;	/* No wait. */
        if (select (fd+1, &readMask, NULL, NULL, &timeout) >= 0)  break ;
        SET_ERRNO (GET_NETERRNO ()) ;
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(sktIsUp) Error polling socket %d.\nselect: ", fd) ;
        return (false) ;			/* Connection is down. */
    }

    if (!FD_ISSET (fd, &readMask))		/* No input pending? */
        return (true) ;				/* Connection is up. */

/* Input is pending.  Find out how many bytes of data are actually available
   for input.  If SELECT(2) indicates pending input, but IOCTL(2) indicates
   zero bytes of pending input, the connection is broken. */

    if (sktPeek (fd, 0, NULL, &length)) {
        LGE "(sktIsUp) Error polling socket %d.\nsktPeek: ", fd) ;
        return (false) ;
    }

    if (length > 0) {				/* Pending input? */
        return (true) ;				/* Connection is up. */
    } else {
        SET_ERRNO (EPIPE) ;
        LGE "(sktIsUp) Broken connection to socket %d.\n", fd) ;
        return (false) ;			/* Connection is down. */
    }

}

/*!*****************************************************************************

Procedure:

    sktIsWriteable ()

    Check if Data can be Written.


Purpose:

    The sktIsWriteable() function checks to see if data can be written
    to a connection.


    Invocation:

        isWriteable = sktIsWriteable (fd) ;

    where

        <fd>		- I
            is the UNIX file descriptor for the socket.
        <isWriteable>	- O
            returns true (a non-zero value) if data connection is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  sktIsWriteable (

#    if PROTOTYPES
        IoFd  fd)
#    else
        fd)

        IoFd  fd ;
#    endif

{    /* Local variables. */
    fd_set  writeMask ;
    struct  timeval  timeout ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (false) ;
    }

/* Poll the data socket for output. */

    for ( ; ; ) {
        FD_ZERO (&writeMask) ;  FD_SET (fd, &writeMask) ;
        timeout.tv_sec = timeout.tv_usec = 0 ;	/* No wait. */
        if (select (fd+1, NULL, &writeMask, NULL, &timeout) >= 0)  break ;
        SET_ERRNO (GET_NETERRNO ()) ;
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(sktIsWriteable) Error polling socket %d.\nselect: ", fd) ;
        return (false) ;
    }
						/* Ready for output? */
    return (FD_ISSET (fd, &writeMask)) ;

}

/*!*****************************************************************************

Procedure:

    sktPeek ()

    Peek at a Socket's Pending Input.


Purpose:

    Function sktPeek() peeks at a socket's pending input.


    Invocation:

        status = sktPeek (fd, maxBytes, buffer, &numBytes) ;

    where

        <fd>		- I
            is the UNIX file descriptor for the socket.
        <maxBytes>	- I
            specifies the maximum number of bytes to pull from the input.
        <buffer>	- O
            receives the pending input data.  This buffer should be at least
            maxBytes in size.
        <numBytes>	- O
            returns the number of bytes actually copied into the buffer.
        <status>	- O
            returns the status of peeking at the pending input, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  sktPeek (

#    if PROTOTYPES
        IoFd  fd,
        size_t  maxBytes,
        char  *buffer,
        size_t  *numBytes)
#    else
        fd, maxBytes, buffer, numBytes)

        IoFd  fd ;
        size_t  maxBytes ;
        char  *buffer ;
        size_t  *numBytes ;
#    endif

{    /* Local variables. */
    int  byteCount ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (errno) ;
    }

    if (numBytes != NULL)  *numBytes = 0 ;

/* If no buffer was specified, then just check how many bytes are available
   to be read. */

    if (buffer == NULL) {
#if defined(HAVE_IOCTL) && !HAVE_IOCTL
        static  char  character ;
        buffer = &character ;
        maxBytes = 1 ;
#else
        IoctlArg  length ;
        while (IOCTLSOCKET (fd, FIONREAD, &length) == -1) {
            SET_ERRNO (GET_NETERRNO ()) ;
            if (errno == EINTR)  continue ;	/* Retry on signal interrupt. */
            LGE "(sktPeek) Error polling socket %d.\nioctl: ", fd) ;
            return (errno) ;
        }
        if (numBytes != NULL)  *numBytes = length ;
        return (0) ;
#endif
    }

/* Peek at the pending input data. */

    byteCount = recv (fd, buffer, maxBytes, MSG_PEEK) ;
    if (byteCount < 0) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(sktPeek) Error peeking at socket %d's pending input.\nrecv: ",
            fd) ;
        return (errno) ;
    }

    if (numBytes != NULL)  *numBytes = (size_t) byteCount ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sktPeer ()

    Get a Socket's Peer Name.


Purpose:

    Function sktPeer() returns the name (i.e., the Internet address) of
    the host at the other end of a network socket connection.


    Invocation:

        host = sktPeer (fd) ;

    where

        <fd>	- I
            is the UNIX file descriptor for the socket.
        <host>	- O
            returns the Internet address of the connected host.  The address
            is formatted in ASCII using the standard Internet dot notation:
            "a.b.c.d".  NULL is returned in the event of an error.  The
            ASCII host string is stored local to sktPeer() and it should
            be used or duplicated before calling sktPeer() again.

*******************************************************************************/


const  char  *sktPeer (

#    if PROTOTYPES
        IoFd  fd)
#    else
        fd)

        IoFd  fd ;
#    endif

{    /* Local variables. */
    struct  sockaddr  peerAddress ;
    struct  sockaddr_in  *ipAddress ;
    socklen_t  length ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (NULL) ;
    }

/* Get the IP address of the host on the other end of the network connection. */

    length = sizeof (peerAddress) ;
    if (getpeername (fd, &peerAddress, &length)) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(net_peer) Error getting peer's host for socket %d.\ngetpeername: ",
            fd) ;
        return (NULL) ;
    }
    ipAddress = (struct sockaddr_in *) &peerAddress ;

/* Convert the peer's IP address to a host name. */

    return (netHostOf (ipAddress->sin_addr.s_addr, false)) ;

}

/*!*****************************************************************************

Procedure:

    sktPort ()

    Get a Socket's Port Number.


Purpose:

    Function sktPort() returns the number of the port to which a socket
    (either listening or data) is bound.


    Invocation:

        number = sktPort (fd) ;

    where

        <fd>		- I
            is the UNIX file descriptor for the socket.
        <number>	- O
            returns the socket's port number.

*******************************************************************************/


unsigned  int  sktPort (

#    if PROTOTYPES
        IoFd  fd)
#    else
        fd)

        IoFd  fd ;
#    endif

{    /* Local variables. */
    struct  sockaddr_in  socketName ;
    socklen_t  length ;



    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (0) ;
    }

/* Get the socket's port number and convert it to host byte ordering. */

    length = sizeof socketName ;
    if (getsockname (fd, (struct sockaddr *) &socketName, &length)) {
        SET_ERRNO (GET_NETERRNO ()) ;
        LGE "(sktPort) Error getting port number for socket %d.\ngetsockname: ",
            fd) ;
        return (0) ;
    }

    return ((unsigned int) ntohs (socketName.sin_port)) ;

}

/*!*****************************************************************************

Procedure:

    sktSetBuf ()

    Change the Sizes of a Socket's Receive and Send Buffers.


Purpose:

    Function sktSetBuf() changes the sizes of a socket's receive and/or send
    buffers.


    Invocation:

        status = sktSetBuf (fd, receiveSize, sendSize) ;

    where

        <name>		- I
            is the name of the socket for debugging purposes; this argument
            can be NULL.
        <fd>		- I
            is the UNIX file descriptor for the socket.
        <receiveSize>	- I
            specifies the new size of the socket's receive system buffer.
            If this argument is less than zero, the receive buffer retains
            its current size.
        <sendSize>	- I
            specifies the new size of the socket's send system buffer.  If
            this argument is less than zero, the send buffer retains its
            current size.
        <status>	- O
            returns the status of changing the buffers' sizes, zero if no
            error occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  sktSetBuf (

#    if PROTOTYPES
        IoFd  fd,
        int  receiveSize,
        int  sendSize)
#    else
        fd, receiveSize, sendSize)

        IoFd  fd ;
        int  receiveSize ;
        int  sendSize ;
#    endif

{

    if (!VALID_FD (fd)) {			/* Socket not open yet? */
        SET_ERRNO (EBADF) ;
        return (errno) ;
    }

/* Change the size of the socket's receive system buffer. */

    if ((receiveSize >= 0) &&
        setsockopt (fd, SOL_SOCKET, SO_RCVBUF,
                    (void *) &receiveSize, sizeof (int))) {
        LGE "(sktSetBuf) Error setting receive buffer size (%d) for socket %d.\nsetsockopt: ",
            receiveSize, fd) ;
        return (errno) ;
    }

/* Change the size of the socket's send system buffer. */

    if ((sendSize >= 0) &&
        setsockopt (fd, SOL_SOCKET, SO_SNDBUF,
                    (void *) &sendSize, sizeof (int))) {
        LGE "(sktSetBuf) Error setting send buffer size (%d) for socket %d.\nsetsockopt: ",
            sendSize, fd) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sktStartup ()

    Starts Up the Socket Library.


Purpose:

    The sktStartup() function starts up the socket library on platforms
    that require it (e.g., Windows).


    Invocation:

        status = sktStartup () ;

    where

        <status>	- O
            returns the status of starting up the library, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  sktStartup (

#    if PROTOTYPES
        void)
#    else
        )
#    endif

{
#if defined(_WIN32)
    static  WSADATA  wsData ;
    if (WSAStartup (WINSOCK_VERSION, &wsData)) {	/* WINSOCK startup. */
        SET_ERRNO (GET_NETERRNO ()) ;
        return (errno) ;
    }
#elif defined(__palmos__)
    UInt16  firstError, i, instance, length ;
    UInt32  creator, ipAddress ;
    SET_ERRNO (SysLibFind ("Net.lib", &AppNetRefnum)) ;
    HostFPrintF (HostLogFile(), "(sktStartup) SysLibFind() = %d\n", (int) errno) ;
    if (errno)  return (errno) ;
    SET_ERRNO (NetLibOpen (AppNetRefnum, &firstError)) ;
    HostFPrintF (HostLogFile(), "(sktStartup) NetLibOpen() = %d (%u)\n", (int) errno, firstError) ;
    if (errno && (errno != netErrAlreadyOpen))  return (errno) ;
    AppNetTimeout = SysTicksPerSecond () * 20 ;		/* N-second timeout. */
    for (i = 0 ;  ;  i++) {
        if (NetLibIFGet (AppNetRefnum, i, &creator, &instance))  break ;
        length = sizeof (UInt32) ;
        if (NetLibIFSettingGet (AppNetRefnum, creator, instance,
                                netIFSettingActualIPAddr, (void *) &ipAddress,
                                &length))  break ;
        HostFPrintF (HostLogFile(), "(sktStartup) %lu:  Creator: %lu  Instance: %lu  IP: %s\n",
                     (unsigned long) i, creator, (unsigned long) instance,
                     netHostOf (ipAddress, true)) ;
    }
    if (sethostname ("172.16.1.202", 11))  return (errno) ;
#endif
    return (0) ;
}
