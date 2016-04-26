/* $Id: sem_util.h,v 1.6 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    sem_util.h

    Semaphore Utility Definitions.

*******************************************************************************/

#ifndef  SEM_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  SEM_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Semaphore (Client View).
*******************************************************************************/

typedef  struct  _Semaphore  *Semaphore ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  sem_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  sem_create P_((const char *name,
                            int initial_value,
                            Semaphore *semaphore)) ;

extern  int  sem_delete P_((Semaphore semaphore)) ;

extern  int  sem_give P_((Semaphore semaphore)) ;

extern  int  sem_id P_((Semaphore semaphore)) ;

extern  int  sem_take P_((Semaphore semaphore,
                          double timeout)) ;

extern  int  sem_value P_((Semaphore semaphore)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
