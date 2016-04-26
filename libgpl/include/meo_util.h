/* $Id: meo_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    meo_util.h

    Memory Operations.

*******************************************************************************/

#ifndef  MEO_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  MEO_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  <stdio.h>			/* Standard I/O definitions. */
#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Numerical bases for meoDump().
*******************************************************************************/

typedef  enum  MeoBase {
    MeoNone = 0,
    MeoOctal = 8,
    MeoDecimal = 10,
    MeoHexadecimal = 16,
    MeoText = 26
}  MeoBase ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  meo_util_debug  OCD ("meo_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  meoDump P_((FILE *file,
                             const char *indentation,
                             MeoBase base,
                             int numBytesPerLine,
                             void *address,
                             const void *buffer,
                             int numBytesToDump))
    OCD ("meo_util") ;

#define  meoDumpD(file, indentation, address, buffer, numBytesToDump) \
    meoDump (file, indentation, MeoDecimal, 8, address, buffer, numBytesToDump)

#define  meoDumpO(file, indentation, address, buffer, numBytesToDump) \
    meoDump (file, indentation, MeoOctal, 8, address, buffer, numBytesToDump)

#define  meoDumpT(file, indentation, address, buffer, numBytesToDump) \
    meoDump (file, indentation, MeoText, 40, address, buffer, numBytesToDump)

#define  meoDumpX(file, indentation, address, buffer, numBytesToDump) \
    meoDump (file, indentation, MeoHexadecimal, 16, address, buffer, numBytesToDump)

extern  errno_t  meoLoad P_((const char *fileName,
                             long offset,
                             void **startAddress,
                             long *numBytes))
    OCD ("meo_util") ;

extern  errno_t  meoSave P_((void *startAddress,
                             long numBytes,
                             const char *fileName,
                             long offset))
    OCD ("meo_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
