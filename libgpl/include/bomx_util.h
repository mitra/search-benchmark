/* $Id: bomx_util.h,v 1.1 2009/09/23 13:21:34 alex Exp $ */
/*******************************************************************************

    bomx_util.h

    GNOME Bonobo Marshaling Utilities.

*******************************************************************************/

#ifndef  BOMX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  BOMX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stddef.h>			/* Standard C definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "gimx_util.h"			/* GIOP marshaling utilities. */


#if HAVE_NAMESPACES
    using  namespace  CoLi ;
    namespace  BoCoLi {
#endif


/*******************************************************************************
    Auto-generated definitions - generated from the CORBA IDL files themselves.
*******************************************************************************/

#include  "bomx_idl.h"			/* Auto-generated IDL definitions. */


/*******************************************************************************
    Additional tables for mapping "#define"d values to names and vice-versa;
    see the coliToName() and coliToNumber() functions.
*******************************************************************************/

extern  const  ColiMap  ActivationFlagsLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  RegistrationFlagsLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  ActivationEnvValueLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  ResolveFlagLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  StorageInfoFieldsLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  OpenModeLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  PropertyFlagsLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  BufFlagsLUT[]  OCD ("Bonobo") ;


#if HAVE_NAMESPACES
    } ;     /* BoCoLi namespace. */
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
