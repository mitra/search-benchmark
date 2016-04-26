/* $Id: hash_util.h,v 1.10 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    hash_util.h

    Hash Table Definitions.

*******************************************************************************/

#ifndef  HASH_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  HASH_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stdio.h>			/* Standard I/O definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Hash Table Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _HashTable  *HashTable ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  hash_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  hashAdd P_((HashTable table,
                         const char *key,
                         const void *data)) ;

extern  int  hashCount P_((HashTable table)) ;

extern  int  hashCreate P_((int maxEntries,
                            HashTable *table)) ;

extern  int  hashDelete P_((HashTable table,
                            const char *key)) ;

extern  int  hashDestroy P_((HashTable table)) ;

extern  int  hashDump P_((FILE *outfile,
                          const char *header,
                          HashTable table)) ;

extern  void  *hashFind P_((HashTable table,
                            const char *name)) ;

extern  const  char  *hashGet P_((HashTable table,
                                  int index,
                                  void **value)) ;

extern  int  hashSearch P_((HashTable table,
                            const char *key,
                            void **data)) ;

#ifdef HASH_STATISTICS			/* Requires math library for sqrt(). */
extern  int  hashStatistics P_((FILE *outfile,
                                HashTable table)) ;
#endif


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
