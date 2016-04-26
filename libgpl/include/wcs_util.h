/* $Id: wcs_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    wcs_util.h

    Wide-Character String Manipulation Functions.

*******************************************************************************/

#ifndef  WCS_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  WCS_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if defined(HAVE_STDDEF_H) && !HAVE_STDDEF_H
    typedef  unsigned  int  wchar_t
#else
#    include  <stddef.h>		/* Standard C definitions. */
#endif
#if !defined(HAVE_WCHAR_H) || HAVE_WCHAR_H
#    include  <wchar.h>			/* C Library wide string functions. */
#endif

#if defined(HAVE_WCS) && !HAVE_WCS	/* No WCS?  Set individual flags. */
#    ifndef HAVE_WCSCMP
#        define  HAVE_WCSCMP  0
#    endif
#    ifndef HAVE_WCSNCMP
#        define  HAVE_WCSNCMP  0
#    endif
#    ifndef HAVE_WCSCPY
#        define  HAVE_WCSCPY  0
#    endif
#    ifndef HAVE_WCSNCPY
#        define  HAVE_WCSNCPY  0
#    endif
#    ifndef HAVE_WCSDUP
#        define  HAVE_WCSDUP  0
#    endif
#    ifndef HAVE_WCSLEN
#        define  HAVE_WCSLEN  0
#    endif
#    ifndef HAVE_MBSTOWCS
#        define  HAVE_MBSTOWCS  0
#    endif
#    ifndef HAVE_WCSTOMBS
#        define  HAVE_WCSTOMBS  0
#    endif
#endif


/*******************************************************************************
    Public functions.
*******************************************************************************/

#if defined(HAVE_WCSCMP) && !HAVE_WCSCMP
    extern  int  wcscmp P_((const wchar_t *thisString,
                            const wchar_t *thatString))
        OCD ("wcs_util") ;
#endif

#if defined(HAVE_WCSNCMP) && !HAVE_WCSNCMP
    extern  int  wcsncmp P_((const wchar_t *thisString,
                             const wchar_t *thatString,
                             size_t length))
        OCD ("wcs_util") ;
#endif

#if defined(HAVE_WCSCPY) && !HAVE_WCSCPY
    extern  wchar_t  *wcscpy P_((wchar_t *destination,
                                 const wchar_t *source))
        OCD ("wcs_util") ;
#endif

#if defined(HAVE_WCSNCPY) && !HAVE_WCSNCPY
    extern  wchar_t  *wcsncpy P_((wchar_t *destination,
                                  const wchar_t *source,
                                  size_t length))
        OCD ("wcs_util") ;
#endif

#if defined(HAVE_WCSDUP) && !HAVE_WCSDUP
    extern  wchar_t  *wcsdup P_((const wchar_t *wideString))
        OCD ("wcs_util") ;
#endif

extern  wchar_t  *wcsndup P_((const wchar_t *wideString,
                              size_t length))
    OCD ("wcs_util") ;

#if defined(HAVE_WCSLEN) && !HAVE_WCSLEN
    extern  size_t  wcslen P_((const wchar_t *wideString))
        OCD ("wcs_util") ;
#endif

#if defined(HAVE_MBSTOWCS) && !HAVE_MBSTOWCS
    extern  size_t  mbstowcs P_((wchar_t *wideString,
                                 const char *narrowString,
                                 size_t maxWide))
        OCD ("wcs_util") ;
#endif

#if defined(HAVE_WCSTOMBS) && !HAVE_WCSTOMBS
    extern  size_t  wcstombs P_((char *narrowString,
                                 const wchar_t *wideString,
                                 size_t maxNarrow))
        OCD ("wcs_util") ;
#endif

extern  char  *wcsNarrow P_((const wchar_t *wideString,
                             bool dynamic))
    OCD ("wcs_util") ;

extern  wchar_t  *wcsWiden P_((const char *narrowString,
                               bool dynamic))
    OCD ("wcs_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
