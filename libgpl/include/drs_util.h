/* $Id: drs_util.h,v 1.6 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    drs_util.h

    Directory Scanning Utility Definitions.

*******************************************************************************/

#ifndef  DRS_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  DRS_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Directory Scan (Client View) and Definitions.
*******************************************************************************/

					/* Scan handle. */
typedef  struct  _DirectoryScan  *DirectoryScan ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  drs_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  drsCount P_((DirectoryScan scan)) ;

extern  errno_t  drsCreate P_((const char *pathname,
                               DirectoryScan *scan)) ;

extern  errno_t  drsDestroy P_((DirectoryScan scan)) ;

extern  const  char  *drsFirst P_((DirectoryScan scan)) ;

extern  const  char  *drsGet P_((DirectoryScan scan,
                                 int which)) ;

extern  const  char  *drsNext P_((DirectoryScan scan)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
