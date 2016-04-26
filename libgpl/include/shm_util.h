/* $Id: shm_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    shm_util.h

    Shared Memory Utility Definitions.

*******************************************************************************/

#ifndef  SHM_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  SHM_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Shared Memory (Client View).
*******************************************************************************/

typedef  struct  _SharedMemory  *SharedMemory ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  shm_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  void  *shmAddress P_((SharedMemory memory)) ;

extern  int  shmCreate P_((const char *name,
                           long size,
                           void **address,
                           SharedMemory *memory)) ;

extern  int  shmDestroy P_((SharedMemory memory)) ;

extern  int  shmId P_((SharedMemory memory)) ;

extern  int  shmLoad P_((SharedMemory memory,
                         const char *fileName)) ;

extern  int  shmSave P_((SharedMemory memory,
                         const char *fileName)) ;

extern  long  shmSizeOf P_((SharedMemory memory)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
