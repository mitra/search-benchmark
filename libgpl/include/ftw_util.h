/* $Id: ftw_util.h,v 1.4 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    ftw_util.h

    File Tree Walk Definitions.

*******************************************************************************/

#ifndef  FTW_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  FTW_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <sys/stat.h>			/* File status definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Miscellaneous Definitions.
*******************************************************************************/

typedef  enum  FtwFileType {
    FtwFile,
    FtwDirectory,
    FtwDirNoRead,
    FtwNoStat,
    FtwVisited,
    FtwSymbolicLink
}  FtwFileType ;

typedef  enum  FtwFlag {
    FtwPhysical = 1,
    FtwMount = 2,
    FtwDFS = 4,
    FtwChdir = 8,
    FtwCDF = 16,
    FtwStatErr = 32
}  FtwFlag ;

					/* Callback function prototype. */
typedef  int  (*FtwFileCallback) P_((const char *pathname,
                                     const char *filename,
                                     FtwFileType type,
                                     struct stat *info,
                                     int level,
                                     void *clientData)) ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  ftw_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  fileTreeWalk P_((const char *root,
                              const char *filename,
                              FtwFlag flags,
                              FtwFileCallback callbackF,
                              void *clientData)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
