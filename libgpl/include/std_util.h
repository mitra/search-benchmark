/* $Id: std_util.h,v 1.3 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    std_util.h

    Standard C Functions.

*******************************************************************************/

#ifndef  STD_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  STD_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stddef.h>			/* Standard C definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

#if defined(HAVE_ATOF) && !HAVE_ATOF
    extern  double  atof P_((const char *nptr)) ;
#endif

#if defined(HAVE_GETENV) && !HAVE_GETENV
    extern  char  *getenv P_((const char *name)) ;
#endif

#if defined(HAVE_STRTOD) && !HAVE_STRTOD
    extern  double  strtod P_((const char *nptr,
                               char **endptr)) ;
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
