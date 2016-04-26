/* $Id: info_util.h,v 1.4 2009/09/09 22:38:13 alex Exp alex $ */
/*******************************************************************************

    info_util.h

    Information Repository Definitions.

*******************************************************************************/

#ifndef  INFO_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  INFO_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Repository (Client View) and Definitions.
*******************************************************************************/

					/* Repository handle. */
typedef  struct  _Repository  *Repository ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  info_util_debug  OCD ("info_uti") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  infoCreate P_((const char *pathName,
                                const char *options,
                                Repository *dictionary))
    OCD ("info_uti") ;

extern  errno_t  infoDelete P_((Repository dictionary,
                                const char *path,
                                const char *name))
    OCD ("info_uti") ;

extern  errno_t  infoDestroy P_((Repository dictionary))
    OCD ("info_uti") ;

extern  const  char  *infoGet P_((Repository dictionary,
                                  const char *path,
                                  const char *name))
    OCD ("info_uti") ;

extern  errno_t  infoMerge P_((Repository dictionary,
                               const char *pathName))
    OCD ("info_uti") ;

extern  errno_t  infoSave P_((Repository dictionary,
                              const char *pathName))
    OCD ("info_uti") ;

extern  errno_t  infoSet P_((Repository dictionary,
                             const char *path,
                             const char *name,
                             const char *value))
    OCD ("info_uti") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
