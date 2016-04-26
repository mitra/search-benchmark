/* $Id: net_util.h,v 1.9 2009/09/09 22:38:13 alex Exp alex $ */
/*******************************************************************************

    net_util.h

    Network Utility Definitions.

*******************************************************************************/

#ifndef  NET_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  NET_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "skt_util.h"			/* Internet IPC domain definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  in_addr_t  netAddrOf P_((const char *hostName))
    OCD ("net_util") ;

extern  const  char  *netHostOf P_((in_addr_t ipAddress,
                                    bool dotted))
    OCD ("net_util") ;

extern  int  netPortOf P_((const char *serverName,
                           const char *protocol))
    OCD ("net_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
