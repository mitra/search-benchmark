/* $Id: port_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    port_util.h

    Port Utility Definitions.

*******************************************************************************/

#ifndef  PORT_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  PORT_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */


/*******************************************************************************
    Port Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _ListeningPort  *ListeningPort ;

					/* Client creation function prototype. */
typedef  errno_t  (*ClientCreateFunc) P_((TcpEndpoint connection,
                                          IoxDispatcher dispatcher,
                                          void *parameter,
                                          void **client)) ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  port_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  portCreate P_((const char *name,
                                IoxDispatcher dispatcher,
                                ClientCreateFunc clientCreateF,
                                void *clientParameter,
                                ListeningPort *port)) ;

extern  errno_t  portDestroy P_((ListeningPort port)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
