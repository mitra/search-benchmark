/* $Id: utf_util.c,v 1.1 2012/08/02 10:11:29 alex Exp alex $ */
/*******************************************************************************

File:

    utf_util.c

    Unicode Transformation Format (UTF) Utilities.


Author:    Alex Measday


Purpose:

    The UTF utilities are used to convert between 8-, 16-, and 32-bit Unicode
    Transformation Format strings.  The Unicode character space is 21 bits
    wide.  Consequently, the 8-bit encoding (UTF-8) requires from 1 to 4
    bytes to represent a Unicode character.  The 16-bit encoding (UTF-16)
    requires one or two 16-bit code units.  32-bit encoding (UTF-32), of
    course, can represent any Unicode character as is.

    UTF-16 and UTF-32 encodings arrange bytes in most significant to least
    significant (big-endian) order or least significant to most significant
    (little-endian) order.  A byte-order marker (BOM) may be inserted at the
    beginning of an encoded string to indicate the byte order; if there is no
    BOM, big-endian order is assumed.

    UTF-8 strings need no BOM, but some applications look for a BOM at the
    beginning of a file so as to detect that the file is UTF-8 as opposed
    to straight ASCII/Latin.

    The conversion functions automatically append a NUL-/null-terminator to
    converted strings: 0x00 for UTF-8, 0x0000 for UTF-16, and 0x00000000 for
    UTF-32 strings.  (Assuming enough room in the destination buffer.)

    The UTF_UTIL package was written as a small, self-contained set of
    straightforward functions intended for converting Unicode strings
    in MP3 ID3v2 tags.  As such, portability was of higher concern than
    efficiency.  If you're doing a lot of Unicode string handling, you
    might wish to research some of the full-featured Unicode libraries.


Public Procedures:

    utf16bom() - returns a UTF-16 byte-order marker indication.
    utf32bom() - returns a UTF-32 byte-order marker indication.
    utf16len() - returns the number of code units in a UTF-16 string.
    utf32len() - returns the number of code units in a UTF-32 string.
    utf8get() - decodes a code point from a UTF-8 string.
    utf8put() - encodes a code point to a UTF-8 string.
    utf16get() - decodes a code point from a UTF-16 string.
    utf16put() - encodes a code point to a UTF-16 string.
    utf32get() - decodes a code point from a UTF-32 string.
    utf32put() - encodes a code point to a UTF-32 string.
    utf8utf16() - converts a UTF-8 string to a UTF-16 string.
    utf8utf32() - converts a UTF-8 string to a UTF-32 string.
    utf16utf8() - converts a UTF-16 string to a UTF-8 string.
    utf32utf8() - converts a UTF-32 string to a UTF-8 string.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "utf_util.h"			/* Unicode UTF utilities. */
#ifdef TEST
#    include  "meo_util.h"		/* Memory operations. */
#endif

/*!*****************************************************************************

Procedure:

    utf16bom ()

    Check for a Byte-Order Marker in a UTF-16 String.


Purpose:

    Function utf16bom() checks for a byte-order marker (0xFEFF) at the
    beginning of a UTF-16 string and returns an indication of the string's
    byte order.  It is assumed that the deprecated ZWNBSP character (U+FEFF)
    is not used at the beginning of the string.


    Invocation:

        endian = utf16bom (src) ;

    where

        <src>		- I
            is a UTF-16 string.
        <endian>	- O
            returns (i) -1 if the BOM is present and the string is
            little-endian, (ii) +1 if the BOM is present and the string is
            big-endian, and (iii) 0 if there is no BOM.  In the latter case,
            the caller should assume the string is big-endian per the Unicode
            specification.

*******************************************************************************/


int  utf16bom (

#    if PROTOTYPES
        const  char  *src)
#    else
        src)

        const  char  *src ;
#    endif

{

    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16bom) NULL string: ") ;
        return (0) ;
    }

    if ((unsigned char) src[0] == 0xFE) {
        if ((unsigned char) src[1] == 0xFF)
            return (1) ;			/* BOM, big-endian? */
    } else if ((unsigned char) src[0] == 0xFF) {
        if ((unsigned char) src[1] == 0xFE)
            return (-1) ;			/* BOM, little-endian? */
    }

    return (0) ;				/* No BOM */

}

/*!*****************************************************************************

Procedure:

    utf32bom ()

    Check for a Byte-Order Marker in a UTF-32 String.


Purpose:

    Function utf32bom() checks for a byte-order marker (0x0000FEFF) at the
    beginning of a UTF-32 string and returns an indication of the string's
    byte order.  It is assumed that the deprecated ZWNBSP character (U+FEFF)
    is not used at the beginning of the string.


    Invocation:

        endian = utf32bom (src) ;

    where

        <src>		- I
            is a UTF-32 string.
        <endian>	- O
            returns (i) -1 if the BOM is present and the string is
            little-endian, (ii) +1 if the BOM is present and the string is
            big-endian, and (iii) 0 if there is no BOM.  In the latter case,
            the caller should assume the string is big-endian per the Unicode
            specification.

*******************************************************************************/


int  utf32bom (

#    if PROTOTYPES
        const  char  *src)
#    else
        src)

        const  char  *src ;
#    endif

{

    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16bom) NULL string: ") ;
        return (0) ;
    }

    if (((unsigned char) src[0] == 0) &&
        ((unsigned char) src[1] == 0) &&
        ((unsigned char) src[2] == 0xFE) &&
        ((unsigned char) src[3] == 0xFF)) {
        return (1) ;				/* BOM, big-endian */
    } else if (((unsigned char) src[0] == 0xFF) &&
               ((unsigned char) src[1] == 0xFE) &&
               ((unsigned char) src[2] == 0) &&
               ((unsigned char) src[3] == 0)) {
        return (-1) ;				/* BOM, little-endian */
    }

    return (0) ;				/* No BOM */

}

/*!*****************************************************************************

Procedure:

    utf16len ()

    Count the Number of Code Units in a UTF-16 String.


Purpose:

    Function utf16len() returns the number of 16-bit code units in a
    null-terminated (0x0000) UTF-16 string.


    Invocation:

        number = utf16len (src) ;

    where

        <src>		- I
            is a null-terminated UTF-16 string.
        <number>	- O
            returns the number of 16-bit code units in the UTF-16 string,
            not including the BOM if present.

*******************************************************************************/


size_t  utf16len (

#    if PROTOTYPES
        const  char  *src)
#    else
        src)

        const  char  *src ;
#    endif

{    /* Local variables. */
    size_t  i ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16len) NULL string: ") ;
        return (0) ;
    }

					/* Skip byte-order marker? */
    if (utf16bom (src))  src += UTF_16_UNIT_BYTES ;

					/* Scan for 2 consecutive zero bytes. */
    for (i = 0 ;  ;  i += UTF_16_UNIT_BYTES) {
        if ((src[i] == 0) && (src[i+1] == 0))  break ;
    }

    return (i / UTF_16_UNIT_BYTES) ;

}

/*!*****************************************************************************

Procedure:

    utf32len ()

    Count the Number of Code Units in a UTF-32 String.


Purpose:

    Function utf32len() returns the number of 32-bit code units in a
    null-terminated (0x00000000) UTF-32 string.


    Invocation:

        number = utf32len (src) ;

    where

        <src>		- I
            is a UTF-32 string.
        <number>	- O
            returns the number of 32-bit code units in the UTF-32 string,
            not including the BOM if present.

*******************************************************************************/


size_t  utf32len (

#    if PROTOTYPES
        const  char  *src)
#    else
        src)

        const  char  *src ;
#    endif

{    /* Local variables. */
    size_t  i ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf32len) NULL string: ") ;
        return (0) ;
    }

					/* Skip byte-order marker? */
    if (utf32bom (src))  src += UTF_32_UNIT_BYTES ;

					/* Scan for 4 consecutive zero bytes. */
    for (i = 0 ;  ;  i += UTF_32_UNIT_BYTES) {
        if ((src[i] == 0) && (src[i+1] == 0) &&
            (src[i+2] == 0) && (src[i+3] == 0))  break ;
    }

    return (i / UTF_32_UNIT_BYTES) ;

}

/*!*****************************************************************************

Procedure:

    utf8get ()

    Get the Next Code Point from a UTF-8 String.


Purpose:

    Function utf8get() decodes and returns the next code point from a UTF-8
    string.  The length of the octet sequence consumed in the process is
    returned, allowing the caller to easily step through an entire UTF-8 string.


    Invocation:

        codePoint = utf8get (src, &numUnits) ;

    where

        <src>		- I
            is an octet sequence in a UTF-8 string.
        <numUnits>	- O
            returns the number (1-4) of UTF-8 code units consumed.
        <codePoint>	- O
            returns the Unicode code point constructed from the UTF-8 octet
            sequence.  -1 is returned in the event of an error.

*******************************************************************************/


int32_t  utf8get (

#    if PROTOTYPES
        const  char  *src,
        size_t  *numUnits)
#    else
        src, numUnits)

        const  char  *src ;
        size_t  *numUnits ;
#    endif

{    /* Local variables. */
    int32_t  codePoint ;



    if (numUnits != NULL)  *numUnits = 0 ;

    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8get) NULL string: ") ;
        return (-1) ;
    }

/* One-octet code point (Unicode range U+000000 to U+00007F)? */

    if ((src[0] & 0x80) == 0x00) {

        codePoint = src[0] ;
        if (numUnits != NULL)  *numUnits = 1 ;

    }

/* Two-octet code point (Unicode range U+000080 to U+0007FF)? */

    else if ((src[0] & 0xE0) == 0xC0) {

        codePoint = src[0] & 0x1F ;
        codePoint <<= 6 ;
        if ((src[1] & 0xC0) != 0x80) {
            SET_ERRNO (EINVAL) ;
            LGE "(utf8get) Invalid code unit: 0x%02X\n", src[1]) ;
        }
        codePoint |= src[1] & 0x3F ;
        if (numUnits != NULL)  *numUnits = 2 ;

    }

/* Three-octet code point (Unicode range U+000800 to U+00FFFF)? */

    else if ((src[0] & 0xF0) == 0xE0) {

        codePoint = src[0] & 0x0F ;
        codePoint <<= 6 ;
        if ((src[1] & 0xC0) != 0x80) {
            SET_ERRNO (EINVAL) ;
            LGE "(utf8get) Invalid code unit: 0x%02X\n", src[1]) ;
        }
        codePoint |= src[1] & 0x3F ;
        codePoint <<= 6 ;
        if ((src[2] & 0xC0) != 0x80) {
            SET_ERRNO (EINVAL) ;
            LGE "(utf8get) Invalid code unit: 0x%02X\n", src[2]) ;
        }
        codePoint |= src[2] & 0x3F ;
        if (numUnits != NULL)  *numUnits = 3 ;

    }

/* Four-octet code point (Unicode range U+010000 to U+10FFFF)? */

    else if ((src[0] & 0xF8) == 0xF0) {

        codePoint = src[0] & 0x07 ;
        codePoint <<= 6 ;
        if ((src[1] & 0xC0) != 0x80) {
            SET_ERRNO (EINVAL) ;
            LGE "(utf8get) Invalid code unit: 0x%02X\n", src[1]) ;
        }
        codePoint |= src[1] & 0x3F ;
        codePoint <<= 6 ;
        if ((src[2] & 0xC0) != 0x80) {
            SET_ERRNO (EINVAL) ;
            LGE "(utf8get) Invalid code unit: 0x%02X\n", src[2]) ;
        }
        codePoint |= src[2] & 0x3F ;
        codePoint <<= 6 ;
        if ((src[3] & 0xC0) != 0x80) {
            SET_ERRNO (EINVAL) ;
            LGE "(utf8get) Invalid code unit: 0x%02X\n", src[3]) ;
        }
        codePoint |= src[3] & 0x3F ;
        if (numUnits != NULL)  *numUnits = 4 ;

    }

    else {

        SET_ERRNO (EINVAL) ;
        LGE "(utf8get) Invalid initial code unit: 0x%02X\n", src[0]) ;
        return (-1) ;

    }

    return (codePoint) ;

}

/*!*****************************************************************************

Procedure:

    utf8put ()

    Put a Code Point in a UTF-8 String.


Purpose:

    Function utf8put() encodes and adds a Unicode code point to a UTF-8 string.
    The length of the octet sequence generated in the process is returned,
    allowing the caller to easily build an entire UTF-8 string.


    Invocation:

        status = utf8put (codePoint, dst, &numUnits) ;

    where

        <codePoint>	- I
            is the Unicode code point to be encoded.
        <dst>		- O
            is the UTF-8 string to which the code point is to be added.
        <numUnits>	- O
            returns the number (1-4) of UTF-8 code units generate.
        <status>	- O
            returns the status of encoding the code point, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  utf8put (

#    if PROTOTYPES
        int32_t  codePoint,
        char  *dst,
        size_t  *numUnits)
#    else
        codePoint, dst, numUnits)

        int32_t  codePoint ;
        char  *dst ;
        size_t  *numUnits ;
#    endif

{

    if (numUnits != NULL)  *numUnits = 0 ;

    if ((codePoint < 0) || (0x10FFFF < codePoint)) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8put) Invalid code point: U+%06X\n", codePoint) ;
        return (errno) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8put) NULL destination string: ") ;
        return (errno) ;
    }

/* One-octet code point (Unicode range U+000000 to U+00007F)? */

    if (codePoint < 0x80) {

        dst[0] = codePoint & 0x7F ;
        if (numUnits != NULL)  *numUnits = 1 ;

    }

/* Two-octet code point (Unicode range U+000080 to U+0007FF)? */

    else if (codePoint < 0x800) {

        dst[0] = 0xC0 | ((codePoint & 0x07C0) >> 6) ;
        dst[1] = 0x80 |  (codePoint & 0x003F) ;
        if (numUnits != NULL)  *numUnits = 2 ;

    }

/* Three-octet code point (Unicode range U+000800 to U+00FFFF)? */

    else if (codePoint < 0x10000) {

        dst[0] = 0xE0 | ((codePoint & 0x0F000) >> 12) ;
        dst[1] = 0x80 | ((codePoint & 0x00FC0) >> 6) ;
        dst[2] = 0x80 |  (codePoint & 0x0003F) ;
        if (numUnits != NULL)  *numUnits = 3 ;

    }

/* Four-octet code point (Unicode range U+010000 to U+10FFFF)? */

    else {

        dst[0] = 0xF0 | ((codePoint & 0x1C0000) >> 18) ;
        dst[1] = 0x80 | ((codePoint & 0x03F000) >> 12) ;
        dst[2] = 0x80 | ((codePoint & 0x000FC0) >> 6) ;
        dst[3] = 0x80 |  (codePoint & 0x00003F) ;
        if (numUnits != NULL)  *numUnits = 4 ;

    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    utf16get ()

    Get the Next Code Point from a UTF-16 String.


Purpose:

    Function utf16get() decodes and returns the next code point from a UTF-16
    string.  The number of 16-bit code units or bytes consumed is easily
    determined from the value of the code point:

        numUnits = (codePoint < 0x10000) ? 1 : 2 ;
        numBytes = (codePoint < 0x10000) ? UTF_16_UNIT_BYTES : UTF_16_PAIR_BYTES ;


    Invocation:

        codePoint = utf16get (bigEndian, src) ;

    where

        <bigEndian>	- I
            specifies the byte order of the UTF-16 string, true for big-endian
            and false for little-endian.
        <src>		- I
            is a code unit or pair in a UTF-16 string.
        <codePoint>	- O
            returns the Unicode code point constructed from the UTF-16 code
            unit(s).  -1 is returned in the event of an error.

*******************************************************************************/


int32_t  utf16get (

#    if PROTOTYPES
        bool  bigEndian,
        const  char  *src)
#    else
        bigEndian, src)

        bool  bigEndian ;
        const  char  *src ;
#    endif

{    /* Local variables. */
    int32_t  codePoint, surrogate ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16get) NULL string: ") ;
        return (-1) ;
    }

/* Single-unit code point (Unicode range U+000000 to U+00FFFF)? */

    if (bigEndian)
        codePoint = ((unsigned char) src[0] << 8) | (unsigned char) src[1] ;
    else
        codePoint = ((unsigned char) src[1] << 8) | (unsigned char) src[0] ;

    if ((codePoint < 0xD800) || (0xDFFF < codePoint))  return (codePoint) ;

/* Two-unit code point (Unicode range U+010000 to U+10FFFF)? */

    if (bigEndian)
        surrogate = ((unsigned char) src[2] << 8) | (unsigned char) src[3] ;
    else
        surrogate = ((unsigned char) src[3] << 8) | (unsigned char) src[2] ;

    if ((surrogate & 0xDC00) != 0xDC00) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16get) Invalid low surrogate: U+%04X\n", surrogate) ;
        return (-1) ;
    }

    codePoint = ((codePoint & 0x03FF) << 10) | (surrogate & 0x03FF) ;

    codePoint += 0x010000 ;	/* Extend from 20 to 21 bits. */

    return (codePoint) ;

}

/*!*****************************************************************************

Procedure:

    utf16put ()

    Put a Code Point in a UTF-16 String.


Purpose:

    Function utf16put() encodes and adds a Unicode code point to a UTF-16
    string.  The number of 16-bit code units or bytes generated is easily
    determined from the value of the code point:

        numUnits = (codePoint < 0x10000) ? 1 : 2 ;
        numBytes = (codePoint < 0x10000) ? UTF_16_UNIT_BYTES : UTF_16_PAIR_BYTES ;


    Invocation:

        status = utf16put (codePoint, bigEndian, dst) ;

    where

        <codePoint>	- I
            is the Unicode code point to be encoded.
        <bigEndian>	- I
            specifies the byte order of the UTF-16 string, true for big-endian
            and false for little-endian.
        <dst>		- O
            is the UTF-16 string to which the code point is to be added.
        <status>	- O
            returns the status of encoding the code point, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  utf16put (

#    if PROTOTYPES
        int32_t  codePoint,
        bool  bigEndian,
        char  *dst)
#    else
        codePoint, bigEndian, dst)

        int32_t  codePoint ;
        bool  bigEndian,
        char  *dst ;
#    endif

{    /* Local variables. */
    int32_t  surrogate ;



    if ((codePoint < 0) || (0x10FFFF < codePoint)) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16put) Invalid code point: U+%06X\n", codePoint) ;
        return (errno) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16put) NULL destination string: ") ;
        return (errno) ;
    }

/* Single-unit code point (Unicode range U+000000 to U+00FFFF)? */

    if (codePoint < 0x10000) {

        if (bigEndian) {
            dst[0] = codePoint >> 8 ;
            dst[1] = codePoint & 0xFF ;
        } else {
            dst[1] = codePoint >> 8 ;
            dst[0] = codePoint & 0xFF ;
        }

    }

/* Two-unit code point (Unicode range U+010000 to U+10FFFF)? */

    else {

        codePoint -= 0x010000 ;
        surrogate = (codePoint & 0x03FF) | 0xDC00 ;
        codePoint = (codePoint >> 10) | 0xD800 ;

        if (bigEndian) {
            dst[0] = codePoint >> 8 ;
            dst[1] = codePoint & 0xFF ;
            dst[2] = surrogate >> 8 ;
            dst[3] = surrogate & 0xFF ;
        } else {
            dst[1] = codePoint >> 8 ;
            dst[0] = codePoint & 0xFF ;
            dst[3] = surrogate >> 8 ;
            dst[2] = surrogate & 0xFF ;
        }

    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    utf32get ()

    Get the Next Code Point from a UTF-32 String.


Purpose:

    Function utf32get() decodes and returns the next code point from a UTF-32
    string.  The number of 32-bit code units consumed is always 1.


    Invocation:

        codePoint = utf32get (bigEndian, src) ;

    where

        <bigEndian>	- I
            specifies the byte order of the UTF-32 string, true for big-endian
            and false for little-endian.
        <src>		- I
            is a code unit in a UTF-32 string.
        <codePoint>	- O
            returns the Unicode code point constructed from the UTF-32 code
            unit.  -1 is returned in the event of an error.

*******************************************************************************/


int32_t  utf32get (

#    if PROTOTYPES
        bool  bigEndian,
        const  char  *src)
#    else
        bigEndian, src)

        bool  bigEndian ;
        const  char  *src ;
#    endif

{    /* Local variables. */
    int32_t  codePoint ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf32get) NULL string: ") ;
        return (-1) ;
    }

    if (bigEndian) {
        codePoint = ((unsigned char) src[0] << 24) |
                    ((unsigned char) src[1] << 16) |
                    ((unsigned char) src[2] << 8) |
                    (unsigned char) src[3] ;
    } else {
        codePoint = ((unsigned char) src[3] << 24) |
                    ((unsigned char) src[2] << 16) |
                    ((unsigned char) src[1] << 8) |
                    (unsigned char) src[0] ;
    }

    return (codePoint) ;

}

/*!*****************************************************************************

Procedure:

    utf32put ()

    Put a Code Point in a UTF-32 String.


Purpose:

    Function utf32put() encodes and adds a Unicode code point to a UTF-32
    string.  The number of 32-bit code units generated is always 1.


    Invocation:

        status = utf32put (codePoint, bigEndian, dst) ;

    where

        <codePoint>	- I
            is the Unicode code point to be encoded.
        <bigEndian>	- I
            specifies the byte order of the UTF-32 string, true for big-endian
            and false for little-endian.
        <dst>		- O
            is the UTF-32 string to which the code point is to be added.
        <status>	- O
            returns the status of encoding the code point, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  utf32put (

#    if PROTOTYPES
        int32_t  codePoint,
        bool  bigEndian,
        char  *dst)
#    else
        codePoint, bigEndian, dst)

        int32_t  codePoint ;
        bool  bigEndian,
        char  *dst ;
#    endif

{

    if ((codePoint < 0) || (0x10FFFF < codePoint)) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf32put) Invalid code point: U+%06X\n", codePoint) ;
        return (errno) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf32put) NULL destination string: ") ;
        return (errno) ;
    }

    if (bigEndian) {
        dst[0] = (codePoint >> 24) & 0xFF ;
        dst[1] = (codePoint >> 16) & 0xFF ;
        dst[2] = (codePoint >> 8) & 0xFF ;
        dst[3] = codePoint & 0xFF ;
    } else {
        dst[3] = (codePoint >> 24) & 0xFF ;
        dst[2] = (codePoint >> 16) & 0xFF ;
        dst[1] = (codePoint >> 8) & 0xFF ;
        dst[0] = codePoint & 0xFF ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    utf8utf16 ()

    Convert a UTF-8 String to a UTF-16 String.


Purpose:

    Function utf8utf16() converts a UTF-8 encoding of a Unicode string to
    the UTF-16 representation of the string.  A null terminator (0x0000)
    is appended to the converted string if enough space remains in the
    destination buffer.



    Invocation:

        length = utf8utf16 (src, srclen, bom, dstlen, dst) ;

    where

        <src>		- I
            is the UTF-8 string to be converted.
        <srclen>	- I
            is the length in bytes of the UTF-8 string.  If this argument is
            less than zero, the string is assumed to be NUL-terminated.
        <bom>		- I
            specifies the byte order of the converted string and whether or
            not a byte-order marker (BOM) is to be inserted at the beginning
            of the string: -1 indicates little-endian with a BOM, 0 indicates
            big-endian without a BOM, and 1 indicates big-endian with a BOM.
        <dstlen>	- I
            is the maximum length in bytes of the UTF-16 string buffer.
        <dst>		- O
            is a buffer into which the UTF-16 string is written.
        <length>	- O
            returns the number of code units written to the UTF-16 buffer;
            -1 is returned in the event of an error.

*******************************************************************************/


ssize_t  utf8utf16 (

#    if PROTOTYPES
        const  char  *src,
        ssize_t  srclen,
        int  bom,
        size_t  dstlen,
        char  *dst)
#    else
        src, srclen, bom, dstlen, dst)

        const  char  *src ;
        ssize_t  srclen ;
        int  bom ;
        size_t  dstlen ;
        char  *dst ;
#    endif

{    /* Local variables. */
    int32_t  codePoint ;
    size_t  length, numUnits ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8utf16) NULL source string: ") ;
        return (-1) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8utf16) NULL destination string: ") ;
        return (-1) ;
    }

    if (srclen < 0)  srclen = strlen (src) ;

    length = 0 ;

/* If the caller specified that a byte-order marker appear at the beginning
   of the string, then insert it. */

    if (bom != 0) {
        if (dstlen < UTF_16_UNIT_BYTES) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf8utf16) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf16put (0x0000FEFF, bom > 0, dst)) {
            LGE "(utf8utf16) Error writing BOM to destination string (%p): ",
                dst) ;
            return (-1) ;
        }
        dst += 2 ;  dstlen -= 2 ;  length++ ;
    }

/* Encode the UTF-8 code points as UTF-16. */

    while (srclen > 0) {
        codePoint = utf8get (src, &numUnits) ;
        if (codePoint < 0) {
            LGE "(utf8utf16) Error in source string (%p): ", src) ;
            return (-1) ;
        }
        src += numUnits ;  srclen -= numUnits ;
        if ((dstlen < UTF_16_PAIR_BYTES) &&
            ((codePoint > 0x0FFFF) || (dstlen < UTF_16_UNIT_BYTES))) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf8utf16) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf16put (codePoint, bom >= 0, dst)) {
            LGE "(utf8utf16) Error writing to destination string (%p): ", dst) ;
            return (-1) ;
        }
        numUnits = (codePoint < 0x10000) ? 1 : 2 ;
        dst += numUnits * UTF_16_UNIT_BYTES ;
        dstlen -= numUnits * UTF_16_UNIT_BYTES ;
        length += numUnits ;
    }

/* Add a null terminator (0x0000). */

    if (dstlen >= UTF_16_UNIT_BYTES) {
        *dst++ = 0 ;  *dst++ = 0 ;
    }

    return (length) ;

}

/*!*****************************************************************************

Procedure:

    utf8utf32 ()

    Convert a UTF-8 String to a UTF-32 String.


Purpose:

    Function utf8utf32() converts a UTF-8 encoding of a Unicode string to
    the UTF-32 representation of the string.  A null terminator (0x00000000)
    is appended to the converted string if enough space remains in the
    destination buffer.


    Invocation:

        length = utf8utf32 (src, srclen, bom, dstlen, dst) ;

    where

        <src>		- I
            is the UTF-8 string to be converted.
        <srclen>	- I
            is the length in bytes of the UTF-8 string.  If this argument is
            less than zero, the string is assumed to be NUL-terminated.
        <bom>		- I
            specifies the byte order of the converted string and whether or
            not a byte-order marker (BOM) is to be inserted at the beginning
            of the string: -1 indicates little-endian with a BOM, 0 indicates
            big-endian without a BOM, and 1 indicates big-endian with a BOM.
        <dstlen>	- I
            is the maximum length in bytes of the UTF-32 string buffer.
        <dst>		- O
            is a buffer into which the UTF-32 string is written.
        <length>	- O
            returns the number of code units written to the UTF-32 buffer;
            -1 is returned in the event of an error.

*******************************************************************************/


ssize_t  utf8utf32 (

#    if PROTOTYPES
        const  char  *src,
        ssize_t  srclen,
        int  bom,
        size_t  dstlen,
        char  *dst)
#    else
        src, srclen, bom, dstlen, dst)

        const  char  *src ;
        ssize_t  srclen ;
        int  bom ;
        size_t  dstlen ;
        char  *dst ;
#    endif

{    /* Local variables. */
    int32_t  codePoint ;
    size_t  length, numUnits ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8utf32) NULL source string: ") ;
        return (-1) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf8utf32) NULL destination string: ") ;
        return (-1) ;
    }

    if (srclen < 0)  srclen = strlen (src) ;

    length = 0 ;

/* If the caller specified that a byte-order marker appear at the beginning
   of the string, then insert it. */

    if (bom != 0) {
        if (dstlen < UTF_32_UNIT_BYTES) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf8utf32) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf32put (0x0000FEFF, bom > 0, dst)) {
            LGE "(utf8utf32) Error writing BOM to destination string (%p): ",
                dst) ;
            return (-1) ;
        }
        dst += UTF_32_UNIT_BYTES ;  dstlen -= UTF_32_UNIT_BYTES ;  length++ ;
    }

/* Encode the UTF-8 code points as UTF-32. */

    while (srclen > 0) {
        codePoint = utf8get (src, &numUnits) ;
        if (codePoint < 0) {
            LGE "(utf8utf32) Error in source string (%p): ", src) ;
            return (-1) ;
        }
        src += numUnits ;  srclen -= numUnits ;
        if (dstlen < UTF_32_UNIT_BYTES) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf8utf32) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf32put (codePoint, bom >= 0, dst)) {
            LGE "(utf8utf32) Error writing to destination string (%p): ", dst) ;
            return (-1) ;
        }
        numUnits = 1 ;
        dst += numUnits * UTF_32_UNIT_BYTES ;
        dstlen -= numUnits * UTF_32_UNIT_BYTES ;
        length += numUnits ;
    }

/* Add a null terminator (0x00000000). */

    if (dstlen >= UTF_32_UNIT_BYTES) {
        *dst++ = 0 ;  *dst++ = 0 ;
        *dst++ = 0 ;  *dst++ = 0 ;
    }

    return (length) ;

}

/*!*****************************************************************************

Procedure:

    utf16utf8 ()

    Convert a UTF-16 String to a UTF-8 String.


Purpose:

    Function utf16utf8() converts a UTF-16 encoding of a Unicode string to
    the UTF-8 representation of the string.

    The UTF-16 code units are assumed to be little-endian unless a byte-order
    marker (BOM) is present.  The BOM is not copied to the UTF-8 string,
    although a BOM can be inserted by specifying a "bom" argument of one.
    (A UTF-8 string is not little-endian or big-endian, but some programs
    look for a BOM at the beginning of a file to detect that the contents
    of the file are encoded in UTF-8.)


    Invocation:

        length = utf16utf8 (src, srclen, bom, dstlen, dst) ;

    where

        <src>		- I
            is the UTF-16 string to be converted.  The byte-order marker (BOM),
            if present, is not copied.  The BOM must be present if the UTF-16
            code units are little-endian; otherwise, big-endian is assumed.
        <srclen>	- I
            is the length in bytes of the UTF-16 string.  If this argument
            is less than zero, the string is assumed to be null-terminated
            (i.e., 0x0000).
        <bom>		- I
            specifies whether or not a byte-order marker (BOM) is to be
            inserted at the beginning of the UTF-8 string: 0 indicates no
            and 1 indicates a big-endian BOM.  Normally, you would not want
            a BOM and thus should specify 0 for this argument.
        <dstlen>	- I
            is the maximum length in bytes of the UTF-8 string buffer.
        <dst>		- O
            is a buffer into which the UTF-8 string is written.
        <length>	- O
            returns the number of code units/bytes written to the UTF-8 buffer;
            -1 is returned in the event of an error.

*******************************************************************************/


ssize_t  utf16utf8 (

#    if PROTOTYPES
        const  char  *src,
        ssize_t  srclen,
        int  bom,
        size_t  dstlen,
        char  *dst)
#    else
        src, srclen, bom, dstlen, dst)

        const  char  *src ;
        ssize_t  srclen ;
        int  bom ;
        size_t  dstlen ;
        char  *dst ;
#    endif

{    /* Local variables. */
    bool  bigEndian ;
    int32_t  codePoint ;
    size_t  length, numUnits ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16utf8) NULL source string: ") ;
        return (-1) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf16utf8) NULL destination string: ") ;
        return (-1) ;
    }

    if (srclen < 0)  srclen = utf16len (src) ;

    length = 0 ;

/* Determine the byte order of the UTF-16 source string and skip the BOM,
   if present. */

    bigEndian = (utf16bom (src) >= 0) ;
    if (utf16bom (src)) {
        src += UTF_16_UNIT_BYTES ;  srclen -= UTF_16_UNIT_BYTES ;
    }

/* If the caller specified that a byte-order marker appear at the beginning
   of the string, then insert it. */

    if (bom != 0) {
        if (dstlen < 3) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf16utf8) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf8put (0x0000FEFF, dst, &numUnits)) {
            LGE "(utf16utf8) Error writing BOM to destination string (%p): ",
                dst) ;
            return (-1) ;
        }
        dst += numUnits ;  dstlen -= numUnits ;  length += numUnits ;
    }

/* Encode the UTF-16 code points as UTF-8. */

    while (srclen > 0) {
        codePoint = utf16get (bigEndian, src) ;
        if (codePoint < 0) {
            LGE "(utf16utf8) Error in source string (%p): ", src) ;
            return (-1) ;
        }
        if (codePoint < 0x10000) {
            src += UTF_16_UNIT_BYTES ;  srclen -= UTF_16_UNIT_BYTES ;
        } else {
            src += UTF_16_PAIR_BYTES ;  srclen -= UTF_16_PAIR_BYTES ;
        }
        if ((dstlen < 4) && ((codePoint > 0x0FFFF) ||
              ((dstlen < 3) && ((codePoint > 0x00800) ||
                ((dstlen < 2) && ((codePoint > 0x0007F) ||
                  (dstlen < 1))))))) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf16utf8) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf8put (codePoint, dst, &numUnits)) {
            LGE "(utf16utf8) Error writing to destination string (%p): ", dst) ;
            return (-1) ;
        }
        dst += numUnits ;  dstlen -= numUnits ;
        length += numUnits ;
    }

    if (dstlen >= 1)  *dst = '\0' ;

    return (length) ;

}

/*!*****************************************************************************

Procedure:

    utf32utf8 ()

    Convert a UTF-32 String to a UTF-8 String.


Purpose:

    Function utf32utf8() converts a UTF-32 encoding of a Unicode string to
    the UTF-8 representation of the string.

    The UTF-32 code units are assumed to be little-endian unless a byte-order
    marker (BOM) is present.  The BOM is not copied to the UTF-8 string,
    although a BOM can be inserted by specifying a "bom" argument of one.
    (A UTF-8 string is not little-endian or big-endian, but some programs
    look for a BOM at the beginning of a file to detect that the contents
    of the file are encoded in UTF-8.)


    Invocation:

        length = utf32utf8 (src, srclen, bom, dstlen, dst) ;

    where

        <src>		- I
            is the UTF-32 string to be converted.  The byte-order marker (BOM),
            if present, is not copied.  The BOM must be present if the UTF-32
            code units are little-endian; otherwise, big-endian is assumed.
        <srclen>	- I
            is the length in bytes of the UTF-32 string.  If this argument
            is less than zero, the string is assumed to be null-terminated
            (i.e., 0x0000).
        <bom>		- I
            specifies whether or not a byte-order marker (BOM) is to be
            inserted at the beginning of the UTF-8 string: 0 indicates no
            and 1 indicates a big-endian BOM.  Normally, you would not want
            a BOM and thus should specify 0 for this argument.
        <dstlen>	- I
            is the maximum length in bytes of the UTF-8 string buffer.
        <dst>		- O
            is a buffer into which the UTF-8 string is written.
        <length>	- O
            returns the number of code units/bytes written to the UTF-8 buffer;
            -1 is returned in the event of an error.

*******************************************************************************/


ssize_t  utf32utf8 (

#    if PROTOTYPES
        const  char  *src,
        ssize_t  srclen,
        int  bom,
        size_t  dstlen,
        char  *dst)
#    else
        src, srclen, bom, dstlen, dst)

        const  char  *src ;
        ssize_t  srclen ;
        int  bom ;
        size_t  dstlen ;
        char  *dst ;
#    endif

{    /* Local variables. */
    bool  bigEndian ;
    int32_t  codePoint ;
    size_t  length, numUnits ;



    if (src == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf32utf8) NULL source string: ") ;
        return (-1) ;
    }

    if (dst == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(utf32utf8) NULL destination string: ") ;
        return (-1) ;
    }

    if (srclen < 0)  srclen = utf16len (src) ;

    length = 0 ;

/* Determine the byte order of the UTF-16 source string and skip the BOM,
   if present. */

    bigEndian = (utf32bom (src) >= 0) ;
    if (utf32bom (src)) {
        src += UTF_32_UNIT_BYTES ;  srclen -= UTF_32_UNIT_BYTES ;
    }

/* If the caller specified that a byte-order marker appear at the beginning
   of the string, then insert it. */

    if (bom != 0) {
        if (dstlen < 3) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf32utf8) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf8put (0x0000FEFF, dst, &numUnits)) {
            LGE "(utf32utf8) Error writing BOM to destination string (%p): ",
                dst) ;
            return (-1) ;
        }
        dst += numUnits ;  dstlen -= numUnits ;  length += numUnits ;
    }

/* Encode the UTF-32 code points as UTF-8. */

    while (srclen > 0) {
        codePoint = utf32get (bigEndian, src) ;
        if (codePoint < 0) {
            LGE "(utf32utf8) Error in source string (%p): ", src) ;
            return (-1) ;
        }
        src += UTF_32_UNIT_BYTES ;  srclen -= UTF_32_UNIT_BYTES ;
        if ((dstlen < 4) && ((codePoint > 0x0FFFF) ||
              ((dstlen < 3) && ((codePoint > 0x00800) ||
                ((dstlen < 2) && ((codePoint > 0x0007F) ||
                  (dstlen < 1))))))) {
            SET_ERRNO (ENOMEM) ;
            LGE "(utf32utf8) Destination string is too small: ") ;
            return (-1) ;
        }
        if (utf8put (codePoint, dst, &numUnits)) {
            LGE "(utf32utf8) Error writing to destination string (%p): ", dst) ;
            return (-1) ;
        }
        dst += numUnits ;  dstlen -= numUnits ;
        length += numUnits ;
    }

    if (dstlen >= 1)  *dst = '\0' ;

    return (length) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the UTF_UTIL functions.

    Under UNIX:
        Compile and link as follows:
            % cc -DTEST utf_util.c <libraries> -o utf_test
        Run the program from the command line:
            % utf_test

*******************************************************************************/

int  main (argc, argv)
    int  argc ;
    char  *argv[] ;

{    /* Local variables. */
    char  buffer[32] ;
    char  *s, *t ;
    int  i ;
    int32_t  codePoint ;
    size_t  length, numUnits ;



    aperror_print = 1 ;

  { char  example1[] = { 0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E } ;
    char  example2[] = { 0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4 } ;
    char  example3[] = { 0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E } ;
    char  example4[] = { 0xEF, 0xBB, 0xBF, 0xF0, 0xA3, 0x8E, 0xB4 } ;

    printf ("RFC 3269 Example 1:\n") ;
    s = example1 ;
    for (i = 0, length = 0, t = buffer ;  i < 4 ;  i++) {
        codePoint = utf8get (s, &numUnits) ;
        printf ("    U+%04X\n", codePoint) ;
        s += numUnits ;
        utf8put (codePoint, t, &numUnits) ;
        t += numUnits ;  length += numUnits ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("RFC 3269 Example 2:\n") ;
    s = example2 ;
    for (i = 0, length = 0, t = buffer ;  i < 3 ;  i++) {
        codePoint = utf8get (s, &numUnits) ;
        printf ("    U+%04X\n", codePoint) ;
        s += numUnits ;
        utf8put (codePoint, t, &numUnits) ;
        t += numUnits ;  length += numUnits ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("RFC 3269 Example 3:\n") ;
    s = example3 ;
    for (i = 0, length = 0, t = buffer ;  i < 3 ;  i++) {
        codePoint = utf8get (s, &numUnits) ;
        printf ("    U+%04X\n", codePoint) ;
        s += numUnits ;
        utf8put (codePoint, t, &numUnits) ;
        t += numUnits ;  length += numUnits ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("RFC 3269 Example 4:\n") ;
    s = example4 ;
    for (i = 0, length = 0, t = buffer ;  i < 2 ;  i++) {
        codePoint = utf8get (s, &numUnits) ;
        printf ("    U+%04X\n", codePoint) ;
        s += numUnits ;
        utf8put (codePoint, t, &numUnits) ;
        t += numUnits ;  length += numUnits ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }
  }

    printf ("====================\n") ;

  { char  example1[] = { 0xD8, 0x08, 0xDF, 0x45, 0x00, 0x3D, 0x00, 0x52, 0x00, 0x61 } ;
    char  example2[] = { 0x08, 0xD8, 0x45, 0xDF, 0x3D, 0x00, 0x52, 0x00, 0x61, 0x00 } ;
    char  example3[] = { 0xFE, 0xFF, 0xD8, 0x08, 0xDF, 0x45, 0x00, 0x3D, 0x00, 0x52, 0x00, 0x61 } ;
    char  example4[] = { 0xFF, 0xFE, 0x08, 0xD8, 0x45, 0xDF, 0x3D, 0x00, 0x52, 0x00, 0x61, 0x00 } ;

    printf ("RFC 2781 Example 1:\n") ;
    s = example1 ;
    for (i = 0, length = 0, t = buffer ;  i < 4 ;  i++) {
        codePoint = utf16get (true, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += (codePoint < 0x10000) ? 2 : 4 ;
        utf16put (codePoint, true, t) ;
        t += (codePoint < 0x10000) ? 2 : 4 ;
        length += (codePoint < 0x10000) ? 2 : 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("RFC 2781 Example 2:\n") ;
    s = example2 ;
    for (i = 0, length = 0, t = buffer ;  i < 4 ;  i++) {
        codePoint = utf16get (false, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += (codePoint < 0x10000) ? 2 : 4 ;
        utf16put (codePoint, false, t) ;
        t += (codePoint < 0x10000) ? 2 : 4 ;
        length += (codePoint < 0x10000) ? 2 : 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("RFC 2781 Example 3:\n") ;
    s = example3 ;
    for (i = 0, length = 0, t = buffer ;  i < 5 ;  i++) {
        codePoint = utf16get (true, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += (codePoint < 0x10000) ? 2 : 4 ;
        utf16put (codePoint, true, t) ;
        t += (codePoint < 0x10000) ? 2 : 4 ;
        length += (codePoint < 0x10000) ? 2 : 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("RFC 2781 Example 4:\n") ;
    s = example4 ;
    for (i = 0, length = 0, t = buffer ;  i < 5 ;  i++) {
        codePoint = utf16get (false, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += (codePoint < 0x10000) ? 2 : 4 ;
        utf16put (codePoint, false, t) ;
        t += (codePoint < 0x10000) ? 2 : 4 ;
        length += (codePoint < 0x10000) ? 2 : 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }
  }

    printf ("====================\n") ;

  { char  example1[] = { 0x00, 0x01, 0x23, 0x45, 0x00, 0x00, 0x00, 0x3D } ;
    char  example2[] = { 0x45, 0x23, 0x01, 0x00, 0x3D, 0x00, 0x00, 0x00 } ;
    char  example3[] = { 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x01, 0x23, 0x45, 0x00, 0x00, 0x00, 0x3D } ;
    char  example4[] = { 0xFF, 0xFE, 0x00, 0x00, 0x45, 0x23, 0x01, 0x00, 0x3D, 0x00, 0x00, 0x00 } ;

    printf ("UTF-32 Example 1:\n") ;
    s = example1 ;
    for (i = 0, length = 0, t = buffer ;  i < 2 ;  i++) {
        codePoint = utf32get (true, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += 4 ;
        utf32put (codePoint, true, t) ;
        t += 4 ;
        length += 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("UTF-32 Example 2:\n") ;
    s = example2 ;
    for (i = 0, length = 0, t = buffer ;  i < 2 ;  i++) {
        codePoint = utf32get (false, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += 4 ;
        utf32put (codePoint, false, t) ;
        t += 4 ;
        length += 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("UTF-32 Example 3:\n") ;
    s = example3 ;
    for (i = 0, length = 0, t = buffer ;  i < 3 ;  i++) {
        codePoint = utf32get (true, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += 4 ;
        utf32put (codePoint, true, t) ;
        t += 4 ;
        length += 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }

    printf ("UTF-32 Example 4:\n") ;
    s = example4 ;
    for (i = 0, length = 0, t = buffer ;  i < 3 ;  i++) {
        codePoint = utf32get (false, s) ;
        printf ("    U+%04X\n", codePoint) ;
        s += 4 ;
        utf32put (codePoint, false, t) ;
        t += 4 ;
        length += 4 ;
    }
    for (i = 0 ;  i < length ;  i++) {
        printf ("    0x%02X\n", (unsigned char) buffer[i]) ;
    }
  }

    printf ("====================\n") ;

  { char  ping[256], pong[256] ;
    const  char  *example = "The quick brown fox jumped over the lazy dog!" ;

    length = utf8utf16 (example, -1, -1, sizeof ping, ping) ;
    printf ("\nUTF-16:\n") ;
    meoDumpX (stdout, "    ", 0, ping, length * UTF_16_UNIT_BYTES) ;

    length = utf16utf8 (ping, length * UTF_16_UNIT_BYTES, 1,
                        sizeof pong, pong) ;
    printf ("\nUTF-8:\n") ;
    meoDumpX (stdout, "    ", 0, pong, length) ;

    length = utf8utf32 (example, -1, 1, sizeof ping, ping) ;
    printf ("\nUTF-32:\n") ;
    meoDumpX (stdout, "    ", 0, ping, length * UTF_32_UNIT_BYTES) ;

    length = utf32utf8 (ping, length * UTF_32_UNIT_BYTES, 0,
                        sizeof pong, pong) ;
    printf ("\nUTF-8:\n") ;
    meoDumpX (stdout, "    ", 0, pong, length) ;

  }

    exit (0) ;

}

#endif  /* TEST */
