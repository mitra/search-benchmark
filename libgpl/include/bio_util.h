/* $Id: bio_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    bio_util.h

    Buffered Input/Output Utility Definitions.

*******************************************************************************/

#ifndef  BIO_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  BIO_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Buffered I/O Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _BioStream  *BioStream ;	/* Buffered I/O stream handle. */

					/* Input/output function prototypes. */
typedef  errno_t  (*BioInputF) P_((void *, double, ssize_t, char *, size_t *)) ;
typedef  errno_t  (*BioOutputF) P_((void *, double, ssize_t, const char *, size_t *)) ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  bio_util_debug  OCD ("bio_util") ;
					/* I/O timing debug switch (1/0 = yes/no). */
extern  int  bio_timing_debug  OCD ("bio_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  bioCreate P_((void *ioStream,
                               BioInputF inputF,
                               size_t inputBufferSize,
                               BioOutputF outputF,
                               size_t outputBufferSize,
                               BioStream *stream))
    OCD ("bio_util") ;

extern  errno_t  bioDestroy P_((BioStream stream))
    OCD ("bio_util") ;

extern  errno_t  bioFlush P_((BioStream stream))
    OCD ("bio_util") ;

extern  size_t  bioPendingInput P_((BioStream stream))
    OCD ("bio_util") ;

extern  size_t  bioPendingOutput P_((BioStream stream))
    OCD ("bio_util") ;

extern  errno_t  bioRead P_((BioStream stream,
                             double timeout,
                             size_t numBytesToRead,
                             char *buffer,
                             size_t *numBytesRead))
    OCD ("bio_util") ;

extern  errno_t  bioWrite P_((BioStream stream,
                              size_t numBytesToWrite,
                              const char *data))
    OCD ("bio_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
