/* $Id: excp_util.h,v 1.3 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    excp_util.h

    Exception Handling Definitions.

*******************************************************************************/

#ifndef  EXCP_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  EXCP_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <setjmp.h>			/* Condition handling definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  ExcpContext  excpPop P_((void)) ;

extern  int  excpPush P_((ExcpContext *context)) ;

#define  excpTry \
  { jmp_buf  state ; \
    if (setjmp (state)

extern  void  excpThrow P_((const char *fileName,
                            int lineNumber,
                            void *error)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
