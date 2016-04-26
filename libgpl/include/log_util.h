/* $Id: log_util.h,v 1.4 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    log_util.h

    Logging Utilities.

*******************************************************************************/

#ifndef  LOG_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  LOG_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Log File Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _LogFile  *LogFile ;	/* Event log handle. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  log_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  logClose P_((LogFile log)) ;

extern  int  logFlush P_((LogFile log)) ;

extern  const  char  *logName P_((LogFile log)) ;

extern  int  logOpen P_((const char *name,
                         const char *options,
                         LogFile *log)) ;

extern  int  logWrite P_((LogFile log,
                          const char *format,
                          ...)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
