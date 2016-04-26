/* $Id: wwwClient.h,v 1.5 2008/01/14 20:02:48 alex Exp $ */
/*******************************************************************************

    wwwClient.h

    WWW Client Definitions.

*******************************************************************************/

#ifndef  WWWCLIENT_H		/* Has the file been INCLUDE'd already? */
#define  WWWCLIENT_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "log_util.h"			/* Log file utilities. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */


/*******************************************************************************
    WWW Client Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _WwwClient  *WwwClient ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  wwwClientCreate P_((TcpEndpoint connection,
                                     IoxDispatcher dispatcher,
                                     LogFile logFile,
                                     WwwClient *client)) ;

extern  errno_t  wwwClientDestroy P_((WwwClient client)) ;

extern  bool  wwwClientIsReadable P_((WwwClient client)) ;

extern  bool  wwwClientIsUp P_((WwwClient client)) ;


#endif				/* If this file was not INCLUDE'd previously. */
