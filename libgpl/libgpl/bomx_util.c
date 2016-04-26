/* $Id: bomx_util.c,v 1.1 2009/09/23 13:37:41 alex Exp $ */
/*******************************************************************************

File:

    bomx_util.c

    GNOME Bonobo 2.0 Marshaling Utilities.


Author:    Alex Measday


Purpose:

    The BOMX utilities are used to convert various GNOME Bonobo data types
    to and from the Common Data Representation (CDR) encodings defined for
    the General Inter-ORB Protocol (GIOP).  (The primitive CDR types are
    handled by the COMX utilities.)


Public Procedures:

    bomx<Type>() - decode/encode/erase BOMX types.
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
#include  "bomx_util.h"			/* GNOME Bonobo marshaling utilities. */


#if HAVE_NAMESPACES
    namespace  BoCoLi {
#endif

/*******************************************************************************

Procedures:

    bomx<Type> ()

    Decode/Encode/Erase GNOME Bonobo Data Types.


Purpose:

    These automatically generated functions decode, encode, and erase
    GNOME Bonobo data types.


    Invocation:

        status = bomx<Type> (channel, &value) ;

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

#include  "bomx_idl.c"			/* Auto-generated marshaling functions. */

/*******************************************************************************
    Lookup Tables - for converting named constants to numbers and vice-versa.
*******************************************************************************/

					/* Bonobo_Activation_types.idl */
const  ColiMap  ActivationFlagsLUT[] = {
  { (long) ACTIVATION_FLAG_NO_LOCAL, "ACTIVATION_FLAG_NO_LOCAL" },
  { (long) ACTIVATION_FLAG_PRIVATE, "ACTIVATION_FLAG_PRIVATE" },
  { (long) ACTIVATION_FLAG_EXISTING_ONLY, "ACTIVATION_FLAG_EXISTING_ONLY" },
  { 0L, NULL }
} ;

					/* Bonobo_Activation_types.idl */
const  ColiMap  RegistrationFlagsLUT[] = {
  { (long) REGISTRATION_FLAG_NO_SERVERINFO, "REGISTRATION_FLAG_NO_SERVERINFO" },
  { 0L, NULL }
} ;

					/* Bonobo_Activation_types.idl */
const  ColiMap  ActivationEnvValueLUT[] = {
  { (long) ACTIVATION_ENV_FLAG_UNSET, "ACTIVATION_ENV_FLAG_UNSET" },
  { 0L, NULL }
} ;

					/* Bonobo_Moniker.idl */
const  ColiMap  ResolveFlagLUT[] = {
  { (long) MONIKER_ALLOW_USER_INTERACTION, "MONIKER_ALLOW_USER_INTERACTION" },
  { 0L, NULL }
} ;

					/* Bonobo_Storage.idl */
const  ColiMap  StorageInfoFieldsLUT[] = {
  { (long) FIELD_CONTENT_TYPE, "FIELD_CONTENT_TYPE" },
  { (long) FIELD_SIZE, "FIELD_SIZE" },
  { (long) FIELD_TYPE, "FIELD_TYPE" },
  { 0L, NULL }
} ;

					/* Bonobo_Storage.idl */
const  ColiMap  OpenModeLUT[] = {
  { (long) BONOBO_READ, "BONOBO_READ" },
  { (long) BONOBO_WRITE, "BONOBO_WRITE" },
  { (long) BONOBO_CREATE, "BONOBO_CREATE" },
  { (long) BONOBO_FAILIFEXIST, "BONOBO_FAILIFEXIST" },
  { (long) BONOBO_COMPRESSED, "BONOBO_COMPRESSED" },
  { (long) BONOBO_TRANSACTED, "BONOBO_TRANSACTED" },
  { 0L, NULL }
} ;

					/* Bonobo_Property.idl */
const  ColiMap  PropertyFlagsLUT[] = {
  { (long) PROPERTY_READABLE, "PROPERTY_READABLE" },
  { (long) PROPERTY_WRITEABLE, "PROPERTY_WRITEABLE" },
  { (long) PROPERTY_NO_LISTENING, "PROPERTY_NO_LISTENING" },
  { (long) PROPERTY_NO_AUTONOTIFY, "PROPERTY_NO_AUTONOTIFY" },
  { (long) PROPERTY_NO_PERSIST, "PROPERTY_NO_PERSIST" },
  { 0L, NULL }
} ;

					/* Bonobo_Canvas.idl */
const  ColiMap  BufFlagsLUT[] = {
  { (long) IS_BG, "IS_BG" },
  { (long) IS_BUF, "IS_BUF" },
  { 0L, NULL }
} ;


#if HAVE_NAMESPACES
    } ;     /* BoCoLi namespace. */
#endif
