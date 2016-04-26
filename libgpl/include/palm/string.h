/* $Id: string.h,v 1.3 2005/02/12 00:07:08 alex Exp $ */
/*******************************************************************************

    string.h

    Standard C Library String Functions.

*******************************************************************************/

#ifndef  PALM_STRING_H		/* Has the file been INCLUDE'd already? */
#define  PALM_STRING_H  yes


#include  <MemoryMgr.h>			/* Palm SDK header. */
#include  <StringMgr.h>			/* Palm SDK header. */
#include  <SysUtils.h>			/* Palm SDK header. */
#include  <sys_types.h>			/* Palm SDK header. */
#include  <unix_string.h>		/* Palm SDK header. */
#include  "str_util.h"			/* String manipulation functions. */


static  char  palmErrorString[128]  OCD ("string_h") ;
#define  strerror(code) \
	SysErrString (code, palmErrorString, sizeof palmErrorString)

#define  memmove(dst,src,len) \
	(MemMove (dst, src, len) ? dst : dst)

#define  strcasecmp(s1,s2) \
	StrCaselessCompare (s1, s2)

#define  strncasecmp(s1,s2,len) \
	StrNCaselessCompare (s1, s2, len)

#define  strncat(dst,src,len) \
	StrNCat (dst, src, len)

#define  strncmp(s1,s2,len) \
	StrNCompare (s1, s2, len)


#endif				/* If this file was not INCLUDE'd previously. */
