/* $Id: xqt_util.h,v 1.6 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    xqt_util.h

    Shell Execution Utility Definitions.

*******************************************************************************/

#ifndef  XQT_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  XQT_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "skt_util.h"			/* Socket support functions. */


/*******************************************************************************
    XQT Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _XqtStream  *XqtStream ;	/* Execution stream handle. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  xqt_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  xqtClose P_((XqtStream stream)) ;

extern  IoFd  xqtFd P_((XqtStream stream)) ;

extern  int  xqtOpen P_((const char *shell,
                         XqtStream *stream)) ;

extern  int  xqtPoll P_((XqtStream stream)) ;

extern  int  xqtRead P_((XqtStream stream,
                         int maxLength,
                         char *buffer)) ;

extern  int  xqtWrite P_((XqtStream stream,
                          const char *format,
                          ...)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
