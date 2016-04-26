/* $Id: f1750a_util.c,v 1.9 2011/07/18 17:46:04 alex Exp $ */
/*******************************************************************************

File:

    f1750a_util.c

    MIL-STD-1750A Floating Point Utilities.


Author:    Alex Measday


Purpose:

    The F1750A utilities are used to convert floating-point numbers between
    the host CPU's native format and MIL-STD-1750A floating-point format.

    The supported MIL-STD-1750A floating-point formats are a non-standard
    16-bit representation and the standard 32- and 48-bit representations.
    In the following, big-endian bit layouts, "M" is a mantissa bit and "E"
    is an exponent bit.  Both the mantissa and the exponent are treated as
    twos-complement numbers; diagrams misleadingly show the most-significant
    bit of the mantissa as a sign bit.

        16-bit Representation (6-bit exponent ranging from -32 to +31;
                               10-bit mantissa ranging from -512 to +511)

            MMMMMMMM MMEEEEEE

        32-bit Representation (8-bit exponent ranging from -128 to +127;
                               24-bit mantissa ranging from -8,388,608
                               to +8,388,607)

            MMMMMMMM MMMMMMMM MMMMMMMM EEEEEEEE

        48-bit Representation (8-bit exponent ranging from -128 to +127;
                               40-bit mantissa ranging from -549,755,813,888
                               to +549,755,813,887)

            MMMMMMMM MMMMMMMM MMMMMMMM EEEEEEEE
            MMMMMMMM MMMMMMMM

    In all representations, the radix point is immediately to the right of
    the most-significant bit of the mantissa, giving a nominal range of:

        -1.0 <= mantissa < +1.0

    (Normalization of negative numbers produces a gap down in the negative
    range.)  Taking the radix point into account, the value of a floating-point
    number in MIL-STD-1750A format equals:

        mantissa * (2 ** exponent)

    If you treat the full N bits of the mantissa as a signed integer,
    the floating-point value can then be computed simply as:

        mantissa * (2 ** (exponent - precision))

    where the precision is one less than the width in bits of the mantissa.


Public Procedures:

    double2f1750a() - converts a native double to MIL-STD-1750A format.
    f1750a2double() - converts MIL-STD-1750A format to a native double.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <math.h>			/* Math library definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  "f1750a_util.h"		/* 1750A floating point utilities. */


#define  COMPLEMENT_40_BITS(mantissa)			\
  { (mantissa)[0] = ~(mantissa)[0] & 0x00FFFFFF ;	\
    (mantissa)[1] = ~(mantissa)[1] & 0x0000FFFF ;	\
  }

#define  INCREMENT_40_BITS(mantissa) 					\
  { (mantissa)[1]++ ;							\
    if (((mantissa)[1] & (1UL << 16)) == (1UL << 16))  (mantissa)[0]++ ;\
  }

#define  SHIFT_LEFT_40_BITS(mantissa)			\
  { (mantissa)[0] <<= 1 ;  (mantissa)[1] <<= 1 ;	\
    if (((mantissa)[1] & (1UL << 16)) == (1UL << 16))	\
        (mantissa)[0] |= 0x00000001 ;			\
  }

#define  SHIFT_RIGHT_40_BITS(mantissa)			\
  { if ((mantissa)[0] & 0x00000001)			\
        (mantissa)[1] |= (1UL << 16) ;			\
    (mantissa)[0] >>= 1 ;  (mantissa)[1] >>= 1 ;	\
  }

/*!*****************************************************************************

Procedure:

    double2f1750a ()

    Convert a Native Double to a MIL-STD-1750A Floating-Point Number.


Purpose:

    The f1750a2double() function converts a floating-point number from
    the host CPU's native floating-point format to MIL-STD-1750A format.


    Invocation:

        status = f1750a2double (value, numBits, &buffer) ;

    where

        <value>		- I
            is the floating-point number in the host CPU's native format.
        <numBits>	- I
            is the number of bits in the MIL-STD-1750A-format number;
            supported representations are 16-, 32-, and 48-bit numbers.
        <buffer>	- O
            receives the MIL-STD-1750A-format number.  The byte order
            is always big-endian.
        <status>	- O
            returns the conversion status, zero if the conversion was
            successful and ERRNO otherwise.

*******************************************************************************/


errno_t  double2f1750a (

#    if PROTOTYPES
        double  value,
        size_t  numBits,
        uint8_t  *buffer)
#    else
        value, numBits, buffer)

        double  value ;
        size_t  numBits ;
        uint8_t  *buffer ;
#    endif

{    /* Local variables. */
    bool  isNegative ;
    double  fraction, integer ;
    int  exponent ;
    unsigned  long  mantissa[2] ;




    if ((numBits != 16) && (numBits != 32) && (numBits != 48)) {
        SET_ERRNO (EINVAL) ;
        LGE "(double2f1750a) Invalid number of bits: %d\n", numBits) ;
        return (errno) ;
    } else if (buffer == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(double2f1750a) NULL buffer: ") ;
        return (errno) ;
    }

/*******************************************************************************
    Convert the host CPU value to a non-standard, 16-bit, 1750A floating-point
    number.
*******************************************************************************/

    if (numBits == 16) {

        isNegative = (value < 0.0) ;
        if (isNegative)  value = -value ;

/* Break the absolute value into a fraction (0.5 < = f < 1.0) and a
   power-of-two exponent. */

        fraction = frexp (value, &exponent) ;

/* Get the 10 most-significant bits of the mantissa by truncating the result
   of multiplying the fraction by 2**10.  Only 9 bits are needed for the
   1750A mantissa (since this is a positive number); the 10th bit is used
   for rounding. */

        mantissa[0] = (unsigned long) ldexp (fraction, 10) ;

/* If the least-significant bit is set, then round the mantissa upwards
   by incrementing it.  Then, shift the rounding bit out of the mantissa. */

        if (mantissa[0] & 0x00000001)  mantissa[0]++ ;
        mantissa[0] >>= 1 ;

/* If the rounding overflowed into a new most-significant bit position,
   then drop the least-significant bit and adjust the exponent. */

        if (mantissa[0] & (1UL << 9)) {
            mantissa[0] >>= 1 ;
            exponent++ ;
        }

/* If the floating-point value is negative, then negate the mantissa
   using twos-complement arithmetic. */

        if (isNegative) {
            mantissa[0] = ~mantissa[0] ;
            mantissa[0]++ ;
        }

/* Normalize the mantissa.  A 1750A floating-point number is normalized if
   its two most-significant bits have opposite values.  For a positive input
   value, the twos-complement mantissa is already normalized (binary 01...).
   Negative mantisssas with the two most-significant bits set (binary 11...)
   must be shifted left and the exponent adjusted until binary "10" is in the
   two most-significant bits. */

        while ((mantissa[0] & (3UL << 8)) == (3UL << 8)) {
            mantissa[0] <<= 1 ;
            exponent-- ;
        }

/* Restrict the exponent to its valid range and compute its twos-complement
   representation. */

        if (exponent > 31)
            exponent = 31 ;
        else if (exponent < -32)
            exponent = -32 ;

        if (exponent < 0) {
            exponent = -exponent ;
            exponent = ~exponent ;
            exponent++ ;
        }

/* Finally, construct the 1750A floating-point representation of the input
   value. */

        buffer[0] = (uint8_t) ((mantissa[0] >> 2) & 0x000000FF) ;
        buffer[1] = (uint8_t) (((mantissa[0] & 0x00000003) << 6) |
                              (exponent & 0x0000003F)) ;

    }

/*******************************************************************************
    Convert the host CPU value to a 32-bit, 1750A floating-point number.
*******************************************************************************/

    else if (numBits == 32) {

        isNegative = (value < 0.0) ;
        if (isNegative)  value = -value ;

/* Break the absolute value into a fraction (0.5 < = f < 1.0) and a
   power-of-two exponent. */

        fraction = frexp (value, &exponent) ;

/* Get the 24 most-significant bits of the mantissa by truncating the result
   of multiplying the fraction by 2**24.  Only 23 bits are needed for the
   1750A mantissa (since this is a positive number); the 24th bit is used
   for rounding. */

        mantissa[0] = (unsigned long) ldexp (fraction, 24) ;

/* If the least-significant bit is set, then round the mantissa upwards
   by incrementing it.  Then, shift the rounding bit out of the mantissa. */

        if (mantissa[0] & 0x00000001)  mantissa[0]++ ;
        mantissa[0] >>= 1 ;

/* If the rounding overflowed into a new most-significant bit position,
   then drop the least-significant bit and adjust the exponent. */

        if (mantissa[0] & (1UL << 23)) {
            mantissa[0] >>= 1 ;
            exponent++ ;
        }

/* If the floating-point value is negative, then negate the mantissa
   using twos-complement arithmetic. */

        if (isNegative) {
            mantissa[0] = ~mantissa[0] ;
            mantissa[0]++ ;
        }

/* Normalize the mantissa.  A 1750A floating-point number is normalized if
   its two most-significant bits have opposite values.  For a positive input
   value, the twos-complement mantissa is already normalized (binary 01...).
   Negative mantisssas with the two most-significant bits set (binary 11...)
   must be shifted left and the exponent adjusted until binary "10" is in the
   two most-significant bits. */

        while ((mantissa[0] & (3UL << 22)) == (3UL << 22)) {
            mantissa[0] <<= 1 ;
            exponent-- ;
        }

/* Restrict the exponent to its valid range and compute its twos-complement
   representation. */

        if (exponent > 127)
            exponent = 127 ;
        else if (exponent < -128)
            exponent = -128 ;

        if (exponent < 0) {
            exponent = -exponent ;
            exponent = ~exponent ;
            exponent++ ;
        }

/* Finally, construct the 1750A floating-point representation of the input
   value. */

        buffer[0] = (uint8_t) ((mantissa[0] >> 16) & 0x000000FF) ;
        buffer[1] = (uint8_t) ((mantissa[0] >> 8) & 0x000000FF) ;
        buffer[2] = (uint8_t) (mantissa[0] & 0x000000FF) ;
        buffer[3] = (uint8_t) (exponent & 0x000000FF) ;

    }

/*******************************************************************************
    Convert the host CPU value to a 48-bit, 1750A floating-point number.
*******************************************************************************/

    else if (numBits == 48) {

        isNegative = (value < 0.0) ;
        if (isNegative)  value = -value ;

/* Break the absolute value into a fraction (0.5 < = f < 1.0) and a
   power-of-two exponent. */

        fraction = frexp (value, &exponent) ;

/* Get the 40 most-significant bits of the mantissa.  Only 39 bits are needed
   for the 1750A mantissa (since this is a positive number); the 40th bit is
   used for rounding.  The 24 most-significant bits are obtained by truncating
   the result of multiplying the fraction by 2**24; these 24 bits are stored
   in mantissa[0].  The truncated portion of the result is then multiplied by
   2**16 to get the remaining 16 bits, which are stored in mantissa[1]. */

        fraction = ldexp (fraction, 24) ;
        mantissa[0] = (unsigned long) fraction ;
#if defined(HAVE_MODF) && !HAVE_MODF
#    define  modf(f,i)  ((f) - (double) ((long) (f)))
#endif
        mantissa[1] = (unsigned long) ldexp (modf (fraction, &integer), 16) ;

/* If the least-significant bit is set, then round the mantissa upwards
   by incrementing it.  Then, shift the rounding bit out of the mantissa. */

        if (mantissa[1] & 0x00000001)  INCREMENT_40_BITS (mantissa) ;
        SHIFT_RIGHT_40_BITS (mantissa) ;

/* If the rounding overflowed into a new most-significant bit position,
   then drop the least-significant bit and adjust the exponent. */

        if (mantissa[0] & (1UL << 23)) {
            SHIFT_RIGHT_40_BITS (mantissa) ;
            exponent++ ;
        }

/* If the floating-point value is negative, then negate the mantissa
   using twos-complement arithmetic. */

        if (isNegative) {
            COMPLEMENT_40_BITS (mantissa) ;
            INCREMENT_40_BITS (mantissa) ;
        }

/* Normalize the mantissa.  A 1750A floating-point number is normalized if
   its two most-significant bits have opposite values.  For a positive input
   value, the twos-complement mantissa is already normalized (binary 01...).
   Negative mantisssas with the two most-significant bits set (binary 11...)
   must be shifted left and the exponent adjusted until binary "10" is in the
   two most-significant bits. */

        while ((mantissa[0] & (3UL << 22)) == (3UL << 22)) {
            SHIFT_LEFT_40_BITS (mantissa) ;
            exponent-- ;
        }

/* Restrict the exponent to its valid range and compute its twos-complement
   representation. */

        if (exponent > 127)
            exponent = 127 ;
        else if (exponent < -128)
            exponent = -128 ;

        if (exponent < 0) {
            exponent = -exponent ;
            exponent = ~exponent ;
            exponent++ ;
        }

/* Finally, construct the 1750A floating-point representation of the input
   value. */

        buffer[0] = (uint8_t) ((mantissa[0] >> 16) & 0x000000FF) ;
        buffer[1] = (uint8_t) ((mantissa[0] >> 8) & 0x000000FF) ;
        buffer[2] = (uint8_t) (mantissa[0] & 0x000000FF) ;
        buffer[3] = (uint8_t) (exponent & 0x000000FF) ;
        buffer[4] = (uint8_t) ((mantissa[1] >> 8) & 0x000000FF) ;
        buffer[5] = (uint8_t) (mantissa[1] & 0x000000FF) ;

    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    f1750a2double ()

    Convert a MIL-STD-1750A Floating-Point Number to a Native Double.


Purpose:

    The f1750a2double() function converts a floating-point number in
    MIL-STD-1750A format to the host CPU's native floating-point format.


    Invocation:

        value = f1750a2double (numBits, buffer) ;

    where

        <numBits>	- I
            is the number of bits in the MIL-STD-1750A-format number;
            supported representations are 16-, 32-, and 48-bit numbers.
        <buffer>	- I
            is a buffer containing the MIL-STD-1750A-format number.
            The bytes are assumed to be in big-endian order.
        <value>		- O
            returns the floating-point number in the host CPU's native format.

*******************************************************************************/


double  f1750a2double (

#    if PROTOTYPES
        size_t  numBits,
        uint8_t  *buffer)
#    else
        numBits, buffer)

        size_t  numBits ;
        uint8_t  *buffer ;
#    endif

{    /* Local variables. */
    double  value ;
    int  exponent, precision ;
    long  mantissa ;



    if ((numBits != 16) && (numBits != 32) && (numBits != 48)) {
        SET_ERRNO (EINVAL) ;
        LGE "(f1750a2double) Invalid number of bits: %d\n", numBits) ;
        return (0.0) ;
    } else if (buffer == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(f1750a2double) NULL buffer: ") ;
        return (0.0) ;
    }

/* Extract the mantissa and exponent to form the floating-point value. */

    switch (numBits) {
    case 16:	/* 10 bits of mantissa, 6-bit exponent. */
        mantissa = ((int8_t) buffer[0] << 2) | (buffer[1] >> 6) ;
        if (buffer[1] & 0x20)
            exponent = (int8_t) ((buffer[1] & 0x3F) | 0xC0) ;
        else
            exponent = buffer[1] & 0x3F ;
        precision = 10 - 1 ;
        value = ldexp ((double) mantissa, exponent - precision) ;
        break ;
    case 32:	/* 24 bits of mantissa, 8-bit exponent. */
    case 48:	/* 24 bits of mantissa, 8-bit exponent, 16 bits of mantissa. */
    default:
        mantissa = (((long) ((int8_t) buffer[0])) << 16) |
                   ((unsigned long) buffer[1] << 8) |
                   buffer[2] ;
        exponent = (int8_t) buffer[3] ;
        precision = 24 - 1 ;
        value = ldexp ((double) mantissa, exponent - precision) ;
        if (numBits == 48) {	/* Add mantissa's 16 least-significant bits. */
            mantissa = ((unsigned long) buffer[4] << 8) | buffer[5] ;
            precision += 16 ;
            value += ldexp ((double) mantissa, exponent - precision) ;
        }
        break ;
    }

    return (value) ;

}

#ifdef TEST
/*******************************************************************************
    Test Program - examples are from the MIL-STD-1750A specification.
*******************************************************************************/

typedef  struct  Example16 {
    uint8_t  buffer[2] ;
    char  *result ;
}  Example16 ;

static  Example16  examples16[] = {	/* Made these up myself! */
    {{0x7F, 0xC0}, "0.9980469 x 2^0" },
    {{0x7F, 0xDF}, "0.9980469 x 2^31" },
    {{0x7F, 0xE0}, "0.9980469 x 2^-32" },
    {{0x40, 0x00}, "0.5 x 2^0" },
    {{0x40, 0x1F}, "0.5 x 2^31" },
    {{0x40, 0x20}, "0.5 x 2^-32" },
    {{0x00, 0x00}, "0.0 x 2^0" },
    {{0x80, 0x00}, "-1.0 x 2^0" },
    {{0x80, 0x1F}, "-1.0 x 2^31" },
    {{0x80, 0x20}, "-1.0 x 2^-32" },
    {{0xBF, 0xC0}, "-0.5019531 x 2^0" },
    {{0xBF, 0xDF}, "-0.5019531 x 2^31" },
    {{0xBF, 0xE0}, "-0.5019531 x 2^-32" },
    {{0x9F, 0xC0}, "-0.7519531 x 2^0" },
    {{0x9F, 0xDF}, "-0.7519531 x 2^31" },
    {{0x9F, 0xE0}, "-0.7519531 x 2^-32" },
    {{0xFF, 0xC0}, "-1.0 x 2^-9" },
    {{0xFF, 0xDF}, "-1.0 x 2^22" },
    {{0xFF, 0xE0}, "-1.0 x 2^-41" },
    {{0, 0}, NULL}
} ;

typedef  struct  Example32 {
    uint8_t  buffer[4] ;
    char  *result ;
}  Example32 ;

static  Example32  examples32[] = {	/* From TABLE III in MIL-STD-1750A. */
    {{0x7F, 0xFF, 0xFF, 0x7F}, "0.9999998 x 2^127" },
    {{0x40, 0x00, 0x00, 0x7F}, "0.5 x 2^127" },
    {{0x50, 0x00, 0x00, 0x04}, "0.625 x 2^4" },
    {{0x40, 0x00, 0x00, 0x01}, "0.5 x 2^1" },
    {{0x40, 0x00, 0x00, 0x00}, "0.5 x 2^0" },
    {{0x40, 0x00, 0x00, 0xFF}, "0.5 x 2^-1" },
    {{0x40, 0x00, 0x00, 0x80}, "0.5 x 2^-128" },
    {{0x00, 0x00, 0x00, 0x00}, "0.0 x 2^0" },
    {{0x80, 0x00, 0x00, 0x00}, "-1.0 x 2^0" },
    {{0xBF, 0xFF, 0xFF, 0x80}, "-0.5000001 x 2^-128" },
    {{0x9F, 0xFF, 0xFF, 0x04}, "-0.7500001 x 2^4" },
    {{0, 0, 0, 0}, NULL}
} ;

typedef  struct  Example48 {
    uint8_t  buffer[6] ;
    char  *result ;
}  Example48 ;

static  Example48  examples48[] = {	/* From TABLE IV in MIL-STD-1750A. */
    {{0x40, 0x00, 0x00, 0x7F, 0x00, 0x00}, "  0.5 x 2^127"},
    {{0x40, 0x00, 0x00, 0x00, 0x00, 0x00}, "  0.5 x 2^0"},
    {{0x40, 0x00, 0x00, 0xFF, 0x00, 0x00}, "  0.5 x 2^-1"},
    {{0x40, 0x00, 0x00, 0x80, 0x00, 0x00}, "  0.5 x 2^-128"},
    {{0x80, 0x00, 0x00, 0x7F, 0x00, 0x00}, " -1.0 x 2^127"},
    {{0x80, 0x00, 0x00, 0x00, 0x00, 0x00}, " -1.0 x 2^0"},
    {{0x80, 0x00, 0x00, 0xFF, 0x00, 0x00}, " -1.0 x 2^-1"},
    {{0x80, 0x00, 0x00, 0x80, 0x00, 0x00}, " -1.0 x 2^-128"},
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "  0.0 x 2^0"},
    {{0xA0, 0x00, 0x00, 0xFF, 0x00, 0x00}, "-0.75 x 2^-1"},
    {{0, 0, 0, 0, 0, 0}, NULL}
} ;


int  main (

    int argc,
    char *argv[])

{    /* Local variables. */
    uint8_t  buffer[6] ;
    double  mantissa, value ;
    int  exponent, i ;



    printf ("1750A 16-bit Floating-Point:\n") ;

    for (i = 0 ;  examples16[i].result != NULL ;  i++) {
        value = f1750a2double (16, examples16[i].buffer) ;
        mantissa = frexp (value, &exponent) ;
        if (mantissa == -0.5) {
            mantissa *= 2.0 ;  exponent-- ;
        }
        double2f1750a (value, 16, buffer) ;
        printf ("0x%02X%02X  %11.8g x 2^%d\t(%s)\t(0x%02X%02X)\n",
                buffer[0], buffer[1],
                mantissa, exponent, examples16[i].result,
                examples16[i].buffer[0], examples16[i].buffer[1]) ;
    }

    printf ("\n1750A 32-bit Floating-Point:\n") ;

    for (i = 0 ;  examples32[i].result != NULL ;  i++) {
        value = f1750a2double (32, examples32[i].buffer) ;
        mantissa = frexp (value, &exponent) ;
        if (mantissa == -0.5) {
            mantissa *= 2.0 ;  exponent-- ;
        }
        double2f1750a (value, 32, buffer) ;
        printf ("0x%02X%02X%02X%02X  %11.8g x 2^%d\t(%s)\t(0x%02X%02X%02X%02X)\n",
                buffer[0], buffer[1], buffer[2], buffer[3],
                mantissa, exponent, examples32[i].result,
                examples32[i].buffer[0], examples32[i].buffer[1],
                examples32[i].buffer[2], examples32[i].buffer[3]) ;
    }

    printf ("\n1750A 48-bit Floating-Point:\n") ;

    for (i = 0 ;  examples48[i].result != NULL ;  i++) {
        value = f1750a2double (48, examples48[i].buffer) ;
        mantissa = frexp (value, &exponent) ;
        if (mantissa == -0.5) {
            mantissa *= 2.0 ;  exponent-- ;
        }
        double2f1750a (value, 48, buffer) ;
        printf ("0x%02X%02X%02X%02X%02X%02X  %11.8g x 2^%d\t(%s)\t(0x%02X%02X%02X%02X%02X%02X)\n",
                buffer[0], buffer[1], buffer[2],
                buffer[3], buffer[4], buffer[5],
                mantissa, exponent, examples48[i].result,
                examples48[i].buffer[0], examples48[i].buffer[1],
                examples48[i].buffer[2], examples48[i].buffer[3],
                examples48[i].buffer[4], examples48[i].buffer[5]) ;
    }

    exit (0) ;

}

#endif
