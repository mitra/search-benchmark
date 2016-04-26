/* $Id: nob_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    nob_util.h

    Named Object Definitions.

*******************************************************************************/

#ifndef  NOB_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  NOB_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Named Object (Client View).
*******************************************************************************/

typedef  struct  _NamedObject  *NamedObject ;

typedef  enum  NamedObjectScope {
    singleCPU,
    multiCPU				/* Under VxMP only. */
}  NamedObjectScope ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  nob_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  nobAbort P_((NamedObject object)) ;

extern  int  nobCommit P_((NamedObject object,
                           void *value)) ;

extern  int  nobCount P_((NamedObject)) ;

extern  int  nobCreate P_((const char *name,
                           NamedObjectScope scope,
                           NamedObject *object)) ;

extern  int  nobDestroy P_((NamedObject object)) ;

extern  NamedObject  nobExists P_((const char *name,
                                   NamedObjectScope scope)) ;

extern  const  char  *nobName P_((NamedObject object)) ;

extern  int  nobReference P_((NamedObject object)) ;

extern  void  *nobValue P_((NamedObject object)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
