/* $Id: passClient.h,v 1.4 2008/01/14 20:02:48 alex Exp $ */
/*******************************************************************************

    passClient.h

    Pass-Through Client Definitions.

*******************************************************************************/

#ifndef  PASSCLIENT_H		/* Has the file been INCLUDE'd already? */
#define  PASSCLIENT_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */


/*******************************************************************************
    Pass-Through Client Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _PassClient  *PassClient ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  passClientCreate P_((TcpEndpoint connection,
                                      IoxDispatcher dispatcher,
                                      const char *target,
                                      PassClient *client)) ;

extern  errno_t  passClientDestroy P_((PassClient client)) ;


#endif				/* If this file was not INCLUDE'd previously. */
