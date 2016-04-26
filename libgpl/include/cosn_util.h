/* $Id: cosn_util.h,v 1.2 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    cosn_util.h

    CORBAservices (COS) Naming Service Utilities.

*******************************************************************************/

#ifndef  COSN_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  COSN_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "comx_util.h"			/* CORBA marshaling utilities. */
#if HAVE_NAMESPACES
    using  namespace  CoLi ;
#endif
#include  "gimx_util.h"			/* GIOP marshaling utilities. */
#include  "iiop_util.h"			/* Internet Inter-ORB Protocol streams. */


#if HAVE_NAMESPACES
    namespace  CoLi {
#endif

/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  cosn_util_debug		/* Global debug switch (1/0 = yes/no). */
    OCD ("CosNaming") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  char  *cosnN2S P_((const Name *name,
                           bool dynamic))
    OCD ("CosNaming") ;

extern  char  *cosnN2URL P_((const Name *name,
                             bool dynamic))
    OCD ("CosNaming") ;

extern  errno_t  cosnS2N P_((const char *string,
                             Name *name))
    OCD ("CosNaming") ;

extern  errno_t  cosnURL2N P_((const char *url,
                               Name *name))
    OCD ("CosNaming") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
