/* $Id: lemx_util.h,v 1.2 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    lemx_util.h

    Laboratory Equipment Control Interface Specification (LECIS)
    Marshaling Utilities.

*******************************************************************************/

#ifndef  LEMX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  LEMX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stddef.h>			/* Standard C definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "gimx_util.h"			/* GIOP marshaling utilities. */


#if HAVE_NAMESPACES
    using  namespace  CoLi ;
    namespace  LecisCoLi {
#endif


/*******************************************************************************
    Auto-generated definitions - generated from the CORBA IDL files themselves.
*******************************************************************************/

#include  "lemx_idl.h"			/* Auto-generated IDL definitions. */


#if HAVE_NAMESPACES
    } ;     /* LecisCoLi namespace. */
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
