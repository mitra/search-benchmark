/* $Id: bit_util.h,v 1.5 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    bit_util.h

    Bit Manipulation Utility Definitions.

*******************************************************************************/

#ifndef  BIT_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  BIT_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

					/* Returns I-th bit from value. */
#define  bitBit(value, which)  (((value) >> (which)) & 0x01)
					/* Returns I-th nibble from value. */
#define  bitNibble(value, which)  (((value) >> ((which) * 4)) & 0x0F)
					/* Returns I-th byte from value. */
#define  bitByte(value, which)  (((value) >> ((which) * 8)) & 0x0FF)
					/* Returns I-th 16-bit word from value. */
#define  bitWord(value, which)  (((value) >> ((which) * 16)) & 0x0FFFF)

extern  unsigned  char  bitReverseByte P_((unsigned char value)) ;

extern  unsigned  char  bitReverseNibble P_((unsigned char value)) ;

extern  unsigned  char  bitSwapNibble P_((unsigned char value)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
