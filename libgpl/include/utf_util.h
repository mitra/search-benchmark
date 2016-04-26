/* $Id: utf_util.h,v 1.1 2012/08/02 10:11:22 alex Exp $ */
/*******************************************************************************

    utf_util.h

    Unicode Transformation Format (UTF) Utilities.

*******************************************************************************/

#ifndef  UTF_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  UTF_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stddef.h>			/* Standard C definitions. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

#define  UTF_16_UNIT_BYTES  2
#define  UTF_16_PAIR_BYTES  4
#define  UTF_32_UNIT_BYTES  4


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  utf16bom P_((const char *src))
    OCD ("utf_util") ;

extern  int  utf32bom P_((const char *src))
    OCD ("utf_util") ;

extern  size_t  utf16len P_((const char *src))
    OCD ("utf_util") ;

extern  size_t  utf32len P_((const char *src))
    OCD ("utf_util") ;

extern  int32_t  utf8get P_((const char *src,
                             size_t *numUnits))
    OCD ("utf_util") ;

extern  errno_t  utf8put P_((int32_t codePoint,
                             char *dst,
                             size_t *numUnits))
    OCD ("utf_util") ;

extern  int32_t  utf16get P_((bool bigEndian,
                              const char *src))
    OCD ("utf_util") ;

extern  errno_t  utf16put P_((int32_t codePoint,
                              bool bigEndian,
                              char *dst))
    OCD ("utf_util") ;

extern  int32_t  utf32get P_((bool bigEndian,
                              const char *src))
    OCD ("utf_util") ;

extern  errno_t  utf32put P_((int32_t codePoint,
                              bool bigEndian,
                              char *dst))
    OCD ("utf_util") ;

extern  ssize_t  utf8utf16 P_((const char *src,
                               ssize_t srclen,
                               int bom,
                               size_t dstlen,
                               char *dst))
    OCD ("utf_util") ;

extern  ssize_t  utf8utf32 P_((const char *src,
                               ssize_t srclen,
                               int bom,
                               size_t dstlen,
                               char *dst))
    OCD ("utf_util") ;

extern  ssize_t  utf16utf8 P_((const char *src,
                               ssize_t srclen,
                               int bom,
                               size_t dstlen,
                               char *dst))
    OCD ("utf_util") ;

extern  ssize_t  utf32utf8 P_((const char *src,
                               ssize_t srclen,
                               int bom,
                               size_t dstlen,
                               char *dst))
    OCD ("utf_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
