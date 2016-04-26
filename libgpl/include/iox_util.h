/* $Id: iox_util.h,v 1.3 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    iox_util.h

    I/O Dispatcher Definitions.

*******************************************************************************/

#ifndef  IOX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  IOX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */


/*******************************************************************************
    I/O Dispatcher Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _IoxDispatcher  *IoxDispatcher ;	/* I/O dispatcher. */
typedef  struct  _IoxCallback  *IoxCallback ;		/* I/O callback. */

typedef  int  IoxReason ;				/* Callback reasons. */
#define  IoxNone	0
#define  IoxRead	1
#define  IoxWrite	2
#define  IoxExcept	4
#define  IoxIO		(IoxRead | IoxWrite | IoxExcept)
#define  IoxFire	8
#define  IoxIdle	16
#define  IoxCancel	32

					/* Handler function prototype. */
typedef  errno_t  (*IoxHandler) P_((IoxCallback, IoxReason, void *)) ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  iox_util_debug  OCD ("iox_util") ;


/*******************************************************************************
    Public functions (dispatchers).
*******************************************************************************/

extern  IoxCallback  ioxAfter P_((IoxDispatcher dispatcher,
                                  IoxHandler handlerF,
                                  void *userData,
                                  double interval))
    OCD ("iox_util") ;

extern  errno_t  ioxCreate P_((IoxDispatcher *dispatcher))
    OCD ("iox_util") ;

extern  errno_t  ioxDestroy P_((IoxDispatcher dispatcher))
    OCD ("iox_util") ;

extern  IoxCallback  ioxEvery P_((IoxDispatcher dispatcher,
                                  IoxHandler handlerF,
                                  void *userData,
                                  double delay,
                                  double interval))
    OCD ("iox_util") ;

extern  errno_t  ioxMonitor P_((IoxDispatcher dispatcher,
                                double interval))
    OCD ("iox_util") ;

extern  IoxCallback  ioxOnIO P_((IoxDispatcher dispatcher,
                                 IoxHandler handlerF,
                                 void *userData,
                                 IoxReason reason,
                                 IoFd source))
    OCD ("iox_util") ;

extern  IoxCallback  ioxWhenIdle P_((IoxDispatcher dispatcher,
                                     IoxHandler handlerF,
                                     void *userData))
    OCD ("iox_util") ;


/*******************************************************************************
    Public functions (callbacks).
*******************************************************************************/

extern  errno_t  ioxCancel P_((IoxCallback callback))
    OCD ("iox_util") ;

extern  int  ioxDepth P_((IoxCallback callback))
    OCD ("iox_util") ;

extern  IoxDispatcher  ioxDispatcher P_((IoxCallback callback))
    OCD ("iox_util") ;

extern  struct  timeval  ioxExpiration P_((IoxCallback callback))
    OCD ("iox_util") ;

extern  IoFd  ioxFd P_((IoxCallback callback))
    OCD ("iox_util") ;

extern  double  ioxInterval P_((IoxCallback callback))
    OCD ("iox_util") ;

extern  errno_t  ioxOnCancel P_((IoxCallback callback,
                                 bool onCancel))
    OCD ("iox_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
