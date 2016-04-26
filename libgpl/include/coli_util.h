/* $Id: coli_util.h,v 1.11 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    coli_util.h

    CORBA-Lite Utilities.

*******************************************************************************/


/*******************************************************************************
    Table entry for mapping enumerated values to names and vice-versa.
    Define this separately from the main body of COLI_UTIL declarations
    so that header files #INCLUDE'd by "coli_util.h" can themselves
    #INCLUDE "coli_util.h" for their lookup-table declarations.
*******************************************************************************/

#ifndef  COLI_LUT		/* Lookup-table structure defined already? */
#define  COLI_LUT  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif

typedef  struct  ColiMap {		/* For name/value mappings. */
    long  number ;
    const  char  *name ;
}  ColiMap ;

#endif

				/* Has the file been INCLUDE'd already? */
#if !defined (COLI_UTIL_H) && !defined (COLI_LUT_ONLY)
#define  COLI_UTIL_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "comx_util.h"			/* CORBA marshaling utilities. */
#if HAVE_NAMESPACES
    using  namespace  CoLi ;
#endif
#include  "gimx_util.h"			/* GIOP marshaling utilities. */
#include  "iiop_util.h"			/* Internet Inter-ORB Protocol streams. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  coli_util_debug  OCD ("coli_uti") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  coliGetReply P_((IiopStream stream,
                                  ReplyStatusType *replyStatus,
                                  SystemExceptionReplyBody *exception,
                                  ...))
    OCD ("coli_uti") ;

extern  errno_t  coliGetRequest P_((IiopStream stream,
                                    ObjectKey object,
                                    const char *operation,
                                    IiopHeader *header,
                                    octet **body,
                                    RequestHeader *request,
                                    ...))
    OCD ("coli_uti") ;

extern  errno_t  coliMakeIOR P_((ObjectKey object,
                                 const char *host,
                                 unsigned short port,
                                 Version version,
                                 const char *typeID,
                                 IOR *ior))
    OCD ("coli_uti") ;

extern  char  *coliO2S P_((const IOR *ior,
                           bool dynamic))
    OCD ("coli_uti") ;

extern  char  *coliO2URL P_((const IOR *ior,
                             bool dynamic))
    OCD ("coli_uti") ;

extern  errno_t  coliOpenIOR P_((const IOR *ior,
                                 IiopStream oldStream,
                                 IiopStream *newStream))
    OCD ("coli_uti") ;

extern  const  ProfileBody  *coliProfile P_((const IOR *ior,
                                             int index))
    OCD ("coli_uti") ;

extern  errno_t  coliReply P_((IiopStream stream,
                               unsigned long requestID,
                               ReplyStatusType replyStatus,
                               ...))
    OCD ("coli_uti") ;

extern  errno_t  coliRequest P_((IiopStream stream,
                                 ObjectKey object,
                                 const char *operation,
                                 const ServiceContextList *contexts,
                                 ...))
    OCD ("coli_uti") ;

extern  errno_t  coliS2O P_((const char *string,
                             IOR *ior))
    OCD ("coli_uti") ;

extern  char  *coliS2URL P_((const char *string,
                             bool dynamic))
    OCD ("coli_uti") ;

extern  const  char  *coliToName P_((const ColiMap table[],
                                     long number))
    OCD ("coli_uti") ;

extern  long  coliToNumber P_((const ColiMap table[],
                               const char *name,
                               bool partial))
    OCD ("coli_uti") ;

extern  errno_t  coliURL2O P_((const char *url,
                               IOR *ior))
    OCD ("coli_uti") ;

extern  Version  coliVersion P_((const char *versionString))
    OCD ("coli_uti") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
