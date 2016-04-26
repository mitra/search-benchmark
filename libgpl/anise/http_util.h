/* $Id: http_util.h,v 1.7 2012/05/06 22:31:16 alex Exp $ */
/*******************************************************************************

    http_util.h

    HTTP Utility Definitions.

*******************************************************************************/

#ifndef  HTTP_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  HTTP_UTIL_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "log_util.h"			/* Log file utilities. */


extern  char  *tildeTranslation ;	/* Format string for tilde translation. */


/*******************************************************************************
    HTTP Response Information - used to log the request/response transaction.
*******************************************************************************/

typedef  struct  ResponseInfo {
    int  status ;			/* Status code returned to client. */
    long  numBytes ;			/* # of bytes of data returned; -1 if N/A. */
    char  peer[256] ;			/* IP address of client. */
}  ResponseInfo ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  size_t  httpConvert P_((char *text)) ;

extern  errno_t  httpEvaluate P_((LfnStream client,
                                  int numLines,
                                  char *header[],
                                  char *body,
                                  bool *keepAlive,
                                  ResponseInfo *response)) ;

extern  errno_t  httpLog P_((LogFile logFile,
                             LfnStream client,
                             int numLines,
                             char *header[],
                             ResponseInfo *response)) ;

extern  const  char  *httpResolve P_((const char *resource)) ;

extern  const  char  *httpTypeOf P_((const char *pathname)) ;


#endif				/* If this file was not INCLUDE'd previously. */
