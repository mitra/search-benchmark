/* $Id: skt_util.h,v 1.12 2009/09/09 22:38:13 alex Exp alex $ */
/*******************************************************************************

    skt_util.h

    Socket Utility Definitions.

*******************************************************************************/

#ifndef  SKT_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  SKT_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if defined (_WIN32)
#    include  <winsock.h>			/* Windows socket definitions. */
#    include  <process.h>			/* Process control functions. */
    typedef  u_long  IoctlArg ;			/* 3rd argument to ioctlsocket(). */
    typedef  SOCKET  IoFd ;			/* Windows file descriptor. */
    typedef  int  socklen_t ;			/* Length of socket address. */
#    define  VALID_FD(fd)  (fd != INVALID_SOCKET)
#    define  CLOSESOCKET  closesocket
#    define  IOCTLSOCKET  ioctlsocket
#    define  GET_NETERRNO()  (WSAGetLastError ())
#    define  SET_NETERRNO(error)  WSASetLastError (error)
#    if _MSC_VER < 1600				/* Earlier than VS 2010? */
#        define  EWOULDBLOCK  WSAEWOULDBLOCK
#        define  EINPROGRESS  WSAEINPROGRESS
#    endif
#    ifndef WINSOCK_VERSION
#        ifndef  MAKEWORD
#            define  MAKEWORD(low,high)	\
                ((WORD)((BYTE)(low)) | (((WORD)(BYTE)(high))<<8)))
#        endif
#        define  WINSOCK_VERSION_MAJOR  1
#        define  WINSOCK_VERSION_MINOR  1
#        define  WINSOCK_VERSION	\
            MAKEWORD (WINSOCK_VERSION_MAJOR, WINSOCK_VERSION_MINOR)
#    endif
#elif defined (__palmos__)
#    include  <Unix/netinet_in.h>		/* htonl(3), ntohl(3), etc. */
#    include  <Unix/sys_socket.h>		/* UNIX socket definitions. */
#    include  <Unix/sys_time.h>			/* UNIX timeval structure. */
    typedef  long  IoctlArg ;			/* 3rd argument to ioctl(2). */
    typedef  NetSocketRef  IoFd ;		/* PalmOS file descriptor. */
    typedef  int  socklen_t ;
#    define  INVALID_SOCKET  (-1)
#    define  VALID_FD(fd)  (fd >= 0)
#    define  CLOSESOCKET  close
#    define  IOCTLSOCKET  ioctl
#    define  GET_NETERRNO()  errno
#    define  SET_NETERRNO(error)  (errno = error)
#    define  getpid()  0x1234
#else
#    include  <unistd.h>			/* UNIX I/O definitions. */
#    include  <netinet/in.h>			/* htonl(3), ntohl(3), etc. */
    typedef  long  IoctlArg ;			/* 3rd argument to ioctl(2). */
    typedef  int  IoFd ;			/* UNIX file descriptor. */
#    if defined(HAVE_SOCKLEN_T) && !HAVE_SOCKLEN_T
#        define  socklen_t  int
#    endif
#    define  INVALID_SOCKET  (-1)
#    define  VALID_FD(fd)  (fd >= 0)
#    ifdef NDS
#        define  CLOSESOCKET  closesocket
#    else
#        define  CLOSESOCKET  close
#    endif
#    define  IOCTLSOCKET  ioctl
#    define  GET_NETERRNO()  errno
#    define  SET_NETERRNO(error)  (errno = error)
#endif

#ifndef MAX_LISTEN_BACKLOG
#    define  MAX_LISTEN_BACKLOG  5
#endif


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  sktBlock P_((IoFd fd,
                              bool blocking))
    OCD ("skt_util") ;

extern  errno_t  sktCleanup P_((void))
    OCD ("skt_util") ;

extern  bool  sktIsReadable P_((IoFd fd))
    OCD ("skt_util") ;

extern  bool  sktIsUp P_((IoFd fd))
    OCD ("skt_util") ;

extern  bool  sktIsWriteable P_((IoFd fd))
    OCD ("skt_util") ;

extern  errno_t  sktPeek P_((IoFd fd,
                             size_t maxBytes,
                             char *buffer,
                             size_t *numBytes))
    OCD ("skt_util") ;

extern  const  char  *sktPeer P_((IoFd fd))
    OCD ("skt_util") ;

extern  unsigned  int  sktPort P_((IoFd fd))
    OCD ("skt_util") ;

extern  errno_t  sktSetBuf P_((IoFd fd,
                               int receiveSize,
                               int sendSize))
    OCD ("skt_util") ;

extern  errno_t  sktStartup P_((void))
    OCD ("skt_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
