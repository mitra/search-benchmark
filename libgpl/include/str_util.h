/* $Id: str_util.h,v 1.18 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    str_util.h

    String Manipulation Functions.

*******************************************************************************/

#ifndef  STR_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  STR_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stddef.h>			/* Standard C definitions. */
#include  <string.h>			/* Standard C string functions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  char  *strConvert P_((char *string))
    OCD ("str_util") ;

extern  char  *strDestring P_((char *string,
                               ssize_t length,
                               const char *quotes))
    OCD ("str_util") ;

extern  size_t  strDetab P_((char *stringWithTabs,
                             ssize_t length,
                             int tabStops,
                             char *stringWithoutTabs,
                             size_t maxLength))
    OCD ("str_util") ;

extern  void  strEnv P_((const char *string,
                         ssize_t length,
                         char *translation,
                         size_t maxLength))
    OCD ("str_util") ;

extern  char  *strEtoA P_((char *string,
                           ssize_t length))
    OCD ("str_util") ;

extern  size_t  strInsert P_((const char *substring,
                              ssize_t subLength,
                              size_t offset,
                              char *string,
                              size_t maxLength))
    OCD ("str_util") ;

extern  bool  strMatch P_((const char *target,
                           const char *model))
    OCD ("str_util") ;

extern  size_t  strRemove P_((size_t numChars,
                              size_t offset,
                              char *string))
    OCD ("str_util") ;

extern  char  *strToLower P_((char *string,
                              ssize_t length))
    OCD ("str_util") ;

extern  char  *strToUpper P_((char *string,
                              ssize_t length))
    OCD ("str_util") ;

extern  size_t  strTrim P_((char *string,
                            ssize_t length))
    OCD ("str_util") ;

#if !defined(HAVE_MEMDUP) || !HAVE_MEMDUP
    extern  void  *memdup P_((const void *source,
                              size_t length))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STPCPY) && !HAVE_STPCPY
    extern  char  *stpcpy P_((char *destination,
                              const char *source))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STRLCAT) && !HAVE_STRLCAT
    extern  size_t  strlcat P_((char *destination,
                                const char *source,
                                size_t length))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STRLCPY) && !HAVE_STRLCPY
    extern  size_t  strlcpy P_((char *destination,
                                const char *source,
                                size_t length))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STRCASECMP) && !HAVE_STRCASECMP
    extern  int  strcasecmp P_((const char *thisString,
                                const char *thatString))
        OCD ("str_util") ;

    extern  int  strncasecmp P_((const char *thisString,
                                 const char *thatString,
                                 size_t length))
        OCD ("str_util") ;
#endif

extern  char  *strnchr P_((const char *string,
                           int c,
                           size_t length))
    OCD ("str_util") ;

extern  char  *strncpym P_((char *destination,
                            const char *source,
                            size_t length,
                            size_t maximum))
    OCD ("str_util") ;

#if defined(HAVE_STRDUP) && !HAVE_STRDUP
    extern  char  *strdup P_((const char *string))
        OCD ("str_util") ;
#endif

extern  char  *strndup P_((const char *string,
                           size_t length))
    OCD ("str_util") ;

#if defined(HAVE_STRRCHR) && !HAVE_STRRCHR
    extern  char  *strrchr P_((const char *string,
                               int c))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STRSPN) && !HAVE_STRSPN
    extern  size_t  strspn P_((const char *string,
                               const char *accept))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STRCSPN) && !HAVE_STRCSPN
    extern  size_t  strcspn P_((const char *string,
                                const char *reject))
        OCD ("str_util") ;
#endif

#if defined(HAVE_STRTOK) && !HAVE_STRTOK
    extern  char  *strtok P_((char *string,
                              const char *delimiters))
        OCD ("str_util") ;
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
