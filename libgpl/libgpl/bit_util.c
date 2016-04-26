/* $Id: bit_util.c,v 1.5 2003/03/26 02:17:11 alex Exp $ */
/*******************************************************************************

File:

    bit_util.c

    Bit Manipulation Utilities.


Author:    Alex Measday


Purpose:

    The BIT_UTIL functions perform various operations on bits and bytes.


Public Procedures (* defined as macros):

  * bitBit() - extracts a bit from a value.
  * bitByte() - extracts an 8-bit byte from a value.
  * bitNibble() - extracts a 4-bit nibble from a value.
    bitReverseByte() - reverse the order of bits in an 8-bit byte.
    bitReverseNibble() - reverse the order of bits in a 4-bit nibble.
  * bitWord() - extracts a 16-bit word from a value.

*******************************************************************************/


#include  "bit_util.h"			/* Bit manipulation functions. */

/*******************************************************************************

Procedure:

    bitReverseByte ()

    Reverse the Order of Bits in an 8-bit Byte.


Purpose:

    Function bitReverseByte() reverses the order of bits in an 8-bit byte;
    e.g., the reverse of 10110101 is 10101101.


    Invocation:

        result = bitReverseByte (value) ;

    where

        <value>		- I
            is the byte to be reversed.
        <result>	- O
            returns the byte with the bits in reverse order.

*******************************************************************************/


unsigned  char  bitReverseByte (

#    if PROTOTYPES
        unsigned  char  value)
#    else
        value)

        unsigned  char  value ;
#    endif

{

    return ((bitReverseNibble (bitNibble (value, 0)) << 4) |
            bitReverseNibble (bitNibble (value, 1))) ;

}

/*******************************************************************************

Procedure:

    bitReverseNibble ()

    Reverse the Order of Bits in a 4-bit Nibble.


Purpose:

    Function bitReverseNibble() reverses the order of bits in a 4-bit nibble;
    e.g., the reverse of 1100 is 0011.


    Invocation:

        result = bitReverseNibble (value) ;

    where

        <value>		- I
            is a value containing, in its low 4 bits, the nibble to be reversed.
        <result>	- O
            returns the nibble with the bits in reverse order.

*******************************************************************************/


unsigned  char  bitReverseNibble (

#    if PROTOTYPES
        unsigned  char  value)
#    else
        value)

        unsigned  char  value ;
#    endif

{    /* Local variables. */
    static  unsigned  char  reversedBits[16] = {
        0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E,
        0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F
    } ;



    return (reversedBits[value & 0x0F]) ;

}
