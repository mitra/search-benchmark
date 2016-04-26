/* $Id: ddmx_util.c,v 1.1 2004/07/15 18:43:41 alex Exp $ */
/*******************************************************************************

File:

    ddmx_util.c

    Data Distribution Service for Real-Time Systems (DDS) Marshaling Utilities.


Author:    Alex Measday


Purpose:

    The DDMX utilities are used to convert various Data Distribution Service
    (DDS) data types to and from the Common Data Representation (CDR) encodings
    defined for the General Inter-ORB Protocol (GIOP).  (The primitive CDR
    types are handled by the COMX utilities.)


Public Procedures:

    ddmx<Type>() - decode/encode/erase DDS types.
    .
    .
    .

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  "ddmx_util.h"			/* DDS marshaling utilities. */


#if HAVE_NAMESPACES
    namespace  DdsCoLi {
#endif

/*******************************************************************************

Procedures:

    ddmx<Type> ()

    Decode/Encode/Erase DDS Data Types.


Purpose:

    These automatically generated functions decode, encode, and erase
    DDS data types.


    Invocation:

        status = ddmx<Type> (channel, &value) ;

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

#include  "ddmx_idl.c"			/* Auto-generated marshaling functions. */


#if HAVE_NAMESPACES
    } ;     /* DdsCoLi namespace. */
#endif
