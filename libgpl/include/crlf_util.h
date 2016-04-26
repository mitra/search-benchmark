/* $Id: crlf_util.h,v 1.6 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    crlf_util.h

    Carriage Return/Line Feed Utility Definitions.

*******************************************************************************/

#ifndef  CRLF_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  CRLF_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  char  *crlf2nl P_((char *string,
                           int length,
                           char *lastChar)) ;

extern  char  *nl2crlf P_((char *string,
                           int length,
                           int maxLength)) ;

extern  size_t  nlCount P_((const char *string,
                            int length)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
