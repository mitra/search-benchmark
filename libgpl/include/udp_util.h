/* $Id: udp_util.h,v 1.10 2011/03/31 22:16:03 alex Exp $ */
/*******************************************************************************

    udp_util.h

    UDP Utility Definitions.

*******************************************************************************/

#ifndef  UDP_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  UDP_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "skt_util.h"			/* Socket support functions. */


/*******************************************************************************
    UDP Networking Structures (Client View) and Definitions.
*******************************************************************************/

					/* Endpoint handle. */
typedef  struct  _UdpEndpoint  *UdpEndpoint ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  udp_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  udpCreate P_((const char *serverName,
                               UdpEndpoint parent,
                               UdpEndpoint *endpoint)) ;

extern  errno_t  udpDestroy P_((UdpEndpoint endpoint)) ;

extern  IoFd  udpFd P_((UdpEndpoint endpoint)) ;

extern  bool  udpIsReadable P_((UdpEndpoint endpoint)) ;

extern  bool  udpIsUp P_((UdpEndpoint endpoint)) ;

extern  bool  udpIsWriteable P_((UdpEndpoint endpoint)) ;

extern  const  char  *udpName P_((UdpEndpoint endpoint)) ;

extern  errno_t  udpRead P_((UdpEndpoint endpoint,
                             double timeout,
                             size_t maxBytesToRead,
                             char *buffer,
                             size_t *numBytesRead,
                             UdpEndpoint *source)) ;

#define  udpSetBuf(endpoint, receiveSize, sendSize) \
    sktSetBuf (udpFd (endpoint), receiveSize, sendSize)

extern  errno_t  udpWrite P_((UdpEndpoint destination,
                              double timeout,
                              size_t numBytesToWrite,
                              const char *buffer)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
