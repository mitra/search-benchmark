/* $Id: get_util.h,v 1.11 2009/09/09 22:38:13 alex Exp alex $ */
/*******************************************************************************

    get_util.h

    "Get Next" Functions.

*******************************************************************************/

#ifndef  GET_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  GET_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  char  *getarg P_((const char *arg,
                          ssize_t *length))
    OCD ("get_util") ;

extern  char  *getfield P_((const char *s,
                            size_t *length))
    OCD ("get_util") ;

extern  char  *getfield_c P_((const char *s,
                              char delimiter,
                              size_t *length))
    OCD ("get_util") ;

extern  char  *getfield_s P_((const char *s,
                              const char *separator,
                              size_t *length))
    OCD ("get_util") ;

extern  char  *getstring P_((const char *lastArgument,
                             const char *quotes,
                             size_t *length))
    OCD ("get_util") ;

extern  char  *getword P_((const char *string,
                           const char *delimiters,
                           size_t *length))
    OCD ("get_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
