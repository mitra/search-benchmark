/* $Id: comx_util.h,v 1.16 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    comx_util.h

    CORBA Marshaling Utilities.

*******************************************************************************/

#ifndef  COMX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  COMX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stddef.h>			/* Standard C definitions. */
#ifdef _WIN32
#    include  <windows.h>		/* Windows data types. */
#    define  LONGDOUBLE  long double
#elif defined(vaxc)
#    define  LONGLONG  long		/* VAX data types. */
#    define  ULONGLONG  unsigned long
#    define  LONGDOUBLE  double
#else
#    define  LONGLONG  long long	/* GNU data types. */
#    define  ULONGLONG  unsigned long long
#    define  LONGDOUBLE  long double
#endif
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


#if HAVE_NAMESPACES
    namespace  CoLi {
#endif


/*******************************************************************************
    Marshaling Channel - represents the decoding/encoding data flow between
        a CORBA message buffer and host CPU data structures.
*******************************************************************************/

					/* Marshaling channel handle. */
typedef  struct  _ComxChannel  *ComxChannel ;

typedef  enum  ComxOperation {
    MxDECODE, MxENCODE, MxERASE
}  ComxOperation ;

typedef  errno_t  (*ComxFunc) P_((ComxChannel channel, void *cpuValue)) ;

					/* Global debug switch (1/0 = yes/no). */
extern  int  comx_util_debug  OCD ("comx_uti") ;

/*******************************************************************************
    Common Data Representation (CDR) definitions.
*******************************************************************************/

/*  Define an anonymous structure (if your compiler allows it!) for a sequence.
    The structure contains two fields: a count of the # of elements in the
    sequence and a pointer to the array of elements. */

#define  SEQUENCE(type, name)	\
    struct  {			\
        unsigned  long  count ;	\
        type  *elements ;	\
    }  name

typedef  SEQUENCE (void, GenericSeq) ;

typedef  unsigned  char  octet ;
typedef  SEQUENCE (octet, OctetSeq) ;

/*  Define the structure for the GIOP version number.  At the time this code
    was written, GIOP 1.3 was the latest version.  The contents of and/or the
    serialized representations of some of the CDR and GIOP types have changed
    as the GIOP version has progressed from the original 1.0.  When writing
    version-dependent marshaling code, the GIOP_VERSION_GE macro provides an
    easy means of determining if the GIOP version in use is greater than or
    equal to a desired major and minor version number. */

typedef  struct  Version {
    octet  major ;
#define  GIOP_VERSION_MAJOR  1
    octet  minor ;
#define  GIOP_VERSION_MINOR  3
}  Version ;

#define  GIOP_VERSION_GE(ver, maj, min)	\
    (((ver).major > (maj)) ||		\
     (((ver).major == (maj)) && ((ver).minor >= (min))))

/*******************************************************************************
    Public functions - marshaling channel creation, etc.
*******************************************************************************/

extern  octet  *comxBuffer P_((ComxChannel channel,
                               bool release))
    OCD ("comx_uti") ;

extern  errno_t  comxCreate P_((Version version,
                                bool littleEndian,
                                unsigned long offset,
                                octet *buffer,
                                unsigned long length,
                                ComxChannel *channel))
    OCD ("comx_uti") ;

extern  errno_t  comxDestroy P_((ComxChannel channel))
    OCD ("comx_uti") ;

extern  errno_t  comxErase P_((ComxFunc marshalF,
                               void *value))
    OCD ("comx_uti") ;

extern  errno_t  comxExtend P_((ComxChannel channel,
                                long numOctets))
    OCD ("comx_uti") ;

extern  ComxOperation  comxGetOp P_((ComxChannel channel))
    OCD ("comx_uti") ;

extern  Version  comxGetVersion P_((ComxChannel channel))
    OCD ("comx_uti") ;

#define  comxReset(channel)  (comxSkip ((channel), -(comxSkip ((channel), 0))))

extern  errno_t  comxSetOp P_((ComxChannel channel,
                               ComxOperation operation))
    OCD ("comx_uti") ;

extern  long  comxSkip P_((ComxChannel channel,
                           long numOctets,
                           long alignment))
    OCD ("comx_uti") ;

extern  void  comxToHost P_((bool littleEndian,
                             int numBytes,
                             void *cdrValue,
                             void *cpuValue))
    OCD ("comx_uti") ;

/*******************************************************************************
    Public functions - CDR Primitive Data Types.
*******************************************************************************/

extern  errno_t  comxBoolean P_((ComxChannel channel,
                                 bool *value))
    OCD ("comx_uti") ;

extern  errno_t  comxChar P_((ComxChannel channel,
                              char *value))
    OCD ("comx_uti") ;

extern  errno_t  comxDouble P_((ComxChannel channel,
                                double *value))
    OCD ("comx_uti") ;

extern  errno_t  comxEnum P_((ComxChannel channel,
                              unsigned long *value))
    OCD ("comx_uti") ;

extern  errno_t  comxFloat P_((ComxChannel channel,
                               float *value))
    OCD ("comx_uti") ;

extern  errno_t  comxLong P_((ComxChannel channel,
                              long *value))
    OCD ("comx_uti") ;

extern  errno_t  comxLongDouble P_((ComxChannel channel,
                                    LONGDOUBLE *value))
    OCD ("comx_uti") ;

extern  errno_t  comxLongLong P_((ComxChannel channel,
                                  LONGLONG *value))
    OCD ("comx_uti") ;

extern  errno_t  comxOctet P_((ComxChannel channel,
                               octet *value))
    OCD ("comx_uti") ;

extern  errno_t  comxShort P_((ComxChannel channel,
                               short *value))
    OCD ("comx_uti") ;

extern  errno_t  comxULong P_((ComxChannel channel,
                               unsigned long *value))
    OCD ("comx_uti") ;

extern  errno_t  comxULongLong P_((ComxChannel channel,
                                   ULONGLONG *value))
    OCD ("comx_uti") ;

extern  errno_t  comxUShort P_((ComxChannel channel,
                                unsigned short *value))
    OCD ("comx_uti") ;

extern  errno_t  comxWChar P_((ComxChannel channel,
                               wchar_t *value))
    OCD ("comx_uti") ;

/*******************************************************************************
    Public functions - GIOP Constructed Data Types.
*******************************************************************************/

/* Special-case marshaling. */

extern  errno_t  comxArray P_((ComxChannel channel,
                               void *value,
                               ComxFunc marshalF,
                               size_t size,
                               unsigned long count))
    OCD ("comx_uti") ;

extern  errno_t  comxEncapsule P_((Version version,
                                   ComxOperation operation,
                                   OctetSeq *encapsulation,
                                   ...))
    OCD ("comx_uti") ;

extern  errno_t  comxSequence P_((ComxChannel channel,
                                  void *value,
                                  ComxFunc marshalF,
                                  size_t size))
    OCD ("comx_uti") ;

extern  errno_t  comxString P_((ComxChannel channel,
                                char **value))
    OCD ("comx_uti") ;

extern  errno_t  comxWString P_((ComxChannel channel,
                                 wchar_t **value))
    OCD ("comx_uti") ;

extern  errno_t  comxOctetSeq P_((ComxChannel channel,
                                  OctetSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxVersion P_((ComxChannel channel,
                                 Version *value))
    OCD ("comx_uti") ;

/* Sequences of primitive CDR data types. */

typedef  SEQUENCE (bool, BooleanSeq) ;
typedef  SEQUENCE (char, CharSeq) ;
typedef  SEQUENCE (double, DoubleSeq) ;
typedef  SEQUENCE (unsigned long, EnumSeq) ;
typedef  SEQUENCE (float, FloatSeq) ;
typedef  SEQUENCE (long, LongSeq) ;
typedef  SEQUENCE (LONGLONG, LongLongSeq) ;
typedef  SEQUENCE (short, ShortSeq) ;
typedef  SEQUENCE (char *, StringSeq) ;
typedef  SEQUENCE (unsigned long, ULongSeq) ;
typedef  SEQUENCE (ULONGLONG, ULongLongSeq) ;
typedef  SEQUENCE (unsigned short, UShortSeq) ;
typedef  SEQUENCE (wchar_t, WCharSeq) ;
typedef  SEQUENCE (wchar_t *, WStringSeq) ;

extern  errno_t  comxBooleanSeq P_((ComxChannel channel,
                                    BooleanSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxCharSeq P_((ComxChannel channel,
                                 CharSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxDoubleSeq P_((ComxChannel channel,
                                   DoubleSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxEnumSeq P_((ComxChannel channel,
                                 EnumSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxFloatSeq P_((ComxChannel channel,
                                  FloatSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxLongSeq P_((ComxChannel channel,
                                 LongSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxLongLongSeq P_((ComxChannel channel,
                                     LongLongSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxShortSeq P_((ComxChannel channel,
                                  ShortSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxStringSeq P_((ComxChannel channel,
                                   StringSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxULongSeq P_((ComxChannel channel,
                                  ULongSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxULongLongSeq P_((ComxChannel channel,
                                      ULongLongSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxUShortSeq P_((ComxChannel channel,
                                   UShortSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxWCharSeq P_((ComxChannel channel,
                                  WCharSeq *value))
    OCD ("comx_uti") ;

extern  errno_t  comxWStringSeq P_((ComxChannel channel,
                                    WStringSeq *value))
    OCD ("comx_uti") ;


#if HAVE_NAMESPACES
    } ;     /* CoLi namespace. */
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
