/* $Id: nvl_util.h,v 1.8 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    nvl_util.h

    Name/Value Pair List Definitions.

*******************************************************************************/

#ifndef  NVL_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  NVL_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "nvp_util.h"			/* Name/value pairs. */


/*******************************************************************************
    Name/Value-Pair List Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _NVList  *NVList ;	/* List handle. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  nvl_util_debug  OCD ("nvl_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  nvlAdd P_((NVList list,
                            NVPair pair))
    OCD ("nvl_util") ;

extern  int  nvlCount P_((NVList list))
    OCD ("nvl_util") ;

extern  errno_t  nvlCreate P_((const char *name,
                               NVList *list))
    OCD ("nvl_util") ;

extern  NVPair  nvlDelete P_((NVList list,
                              const char *name))
    OCD ("nvl_util") ;

extern  errno_t  nvlDestroy P_((NVList list))
    OCD ("nvl_util") ;

extern  errno_t  nvlDump P_((FILE *file,
                             const char *indent,
                             NVList list))
    OCD ("nvl_util") ;

extern  NVPair  nvlFind P_((NVList list,
                            const char *name))
    OCD ("nvl_util") ;

extern  NVPair  nvlGet P_((NVList list,
                           int index))
    OCD ("nvl_util") ;

extern  const  char  *nvlName P_((NVList list))
    OCD ("nvl_util") ;

extern  bool_t  xdr_NVList P_((XDR *xdrStream,
                               NVList *list))
    OCD ("nvl_util") ;

/* LISP-like property list functions. */

extern  const  char  *nvlGetProp P_((NVList list,
                                     const char *name))
    OCD ("nvl_util") ;

extern  errno_t  nvlPutProp P_((NVList list,
                                const char *name,
                                const char *value))
    OCD ("nvl_util") ;

extern  errno_t  nvlRemProp P_((NVList list,
                                const char *name))
    OCD ("nvl_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
