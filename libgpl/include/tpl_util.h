/* $Id: tpl_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    tpl_util.h

    Tuple Manipulation Definitions.

*******************************************************************************/

#ifndef  TPL_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  TPL_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Tuple (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _Tuple  *Tuple ;	/* Tuple handle. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  tplArity P_((Tuple tuple)) ;

extern  Tuple  tplCreate P_((int arity,
                             ...)) ;

extern  errno_t  tplDestroy P_((Tuple tuple)) ;

extern  void  *tplGet P_((Tuple tuple,
                          int index)) ;

extern  errno_t  tplSet P_((Tuple tuple,
                            int index,
                            void *value)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
