/* $Id: iiop_util.h,v 1.8 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    iiop_util.h

    Internet Inter-ORB Protocol (IIOP) Streams.

*******************************************************************************/

#ifndef  IIOP_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  IIOP_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "gimx_util.h"			/* GIOP marshaling utilities. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */


/*******************************************************************************
    IIOP Stream Structures (Client View) and Definitions.
*******************************************************************************/

					/* IIOP stream handle. */
typedef  struct  _IiopStream  *IiopStream ;

typedef  struct  IiopHeader {
    Version  GIOP_version ;		/* 1.0, 1.1, 1.2, etc. */
    unsigned  short  flags ;
    MsgType  message_type ;		/* See GIOP::MsgType. */
    unsigned  long  message_size ;	/* # of bytes excluding header. */
}  IiopHeader ;

					/* Global debug switch (1/0 = yes/no). */
extern  int  iiop_util_debug  OCD ("iiop_uti") ;

/*******************************************************************************
    Public functions (IIOP streams).
*******************************************************************************/

extern  errno_t  iiopCreate P_((TcpEndpoint dataPoint,
                                IiopStream *stream))
    OCD ("iiop_uti") ;

extern  errno_t  iiopDestroy P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  IoFd  iiopFd P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  ServiceContextList  *iiopGetContexts P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  bool  iiopIsReadable P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  bool  iiopIsUp P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  bool  iiopIsWriteable P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  const  char  *iiopName P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  errno_t  iiopRead P_((IiopStream stream,
                              double timeout,
                              IiopHeader *header,
                              octet **body))
    OCD ("iiop_uti") ;

extern  unsigned  long  iiopRequestID P_((IiopStream stream))
    OCD ("iiop_uti") ;

extern  errno_t  iiopSetContexts P_((IiopStream stream,
                                     ServiceContextList *contexts))
    OCD ("iiop_uti") ;

extern  errno_t  iiopWrite P_((IiopStream stream,
                               double timeout,
                               const IiopHeader *header,
                               const octet *body))
    OCD ("iiop_uti") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
