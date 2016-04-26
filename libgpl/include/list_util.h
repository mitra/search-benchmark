/* $Id: list_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    list_util.h

    List Manipulation Utility Definitions.

*******************************************************************************/

#ifndef  LIST_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  LIST_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    List Structure (Client View) and Definitions.
*******************************************************************************/

typedef  struct  ListNode  *List ;	/* List handle. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  listAdd P_((List *list,
                         int position,
                         void *item)) ;

extern  void  *listDelete P_((List *list,
                              int position)) ;

extern  int  listFind P_((List list,
                          void *item)) ;

extern  void  *listGet P_((List list,
                           int position)) ;

extern  int  listLength P_((List list)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
