/* $Id: ddmx_util.h,v 1.2 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    ddmx_util.h

    Data Distribution Service for Real-Time Systems (DDS) Marshaling Utilities.

*******************************************************************************/

#ifndef  DDMX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  DDMX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stddef.h>			/* Standard C definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "gimx_util.h"			/* GIOP marshaling utilities. */


#if HAVE_NAMESPACES
    using  namespace  CoLi ;
    namespace  DdsCoLi {
#endif


/*******************************************************************************
    Auto-generated definitions - generated from the CORBA IDL files themselves.
*******************************************************************************/

#include  "ddmx_idl.h"			/* Auto-generated IDL definitions. */


#if HAVE_NAMESPACES
    } ;     /* DdsCoLi namespace. */
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
