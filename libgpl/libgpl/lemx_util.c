/* $Id: lemx_util.c,v 1.2 2004/02/16 20:52:07 alex Exp $ */
/*******************************************************************************

File:

    lemx_util.c

    Laboratory Equipment Control Interface Specification (LECIS)
    Marshaling Utilities.


Author:    Alex Measday


Purpose:

    The LEMX utilities are used to convert various Laboratory Equipment
    Control Interface Specification (LECIS) data types to and from the
    Common Data Representation (CDR) encodings defined for the General
    Inter-ORB Protocol (GIOP).  (The primitive CDR types are handled by
    the COMX utilities.)


Public Procedures:

    lemx<Type>() - decode/encode/erase LECIS types.
    .
    .
    .

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <wchar.h>			/* C Library wide string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "lemx_util.h"			/* LECIS marshaling utilities. */


#if HAVE_NAMESPACES
    namespace  LecisCoLi {
#endif

/*******************************************************************************

Procedures:

    lemx<Type> ()

    Decode/Encode/Erase LECIS Data Types.


Purpose:

    These automatically generated functions decode, encode, and erase
    LECIS data types.


    Invocation:

        status = lemx<Type> (channel, &value) ;

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


/*******************************************************************************
    Auto-generated marshaling functions - generated from the auto-generated
        header file.
*******************************************************************************/

#include  "lemx_idl.c"			/* Auto-generated marshaling functions. */


#if HAVE_NAMESPACES
    } ;     /* LecisCoLi namespace. */
#endif
