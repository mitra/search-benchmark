/* $Id: gimx_util.h,v 1.15 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    gimx_util.h

    GIOP Marshaling Utilities.

*******************************************************************************/

#ifndef  GIMX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  GIMX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#define  COLI_LUT_ONLY  1
#include  "coli_util.h"			/* CORBA-Lite utilities. */
#undef  COLI_LUT_ONLY
#include  "comx_util.h"			/* CORBA marshaling utilities. */
#ifdef __cplusplus
#    include  "TimeValue.h"		/* TimeValue class definitions. */
#else
#    include  "tv_util.h"		/* "timeval" manipulation functions. */
#endif


#if HAVE_NAMESPACES
    namespace  CoLi {
#endif

/*******************************************************************************
    Useful types.
*******************************************************************************/

typedef  SEQUENCE (octet, ObjectKey) ;

typedef  SEQUENCE (struct timeval, TimevalSeq) ;

/* GIOP MessageHeader flags. */

#define  ENDIAN_MASK  0x01		/* 0 = big-endian, 1 = little-endian */
#define  FRAGMENT_MASK  0x02		/* 0 = last fragment, 1 = more fragments */


/*******************************************************************************
    Define generic enumerated types for GIOP version-specific enumerations.
    These need to be updated when new CORBA versions are issued.
*******************************************************************************/

typedef  enum  LocateStatusType_1_2 {
    UNKNOWN_OBJECT,
    OBJECT_HERE,
    OBJECT_FORWARD,			/* GIOP 1.0/1.1 stop here. */
    OBJECT_FORWARD_PERM,
    LOC_SYSTEM_EXCEPTION,
    LOC_NEEDS_ADDRESSING_MODE
}  LocateStatusType ;

typedef  enum  MsgType_1_1 {
    Request = 0,
    Reply,
    CancelRequest,
    LocateRequest,
    LocateReply,
    CloseConnection,
    MessageError,			/* GIOP 1.0 stops here. */
    Fragment
}  GIOPMsgType ;			/* MsgType would conflict with CSI.idl type. */

typedef  enum  ReplyStatusType_1_2 {
    NO_EXCEPTION,
    USER_EXCEPTION,
    SYSTEM_EXCEPTION,
    LOCATION_FORWARD,			/* GIOP 1.0/1.1 stop here. */
    LOCATION_FORWARD_PERM,
    NEEDS_ADDRESSING_MODE
}  ReplyStatusType ;

/*******************************************************************************
    Auto-generated definitions - generated from the CORBA IDL files themselves.
*******************************************************************************/

#include  "gimx_idl.h"			/* Auto-generated IDL definitions. */


/*******************************************************************************
    Additional tables for mapping "#define"d values to names and vice-versa;
    see the coliToName() and coliToNumber() functions.  The "static" tables
    need to be updated when new CORBA versions are issued.
*******************************************************************************/

extern  const  ColiMap  AddressingDispositionLUT[]  OCD ("GIOP") ;
extern  const  ColiMap  AssociationOptionsLUT[]  OCD ("CSIIOP") ;
extern  const  ColiMap  CodeSetIdLUT[]  OCD ("CONV_FRA") ;
extern  const  ColiMap  ComponentIdLUT[]  OCD ("IOP") ;
extern  const  ColiMap  GIOPMsgTypeLUT[]  OCD ("GIOP") ;
extern  const  ColiMap  LocateStatusTypeLUT[]  OCD ("GIOP") ;
extern  const  ColiMap  ProfileIdLUT[]  OCD ("IOP") ;
extern  const  ColiMap  ReplyStatusTypeLUT[]  OCD ("GIOP") ;
extern  const  ColiMap  ServiceIdLUT[]  OCD ("IOP") ;
extern  const  ColiMap  SyncScopeLUT[]  OCD ("Messagin") ;


/*******************************************************************************
    Additional mashaling functions.
*******************************************************************************/

extern  errno_t  gimxAny P_((ComxChannel channel,
                             Any *value))
    OCD ("gimx_uti") ;

extern  errno_t  gimxAnySeq P_((ComxChannel channel,
                                AnySeq *value))
    OCD ("gimx_uti") ;

extern  errno_t  gimxObjectKey P_((ComxChannel channel,
                                   ObjectKey *value))
    OCD ("gimx_uti") ;

extern  errno_t  gimxProfileBody P_((ComxChannel channel,
                                     ProfileBody *value))
    OCD ("gimx_uti") ;

extern  errno_t  gimxTaggedProfile P_((ComxChannel channel,
                                       TaggedProfile *value))
    OCD ("gimx_uti") ;

extern  errno_t  gimxTimeval P_((ComxChannel channel,
                                 struct timeval *value))
    OCD ("gimx_uti") ;

extern  errno_t  gimxTimevalSeq P_((ComxChannel channel,
                                    TimevalSeq *value))
    OCD ("gimx_uti") ;


#if HAVE_NAMESPACES
    } ;     /* CoLi namespace. */
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
