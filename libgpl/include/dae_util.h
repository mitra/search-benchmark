/* $Id: dae_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    dae_util.h

    Daemon Utility Definitions.

*******************************************************************************/

#ifndef  DAE_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  DAE_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  daeMonize P_((int numFDs)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
