/* $Id: ieee_util.c,v 1.6 2004/04/23 21:47:28 alex Exp $ */
/*******************************************************************************

File:

    ieee_util.c

    IEEE 754 Floating Point Utilities.


Author:    Alex Measday


Purpose:

    The IEEE utilities are used to convert floating-point numbers between
    the host CPU's native format and IEEE 754 floating-point format.

    The supported IEEE formats are single precision (32 bits), double
    precision (64 bits), and double-extended precision (80 bits).  In
    the following, big-endian bit representations, "S" is the sign bit,
    "E" is an exponent bit, and "M" is a mantissa bit.  Note that the
    most significant bit of the mantissa is implied in the single- and
    double-precision formats, but is represented explicitly in the
    double-extended-precision format.

        Single Precision (8-bit exponent with a bias of 127,
                          24-bit mantissa with the most significant
                          bit implied and 23 explicit bits)

            SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM

        Double Precision (11-bit exponent with a bias of 1023,
                          53-bit mantissa with the most significant
                          bit implied and 52 explicit bits)

            SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM
            MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

        Double-Extended Precision (15-bit exponent with a bias of 16383,
                                   64-bit mantissa with all bits explicit)

            SEEEEEEE EEEEEEEE MMMMMMMM MMMMMMMM
            MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM
            MMMMMMMM MMMMMMMM

    A number's actual exponent (power of 2) is equal to the represented
    exponent minus the bias.  Thus, single-precision exponents in the
    range 1..254 correspond to actual exponents of -126..+127.  Actual
    exponents for double-precision numbers fall in the range -1022..+1023;
    for double-extended-precision numbers, the range is -16382..16383.

    The value of a "normal" floating-point number in IEEE format equals:

        (-S) * (1.MMMM...) * (2 ** (exponent - bias))

    where:

        "S" is the sign bit (0 for positive numbers, 1, for negative numbers).

        "1.MMM..." is the mantissa.  The 1 to the left of the radix point is
        implied in the 32- and 64-bit IEEE representations; the "M"s to the
        right of the radix point are the binary digits explicitly represented
        in the format.  In the 80-bit IEEE representation, the 1 is explicitly
        represented (the most significant "M" in the storage layout shown above)
        and ".MMM..." are the remaining bits of the mantissa.

    The IEEE 754 standard uses the extreme high/low values of the exponent
    to represent certain special values:

        +0 (sign = 0, exponent = 0, mantissa = 0)
        -0 (sign = 1, exponent = 0, mantissa = 0)
        +infinity (sign = 0, exponent is all 1's, mantissa = 0)
        -infinity (sign = 1, exponent is all 1's, mantissa = 0)
        Subnormal (exponent = 0, mantissa is non-zero)
        SNaN (exponent is all 1's, mantissa is non-zero, MSb = 0)
        QNaN (exponent is all 1's, mantissa is non-zero, MSb = 1)

    For the signalling (SNaN) and quiet (QNan) not-a-numbers, "MSb" refers to
    to the most-significant explicit bit in the mantissa, not the implied bit.

    The handling of the different special values is described in the prologs
    of the conversion functions.


Acknowledgements:

    Reference material on IEEE floating-point representation is found
    at a number of web sites.  More interesting reading can be found
    at the web site of William Kahan, the "Old Man of Floating-Point":

        http://http.cs.berkeley.edu/~wkahan/

    The GNU libc info pages have useful information on floating-point
    numbers.  The ConvertFromIeeeExtended() et al conversion functions
    written by Malcolm Slaney and Ken Turkowski (found at various web
    sites) were used as references for the actual bit flipping.  (Their
    functions only work on 80-bit numbers.  My application had to deal
    with 32- and 64-bit numbers; hence my functions.)


Public Procedures:

    double2ieee() - converts a native double to IEEE 754 format.
    ieee2double() - converts IEEE 754 format to a native double.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <float.h>			/* Floating-point parameters. */
#include  <math.h>			/* Math library definitions. */
#ifndef HUGE_VAL
#    warning  ieee_util.c: No HUGE_VAL; using DBL_MAX.
#    define  HUGE_VAL  DBL_MAX
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <rpc/types.h>			/* RPC type definitions. */
#include  <rpc/xdr.h>			/* XDR type definitions. */
#include  "ieee_util.h"			/* IEEE 754 utilities. */

/*!*****************************************************************************

Procedure:

    double2ieee ()

    Convert a Native Double to an IEEE Floating-Point Number.


Purpose:

    The double2ieee() function converts a floating-point number in
    the host CPU's native floating-point format to IEEE format.


    Invocation:

        double2ieee (value, numBits, byteOrder, buffer) ;

    where

        <value>		- I
            is the floating-point number in the host CPU's native format.
        <numBits>	- I
            is the number of bits in the IEEE-format number; supported
            representations are 32-bit, 64-bit, and 80-bit numbers.
        <byteOrder>	- I
            specifies the ordering of the bytes in the IEEE-format number.
            A value of 0 or 1234 indicates big-endian storage; a value of
            1 or 4321 indicates little-endian storage.
        <buffer>	- O
            receives the IEEE-format number; the buffer should be at least
            as large as the specified number of bits in the converted number.

*******************************************************************************/


void  double2ieee (

#    if PROTOTYPES
        double  value,
        int  numBits,
        int  byteOrder,
        unsigned  char  *buffer)
#    else
        value, numBits, byteOrder, buffer)

        double  value ;
        int  numBits ;
        int  byteOrder ;
        unsigned  char  *buffer ;
#    endif

{    /* Local variables. */
    int  i, numBytes = numBits / 8 ;
    XDR  memoryStream ;



    if ((numBits != 32) && (numBits != 64) && (numBits != 80)) {
        SET_ERRNO (EINVAL) ;
        LGE "(double2ieee) Invalid number of bits: %d\n", numBits) ;
        return ;
    } else if (buffer == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(double2ieee) NULL buffer: ") ;
        return ;
    }

/*******************************************************************************
    Until I get around to using Ken Turkowski's and Malcolm Slaney's
    IEEE functions, I'll cheat by using XDR to do the conversion.
*******************************************************************************/

/* Create a memory-based XDR stream for the converted value. */

    xdrmem_create (&memoryStream, (char *) buffer, numBytes, XDR_ENCODE) ;

/* Encode the native value into XDR format (IEEE format, big-endian). */

    if (numBits == 32) {
        float  value32 = (float) value ;
        if (!xdr_float (&memoryStream, &value32)) {
            SET_ERRNO (EINVAL) ;
            LGE "(double2ieee) xdr_float() conversion error for %g: ", value) ;
            return ;
        }
    } else if (numBits == 64) {
        if (!xdr_double (&memoryStream, &value)) {
            SET_ERRNO (EINVAL) ;
            LGE "(double2ieee) xdr_double() conversion error for %g: ", value) ;
            return ;
        }
    } else {
        SET_ERRNO (EINVAL) ;
        LGE "(double2ieee) 80-bit conversion not implemented for %g: ", value) ;
        return ;
    }

/* The IEEE-encoded value is big-endian.  If the bytes need to be reversed,
   then do so. */

    switch (byteOrder) {
    case 1:				/* Little-endian? */
    case 4321:
        for (i = 0 ;  i < (numBytes/2) ;  i++) {
            unsigned  char  temp = buffer[i] ;
            buffer[i] = buffer[numBytes-1-i] ;
            buffer[numBytes-1-i] = temp ;
        }
        break ;
    case 0:				/* Big-endian? */
    case 1234:
    default:
        break ;
    }

    return ;

}

/*!*****************************************************************************

Procedure:

    ieee2double ()

    Convert an IEEE Floating-Point Number to a Native Double.


Purpose:

    The ieee2double() function converts a floating-point number in IEEE format
    to the host CPU's native floating-point format.

    Denormalized and zero values are converted to the appropriate native
    format.  The strtod(3) function is used to determine the host CPU
    representations of infinity and NaN.  If the strtod() calls succeed,
    +/- infinity is returned for signed infinity and NaN is returned for
    both SNaN and QNaN.  (The C99 standard includes macros for INFINITY
    and NAN, but some of our compilers don't support the latest standard.)


    Invocation:

        value = ieee2double (numBits, byteOrder, buffer) ;

    where

        <numBits>	- I
            is the number of bits in the IEEE-format number; supported
            representations are 32-bit, 64-bit, and 80-bit numbers.
        <byteOrder>	- I
            specifies the ordering of the bytes in the IEEE-format number.
            A value of 0 or 1234 indicates big-endian storage; a value of
            1 or 4321 indicates little-endian storage.
        <buffer>	- I
            is a buffer containing the IEEE-format number.
        <value>		- O
            returns the floating-point number in the host CPU's native format.

*******************************************************************************/


double  ieee2double (

#    if PROTOTYPES
        int  numBits,
        int  byteOrder,
        unsigned  char  *buffer)
#    else
        numBits, byteOrder, buffer)

        int  numBits ;
        int  byteOrder ;
        unsigned  char  *buffer ;
#    endif

{    /* Local variables. */
    bool  isNegative, zeroMan ;
    char  *s ;
    unsigned  char  swapped[16] ;
    double  value ;
    int  bias, exponent, firstBit, i, numBytes = numBits / 8 ;
    unsigned  long  mantissa[2] ;

    static  bool  checkNative = true ;
    static  double  nativeInfinity, nativeNaN ;




    if ((numBits != 32) && (numBits != 64) && (numBits != 80)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ieee2double) Invalid number of bits: %d\n", numBits) ;
        return (0.0) ;
    } else if (buffer == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ieee2double) NULL buffer: ") ;
        return (0.0) ;
    }

/* Check for native representations of infinity and Nan. */

    if (checkNative) {
        checkNative = false ;
        nativeInfinity = strtod ("INF", &s) ;
        if (*s != '\0')  nativeInfinity = HUGE_VAL ;
        nativeNaN = strtod ("NAN", &s) ;
        if (*s != '\0')  nativeNaN = HUGE_VAL ;
    }

/* If the bytes in the IEEE floating-point number are not in big-endian order,
   then put the bytes into big-endian order. */

    switch (byteOrder) {
    case 1:				/* Little-endian? */
    case 4321:
        for (i = 0 ;  i < numBytes ;  i++)
            swapped[i] = buffer[numBytes-1-i] ;
        buffer = swapped ;
        break ;
    case 0:				/* Big-endian? */
    case 1234:
    default:
        break ;
    }

/*******************************************************************************
    Extract the sign, exponent, and mantissa from the IEEE number.  Note that
    the mantissa is stored in a 2-element array, the first element containing
    the 32 most significant bits (including the implied bit, if any) of the
    mantissa and the second element containing the 32 least significant bits.
    The mantissa is left-shifted so that bit 31 of the first element is the
    most significiant bit of the mantissa; bits beyond the precision of the
    mantissa are set to 0.
*******************************************************************************/

    isNegative = ((buffer[0] & 0x80) != 0) ;

    switch (numBits) {
    case 32:
        bias = 127 ;
        exponent = (((unsigned long) buffer[0] & 0x7F) << 1) |
                   (((unsigned long) buffer[1] & 0x80) >> 7) ;
        mantissa[0] =					/* Implied bit. */
            0x80000000 |				/* 23 explicit bits. */
            (((unsigned long) buffer[1] & 0x7F) << 24) |
            ((unsigned long) buffer[2] << 16) |
            ((unsigned long) buffer[3] << 8) ;
        mantissa[1] = 0 ;
        zeroMan = ((mantissa[0] == 0x80000000UL) && (mantissa[1] == 0)) ;
        break ;
    case 64:
        bias = 1023 ;
        exponent = (((unsigned long) buffer[0] & 0x7F) << 4) |
                   (((unsigned long) buffer[1] & 0xF0) >> 4) ;
        mantissa[0] =					/* Implied bit. */
            0x80000000UL |				/* 31 explicit bits. */
            (((unsigned long) buffer[1] & 0x0F) << 27) |
            ((unsigned long) buffer[2] << 19) |
            ((unsigned long) buffer[3] << 11) |
            ((unsigned long) buffer[4] << 3) |
            (((unsigned long) buffer[5] & 0xE0) >> 5) ;
        mantissa[1] =					/* 21 explicit bits. */
            (((unsigned long) buffer[5] & 0x1F) << 27) |
            ((unsigned long) buffer[6] << 19) |
            ((unsigned long) buffer[7] << 11) ;
        zeroMan = ((mantissa[0] == 0x80000000UL) && (mantissa[1] == 0)) ;
        break ;
    case 80:
    default:
        bias = 16383 ;
        exponent = (((unsigned long) buffer[0] & 0x7F) << 8) | buffer[1] ;
        mantissa[0] =
            ((unsigned long) buffer[2] << 24) |		/* 32 explicit bits. */
            ((unsigned long) buffer[3] << 16) |
            ((unsigned long) buffer[4] << 8) |
            (unsigned long) buffer[5] ;
        mantissa[1] =
            ((unsigned long) buffer[6] << 24) |		/* 32 explicit bits. */
            ((unsigned long) buffer[7] << 16) |
            ((unsigned long) buffer[8] << 8) |
            (unsigned long) buffer[9] ;
        zeroMan = ((mantissa[0] == 0) && (mantissa[1] == 0)) ;
        break ;
    }

/*******************************************************************************
    Check for the special values.
*******************************************************************************/

/* A zero exponent indicates signed zero or a subnormal number.  The most
   significant bit of the mantissa, whether implicit or explicit (in 80-bit
   numbers), is always zero.  The value is then

        (-S) * (0.MMM...) * (2 ** (1-bias))

   In fact, zero is just a subnormal number with a mantissa of 0. */

    if (exponent == 0) {
        mantissa[0] &= 0x7FFFFFFFUL ;			/* Strip implied bit. */
        if (zeroMan) {
            return (isNegative ? -0.0 : 0.0) ;
        } else {
            exponent = 1 - bias ;
            value = ldexp ((double) mantissa[0], exponent -= 31) ;
            value += ldexp ((double) mantissa[1], exponent -= 32) ;
            return (isNegative ? -value : value) ;
        }
    }

/* A maximum exponent indicates signed infinity or not-a-number. */

    else if (exponent == ((bias * 2) + 1)) {
        firstBit = (numBits == 80) ? (mantissa[0] & 0x80000000UL)
                                   : (mantissa[0] & 0x40000000UL) ;
        if (zeroMan) {
            return (isNegative ? -nativeInfinity : nativeInfinity) ;
        } else if (firstBit) {
            return (nativeNaN) ;		/* QNaN */
        } else {
            return (nativeNaN) ;		/* SNaN */
        }
    }


/*******************************************************************************
    A normal number!  Convert it to the host CPU's floating-point format.
*******************************************************************************/

    exponent -= bias ;

/* Check for floating-point overflow.  If the number's magnitude (judging
   by its exponent) is greater than the maximum representable host value
   and less than infinity, then substitute the host's maximum value. */

    if (exponent >= DBL_MAX_EXP) {		/* Floating-point overflow? */
        value = DBL_MAX ;			/* Use largest number. */
    }

/* Check for floating-point underflow.  If the number's magnitude (judging
   by its exponent) is greater than zero and less than the host's smallest
   non-zero value, then substitute the host's smallest non-zero value. */

    else if (exponent <= DBL_MIN_EXP) {		/* Floating-point underflow? */
        value = DBL_MIN ;
    }

/* Truly a normal number! */

    else {
        value = ldexp ((double) mantissa[0], exponent -= 31) ;
        value += ldexp ((double) mantissa[1], exponent -= 32) ;
    }

    return (isNegative ? -value : value) ;

}

#ifdef TEST
/*******************************************************************************
    Test Program.
*******************************************************************************/


void  dumpDouble (double value) {
    unsigned  char  *s ;
    int  i ;
#ifdef X86
    unsigned  char  buffer[8] ;
    s = (unsigned char *) &value + sizeof (double) - 1 ;
    for (i = 0 ;  i < sizeof (double) ;  i++)
        buffer[i] = *s-- ;
    s = buffer ;
#else
    s = (unsigned char *) &value ;
#endif
    for (i = 0 ;  i < sizeof (double) ;  i++) {
        if ((i > 0) && ((i % 4) == 0))  printf (" ") ;
        printf ("%02X", *s++) ;
    }
    printf ("\n") ;
    return ;
}


int  main (

    int argc,
    char *argv[])

{    /* Local variables. */
    char  *s ;
    double  result, value ;



    if ((argc < 2) || ((value = strtod (argv[1], &s)), (*s != '\0'))) {
        fprintf (stderr, "missing or invalid argument\n") ;
        exit (0) ;
    }

    printf ("Value  = %21.15g\n", value) ;

    dumpDouble (value) ;

    result = ieee2double (sizeof (double) * 8,
#ifdef X86
                          4321,
#else
                          1234,
#endif
                          (unsigned char *) &value) ;

    printf ("Result = %21.15g\n", result) ;

    dumpDouble (result) ;

    exit (0) ;

}

#endif
