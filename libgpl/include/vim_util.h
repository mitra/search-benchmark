/* $Id: vim_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    vim_util.h

    Version-Independent Message Streams.

*******************************************************************************/

#ifndef  VIM_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  VIM_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "nvl_util.h"			/* Name/value pair lists. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */


/*******************************************************************************
    Version-Independent Message Stream Structures (Client View) and Definitions.
*******************************************************************************/

					/* Message stream handle. */
typedef  struct  _VimStream  *VimStream ;

typedef  struct  VimHeader {
    long  ID ;				/* Application-supplied message ID. */
    void  *parameter ;			/* Application-supplied parameter. */
    long  length ;			/* Length (in bytes) of message body. */
}  VimHeader ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  vim_util_debug ;		/* Global debug switch (1/0 = yes/no). */

/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  vimCreate P_((TcpEndpoint dataPoint,
                               VimStream *stream)) ;

extern  NVList  vimDecode P_((VimHeader *header,
                              const char *body)) ;

extern  errno_t  vimDestroy P_((VimStream stream)) ;

extern  IoFd  vimFd P_((VimStream stream)) ;

extern  bool  vimIsReadable P_((VimStream stream)) ;

extern  bool  vimIsUp P_((VimStream stream)) ;

extern  bool  vimIsWriteable P_((VimStream stream)) ;

extern  const  char  *vimName P_((VimStream stream)) ;

extern  errno_t  vimRead P_((VimStream stream,
                             double timeout,
                             VimHeader *header,
                             char **body)) ;

extern  errno_t  vimReadList P_((VimStream stream,
                                 double timeout,
                                 VimHeader *header,
                                 NVList *list)) ;

extern  errno_t  vimWrite P_((VimStream stream,
                              double timeout,
                              const VimHeader *header,
                              const char *body)) ;

extern  errno_t  vimWriteList P_((VimStream stream,
                                  double timeout,
                                  VimHeader *header,
                                  NVList list)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
