/* $Id: ftpClient.h,v 1.4 2008/01/14 20:02:48 alex Exp $ */
/*******************************************************************************

    ftpClient.h

    FTP Client Definitions.

*******************************************************************************/

#ifndef  FTPCLIENT_H		/* Has the file been INCLUDE'd already? */
#define  FTPCLIENT_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "tcp_util.h"			/* TCP/IP network utilities. */


/*******************************************************************************
    FTP Client Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _FtpClient  *FtpClient ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  ftpClientCreate P_((TcpEndpoint connection,
                                     IoxDispatcher dispatcher,
                                     void *parameter,
                                     FtpClient *client)) ;

extern  errno_t  ftpClientDestroy P_((FtpClient client)) ;


#endif				/* If this file was not INCLUDE'd previously. */
