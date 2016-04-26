/* $Id: tcp_util.h,v 1.13 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    tcp_util.h

    TCP Utility Definitions.

*******************************************************************************/

#ifndef  TCP_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  TCP_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "skt_util.h"			/* Socket support functions. */


/*******************************************************************************
    TCP Network Endpoint (Client View) and Definitions.
*******************************************************************************/

					/* Endpoint handle. */
typedef  struct  _TcpEndpoint  *TcpEndpoint ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  tcp_util_debug  OCD ("tcp_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  tcpAnswer P_((TcpEndpoint listeningPoint,
                               double timeout,
                               TcpEndpoint *dataPoint))
    OCD ("tcp_util") ;

extern  errno_t  tcpCall P_((const char *serverName,
                             bool noWait,
                             TcpEndpoint *dataPoint))
    OCD ("tcp_util") ;

extern  errno_t  tcpComplete P_((TcpEndpoint dataPoint,
                                 double timeout,
                                 bool destroyOnError))
    OCD ("tcp_util") ;

extern  errno_t  tcpDestroy P_((TcpEndpoint port))
    OCD ("tcp_util") ;

extern  IoFd  tcpFd P_((TcpEndpoint endpoint))
    OCD ("tcp_util") ;

extern  bool  tcpIsReadable P_((TcpEndpoint dataPoint))
    OCD ("tcp_util") ;

extern  bool  tcpIsUp P_((TcpEndpoint dataPoint))
    OCD ("tcp_util") ;

extern  bool  tcpIsWriteable P_((TcpEndpoint dataPoint))
    OCD ("tcp_util") ;

extern  errno_t  tcpListen P_((const char *portName,
                               int backlog,
                               TcpEndpoint *listeningPoint))
    OCD ("tcp_util") ;

extern  const  char  *tcpName P_((TcpEndpoint endpoint))
    OCD ("tcp_util") ;

extern  errno_t  tcpRead P_((TcpEndpoint dataPoint,
                             double timeout,
                             ssize_t numBytesToRead,
                             char *buffer,
                             size_t *numBytesRead))
    OCD ("tcp_util") ;

extern  bool  tcpRequestPending P_((TcpEndpoint listeningPoint))
    OCD ("tcp_util") ;

extern  errno_t  tcpWrite P_((TcpEndpoint dataPoint,
                              double timeout,
                              size_t numBytesToWrite,
                              const char *buffer,
                              size_t *numBytesWritten))
    OCD ("tcp_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
