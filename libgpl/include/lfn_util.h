/* $Id: lfn_util.h,v 1.8 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    lfn_util.h

    Line Feed-Terminated Networking Utility Definitions.

*******************************************************************************/

#ifndef  LFN_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  LFN_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */


/*******************************************************************************
    LFN Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _LfnStream  *LfnStream ;	/* LFN stream handle. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  lfn_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  lfnCreate P_((TcpEndpoint dataPoint,
                               const char *options,
                               LfnStream *stream)) ;

extern  errno_t  lfnDestroy P_((LfnStream stream)) ;

extern  IoFd  lfnFd P_((LfnStream stream)) ;

extern  errno_t  lfnGetLine P_((LfnStream stream,
                                double timeout,
                                char **string)) ;

extern  bool  lfnIsReadable P_((LfnStream stream)) ;

extern  bool  lfnIsUp P_((LfnStream stream)) ;

extern  bool  lfnIsWriteable P_((LfnStream stream)) ;

extern  const  char  *lfnName P_((LfnStream stream)) ;

extern  errno_t  lfnPutLine P_((LfnStream stream,
                                double timeout,
                                const char *format,
                                ...)) ;

extern  errno_t  lfnRead P_((LfnStream stream,
                             double timeout,
                             ssize_t numBytesToRead,
                             char *buffer,
                             size_t *numBytesRead)) ;

extern  errno_t  lfnWrite P_((LfnStream stream,
                              double timeout,
                              size_t numBytesToWrite,
                              const char *buffer,
                              size_t *numBytesWritten)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
