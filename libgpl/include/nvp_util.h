/* $Id: nvp_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    nvp_util.h

    Name/Value Pairs.

*******************************************************************************/

#ifndef  NVP_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  NVP_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stdio.h>			/* Standard I/O definitions. */
#include  <rpc/types.h>			/* RPC type definitions. */
#include  <rpc/xdr.h>			/* XDR type definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Name/Value Pair (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _NVPair  *NVPair ;	/* Pair handle. */

typedef  enum  NvpDataType {
    NvpUndefined,
    NvpByte,
    NvpDouble,
    NvpLong,
    NvpString,
    NvpTime,
    NvpList
}  NvpDataType ;

typedef  enum  NvpStorageClass {
    NvpNone,
    NvpDynamic,
    NvpStatic,
    NvpVolatile
}  NvpStorageClass ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  nvp_util_debug  OCD ("nvp_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  nvpAssign P_((NVPair pair,
                               int numElements,
                               NvpDataType dataType,
                               ...))
    OCD ("nvp_util") ;

extern  int  nvpCount P_((NVPair pair))
    OCD ("nvp_util") ;

extern  errno_t  nvpCreate P_((const char *name,
                               NVPair *pair))
    OCD ("nvp_util") ;

extern  NVPair  nvpDecode P_((const char *spec))
    OCD ("nvp_util") ;

extern  errno_t  nvpDestroy P_((NVPair pair))
    OCD ("nvp_util") ;

extern  const  char  *nvpEncode P_((NVPair pair))
    OCD ("nvp_util") ;

extern  const  char  *nvpName P_((NVPair pair))
    OCD ("nvp_util") ;

extern  NVPair  nvpNew P_((const char *name,
                           NvpDataType dataType,
                           ...))
    OCD ("nvp_util") ;

extern  size_t  nvpSizeOf P_((NVPair pair))
    OCD ("nvp_util") ;

extern  const  char  *nvpString P_((NVPair pair))
    OCD ("nvp_util") ;

extern  NvpDataType  nvpTypeOf P_((NVPair pair))
    OCD ("nvp_util") ;

extern  void  *nvpValue P_((NVPair pair))
    OCD ("nvp_util") ;

extern  bool_t  xdr_NVPair P_((XDR *xdrStream,
                               NVPair *pair))
    OCD ("nvp_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
