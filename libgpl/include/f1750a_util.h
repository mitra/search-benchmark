/* $Id: f1750a_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    f1750a_util.h

    MIL-STD-1750A Floating Point Utilities.

*******************************************************************************/

#ifndef  F1750A_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  F1750A_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  double2f1750a P_((double value,
                                   size_t numBits,
                                   unsigned char *buffer)) ;

extern  double  f1750a2double P_((size_t numBits,
                                  unsigned char *buffer)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
