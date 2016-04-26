/* $Id: xnet_util.h,v 1.13 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    xnet_util.h

    XDR Networking Utility Definitions.

*******************************************************************************/

#ifndef  XNET_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  XNET_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <rpc/types.h>			/* RPC type definitions. */
#include  <rpc/xdr.h>			/* XDR type definitions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */


/*******************************************************************************
    XNET Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _XnetStream  *XnetStream ;	/* XNET stream handle. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  xnet_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  xnetCreate P_((TcpEndpoint dataPoint,
                                const char *options,
                                XnetStream *stream)) ;

extern  errno_t  xnetDestroy P_((XnetStream stream)) ;

extern  errno_t  xnetEndRecord P_((XnetStream stream)) ;

extern  IoFd  xnetFd P_((XnetStream stream)) ;

extern  XDR  *xnetHandle P_((XnetStream stream)) ;

extern  bool  xnetIsReadable P_((XnetStream stream)) ;

extern  bool  xnetIsUp P_((XnetStream stream)) ;

extern  bool  xnetIsWriteable P_((XnetStream stream)) ;

extern  const  char  *xnetName P_((XnetStream stream)) ;

extern  errno_t  xnetNextRecord P_((XnetStream stream)) ;

extern  errno_t  xnetRead P_((XnetStream stream,
                              double timeout,
                              char **string)) ;

extern  errno_t  xnetSetTimeout P_((XnetStream stream,
                                    double timeout)) ;

extern  errno_t  xnetWrite P_((XnetStream stream,
                               double timeout,
                               const char *format,
                               ...)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
