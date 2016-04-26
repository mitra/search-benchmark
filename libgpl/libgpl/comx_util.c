/* $Id: comx_util.c,v 1.21 2011/07/18 17:54:19 alex Exp $ */
/*******************************************************************************

File:

    comx_util.c

    CORBA Marshaling Utilities.


Author:    Alex Measday


Purpose:

    The COMX utilities are used to convert the primitive and some basic
    constructed data types to and from the Common Data Representation (CDR)
    encodings defined for the General Inter-ORB Protocol (GIOP) in Chapter
    15 of the CORBA specification.

    As discussed in the "History and Caveats" section below, the COMX, GIMX,
    and IIOP packages were developed as a lightweight, low-level implementation
    of CORBA TCP/IP communications.  As such, these packages have provided a
    means of (i) quickly writing test clients and servers and of (ii) linking
    legacy applications to CORBA services - without the complexity and overhead
    of a full-blown CORBA implementation.


    Marshaling Channels
    ===================

    The COMX package is patterned after Sun's eXternal Data Representation (XDR)
    package.  A "marshaling channel" is created for a memory buffer containing
    (or that will contain) the CDR-encoded data.  Three operations can be
    performed on a marshaling channel:

        MxDECODE - decode CDR-encoded data in the channel's buffer
            and store it in host CPU variables or structures.  In
            some cases (e.g., strings), memory is dynamically
            allocated for host CPU values.

        MxENCODE - encode host CPU variables or structures in CDR format
            and store the encoded data in the channel's buffer.

        MxERASE - deallocate host CPU values that were dynamically
            allocated by the MxDECODE operation.


    Decoding Input Data
    ===================

    After reading a CORBA message, an application creates a marshaling channel
    for the buffer containing the received message body; the channel operation
    defaults to MxDECODE:

        bool  byteOrder ;
        ComxChannel  channel ;
        octet  *body ;
        unsigned  long  size ;
        Version  version ;

        ... read message header and body;
            grab GIOP version, byte order, and message size from header ...

        comxCreate (version, byteOrder, 12, body, size, &channel) ;

        ... decode the data in the message body ...

    Subsequent calls to comx<dataType>() functions will step through the
    message body, decoding the specified data types.


    Encoding Output Data
    ====================

    Before writing a CORBA message, an application creates a marshaling channel
    for a buffer in which to store the CDR-encoded message body.  By specifying
    a NULL buffer, the application can let the COMX functions take care of
    allocating a buffer and extending its size when necessary; in this case,
    the channel operation is automatically set to MxENCODE:

        ... set GIOP version explicitly ...

        comxCreate (version, false, 12, NULL, 0, &channel) ;

        ... encode the data into the message body ...

        body = comxBuffer (channel, false) ;
        size = comxSkip (channel, 0) ;

        ... write message header and body ...

    The byte-order flag is ignored in MxENCODE mode; the host CPU byte order
    is assumed.  The comxBuffer() call retrieves the address of the encoded
    data before writing the message.  The comxSkip(zero) call retrieves the
    number of bytes that have been encoded; i.e., the message size.


    CDR Data Alignment
    ==================

    In both examples above, a "virtual" buffer offset of 12 bytes was specified
    when a channel was created.  This is the size of the IIOP message header
    that precedes the message body when the message is transferred over a
    network connection.  CDR requires that primitive data values be aligned
    on "even" boundaries of their size; e.g., 2 bytes for a short integer,
    4 bytes for a long integer, and 8 bytes for a float.  Even boundaries
    are determined relative to the beginning of the message header; hence,
    when skipping (MxDECODE) or inserting (MxENCODE) padding to achieve CDR
    alignment, the COMX utilities must take into the account the "virtual"
    12-byte header.  When decoding or encoding a message body, always specify
    an offset of 12.  The COMX utilities can't simply assume an offset of 12,
    since encapsulated CDR data uses an offset of 0 - see comxEncapsule() for
    more information.


    Conversion Functions
    ====================

    The actual conversion functions have a regular calling sequence:

        ComxChannel  channel ;
        int  status ;
        <DataType>  value ;

        status = comx<DataType> (channel, &value) ;

    The *address* of the value is always passed in.  When decoding data, the
    CDR-encoded data in the channel's buffer is converted to host CPU format
    and stored in the value.  When encoding data, the value is converted from
    host CPU format to CDR and stored in the channel's buffer.  The status
    returned by the conversion function is zero if the marshaling operation
    succeeded and an ERRNO code if the operation failed.

    The conversion functions for the CDR primitive data types automatically
    insert/skip the padding required for data alignment, extend the size of
    a full buffer when encoding, and advance an internal pointer past the
    decoded/encoded value in the buffer when the operation is complete.
    The next call to a conversion function will begin at the new location
    of the pointer.  (Alignment continues to be determined relative to the
    beginning of the buffer minus the virtual offset.)


    Constructed Data Types
    ======================

    Conversion functions for more complex data types ("constructed data type"
    in CDR parlance) are easily implemented using the primitive conversion
    functions (or other complex conversion functions) without regard to the
    channel operation, padding, etc.  For example, a GIOP 1.2 ReplyHeader
    structure contains a long request ID, an enumerated reply status, and
    a service context list.  A ReplyHeader marshaling function is very simple:

        typedef  enum  ReplyStatusType {	// Enumeration declaration.
            ...
        }  ReplyStatusType ;

        typedef  struct  ReplyHeader {		// Structure declaration.
            unsigned  long  request_id ;
            ReplyStatusType  reply_status ;
            ServiceContextList  service_context ;
        }  ReplyHeader ;

        int  gimxReplyHeader (			// Conversion function.
            ComxChannel  channel,
            ReplyHeader  *value)
        {
            comxULong (channel, &value->request_id) ;
            comxEnum (channel, &value->reply_status) ;
            gimxServiceContextList (channel, &value->service_context) ;
            return (0) ;
        }

    [The actual gimxReplyHeader() function also checks for error returns from
    the marshaling functions it calls.  The field references are enclosed in
    NULL_OR() macros that check if the value pointer is NULL.  By passing in
    a NULL value pointer, a value can be decoded and discarded; i.e., not
    returned to the caller.  I originally thought this capability might be
    useful, but I've never had occasion to use it.]

    Marhshaling functions for unions must be explicitly coded, although doing
    so is trivial.  I had hoped to write a comxUnion() function similar to
    XDR's xdr_union(), but the discriminant of a CORBA union can be of any
    data type and a general-purpose marshaling function would probably require
    the caller to jump through hoops (e.g., supplying marshalling and comparison
    functions for the discriminant).  If only discriminants were limited to
    enumerated types ...


    Special CDR Data Types
    ======================

    The COMX package includes marshaling functions for some common constructed
    data types:

        String - is a NUL-terminated (char *) character string.  When decoding
            a string, the comxString() function dynamically allocates space for
            the string using malloc(3).  A subsequent call to free(3) or
            comxErase() is necessary to deallocate the string.

        WString - is a NUL-terminated (wchar_t *) wide-character string.
            When decoding a wide-character string, the comxWString()
            function dynamically allocates space for the string using
            malloc(3).  A subsequent call to free(3) or comxErase() is
            necessary to deallocate the string.

        OctetSeq - is a sequence of octets; i.e., a byte array.  When
            decoding an octet sequence, the comxOctetSeq() function
            dynamically allocates the array using malloc(3).  A subsequent
            call to comxErase() is necessary to deallocate the sequence.

        Version - is a GIOP version structure, containing a major version
            number and a minor version number.

    Three other data types require special-case marshaling functions:

        Array - is an array of some data type.  Unlike the "sequence" below,
            a CDR array does not include information about the number of
            elements in the array; the applications encoding and decoding
            the array are assumed to know the number of elements a priori.

        Encapsulation - is encoded data "encapsulated" in an octet sequence.
            The comxEncapsule() function is passed a list of conversion
            functions and addresses of values.  In MxDECODE mode, the values
            are decoded from an octet sequence (previously decoded from a
            message body).  In MxENCODE mode, the values are encoded into
            an octet sequence (to be encoded into a message body).  Data
            alignment is relative to the beginning of the octet sequence.

        Sequence - is an array of some data type.  A sequence is represented
            in host CPU format as a structure with two fields: the number of
            elements in the array and a pointer to the array of elements.
            When decoding a sequence, the array is dynamically allocated;
            a subsequent call to comxErase() is necessary to deallocate the
            sequence.  The comxSequence() function is passed the conversion
            function for the given data type.


    Erasing Data Values
    ===================

    When decoding strings, sequences, etc., the COMX functions dynamically
    allocate space for the multi-element values using malloc(3).  Once used,
    the values can be deallocated the correct way or the easy way.  The
    correct way is to change the marshaling channel operation from MxDECODE
    to MxERASE and re-call the conversion function(s):

        OctetSeq  object ;
        ... create marshaling channel ...
        comxOctetSeq (channel, &object) ;	// Decode octet sequence.
        ... use object value ...
        comxSetOp (channel, MxERASE) ;
        comxOctetSeq (channel, &object) ;	// Erase octet sequence.

    The easy way is to ignore the original marshaling channel and call
    comxErase():

        comxErase ((ComxFunc) comxOctetSeq, &object) ;

    The second method may silently have problems with values that have GIOP
    version-specific memory allocation; I haven't encountered this problem yet.


    History and Caveats
    ===================

    The COMX, GIMX, and IIOP packages were written out of desperation one
    weekend and refined with use over subsequent weeks.  I had to interface
    a legacy application to some CORBA servers.  TAO was too heavy-weight
    and ORBit's wide-character string functions wouldn't compile.  MICO
    worked, but the documentation was out-of-date.  Some MICO-based test
    clients (correctly) threw exceptions when trying to talk to our TAO-based
    servers.  I put this code together quickly to intercept, dump and analyze
    what our TAO-based *clients* were sending to the servers.

    The code has been compiled and tested under Linux (RedHat 6.2/7.2), under
    various versions of Solaris, and under Windows 95/NT4.0 (in a slightly
    modified C++ version).

    Some known shortcomings:

    (1) Host CPUs are assumed to be little-endian or big-endian.  PDP-11s were
        "mixed-endian" and I understand that the ARM or MIPS processor stores
        64-bit IEEE floats with 32-bit words one-endian and the bytes within
        the words other-endian.

    (2) The standard C integer types are used as the host representation of
        the CDR integer types.  The host CPU types are assumed to be at least
        as wide as the corresponding CDR types (2, 4, and 8 bytes).  If the
        host CPU types are wider, decoded values are sign- or zero-extended
        as necessary and encoded values truncate the most significant bytes
        (of the host CPU representation).

    (3) The fixed-point decimal type is not implemented.

    (4) The host CPU is assumed to represent floating-point numbers in IEEE 754
        format and in the same byte order as integers.  It is assumed that the
        host CPU supports 32-bit "float"s and 64-bit "double"s.  The host CPU's
        "long double"s are assumed to be full or abbreviated versions of CDR's
        128-bit "long double"s with the same or fewer bits in the mantissa; this
        may not be a valid assumption, so test before using!

    (5) The host CPU's wchar_t wide-character type is assumed to hold UNICODE
        characters, an assumption that is not necessarily true and that is at
        odds with the C standard *and* the UNICODE standard.

    (6) Wide characters are partially supported.  Our existing CORBA interfaces
        use wide strings, but not individual wide characters, so this area needs
        more work and testing.  For GIOP versions 1.0 and 1.1, the Transmission
        Code Set (TCS-W) is assumed to be UTF-16: 16-bit characters transmitted
        as unsigned shorts.  For GIOP version 1.2 and later, the TCS-W is again
        assumed to be UTF-16, but each character is encoded in 3 octets.  (I
        have not been able to test GIOP 1.2 wide strings yet.)  The byte-order
        marker (BOM) allowed in GIOP 1.2 is not currently recognized.

    (7) Wide strings are partially supported, enough for my needs.
        For all GIOP versions, the TCS-W is assumed to be UTF-16:
        16-bit characters transmitted as unsigned shorts.  Surrogate
        pairs are supported in accordance with RFC 2781, although
        they obviously will not work on a system whose wide characters
        are themselves 16 bits wide (e.g., Windows) or less.

    ... more if I think of anything else ...

    On a lighter note, I was a better speller in third grade than I am now.
    Spelling "marshaling" and "marshaled" with one "l" (as the CORBA
    specification and, more importantly, Henning and Vinoski do) bothers me;
    I prefer two "l"s and the dictionary lists both forms.  However, I came
    across a rule that says you double the final consonant in a multi-syllable
    base word if the stress is on the last syllable.  MAR-shal: I humbly admit
    defeat by the forces marshaled against me and I will henceforth use one "l"!
    (Of course, Google searches for "marshaling" and "marshalling" return
    47,400 and 101,000 hits, respectively ...)


Public Procedures (* defined in comx_util.h):

    comxBuffer() - gets a marshaling channel's buffer.
    comxCreate() - creates a marshaling channel.
    comxDestroy() - destroys a marshaling channel.
    comxErase() - erases a decoded data structure.
    comxExtend() - extends a marshaling channel's buffer.
    comxGetOp() - gets the current marshaling mode.
    comxGetVersion() - gets the GIOP version number.
  * comxReset() - resets the current location to the beginning of the buffer.
    comxSetOp() - configures a channel for decoding, encoding, or erasing.
    comxSkip() - advances the current location in the channel's buffer.
    comxToHost() - converts numbers to host-byte order.

    comx<Type>() - decode/encode CDR primitive types.
    .
    .
    .

    comxArray() - decodes/encodes/erases a CDR array.
    comxEncapsule() - decodes/encodes/erases a CDR encapsulation.
    comxSequence() - decodes/encodes/erases a GIOP sequence.
    comxString() - decodes/encodes/erases a GIOP string.
    comxWString() - decodes/encodes/erases a GIOP wstring.

    comxOctetSeq() - decodes/encodes/erases a GIOP octet sequence.
    comxVersion() - decodes/encodes/erases a GIOP version number.

    comx<Type>Seq() - decodes/encodes/erases a GIOP sequence of CDR
        primitive types.
    ...

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <wchar.h>			/* C Library wide string functions. */
#include  "ieee_util.h"			/* IEEE 754 utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "wcs_util.h"			/* Wide-character string functions. */
#include  "comx_util.h"			/* CORBA marshaling utilities. */


#if HAVE_NAMESPACES
    namespace  CoLi {
#endif


/*******************************************************************************
    CORBA Marshaling Channel - represents the decoding/encoding data flow
        between a GIOP message buffer and host memory data structures.
        The offset is the virtual offset in bytes of the buffer start from
        the GIOP message start; the GIOP message header begins at offset 0.
        The current pointer is the address in the GIOP buffer where the next
        decode/encode operation will be performed; this pointer is advanced
        by the marshaling functions.
*******************************************************************************/

typedef  struct  _ComxChannel {
    Version  version ;			/* GIOP major/minor version numbers. */
    bool  isLE ;			/* Message in little-endian format? */
    unsigned  long  offset ;		/* Offset in octets of start of buffer. */
    octet  *buffer ;			/* GIOP buffer. */
    unsigned  long  length ;		/* Length of GIOP buffer. */
    bool  dynamic ;			/* Destroy buffer with channel? */
    ComxOperation  operation ;		/* MxDECODE, MxENCODE, or MxERASE. */
    octet  *current ;			/* Location of next operation. */
}  _ComxChannel ;


int  comx_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  comx_util_debug


/*******************************************************************************
    Convenience Macros.
*******************************************************************************/

/* CHECK(status) - return ERRNO on non-zero status. */

#define  CHECK(status)			\
    if ((status))  return (errno) ;

/* ENOUGH(channel,size) - are there <size> octets remaining in buffer? */

#define  ENOUGH(channel, size)				\
    (((channel)->current - (channel)->buffer) <=	\
     ((long) (channel)->length - (long) (size)))

/* MAKE_ROOM(channel,size) - expand buffer to hold <size> more octets. */

#define  MAKE_ROOM(channel, size)		\
    if (!ENOUGH ((channel), (size)) &&		\
        comxExtend ((channel), (size))) {	\
        return (errno) ;			\
    }

/* NULL_OR(pointer,field) - pass NULL if structure pointer is NULL; else
   pass address of field in structure. */

#define  NULL_OR(pointer, field)	\
    (((pointer) == NULL) ? NULL : &(pointer)->field)

/* RETURN_IF_EOB(channel,size) - return ENOSPC if less than <size> octets
   remain in the buffer. */

#define  RETURN_IF_EOB(channel, size)	\
    if (!ENOUGH ((channel), (size))) {	\
        SET_ERRNO (ENOSPC) ;		\
        return (errno) ;		\
    }

/* RETURN_IF_NULL(pointer) - return EINVAL if pointer is NULL. */

#define  RETURN_IF_NULL(pointer)	\
    if ((pointer) == NULL) {		\
        SET_ERRNO (EINVAL) ;		\
        return (errno) ;		\
    }

/*******************************************************************************
    Compute and add padding for byte-boundary alignments.
*******************************************************************************/

#define  PADDING(offset, boundary)					\
    ((((offset) % (boundary)) == 0) ? 0					\
                                    : ((boundary) - ((offset) % (boundary))))

#define  BALIGN(channel, boundary)					\
  { unsigned  long  offset = (channel)->offset +			\
                             ((channel)->current - (channel)->buffer) ;	\
    (channel)->current += PADDING (offset, boundary) ;			\
  }

#define  PAD(address, numOctets)		\
    ((address) = ((char *) (address)) + numOctets)


/*******************************************************************************
    Test to see if host CPU is little-endian or big-endian.  The macro names
    were chosen to match those used in the GNU C Library "endian.h" header.
*******************************************************************************/

#if defined (BYTE_ORDER)		/* BSD conventions. */
#    if !defined (__BYTE_ORDER)
#        define  __LITTLE_ENDIAN  LITTLE_ENDIAN
#        define  __BIG_ENDIAN  BIG_ENDIAN
#        define  __PDP_ENDIAN  PDP_ENDIAN
#        define  __BYTE_ORDER  BYTE_ORDER
#    endif
#elif !defined (__BYTE_ORDER)		/* GNU C conventions. */
    static  unsigned  long  endian_value = 0x11223344 ;
#    define  FIRST_BYTE(value)  (*((uint8_t *) &(value)))
#    define  __LITTLE_ENDIAN  1234
#    define  __BIG_ENDIAN  4321
#    define  __PDP_ENDIAN  3412
#    define  __BYTE_ORDER						\
         ((FIRST_BYTE (endian_value) == 0x11) ? __BIG_ENDIAN :		\
          (FIRST_BYTE (endian_value) == 0x44) ? __LITTLE_ENDIAN :	\
          (FIRST_BYTE (endian_value) == 0x22) ? __PDP_ENDIAN : 0)
#endif

/*******************************************************************************
    Macros/functions to get bytes from the current location in the marshaling
    buffer.  If a host CPU's short, long, and/or long long numbers are wider
    than the 2-, 4-, or 8-byte CDR numbers, the GET<type> macros decode the
    marshaled data into the least significant 2, 4, or 8 bytes of the host
    numbers, sign- or zero-extending the most significant bit as necessary.
*******************************************************************************/

/*  Get an octet as a signed or unsigned character from the current location
    (plus an index) in the marshaling buffer.  UCURRENT is necessary to prevent
    a signed octet from being sign-extended to a wider value before it is cast
    to an unsigned integer of the wider width.  SCURRENT is for completeness. */

#define  SCURRENT(channel,index)  ((int8_t) (channel)->current[(index)])
#define  UCURRENT(channel,index)  ((uint8_t) (channel)->current[(index)])

#define  GETBYTE(channel)  ((channel)->current[0])

#define  GETSHORT(channel)					\
            ((channel)->isLE					\
             ? (((short) SCURRENT ((channel), 1) << 8) |	\
                (unsigned short) UCURRENT ((channel), 0))	\
             : (((short) SCURRENT ((channel), 0) << 8) |	\
                (unsigned short) UCURRENT ((channel), 1)))

#define  GETUSHORT(channel)						\
            ((channel)->isLE						\
             ? (((unsigned short) UCURRENT ((channel), 1) << 8) |	\
                (unsigned short) UCURRENT ((channel), 0))		\
             : (((unsigned short) UCURRENT ((channel), 0) << 8) |	\
                (unsigned short) UCURRENT ((channel), 1)))

#define  GETLONG(channel)						\
            ((channel)->isLE						\
             ? (((long) SCURRENT ((channel), 3) << 24) |		\
                ((unsigned long) UCURRENT ((channel), 2) << 16) |	\
                ((unsigned long) UCURRENT ((channel), 1) << 8) |	\
                (unsigned long) UCURRENT ((channel), 0))		\
             : (((long) SCURRENT ((channel), 0) << 24) |		\
                ((unsigned long) UCURRENT ((channel), 1) << 16) |	\
                ((unsigned long) UCURRENT ((channel), 2) << 8) |	\
                (unsigned long) UCURRENT ((channel), 3)))

#define  GETULONG(channel)						\
            ((channel)->isLE						\
             ? (((unsigned long) UCURRENT ((channel), 3) << 24) |	\
                ((unsigned long) UCURRENT ((channel), 2) << 16) |	\
                ((unsigned long) UCURRENT ((channel), 1) << 8) |	\
                (unsigned long) UCURRENT ((channel), 0))		\
             : (((unsigned long) UCURRENT ((channel), 0) << 24) |	\
                ((unsigned long) UCURRENT ((channel), 1) << 16) |	\
                ((unsigned long) UCURRENT ((channel), 2) << 8) |	\
                (unsigned long) UCURRENT ((channel), 3)))

#define  GETLONGLONG(channel)					\
            ((channel)->isLE					\
             ? (((LONGLONG) SCURRENT ((channel), 7) << 56) |	\
                ((ULONGLONG) UCURRENT ((channel), 6) << 48) |	\
                ((ULONGLONG) UCURRENT ((channel), 5) << 40) |	\
                ((ULONGLONG) UCURRENT ((channel), 4) << 32) |	\
                ((ULONGLONG) UCURRENT ((channel), 3) << 24) |	\
                ((ULONGLONG) UCURRENT ((channel), 2) << 16) |	\
                ((ULONGLONG) UCURRENT ((channel), 1) << 8) |	\
                (ULONGLONG) UCURRENT ((channel), 0))		\
             : (((LONGLONG) SCURRENT ((channel), 0) << 56) |	\
                ((ULONGLONG) UCURRENT ((channel), 1) << 48) |	\
                ((ULONGLONG) UCURRENT ((channel), 2) << 40) |	\
                ((ULONGLONG) UCURRENT ((channel), 3) << 32) |	\
                ((ULONGLONG) UCURRENT ((channel), 4) << 24) |	\
                ((ULONGLONG) UCURRENT ((channel), 5) << 16) |	\
                ((ULONGLONG) UCURRENT ((channel), 6) << 8) |	\
                (ULONGLONG) UCURRENT ((channel), 7)))

#define  GETULONGLONG(channel)					\
            ((channel)->isLE					\
             ? (((ULONGLONG) UCURRENT ((channel), 7) << 56) |	\
                ((ULONGLONG) UCURRENT ((channel), 6) << 48) |	\
                ((ULONGLONG) UCURRENT ((channel), 5) << 40) |	\
                ((ULONGLONG) UCURRENT ((channel), 4) << 32) |	\
                ((ULONGLONG) UCURRENT ((channel), 3) << 24) |	\
                ((ULONGLONG) UCURRENT ((channel), 2) << 16) |	\
                ((ULONGLONG) UCURRENT ((channel), 1) << 8) |	\
                (ULONGLONG) UCURRENT ((channel), 0))		\
             : (((ULONGLONG) UCURRENT ((channel), 0) << 56) |	\
                ((ULONGLONG) UCURRENT ((channel), 1) << 48) |	\
                ((ULONGLONG) UCURRENT ((channel), 2) << 40) |	\
                ((ULONGLONG) UCURRENT ((channel), 3) << 32) |	\
                ((ULONGLONG) UCURRENT ((channel), 4) << 24) |	\
                ((ULONGLONG) UCURRENT ((channel), 5) << 16) |	\
                ((ULONGLONG) UCURRENT ((channel), 6) << 8) |	\
                (ULONGLONG) UCURRENT ((channel), 7)))

/*******************************************************************************
    Macros/functions to put bytes into the current location in the marshaling
    buffer.  If a host CPU's short, long, and/or long long numbers are wider
    than the 2-, 4-, or 8-byte CDR numbers, the <type>TOP macros ensure that
    only the least significant 2, 4, or 8 bytes of the host CPU's numbers are
    put into the marshaling buffer.  For example, assume the host CPU longs
    are 6-bytes wide, 2 more than CDR's 4-byte longs.  If the host is
    little-endian, the first 4 bytes by location (0, 1, 2, and 3) are put
    in the marshaling buffer, thus lopping off the two most significant bytes
    in locations 4 and 5.  If the host is big-endian, the last 4 bytes by
    location (2, 3, 4, and 5) are put in the marshaling buffer, thus lopping
    off the two most significant bytes in locations 0 and 1.
*******************************************************************************/

#define  SHORTTOP  (sizeof (short))
#define  LONGTOP  (sizeof (long))
#define  LONGLONGTOP  (sizeof (LONGLONG))

#define  PUTBYTE(channel, source)  (*(channel)->current = *((octet *) (source)))

/* Note that PUTSHORT takes the channel's endian flag into account as well
   as the host's byte order.  This capability is used by comxWString(),
   via comxUShort(), to generate GIOP 1.2-compatible big-endian wide-string
   encoding on little-endian hosts. */

#define  PUTSHORT(channel, source)					\
    ((void) ((channel)->isLE						\
     ? ((__BYTE_ORDER == __LITTLE_ENDIAN)				\
        ? ((channel)->current[0] = ((octet *) (source))[0],		\
           (channel)->current[1] = ((octet *) (source))[1])		\
        : ((channel)->current[0] = ((octet *) (source))[SHORTTOP-1],	\
           (channel)->current[1] = ((octet *) (source))[SHORTTOP-2]))	\
     : ((__BYTE_ORDER == __LITTLE_ENDIAN)				\
        ? ((channel)->current[0] = ((octet *) (source))[1],		\
           (channel)->current[1] = ((octet *) (source))[0])		\
        : ((channel)->current[0] = ((octet *) (source))[SHORTTOP-2],	\
           (channel)->current[1] = ((octet *) (source))[SHORTTOP-1]))))

#define  PUTLONG(channel, source)					\
    ((void) ((__BYTE_ORDER == __LITTLE_ENDIAN)				\
     ? ((channel)->current[0] = ((octet *) (source))[0],		\
        (channel)->current[1] = ((octet *) (source))[1],		\
        (channel)->current[2] = ((octet *) (source))[2],		\
        (channel)->current[3] = ((octet *) (source))[3])		\
     : ((channel)->current[0] = ((octet *) (source))[LONGTOP-4],	\
        (channel)->current[1] = ((octet *) (source))[LONGTOP-3],	\
        (channel)->current[2] = ((octet *) (source))[LONGTOP-2],	\
        (channel)->current[3] = ((octet *) (source))[LONGTOP-1])))

#define  PUTLONGLONG(channel, source)					\
    ((void) ((__BYTE_ORDER == __LITTLE_ENDIAN)				\
     ? ((channel)->current[0] = ((octet *) (source))[0],		\
        (channel)->current[1] = ((octet *) (source))[1],		\
        (channel)->current[2] = ((octet *) (source))[2],		\
        (channel)->current[3] = ((octet *) (source))[3],		\
        (channel)->current[4] = ((octet *) (source))[4],		\
        (channel)->current[5] = ((octet *) (source))[5],		\
        (channel)->current[6] = ((octet *) (source))[6],		\
        (channel)->current[7] = ((octet *) (source))[7])		\
     : ((channel)->current[0] = ((octet *) (source))[LONGLONGTOP-8],	\
        (channel)->current[1] = ((octet *) (source))[LONGLONGTOP-7],	\
        (channel)->current[2] = ((octet *) (source))[LONGLONGTOP-6],	\
        (channel)->current[3] = ((octet *) (source))[LONGLONGTOP-5],	\
        (channel)->current[4] = ((octet *) (source))[LONGLONGTOP-4],	\
        (channel)->current[5] = ((octet *) (source))[LONGLONGTOP-3],	\
        (channel)->current[6] = ((octet *) (source))[LONGLONGTOP-2],	\
        (channel)->current[7] = ((octet *) (source))[LONGLONGTOP-1])))

/*!*****************************************************************************

Procedure:

    comxBuffer ()

    Get Address of a CORBA Marshaling Channel's Buffer.


Purpose:

    Function comxBuffer() returns a pointer to a channel's buffer.
    If the buffer was dynamically allocated by comxExtend() and
    the release argument is true, ownership of the buffer is
    turned over to the caller.  This capability is useful when
    creating CDR encapsulations.


    Invocation:

        buffer = comxBuffer (channel, release) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <release>	- I
            indicates whether ownership of the buffer should be turned over to
            the caller.  If this argument is true (non-zero) and the buffer was
            dynamically allocated by comxExtend(), the caller takes possession
            of the buffer and is responsible for free()ing it when it is no
            longer needed.  Since the channel no longer owns the buffer, the
            size of the buffer cannot be increased and subsequent attempts to
            encode data may fail.
        <buffer>	- O
            returns a pointer to the channel's buffer.

*******************************************************************************/


octet  *comxBuffer (

#    if PROTOTYPES
        ComxChannel  channel,
        bool  release)
#    else
        channel, release)

        ComxChannel  channel ;
        bool  release ;
#    endif

{

    if (channel == NULL)  return (0) ;

    if (release)  channel->dynamic = false ;	/* Caller takes possession? */

    return (channel->buffer) ;

}

/*!*****************************************************************************

Procedure:

    comxCreate ()

    Create a CORBA Marshaling Channel.


Purpose:

    Function comxCreate() creates a CORBA marshaling channel
    for the purpose of decoding or encoding a GIOP message.


    Invocation:

        status = comxCreate (version, littleEndian, offset,
                             buffer, length, &channel) ;

    where

        <version>	- I
            specifies the GIOP major/minor version numbers.  When decoding
            messages, the GIOP version is found in the GIOP message header.
        <littleEndian>	- I
            indicates whether the encoding of a message being decoded was
            little-endian (1) or big-endian (0).  See the "flags" field
            in the GIOP message header.  This argument is ignored when
            encoding a new message; the host byte-order is used.
        <offset>	- I
            specifies the virtual offset in bytes of the buffer start from the
            GIOP message start; the GIOP message header begins at offset 0.
            The offset is needed by the marshaling functions in order to
            satisfy the CDR/GIOP alignment restrictions.  For example, if you
            create a marshaling channel for the message body, which begins
            immediately after the GIOP message header, you should specify an
            offset of 12 (the size in bytes of the GIOP message header) even
            though the message header and body may actually be millions of
            bytes apart in physical memory.
	<buffer>	- I
            is a buffer that (i) contains GIOP data to be decoded, or
            (ii) will receive GIOP data being encoded.  If this argument
            is NULL, a buffer of the specified length will be dynamically
            allocated (and deallocated when the channel is destroyed).
        <length>	- I
            specifies the length in bytes of the buffer.
        <channel>	- O
            returns a handle for the new marshaling channel.  This handle
            is used in calls to the other COMX functions.
        <status>	- O
            returns the status of creating the channel, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxCreate (

#    if PROTOTYPES
        Version  version,
        bool  littleEndian,
        unsigned  long  offset,
        octet  *buffer,
        unsigned  long  length,
        ComxChannel  *channel)
#    else
        version, littleEndian, offset, buffer, length, channel)

        Version  version ;
        bool  littleEndian ;
        unsigned  long  offset ;
        octet  *buffer ;
        unsigned  long  length ;
        ComxChannel  *channel ;
#    endif

{

/* Create and initialize a channel structure. */

    *channel = (_ComxChannel *) malloc (sizeof (_ComxChannel)) ;
    if (*channel == NULL) {
        LGE "(comxCreate) Error allocating channel structure.\nmalloc: ") ;
        return (errno) ;
    }

    (*channel)->version = version ;
    (*channel)->isLE = littleEndian ;
    (*channel)->offset = offset ;
    (*channel)->buffer = buffer ;
    (*channel)->length = length ;
    (*channel)->dynamic = false ;
    (*channel)->operation = MxDECODE ;
    (*channel)->current = (*channel)->buffer ;

/* Allocate a buffer, if necessary, for encoding. */

    if (buffer == NULL) {
        (*channel)->isLE = (__BYTE_ORDER == __LITTLE_ENDIAN) ;
        (*channel)->length = 0 ;
        (*channel)->dynamic = true ;
        (*channel)->operation = MxENCODE ;
        if ((length > 0) && comxExtend (*channel, length)) {
            LGE "(comxCreate) Error creating buffer for encoding.\ncomxExtend: ") ;
            PUSH_ERRNO ;  comxDestroy (*channel) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

    LGI "(comxCreate) Created marshaling channel for %lu-octet buffer %p.\n",
        (*channel)->length, (*channel)->buffer) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    comxDestroy ()

    Destroy a CORBA Marshaling Channel.


Purpose:

    Function comxDestroy() destroys a CORBA marshaling channel.


    Invocation:

        status = comxDestroy (channel) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <status>	- O
            returns the status of deleting the channel, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxDestroy (

#    if PROTOTYPES
        ComxChannel  channel)
#    else
        channel)

        ComxChannel  channel ;
#    endif

{

    if (channel == NULL)  return (0) ;

    LGI "(comxDestroy) Closing channel for %lu-byte buffer %p ...\n",
        channel->length, channel->buffer) ;

/* If the buffer was dynamically allocated, deallocate it. */

    if (channel->dynamic && (channel->buffer != NULL)) {
        free (channel->buffer) ;
        channel->buffer = NULL ;
    }

/* Deallocate the channel structure. */

    free (channel) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    comxErase ()

    Erase a Decoded Data Structure.


Purpose:

    Function comxErase() erases the dynamically allocated fields in a host
    data structure previously decoded by the COMX functions.  Although you
    can accomplish the same thing by setting a marshaling channel's operation
    to MxERASE, comxErase() is convenient when you have kept a data structure
    around after closing the marshaling channel used to decode it.


    Invocation:

        status = comxErase (marshalF, &value) ;

    where

        <marshalF>	- I
            is a pointer to the marshaling function for the value being erased.
            The function is of type ComxFunc (declared "comx_util.h").
        <value>		- I/O
            is the address of the value to be erased.
        <status>	- O
            returns the status of erasing the value, zero if there were no
            errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxErase (

#    if PROTOTYPES
        ComxFunc  marshalF,
        void  *value)
#    else
        marshalF, value)

        ComxFunc  marshalF ;
        void  *value ;
#    endif

{    /* Local variables. */
    _ComxChannel  channel ;



/* Create a dummy marshaling channel. */

    channel.version.major = 0 ;
    channel.version.minor = 0 ;
    channel.isLE = false ;
    channel.offset = 0 ;
    channel.buffer = NULL ;
    channel.length = 0 ;
    channel.dynamic = false ;
    channel.operation = MxERASE ;
    channel.current = NULL ;

/* Erase the value. */

    return (marshalF (&channel, value)) ;

}

/*!*****************************************************************************

Procedure:

    comxExtend ()

    Extend a CORBA Marshaling Channel's Buffer.


Purpose:

    Function comxExtend() increases the size of a marshaling channel's buffer.
    The marshaling functions for the primitive CDR types call comxExtend() as
    needed when encoding data into the buffer, thus saving the application from
    having to guess how large the buffer should be.

    The channel must be in MxENCODE mode and the channel's buffer must have
    been dynamically allocated.  The increase in size is more than enough to
    cover any padding introduced by the ALIGN() macro.


    Invocation:

        status = comxExtend (channel, numOctets) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <numOctets>	- I
            specifies the number of octets that need to be added to the buffer.
        <status>	- O
            returns the status of extending the buffer, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxExtend (

#    if PROTOTYPES
        ComxChannel  channel,
        long  numOctets)
#    else
        channel, numOctets)

        ComxChannel  channel ;
        long  numOctets ;
#    endif

{    /* Local variables. */
    octet  *newBuffer ;
    unsigned  long  currentOffset, newLength ;



    if ((numOctets < 0) || (channel == NULL) ||
        !channel->dynamic || (channel->operation != MxENCODE)) {
        SET_ERRNO (EINVAL) ;
        LGE "(comxExtend) NULL channel or invalid mode: ") ;
        return (errno) ;
    }

/* Save the current offset and determine the new size of the buffer. */

    currentOffset = (channel->current - channel->buffer) ;

#define  CHUNK  1024
    newLength = (currentOffset * 2) + numOctets + CHUNK ;

    LGI "(comxExtend) Increasing buffer size from %lu to %lu octets.\n",
        channel->length, newLength) ;

    if (newLength <= channel->length)  return (0) ;

/* Allocate a new buffer of increased size and copy over the contents of
   the old buffer. */

    if (channel->buffer == NULL) {		/* Channel's first buffer? */
        newBuffer = (octet *) calloc (newLength, 1) ;
        if (newBuffer == NULL) {
            LGE "(comxExtend) Error allocating %lu-octet buffer.\ncalloc: ",
                newLength) ;
            return (errno) ;
        }
    } else {					/* Reallocate existing buffer. */
        newBuffer = (octet *) realloc (channel->buffer, newLength) ;
        if (newBuffer == NULL) {
            LGE "(comxExtend) Error reallocating %lu-octet buffer.\nrealloc: ",
                newLength) ;
            return (errno) ;
        }
    }
						/* Fill new space with 'FF's. */
    memset (&newBuffer[channel->length], 0xFF, newLength - channel->length) ;

    channel->buffer = newBuffer ;
    channel->length = newLength ;

/* Restore the current offset in the buffer. */

    channel->current = channel->buffer + currentOffset ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    comxGetOp ()

    Get a Channel's Marshaling Mode.


Purpose:

    Function comxGetOp() returns a CORBA marshaling channel's current
    marshaling mode: decoding, encoding, or erasing.


    Invocation:

        operation = comxGetOp (channel) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <operation>	- O
            returns the current marshaling operation: MxDECODE, MxENCODE,
            or MxERASE.

*******************************************************************************/


ComxOperation  comxGetOp (

#    if PROTOTYPES
        ComxChannel  channel)
#    else
        channel)

        ComxChannel  channel ;
#    endif

{

    if (channel == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(comxGetOp) NULL channel: ") ;
        return (MxDECODE) ;
    }

    return (channel->operation) ;

}

/*!*****************************************************************************

Procedure:

    comxGetVersion ()

    Get a Channel's GIOP Version.


Purpose:

    Function comxGetVersion() returns a CORBA marshaling channel's GIOP
    version number.


    Invocation:

        version = comxGetVersion (channel) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <version>	- O
            returns the GIOP version number specified when the channel was
            created.

*******************************************************************************/


Version  comxGetVersion (

#    if PROTOTYPES
        ComxChannel  channel)
#    else
        channel)

        ComxChannel  channel ;
#    endif

{

    if (channel == NULL) {
        Version  version = { 0, 0 } ;
        SET_ERRNO (EINVAL) ;
        LGE "(comxGetVersion) NULL channel: ") ;
        return (version) ;
    }

    return (channel->version) ;

}

/*!*****************************************************************************

Procedure:

    comxSetOp ()

    Configure a CORBA Marshaling Channel for Decoding/Encoding/Erasing.


Purpose:

    Function comxSetOp() configures a CORBA marshaling channel for decoding,
    encoding, or erasing.  The specified operation applies to subsequent calls
    to marshaling functions for this channel.


    Invocation:

        status = comxSetOp (channel, operation) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <operation>	- I
            is the desired marshaling operation: MxDECODE, MxENCODE,
            or MxERASE.
        <status>	- O
            returns the status of configuring the channel,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxSetOp (

#    if PROTOTYPES
        ComxChannel  channel,
        ComxOperation  operation)
#    else
        channel, operation)

        ComxChannel  channel ;
        ComxOperation  operation ;
#    endif

{

    if (channel == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(comxSetOp) NULL channel: ") ;
        return (errno) ;
    }

    channel->operation = operation ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    comxSkip ()

    Skip Ahead in a CORBA Marshaling Channel's Buffer.


Purpose:

    Function comxSkip() repositions the location in a channel's buffer
    at which the next marshaling operation will be performed.  Calling
    comxSkip() with an offset of zero and an alignment of zero returns
    the length of the encoded data in the buffer.


    Invocation:

        offset = comxSkip (channel, numOctets, alignment) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <numOctets>	- I
            is the number of octets to advance from the current position
            in the channel's buffer.
        <alignment>	- I
            specifies the byte boundary to which the new position is to
            be aligned.  If this argument is less than 2, no adjustment
            is made to the new position.
        <offset>	- O
            returns the byte offset of the new location from the start of
            the buffer.

*******************************************************************************/


long  comxSkip (

#    if PROTOTYPES
        ComxChannel  channel,
        long  numOctets,
        long  alignment)
#    else
        channel, numOctets, alignment)

        ComxChannel  channel ;
        long  numOctets ;
        long  alignment ;
#    endif

{

    if (channel == NULL)  return (0) ;

    channel->current += numOctets ;

    if (alignment > 1)  BALIGN (channel, alignment) ;

    return (channel->current - channel->buffer) ;

}

/*!*****************************************************************************

Procedure:

    comxToHost ()

    Convert a Number from Message-Byte Order to Host-Byte Order.


Purpose:

    Function comxToHost() converts a number from message-byte order
    to host-byte order.  If the message number is big-endian and the
    host number is little-endian (or vice-versa), the bytes must be
    reversed.  If the message and host agree on the byte order, no
    reversal is necessary.


    Invocation:

        comxToHost (littleEndian, numBytes, &cdrValue, &cpuValue) ;

    where

        <littleEndian>	- I
            indicates if the message byte order is big-endian (0)
            or little-endian (not 0).
        <numBytes>	- I
            is the number of bytes in the number being converted;
            e.g., 2 for a short, 4 for an int or long, 8 for a
            long long or double, etc.
        <cdrValue>	- I/O
            is the address of the number being converted.  If the
            CPU value address is NULL, the conversion takes place
            in the CDR value.
        <cpuValue>	- O
            is the address to which the converted number is written.
            If this argument is NULL, the converted number overwrites
            the CDR value.

*******************************************************************************/


void  comxToHost (

#    if PROTOTYPES
        bool  littleEndian,
        int  numBytes,
        void  *cdrValue,
        void  *cpuValue)
#    else
        littleEndian, numBytes, cdrValue, cpuValue)

        bool  littleEndian ;
        int  numBytes ;
        void  *cdrValue ;
        void  *cpuValue ;
#    endif

{    /* Local variables. */
    int  i ;



    if (cpuValue == NULL)  cpuValue = cdrValue ;

    if ((littleEndian && (__BYTE_ORDER == __BIG_ENDIAN))  ||
        (!littleEndian && (__BYTE_ORDER == __LITTLE_ENDIAN))) {
        char  *s = (char *) cdrValue ;
        char  *d = (char *) cpuValue + numBytes ;
        if (cdrValue == cpuValue) {
            for (i = 0 ;  i < (numBytes/2) ;  i++) {
                char  c = *s ;  *s++ = *--d ;  *d = c ;
            } ;
        } else {
            for (i = 0 ;  i < numBytes ;  i++) {
                *--d = *s++ ;
            }
        }
    } else {
        if (cdrValue != cpuValue)
            (void) memmove (cpuValue, cdrValue, numBytes) ;
    }

    return ;

}

/*!*****************************************************************************

Procedures:

    comxBoolean ()
    comxChar ()
    comxDouble ()
    comxEnum ()
    comxFloat ()
    comxLong ()
    comxLongDouble ()
    comxLongLong ()
    comxOctet ()
    comxShort ()
    comxULong ()
    comxULongLong ()
    comxUShort ()
    comxWChar ()

    Decode/Encode CDR Primitive Types.


Purpose:

    These functions decode and encode the Common Data Representation (CDR)
    primitive types.


    Invocation:

        status = comx<Type> (channel, &value) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <value>		- I/O
            is the address of the host value involved in the marshaling
            operation.  If the operation is MxDECODE, the data flow is
            from the CDR value (in the channel's buffer) to the host
            value.  If the operation is MxENCODE, the data flow is from
            the host value to the CDR value (in the channel's buffer).
            MxERASE operations have no effect on the primitive data types.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/

/*!*****************************************************************************

    comxBoolean() - decodes/encodes a CDR boolean.

*******************************************************************************/


errno_t  comxBoolean (

#    if PROTOTYPES
        ComxChannel  channel,
        bool  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        bool  *value ;
#    endif

{    /* Local variables. */
    octet  boolean ;



    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        CHECK (comxOctet (channel, &boolean)) ;
        if (value != 0)  *value = (boolean == 1) ;
        break ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        boolean = (*value) ? 1 : 0 ;
        CHECK (comxOctet (channel, &boolean)) ;
        break ;
    default:
        break ;
    }

    return (0) ;

}

/*!*****************************************************************************

    comxChar() - decodes/encodes a CDR char.

*******************************************************************************/


errno_t  comxChar (

#    if PROTOTYPES
        ComxChannel  channel,
        char  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        char  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        RETURN_IF_EOB (channel, 1) ;
        if (value != NULL)  *value = (char) GETBYTE (channel) ;
        channel->current++ ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        MAKE_ROOM (channel, 1) ;
        PUTBYTE (channel, value) ;
        channel->current++ ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxDouble() - decodes/encodes a CDR double.  The host value is assumed
        to be in IEEE floating-point format.

*******************************************************************************/


errno_t  comxDouble (

#    if PROTOTYPES
        ComxChannel  channel,
        double  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        double  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 8) ;
        RETURN_IF_EOB (channel, 8) ;
        if (value != NULL)
#if defined(HAVE_IEEEFP) && !HAVE_IEEEFP
            *value = ieee2double (64, channel->isLE ? 1 : 0, channel->current) ;
#else
            *((LONGLONG *) value) = GETLONGLONG (channel) ;
#endif
        channel->current += 8 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 8) ;
        MAKE_ROOM (channel, 8) ;
#if defined(HAVE_IEEEFP) && !HAVE_IEEEFP
        double2ieee (*value, 64, channel->isLE ? 1 : 0, channel->current) ;
#else
        PUTLONGLONG (channel, value) ;
#endif
        channel->current += 8 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxEnum() - decodes/encodes a CDR enum.

*******************************************************************************/


errno_t  comxEnum (

#    if PROTOTYPES
        ComxChannel  channel,
        unsigned  long  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        unsigned  long  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 4) ;
        RETURN_IF_EOB (channel, 4) ;
        if (value != NULL)  *value = GETULONG (channel) ;
        channel->current += 4 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 4) ;
        MAKE_ROOM (channel, 4) ;
        PUTLONG (channel, value) ;
        channel->current += 4 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxFloat() - decodes/encodes a CDR float.  The host value is assumed
        to be in IEEE floating-point format.

*******************************************************************************/


errno_t  comxFloat (

#    if PROTOTYPES
        ComxChannel  channel,
        float  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        float  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 4) ;
        RETURN_IF_EOB (channel, 4) ;
        if (value != NULL)
#if defined(HAVE_IEEEFP) && !HAVE_IEEEFP
            *value = (float) ieee2double (32, channel->isLE ? 1 : 0,
                                          channel->current) ;
#else
            *((long *) value) = GETLONG (channel) ;
#endif
        channel->current += 4 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 4) ;
        MAKE_ROOM (channel, 4) ;
#if defined(HAVE_IEEEFP) && !HAVE_IEEEFP
        double2ieee ((double) *value, 32,
                     channel->isLE ? 1 : 0, channel->current) ;
#else
        PUTLONG (channel, value) ;
#endif
        channel->current += 4 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxLong() - decodes/encodes a CDR long (32-bit integer).

*******************************************************************************/


errno_t  comxLong (

#    if PROTOTYPES
        ComxChannel  channel,
        long  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        long  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 4) ;
        RETURN_IF_EOB (channel, 4) ;
        if (value != NULL)  *value = GETLONG (channel) ;
        channel->current += 4 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 4) ;
        MAKE_ROOM (channel, 4) ;
        PUTLONG (channel, value) ;
        channel->current += 4 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxLongDouble() - decodes or encodes a CDR long double.  (That's a
        16-byte double; "long double" in one version of GCC gave a 12-byte
        double.)  The host value is assumed to be in IEEE floating-point
        format.

*******************************************************************************/


errno_t  comxLongDouble (

#    if PROTOTYPES
        ComxChannel  channel,
        LONGDOUBLE  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LONGDOUBLE  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 8) ;	/* Note 8-byte boundary for 16-byte value! */
        RETURN_IF_EOB (channel, 16) ;
        if (value != NULL) {
            octet  number[16] ;
            comxToHost (channel->isLE, 16, channel->current, number) ;
            if (channel->isLE) {
                (void) memcpy (value, &number[16 - sizeof (LONGDOUBLE)],
                               sizeof (LONGDOUBLE)) ;
            } else {
                (void) memcpy (value, number, sizeof (LONGDOUBLE)) ;
            }
        }
        channel->current += 16 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 8) ;	/* Note 8-byte boundary for 16-byte value! */
        MAKE_ROOM (channel, 16) ;
        memset (channel->current, 0, 16) ;
        if (channel->isLE) {
            (void) memmove (&channel->current[16 - sizeof (LONGDOUBLE)], value,
                            sizeof (LONGDOUBLE)) ;
        } else {
            (void) memmove (channel->current, value, sizeof (LONGDOUBLE)) ;
        }
        channel->current += 16 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxLongLong() - decodes/encodes a CDR long long (64-bit integer).

*******************************************************************************/


errno_t  comxLongLong (

#    if PROTOTYPES
        ComxChannel  channel,
        LONGLONG  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LONGLONG  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 8) ;
        RETURN_IF_EOB (channel, 8) ;
        if (value != NULL)  *value = GETLONGLONG (channel) ;
        channel->current += 8 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 8) ;
        MAKE_ROOM (channel, 8) ;
        PUTLONGLONG (channel, value) ;
        channel->current += 8 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxOctet() - decodes/encodes a CDR octet.

*******************************************************************************/


errno_t  comxOctet (

#    if PROTOTYPES
        ComxChannel  channel,
        octet  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        octet  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        RETURN_IF_EOB (channel, 1) ;
        if (value != NULL)  *value = (octet) GETBYTE (channel) ;
        channel->current++ ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        MAKE_ROOM (channel, 1) ;
        PUTBYTE (channel, value) ;
        channel->current++ ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxShort() - decodes/encodes a CDR short (16-bit integer).

*******************************************************************************/


errno_t  comxShort (

#    if PROTOTYPES
        ComxChannel  channel,
        short  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        short  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 2) ;
        RETURN_IF_EOB (channel, 2) ;
        if (value != NULL)  *value = GETSHORT (channel) ;
        channel->current += 2 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 2) ;
        MAKE_ROOM (channel, 2) ;
        PUTSHORT (channel, value) ;
        channel->current += 2 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxULong() - decodes/encodes a CDR unsigned long (unsigned 32-bit integer).

*******************************************************************************/


errno_t  comxULong (

#    if PROTOTYPES
        ComxChannel  channel,
        unsigned  long  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        unsigned  long  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 4) ;
        RETURN_IF_EOB (channel, 4) ;
        if (value != NULL)  *value = GETULONG (channel) ;
        channel->current += 4 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 4) ;
        MAKE_ROOM (channel, 4) ;
        PUTLONG (channel, value) ;
        channel->current += 4 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxULongLong() - decodes/encodes a CDR unsigned long long (unsigned
        64-bit integer).

*******************************************************************************/


errno_t  comxULongLong (

#    if PROTOTYPES
        ComxChannel  channel,
        ULONGLONG  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ULONGLONG  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 8) ;
        RETURN_IF_EOB (channel, 8) ;
        if (value != NULL)  *value = GETULONGLONG (channel) ;
        channel->current += 8 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 8) ;
        MAKE_ROOM (channel, 8) ;
        PUTLONGLONG (channel, value) ;
        channel->current += 8 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxUShort() - decodes/encodes a CDR unsigned short (unsigned 16-bit
        integer).

*******************************************************************************/


errno_t  comxUShort (

#    if PROTOTYPES
        ComxChannel  channel,
        unsigned  short  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        unsigned  short  *value ;
#    endif

{

    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        BALIGN (channel, 2) ;
        RETURN_IF_EOB (channel, 2) ;
        if (value != NULL)  *value = GETUSHORT (channel) ;
        channel->current += 2 ;
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        BALIGN (channel, 2) ;
        MAKE_ROOM (channel, 2) ;
        PUTSHORT (channel, value) ;
        channel->current += 2 ;
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

    comxWChar() - decodes/encodes a CDR wchar (wide character).  For
        GIOP versions 1.0 and 1.1, the Transmission Code Set (TCS-W)
        is assumed to be UTF-16: 16-bit characters transmitted as
        unsigned shorts.  For GIOP version 1.2 and later, the TCS-W
        is again assumed to be UTF-16, but each character is encoded
        in 3 octets: an initial octet specifying the width of the
        following character (i.e., 2) and two octets in big-endian
        order composing the wide character.  The byte-order marker
        (BOM) allowed in GIOP 1.2 is not currently recognized.

*******************************************************************************/


errno_t  comxWChar (

#    if PROTOTYPES
        ComxChannel  channel,
        wchar_t  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        wchar_t  *value ;
#    endif

{    /* Local variables. */
    uint8_t  length ;
    unsigned  short  i, number ;



    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {
    case MxDECODE:
        if (GIOP_VERSION_GE (channel->version, 1, 2)) {	/* Version 1.2 or later. */
            RETURN_IF_EOB (channel, 1) ;
            length = (uint8_t) *channel->current++ ;
            RETURN_IF_EOB (channel, length) ;
            /* Assume big-endian; need to check for byte-order marker (BOM). */
            if (value != NULL) {
                *value = 0 ;
                for (i = 0 ;  i < length ;  i++) {
                    *value = (*value << 8) | (channel->current[i] & 0x0FF) ;
                }
            }
            channel->current += length ;
        } else {					/* Versions 1.0, 1.1. */
            CHECK (comxUShort (channel, &number)) ;
            if (value != NULL)  *value = (wchar_t) number ;
        }
        return (0) ;
    case MxENCODE:
        RETURN_IF_NULL (value) ;
        MAKE_ROOM (channel, 4) ;
        if (GIOP_VERSION_GE (channel->version, 1, 2)) {	/* Version 1.2 or later. */
            *channel->current++ = 2 ;			/* 2 bytes, big-endian. */
            *channel->current++ = (*value >> 8) & 0x0FF ;
            *channel->current++ = *value & 0x0FF ;
        } else {					/* Versions 1.0, 1.1. */
            number = (unsigned short) *value ;
            CHECK (comxUShort (channel, &number)) ;
        }
        return (0) ;
    case MxERASE:
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

Procedures:

    comxArray ()
    comxEncapsule ()
    comxSequence ()
    comxString ()
    comxWString ()

    Decode/Encode/Erase Special GIOP Constructed Types.


Purpose:

    These functions decode, encode, and erase special cases of GIOP
    constructed types, either because of atypical calling sequences
    or for performance reasons.

*******************************************************************************/

/*!*****************************************************************************

Procedure:

    comxArray ()

    Decode/Encode/Erase GIOP Arrays.


Purpose:

    Function comxArray() encodes, decodes, and erases GIOP arrays of another
    GIOP data type or a CDR primitive type.  The GIOP representation of an
    array is simply the elements themselves, one after another:

        <element1>
        <element2>
        ...
        <elementN>

    The number of elements is assumed to be known by the sender and receiver.
    The host representation of an array is just an array of elements of the
    given data type.  When decoding an array, the elements are decoded into
    an existing array.  When erasing an array, the individual elements are
    erased, but the array itself remains.

    A marshaling function for the element type is passed to comxArray();
    this function decodes/encodes/erases the individual elements in the
    array.

    The element size passed to comxArray() is the size of the host
    representation of an array element; it is used by comxArray()
    to step through the elements in the array.


    Invocation:

        status = comxArray (channel, &value, marshalF, size, count) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <value>		- I/O
            is the address of the host array involved in the marshaling
            operation.  If the operation is MxDECODE, the data flow is
            from the CDR array (in the channel's buffer) to the host
            array.  If the operation is MxENCODE, the data flow is from
            the host array to the CDR array (in the channel's buffer).
            If the operation is MxERASE, dynamically allocated fields
            of individual elements in the host array are deallocated.
        <marshalF>	- I
            is the address of a comx<Type>() marshaling function for a
            CDR primitive type or a GIOP constructed type, not including
            comxArray() or comxSequence().
        <size>		- I
            is the sizeof() of the host representation of the individual
            elements in the array.
        <count>		- I
            is the number of elements in the array.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxArray (

#    if PROTOTYPES
        ComxChannel  channel,
        void  *value,
        ComxFunc  marshalF,
        size_t  size,
        unsigned  long  count)
#    else
        channel, value, marshalF, size, count)

        ComxChannel  channel ;
        void  *value ;
        ComxFunc  marshalF ;
        size_t  size ;
        unsigned  long  count ;
#    endif

{    /* Local variables. */
    char  *elements = (char *) value ;
    unsigned  long  i ;



    for (i = 0 ;  i < count ;  i++) {
        CHECK (marshalF (channel, &elements[i*size])) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    comxEncapsule ()

    Decode/Encode/Erase CDE Encapsulations.


Purpose:

    Function comxEncapsule() encodes, decodes, and erases CDR encapsulations.
    Data is encapsulated in an octet sequence.  An initial CDR boolean in the
    sequence indicates the byte ordering of the encoded data and is followed
    by the suitably-aligned encoded data.  NOTE that the boolean and the
    encoded data are aligned relative to the beginning of the octet sequence.

    The encapsulated data is specified as a series of marshaling-function/value
    arguments passed to comxEncapsule(), followed by a final NULL marshaling
    function.


    Invocation:

        status = comxEncapsule (version,
                                operation,
                                [marshalF1, value1,]
                                [marshalF2, value2,]
                                ...
                                NULL) ;

    where

        <version>	- I
            specifies the GIOP major/minor version numbers.
        <operation>	- I
            is the desired marshaling operation: MxDECODE, MxENCODE,
            or MxERASE.
        <marshalFn>, <valueN>	- I/O
            specify a marshaling function and value for each item of
            encapsulated data.  <marshalFn> is a pointer to a marshaling
            function of type ComxFunc (declared in "comx_util.h"); <valueN>
            is a (void *) pointer to the value.  A single NULL terminates
            the list of function/value pairs.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxEncapsule (

#    if PROTOTYPES
        Version  version,
        ComxOperation  operation,
        OctetSeq  *encapsulation,
        ...)
#    else
        version, operation, encapsulation, va_alist)

        Version  version ;
        ComxOperation  operation ;
        OctetSeq  *encapsulation ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    bool  byteOrder ;
    ComxChannel  mxchan ;
    ComxFunc  marshalF ;




    byteOrder = (__BYTE_ORDER == __LITTLE_ENDIAN) ;	/* Host little-endian? */


/*******************************************************************************
    MxDECODE - create a marshaling channel for the octet data supplied by
        the caller and decode the encapsulated data items from the channel.
*******************************************************************************/

    if (operation == MxDECODE) {

/* Create a marshaling channel for the octet data. */

        if (comxCreate (version, (encapsulation->elements[0] != 0), 0,
                        encapsulation->elements, encapsulation->count, &mxchan))
            return (errno) ;

/* Decode the encapsulated data items from the octet data. */

        if (comxBoolean (mxchan, &byteOrder)) {
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

#if HAVE_STDARG_H
        va_start (ap, encapsulation) ;
#else
        va_start (ap) ;
#endif

        while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
            if (marshalF (mxchan, va_arg (ap, void *))) {
                va_end (ap) ;
                PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
                return (errno) ;
            }
        }

        va_end (ap) ;

        comxDestroy (mxchan) ;

    }


/*******************************************************************************
    MxENCODE - create a marshaling channel for the encapsulated data, encode
        the encapsulated data items to the channel, and make an octet sequence
        from the channel's buffer.
*******************************************************************************/

    else if (operation == MxENCODE) {

        encapsulation->count = 0 ;
        encapsulation->elements = NULL ;

/* Create a marshaling channel to store the encoded encapsulated data. */

        if (comxCreate (version, byteOrder, 0, NULL, 0, &mxchan))
            return (errno) ;

/* Encode the encapsulated data items to the channel. */

        if (comxBoolean (mxchan, &byteOrder)) {
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

#if HAVE_STDARG_H
        va_start (ap, encapsulation) ;
#else
        va_start (ap) ;
#endif

        while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
            if (marshalF (mxchan, va_arg (ap, void *))) {
                va_end (ap) ;
                PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
                return (errno) ;
            }
        }

        va_end (ap) ;

/* Make an octet sequence from the channel's buffer. */

        encapsulation->count = comxSkip (mxchan, 0, 0) ;
		/* Encapsulation took possession of channels's buffer. */
        encapsulation->elements = comxBuffer (mxchan, 1) ;

        comxDestroy (mxchan) ;

    }


/*******************************************************************************
    MxERASE - erase each of the encapsulated data items.
*******************************************************************************/

    else if (operation == MxERASE) {

#if HAVE_STDARG_H
        va_start (ap, encapsulation) ;
#else
        va_start (ap) ;
#endif

        while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
            comxErase (marshalF, va_arg (ap, void *)) ;
        }

        va_end (ap) ;

    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    comxSequence ()

    Decode/Encode/Erase GIOP Sequences.


Purpose:

    Function comxSequence() encodes, decodes, and erases GIOP sequences of
    another GIOP data type or a CDR primitive type.  The GIOP representation
    of a sequence consists of a count of the number of elements in the
    sequence, followed by the elements themselves:

        <count>
        <element1>
        <element2>
        ...
        <elementN>

    The host representation of a sequence consists of a count of the number
    of elements in the sequence and a pointer to a dynamically-allocated
    array of the elements:

        <count>
        <elements>  ---->  <element1>
                           <element2>
                           ...
                           <elementN>

    When decoding a sequence, the array is automatically allocated.  When
    erasing a sequence, the array is deallocated, the count is set to zero,
    and the pointer is set to NULL.

    A marshaling function for the element type is passed to comxSequence();
    this function decodes/encodes/erases the individual elements in the
    sequence.

    The element size passed to comxSequence() is the size of the host
    representation of the element; it is used by comxSequence() when
    allocating the array of elements.  Since nested sequences in the
    element type are themselves represented by a two-field, count-and-pointer
    structure, the size of the host representation of a sequence is always
    fixed.


    Invocation:

        status = comxSequence (channel, &value, marshalF, size) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <value>		- I/O
            is the address of the host value involved in the marshaling
            operation.  If the operation is MxDECODE, the data flow is
            from the CDR value (in the channel's buffer) to the host
            value.  If the operation is MxENCODE, the data flow is from
            the host value to the CDR value (in the channel's buffer).
            If the operation is MxERASE, the memory allocated for the
            sequence in the host value is deallocated.
        <marshalF>	- I
            is the address of a comx<Type>() marshaling function for a
            CDR primitive type or a GIOP constructed type, not including
            comxArray() or comxSequence().
        <size>		- I
            is the sizeof() of the host representation of the individual
            elements in the sequence.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  comxSequence (

#    if PROTOTYPES
        ComxChannel  channel,
        void  *value,
        ComxFunc  marshalF,
        size_t  size)
#    else
        channel, value, marshalF, size)

        ComxChannel  channel ;
        void  *value ;
        ComxFunc  marshalF ;
        size_t  size ;
#    endif

{    /* Local variables. */
    char  *elements ;
    unsigned  long  count, i ;



    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {

    case MxDECODE:
        CHECK (comxULong (channel, &count)) ;
        elements = NULL ;
        if (value != NULL) {
            *((unsigned long *) value) = count ;
            value = (char *) value + sizeof (unsigned long) ;
            *((void **) value) = NULL ;
            if (count > 0) {
                elements = (char *) calloc (count, size) ;
                if (elements == NULL) {
                    LGE "(ComxSequence) Error allocating array[%lu] of %lu-byte elements.\ncalloc: ",
                        count, size) ;
                    return (errno) ;
                }
            }
        }
        for (i = 0 ;  i < count ;  i++) {
            CHECK (marshalF (channel,
                             (elements == NULL) ? NULL : &elements[i*size])) ;
        }
        if (value != NULL)  *((void **) value) = elements ;
        return (0) ;

    case MxENCODE:
        RETURN_IF_NULL (value) ;
        count = *((unsigned long *) value) ;
        value = (char *) value + sizeof (unsigned long) ;
        elements = (char *) *((void **) value) ;
        CHECK (comxULong (channel, &count)) ;
        for (i = 0 ;  i < count ;  i++) {
            CHECK (marshalF (channel, &elements[i*size])) ;
        }
        return (0) ;

    case MxERASE:
        RETURN_IF_NULL (value) ;
        count = *((unsigned long *) value) ;
        *((unsigned long *) value) = 0 ;
        value = (char *) value + sizeof (unsigned long) ;
        elements = (char *) *((void **) value) ;
        *((void **) value) = NULL ;
        for (i = 0 ;  i < count ;  i++) {
            marshalF (channel, &elements[i*size]) ;
        }
        if (elements != NULL)  free (elements) ;
        return (0) ;

    default:
        return (0) ;

    }

}

/*!*****************************************************************************

    comxString() - decode/encode/erase a GIOP string.  The string is
        represented on the host by a simple (char *) pointer to a
        dynamically-allocated string (when decoded).  Erasing the
        string deallocates the memory pointed to by the (char *)
        pointer and sets the pointer to NULL.

*******************************************************************************/


errno_t  comxString (

#    if PROTOTYPES
        ComxChannel  channel,
        char  **value)
#    else
        channel, value)

        ComxChannel  channel ;
        char  **value ;
#    endif

{    /* Local variables. */
    char  *s ;
    unsigned  long  length ;



    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {

    case MxDECODE:
        CHECK (comxULong (channel, &length)) ;
        RETURN_IF_EOB (channel, length) ;
        if (value != NULL) {
            if (length > 0) {
                s = strndup ((char *) channel->current, length) ;
                if (s == NULL) {
                    LGE "(ComxString) Error duplicating %lu-character string.\nstrndup: ",
                        length) ;
                    return (errno) ;
                }
            } else {
                s = NULL ;
            }
            *((char **) value) = s ;
        }
        channel->current += length ;
        return (0) ;

    case MxENCODE:
        RETURN_IF_NULL (value) ;
        s = *value ;
        length = (s == NULL) ? 0 : (strlen (s) + 1) ;
        CHECK (comxULong (channel, &length)) ;
        MAKE_ROOM (channel, length) ;
        if (length > 0)  (void) memcpy (channel->current, s, length) ;
        channel->current += length ;
        return (0) ;

    case MxERASE:
        RETURN_IF_NULL (value) ;
        s = *value ;
        *value = NULL ;
        if (s != NULL)  free (s) ;
        return (0) ;

    default:
        return (0) ;

    }

}

/*!*****************************************************************************

    comxWString() - decode/encode/erase a GIOP wstring (wide string).
        The string is represented on the host by a simple (wchar_t *)
        pointer to a NUL-terminated, dynamically-allocated wide string
        (when decoded).  Erasing the string deallocates the memory
        pointed to by the (wchar_t *) pointer and sets the pointer to NULL.
        For GIOP versions 1.0, 1.1, and 1.2, the Transmission Code Set (TCS-W)
        is assumed to be UTF-16: wide characters encoded in 16-bit characters
        and transmitted as unsigned shorts.  For GIOP versions 1.0 and 1.1,
        the length of the string is the number of wide characters in the
        string plus the terminating NUL character.  For GIOP version 1.2,
        the length of the string is the number of octets occupied by the
        string; the terminating NUL character is NOT included in the length
        and is NOT included in the transmitted string.

*******************************************************************************/


errno_t  comxWString (

#    if PROTOTYPES
        ComxChannel  channel,
        wchar_t  **value)
#    else
        channel, value)

        ComxChannel  channel ;
        wchar_t  **value ;
#    endif

{    /* Local variables. */
    bool  saveLE, surrogate ;
    unsigned  long  i, length, numSurrogates ;
    unsigned  short  number ;
    wchar_t  *ws ;




    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {

    case MxDECODE:
        saveLE = channel->isLE ;
        CHECK (comxULong (channel, &length)) ;
        if (GIOP_VERSION_GE (channel->version, 1, 2)) {	/* GIPO 1.2 or later. */
            length = length / 2 ;
            channel->isLE = false ;			/* Big-endian is default. */
        } else {					/* GIOP 1.0, 1.1 */
            if (length == 0) {
                if (value != NULL)  *value = NULL ;
                return (0) ;
            }
        }
        ws = (wchar_t *) calloc (length+1, sizeof (wchar_t)) ;
        if (ws == NULL) {
            LGE "(ComxWString) Error allocating %lu-character wide string.\ncalloc: ",
                length) ;
            channel->isLE = saveLE ;
            return (errno) ;
        }
        i = 0 ;  surrogate = false ;
        while (length-- > 0) {
            if (comxUShort (channel, &number)) {
                channel->isLE = saveLE ;
                PUSH_ERRNO ;  free (ws) ;  POP_ERRNO ;
                return (errno) ;
            }
            if (i == 0) {
                if (number == 0xFEFF) {			/* Big-endian BOM? */
                    channel->isLE = false ;  continue ;
                } else if (number == 0xFFFE) {		/* Little-endian BOM? */
                    channel->isLE = true ;  continue ;
                }
            }
            if (surrogate) {	/* Second "character" in surrogate pair? */
                if ((number < 0xDC00) || (0xDFFF < number))  break ;
                ws[i++] |= (wchar_t) (number & 0x03FF) ;
                surrogate = false ;
            } else if ((number < 0xD800) || (0xDFFF < number)) {
                ws[i++] = (wchar_t) number ;
            } else {		/* First "character" in surrogate pair. */
                ws[i] = ((wchar_t) ((number & 0x03FF) | 0x0400)) << 10 ;
                surrogate = true ;
            }
        }
        channel->isLE = saveLE ;
        if (surrogate) {
            SET_ERRNO (EINVAL) ;
            LGE "(ComxWString) Invalid or truncated UTF-16 surrogate pair: ") ;
            PUSH_ERRNO ;  free (ws) ;  POP_ERRNO ;
            return (errno) ;
        }
        ws[i] = (wchar_t) 0 ;			/* Redundant in 1.0, 1.1 */
        if (value == NULL)
            free (ws) ;
        else
            *value = ws ;
        return (0) ;

    case MxENCODE:
        RETURN_IF_NULL (value) ;
        saveLE = channel->isLE ;
        ws = *value ;
        length = (ws == NULL) ? 0 : wcslen (ws) ;
		/* Count the number of wide characters that require
		   two UTF-16 characters for encoding. */
        for (i = 0, numSurrogates = 0 ;  i < length ;  i++) {
            if ((unsigned long) ws[i] > 0x0FFFF)  numSurrogates++ ;
        }
        length += numSurrogates ;
        if (GIOP_VERSION_GE (channel->version, 1, 2)) {	/* GIOP 1.2 or later. */
		/* Length in bytes with no NUL terminator; force big-endian
		   (CDR default) encoding of individual wide characters. */
            length = length * sizeof (unsigned short) ;
            CHECK (comxULong (channel, &length)) ;
            length = length / sizeof (unsigned short) ;
            channel->isLE = false ;
        } else {					/* GIOP 1.0, 1.1 */
		/* Length in wide characters with NUL terminator;
		   little-/big-endian follows message encoding. */
            if (ws != NULL)  length++ ;
            CHECK (comxULong (channel, &length)) ;
        }
        length -= numSurrogates ;
        for (i = 0 ;  i < length ;  i++) {
            if ((unsigned long) ws[i] > 0x0FFFF) {	/* Surrogate pair? */
                number = (unsigned short) ((ws[i] >> 10) & 0x03FF) | 0x0D800 ;
                if (comxUShort (channel, &number)) {
                    channel->isLE = saveLE ;
                    return (errno) ;
                }
                number = (unsigned short) (ws[i] & 0x03FF) | 0x0DC00 ;
            } else {
                number = (unsigned short) ws[i] ;
            }
            if (comxUShort (channel, &number)) {
                channel->isLE = saveLE ;
                return (errno) ;
            }
        }
        channel->isLE = saveLE ;
        return (0) ;

    case MxERASE:
        RETURN_IF_NULL (value) ;
        ws = *value ;
        *value = NULL ;
        if (ws != NULL)  free (ws) ;
        return (0) ;

    default:
        return (0) ;

    }

}

/*!*****************************************************************************

Procedures:

    comxOctetSeq ()
    comxVersion ()

    comxDoubleSeq ()
    comxEnumSeq ()
    comxFloatSeq ()
    comxLongSeq ()
    comxLongLongSeq ()
    comxShortSeq ()
    comxStringSeq ()
    comxULongSeq ()
    comxULongLongSeq ()
    comxUShortSeq ()
    comxWStringSeq ()

    Decode/Encode/Erase GIOP Constructed Types.


Purpose:

    These functions decode, encode, and erase GIOP constructed types,
    which are ultimately broken down into CDR primitive types.  As such,
    these functions largely depend on the COMX primitive functions for
    sorting out the marshaling direction, maintaining alignment, and
    checking for errors.


    Invocation:

        status = comx<Type> (channel, &value) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <value>		- I/O
            is the address of the host value involved in the marshaling
            operation.  If the operation is MxDECODE, the data flow is
            from the CDR value (in the channel's buffer) to the host
            value.  If the operation is MxENCODE, the data flow is from
            the host value to the CDR value (in the channel's buffer).
            If the operation is MxERASE, the dynamically-allocated fields
            of the host value are deallocated.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/

/*!*****************************************************************************

    comxOctetSeq() - decode/encode/erase a GIOP octet sequence.

*******************************************************************************/


errno_t  comxOctetSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        OctetSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        OctetSeq  *value ;
#    endif

{    /* Local variables. */
    unsigned  long  count ;



    RETURN_IF_NULL (channel) ;

    switch (channel->operation) {

    case MxDECODE:
        CHECK (comxULong (channel, &count)) ;
        RETURN_IF_EOB (channel, count) ;
        if (value != NULL) {
            value->count = count ;
            if (count > 0) {
                value->elements = (octet *) memdup (channel->current, count) ;
                if (value->elements == NULL) {
                    LGE "(ComxOctetSeq) Error duplicating %lu-byte sequence.\nmemdup: ",
                        count) ;
                    return (errno) ;
                }
            } else {
                value->elements = NULL ;
            }
        }
        channel->current += count ;
        return (0) ;

    case MxENCODE:
        RETURN_IF_NULL (value) ;
        CHECK (comxULong (channel, &value->count)) ;
        MAKE_ROOM (channel, value->count) ;
        if (value->count > 0)
            (void) memcpy (channel->current, value->elements, value->count) ;
        channel->current += value->count ;
        return (0) ;

    case MxERASE:
        RETURN_IF_NULL (value) ;
        if ((value->count > 0) && (value->elements != NULL))
            free (value->elements) ;
        value->count = 0 ;
        value->elements = NULL ;
        return (0) ;

    default:
        return (0) ;

    }

}

/*!*****************************************************************************

    comxVersion() - decode/encode/erase a GIOP version (which contains
        the major and minor version numbers).

*******************************************************************************/


errno_t  comxVersion (

#    if PROTOTYPES
        ComxChannel  channel,
        Version  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Version  *value ;
#    endif

{

    CHECK (comxOctet (channel, NULL_OR (value, major))) ;
    CHECK (comxOctet (channel, NULL_OR (value, minor))) ;

    return (0) ;

}

/*!*****************************************************************************

    comxBooleanSeq() - decode/encode/erase a sequence of CDR booleans.

*******************************************************************************/


errno_t  comxBooleanSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        BooleanSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        BooleanSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxBoolean, sizeof (bool))) ;

}

/*!*****************************************************************************

    comxCharSeq() - decode/encode/erase a sequence of CDR characters.

*******************************************************************************/


errno_t  comxCharSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        CharSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        CharSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxChar, sizeof (char))) ;

}

/*!*****************************************************************************

    comxDoubleSeq() - decode/encode/erase a sequence of CDR doubles.

*******************************************************************************/


errno_t  comxDoubleSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        DoubleSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DoubleSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxDouble, sizeof (double))) ;

}

/*!*****************************************************************************

    comxEnumSeq() - decode/encode/erase a sequence of CDR enumerations.

*******************************************************************************/


errno_t  comxEnumSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        EnumSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        EnumSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxEnum,
                          sizeof (unsigned long))) ;

}

/*!*****************************************************************************

    comxFloatSeq() - decode/encode/erase a sequence of CDR floats.

*******************************************************************************/


errno_t  comxFloatSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        FloatSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        FloatSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxFloat, sizeof (float))) ;

}

/*!*****************************************************************************

    comxLongSeq() - decode/encode/erase a sequence of CDR longs.

*******************************************************************************/


errno_t  comxLongSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        LongSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LongSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxLong, sizeof (long))) ;

}

/*!*****************************************************************************

    comxLongLongSeq() - decode/encode/erase a sequence of CDR long longs.

*******************************************************************************/


errno_t  comxLongLongSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        LongLongSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LongLongSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxLongLong, sizeof (LONGLONG))) ;

}

/*!*****************************************************************************

    comxShortSeq() - decode/encode/erase a sequence of CDR shorts.

*******************************************************************************/


errno_t  comxShortSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ShortSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ShortSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxShort,
                          sizeof (short))) ;

}

/*!*****************************************************************************

    comxStringSeq() - decode/encode/erase a sequence of CDR strings.

*******************************************************************************/


errno_t  comxStringSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        StringSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        StringSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxString, sizeof (char *))) ;

}

/*!*****************************************************************************

    comxULongSeq() - decode/encode/erase a sequence of CDR unsigned longs.

*******************************************************************************/


errno_t  comxULongSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ULongSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ULongSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxULong,
                          sizeof (unsigned long))) ;

}

/*!*****************************************************************************

    comxULongLongSeq() - decode/encode/erase a sequence of CDR
        unsigned long longs.

*******************************************************************************/


errno_t  comxULongLongSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ULongLongSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ULongLongSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxULongLong, sizeof (ULONGLONG))) ;

}

/*!*****************************************************************************

    comxUShortSeq() - decode/encode/erase a sequence of CDR unsigned shorts.

*******************************************************************************/


errno_t  comxUShortSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        UShortSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        UShortSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxUShort,
                          sizeof (unsigned short))) ;

}

/*!*****************************************************************************

    comxWCharSeq() - decode/encode/erase a sequence of CDR wide characters.

*******************************************************************************/


errno_t  comxWCharSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        WCharSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        WCharSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxWChar,
                          sizeof (wchar_t))) ;

}

/*!*****************************************************************************

    comxWStringSeq() - decode/encode/erase a sequence of CDR wide strings.

*******************************************************************************/


errno_t  comxWStringSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        WStringSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        WStringSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) comxWString, sizeof (wchar_t *))) ;

}


#if HAVE_NAMESPACES
    } ;     /* CoLi namespace. */
#endif
