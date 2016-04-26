/* $Id: bmw_util.h,v 1.9 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    bmw_util.h

    Benchmarking Definitions.

*******************************************************************************/

#ifndef  BMW_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  BMW_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */


/*******************************************************************************
    Benchmarking Structures and Definitions.
*******************************************************************************/

typedef  struct  BmwClock {
    struct  timeval  startTime ;
    struct  timeval  stopTime ;
}  BmwClock ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  double  bmwElapsed P_((BmwClock *timer)) ;

extern  double  bmwRate P_((BmwClock *timer,
                            long numItems)) ;

extern  void  bmwStart P_((BmwClock *timer)) ;

extern  void  bmwStop P_((BmwClock *timer)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
