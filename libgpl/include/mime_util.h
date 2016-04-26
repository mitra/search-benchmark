/* $Id: mime_util.h,v 1.2 2009/09/09 22:38:13 alex Exp alex $ */
/*******************************************************************************

    mime_util.h

    Mime Type Definitions.

*******************************************************************************/

#ifndef  MIME_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  MIME_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */


/*******************************************************************************
    Mime Map Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _MimeMap  *MimeMap ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  mime_util_debug  OCD ("mime_uti") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  mimeAdd P_((MimeMap map,
                             const char *type,
                             const char *extension))
    OCD ("mime_uti") ;

extern  size_t  mimeCount P_((MimeMap map,
                              const char *type))
    OCD ("mime_uti") ;

extern  errno_t  mimeCreate P_((MimeMap *map))
    OCD ("mime_uti") ;

extern  errno_t  mimeDelete P_((MimeMap map,
                                const char *type,
                                const char *extension))
    OCD ("mime_uti") ;

extern  errno_t  mimeDestroy P_((MimeMap map))
    OCD ("mime_uti") ;

extern  const  char  *mimeFind P_((MimeMap map,
                                   const char *extension))
    OCD ("mime_uti") ;

extern  const  char  *mimeGet P_((MimeMap map,
                                  size_t index,
                                  const char *type,
                                  const char **extension))
    OCD ("mime_uti") ;

extern  errno_t  mimeLoad P_((MimeMap map,
                              const char *fileName))
    OCD ("mime_uti") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
