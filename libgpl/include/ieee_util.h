/* $Id: ieee_util.h,v 1.4 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    ieee_util.h

    IEEE 754 Floating Point Utilities.

*******************************************************************************/

#ifndef  IEEE_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  IEEE_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  void  double2ieee P_((double value,
                              int numBits,
                              int byteOrder,
                              unsigned char *buffer)) ;

extern  double  ieee2double P_((int numBits,
                                int byteOrder,
                                unsigned char *buffer)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
