/* $Id: coli_util.c,v 1.16 2011/07/18 17:53:03 alex Exp alex $ */
/*******************************************************************************

File:

    coli_util.c

    CORBA-Lite Utilities.


Author:    Alex Measday


Purpose:

    CORBA-Lite is a lightweight CORBA implementation consisting of:

        COMX_UTIL - marshaling functions for the basic Common Data
            Representation (CDR) primitive and constructed data types.

        GIMX_UTIL - marshalling functions for some of the GIOP data types.

        IIOP_UTIL - networking functions for sending and receiving
            GIOP messages over IIOP (TCP/IP) streams.

        COLI_UTIL - higher-level functions for performing CORBA message
            transactions.

    More detailed information about the other packages is found in the
    respective package prologs.

    CORBA-Lite does not support the CORBA C binding.  It provides a simple,
    low-level means of communicating with CORBA peers and is useful for
    writing test/debug clients and servers.


    COLI Functions
    ==============

    Once an IIOP connection is established, a client application can use
    the following two functions for basic communications with the service:

        coliRequest() - submits a request for an operation on a service-hosted
            object.  A list of marshaling function/value pairs is supplied for
            arguments expected by the operation.

        coliGetReply() - reads a reply from the service.  If the operation was
            successful, a list of marshaling function/value pairs is used to
            decode and store the return values and function result returned
            by the operation.

    A server application, on the other hand, would use these functions:

        coliGetRequest() - reads a request.  If the target object and operation
            in the request match the caller-supplied object and operation, a
            list of marshaling function/value pairs is used to decode and store
            the arguments expected by the operation.

        coliReply() - returns the completion status for a request.  A list of
            marshaling function/value pairs is supplied for the return values
            expected from the operation.

    The COLI package also includes routines for working with Interoperable
    Object References (IORs).  Functions are available for constructing IORs,
    converting between binary and "stringified" IORs, and converting between
    IORs and "corbaloc:" URLs.

    Finally, two table-lookup functions can be used (with the appropriate
    tables) to map enumerated types to their ASCII names and vice-versa.
    These capabilities are useful for displaying enumeration fields in
    human-readable form and, particularly in the case of application- or
    project-specific enumerations, for translating human input of field
    values by name into their binary enumeration values.


Public Procedures:

    coliGetReply() - gets the next reply from a CORBA server.
    coliGetRequest() - gets the next request from a CORBA client.
    coliMakeIOR() - make an IOR for an object.
    coliO2S() - converts an IOR to a stringified reference.
    coliO2URL() - converts an IOR to a URL.
    coliOpenIOR() - opens an IIOP stream for an IOR.
    coliProfile() - returns an IOR's TAG_INTERNET_IOP profile.
    coliReply() - issues a reply to a prior CORBA request.
    coliRequest() - issues a request to a CORBA server object.
    coliS2O() - converts a stringified reference to an IOR.
    coliS2URL() - converts a stringified reference to a URL.
    coliToName() - maps a number (e.g., CORBA enumeration) to a name.
    coliToNumber() - maps a name to a number (e.g., CORBA enumeration).
    coliURL2O() - converts a URL to an IOR.
    coliVersion() - gets/sets the GIOP version.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "net_util.h"			/* Networking utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "coli_util.h"			/* CORBA-Lite utilities. */


int  coli_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  coli_util_debug

#define  GIOP_VERSION  "1.2"		/* Default GIOP version number. */

static  Version  coli_version =		/* GIOP version number to use. */
    { 0, 0 } ;

/*!*****************************************************************************

Procedure:

    coliGetReply ()

    Get the Next Reply from a CORBA Server.


Purpose:

    Function coliGetReply() reads and decodes the next reply message from
    a CORBA server.


    Invocation:

        status = coliGetReply (stream, &replyStatus, &exception,
                               [marshalF1, &value1,]
                               [marshalF2, &value2,]
                               ...
                               NULL) ;

    where

        <stream>		- I
            is the IIOP stream to the CORBA server.
        <replyStatus>		- O
            returns the status field from the GIOP reply header; see the
            ReplyStatusType enumeration in "gimx_util.h" for the possible
            values.
        <exception>		- O
            returns the system exception structure if the reply status was
            SYSTEM_EXCEPTION.
        <marshalFn>, <valueN>	- I
            specify a marshaling function and return value for each item
            of information in the message body returned by the server.
            <marshalFn> is a pointer to a marshaling function of type
            ComxFunc (declared in "comx_util.h"); <valueN> is a (void *)
            pointer to the location where the decoded value will be returned.
            A single NULL terminates the list of function/value pairs.
        <status>		- O
            returns the status of receiving and decoding the reply message,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliGetReply (

#    if PROTOTYPES
        IiopStream  stream,
        ReplyStatusType  *replyStatus,
        SystemExceptionReplyBody  *exception,
        ...)
#    else
        stream, replyStatus, exception, va_alist)

        IiopStream  stream ;
        ReplyStatusType  *replyStatus ;
        SystemExceptionReplyBody  *exception ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    ComxChannel  mxchan ;
    ComxFunc  marshalF ;
    IiopHeader  header ;
    octet  *body ;




/* Read the next message from the server. */

    if (iiopRead (stream, -1.0, &header, &body)) {
        LGE "(coliGetReply) Error reading reply from %s.\niiopRead: ",
            iiopName (stream)) ;
        return (errno) ;
    }

/* Decode the reply header. */

    comxCreate (header.GIOP_version, (header.flags & ENDIAN_MASK) != 0, 12,
                (octet *) body, header.message_size, &mxchan) ;

    if (GIOP_VERSION_GE (header.GIOP_version, 1, 2)) {

        ReplyHeader  rphdr ;				/* GIOP 1.2 and later */

        if (gimxReplyHeader (mxchan, &rphdr)) {
            LGE "(coliGetReply) Error decoding reply from %s.\ngimxReplyHeader: ",
                iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

        comxSkip (mxchan, 0, 8) ;	/* 8-byte alignment following header
					   in GIOP 1.2 and later. */
        *replyStatus = rphdr.reply_status ;

    } else {

        ReplyHeader_1_0  rphdr ;			/* GIOP 1.0, 1.1 */

        if (gimxReplyHeader_1_0 (mxchan, &rphdr)) {
            LGE "(coliGetReply) Error decoding reply from %s.\ngimxReplyHeader_1_0: ",
                iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

        *replyStatus = rphdr.reply_status ;

    }

#ifdef __palmos__
    if (I_DEFAULT_GUARD) {
        HostFPrintF (HostLogFile (),
                     "(coliGetReply) Received %ld reply from %s.\n",
                     (long) *replyStatus, iiopName (stream)) ;
    }
#else
    LGI "(coliGetReply) Received %s reply from %s.\n",
        coliToName (ReplyStatusTypeLUT, (long) *replyStatus),
        iiopName (stream)) ;
#endif

/* Check the reply status for errors. */

    if (*replyStatus != NO_EXCEPTION) {
        if ((*replyStatus == SYSTEM_EXCEPTION) && (exception != NULL)) {
            gimxSystemExceptionReplyBody (mxchan, exception) ;
        }
        SET_ERRNO (EINVAL) ;
        LGE "(coliGetReply) %s reply from %s.\n",
            coliToName (ReplyStatusTypeLUT, (long) *replyStatus),
            iiopName (stream)) ;
        PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Decode and return the information from the server. */

#if HAVE_STDARG_H
    va_start (ap, exception) ;
#else
    va_start (ap) ;
#endif

    while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
        if (marshalF (mxchan, va_arg (ap, void *))) {
            LGE "(coliGetReply) Error decoding argument from %s.\n",
                iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  va_end (ap) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

    va_end (ap) ;

    comxDestroy (mxchan) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    coliGetRequest ()

    Get the Next Request from a CORBA Client.


Purpose:

    Function coliGetRequest() reads and decodes the next request message from
    a CORBA client.


    Invocation:

        status = coliGetRequest (stream, object, operation,
                                 &header, &body, &request,
                                 [marshalF1, &value1,]
                                 [marshalF2, &value2,]
                                 ...
                                 NULL) ;

    where

        <stream>		- I
            is the IIOP stream to the CORBA server.
        <object>		- I
            is the expected target of the request.  If the incoming request
            specifies a different object (or operation), the marshaling
            functions are NOT applied to the message body.
        <operation>		- I
            is the operation expected to be requested.  If the incoming
            request specifies a different operation (or object), the
            marshaling functions are NOT applied to the message body.
        <header>		- O
            returns the message header.  The numeric fields in the header
            are in host-byte order, as opposed to the network-byte order
            in which they were transferred over the network.
        <body>			- O
            returns a pointer to the message body; NULL is returned if the
            message has no body.  The message body must be used or duplicated
            before calling other functions that attempt to read from the IIOP
            stream.
        <request>		- O
            returns the GIOP request header.  The header is returned in a
            RequestHeader structure (see "gimx_util.h"), regardless of the
            message's GIOP version.  If an older request header is received,
            its fields are transferred to the RequestHeader structure.
        <marshalFn>, <valueN>	- I
            specify a marshaling function and return value for each item
            of information in the request body returned by the server.
            <marshalFn> is a pointer to a marshaling function of type
            ComxFunc (declared in "comx_util.h"); <valueN> is a (void *)
            pointer to the location where the decoded value will be returned.
            A single NULL terminates the list of function/value pairs.  The
            marshaling functions are only applied if the target object and
            operation in the request match the caller-supplied arguments.
        <status>		- O
            returns the status of receiving and decoding the request message,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliGetRequest (

#    if PROTOTYPES
        IiopStream  stream,
        ObjectKey  object,
        const  char  *operation,
        IiopHeader  *header,
        octet  **body,
        RequestHeader  *request,
        ...)
#    else
        stream, object, operation, header, body, request, va_alist)

        IiopStream  stream ;
        ObjectKey  object ;
        const  char  *operation ;
        IiopHeader  *header ;
        octet  **body ;
        RequestHeader  *request ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    ComxChannel  mxchan ;
    ComxFunc  marshalF ;




/* Read the next message from the server. */

    if (iiopRead (stream, -1.0, header, body)) {
        LGE "(coliGetRequest) Error reading request from %s.\niiopRead: ",
            iiopName (stream)) ;
        return (errno) ;
    }

/* Decode the reply header. */

    comxCreate (header->GIOP_version, (header->flags & ENDIAN_MASK) != 0, 12,
                *body, header->message_size, &mxchan) ;

    if (GIOP_VERSION_GE (header->GIOP_version, 1, 2)) {

        RequestHeader  rqhdr ;				/* GIOP 1.2 and later */

        if (gimxRequestHeader (mxchan, &rqhdr)) {
            LGE "(coliGetRequest) Error decoding reply from %s.\ngimxRequestHeader: ",
                iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

        comxSkip (mxchan, 0, 8) ;	/* 8-byte alignment following header
					   in GIOP 1.2 and later. */
        *request = rqhdr ;

    } else if (GIOP_VERSION_GE (header->GIOP_version, 1, 1)) {

        RequestHeader_1_1  rqhdr ;			/* GIOP 1.1 */

        if (gimxRequestHeader_1_1 (mxchan, &rqhdr)) {
            LGE "(coliGetRequest) Error decoding reply from %s.\ngimxRequestHeader_1_1: ",
                iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

        request->request_id = rqhdr.request_id ;
        request->response_flags =
            rqhdr.response_expected ? MESSAGING_SYNC_WITH_TARGET
                                    : MESSAGING_SYNC_NONE ;
        request->operation = rqhdr.operation ;
        request->target.which = KeyAddr ;
        request->target.data.object_key = rqhdr.object_key ;
        request->service_context.count = 0 ;
        request->service_context.elements = NULL ;

        rqhdr.operation = NULL ;
        rqhdr.object_key.count = 0 ;
        rqhdr.object_key.elements = NULL ;
        comxErase ((ComxFunc) gimxRequestHeader_1_1, &rqhdr) ;

    } else {

        RequestHeader_1_0  rqhdr ;			/* GIOP 1.0 */

        if (gimxRequestHeader_1_0 (mxchan, &rqhdr)) {
            LGE "(coliGetRequest) Error decoding reply from %s.\ngimxRequestHeader_1_0: ",
                iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
            return (errno) ;
        }

        request->request_id = rqhdr.request_id ;
        request->response_flags =
            rqhdr.response_expected ? MESSAGING_SYNC_WITH_TARGET
                                    : MESSAGING_SYNC_NONE ;
        request->operation = rqhdr.operation ;
        request->target.which = KeyAddr ;
        request->target.data.object_key = rqhdr.object_key ;
        request->service_context.count = 0 ;
        request->service_context.elements = NULL ;

        rqhdr.operation = NULL ;
        rqhdr.object_key.count = 0 ;
        rqhdr.object_key.elements = NULL ;
        comxErase ((ComxFunc) gimxRequestHeader_1_0, &rqhdr) ;

    }

    LGI "(coliGetRequest) Received %s request %lu from %s.\n",
        request->operation, request->request_id, iiopName (stream)) ;

/* If the target object and operation are the expected ones, then decode the
   arguments from the request body.  Otherwise, simply return the raw message
   body. */

    if ((object.count > 0)  &&
        ((request->target.data.object_key.count != object.count) ||
         (memcmp (request->target.data.object_key.elements,
                  object.elements, object.count) != 0))) {
        comxDestroy (mxchan) ;
        return (0) ;				/* Unexpected target object. */
    }

    if ((operation != NULL) && (strcmp (operation, request->operation) != 0)) {
        comxDestroy (mxchan) ;
        return (0) ;				/* Unexpected operation. */
    }

#if HAVE_STDARG_H
    va_start (ap, request) ;
#else
    va_start (ap) ;
#endif

    while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
        if (marshalF (mxchan, va_arg (ap, void *))) {
            LGE "(coliGetRequest) Error decoding argument from %s.\n",
                iiopName (stream)) ;
            PUSH_ERRNO ;
            comxDestroy (mxchan) ;
            comxErase ((ComxFunc) gimxRequestHeader, request) ;
            va_end (ap) ;
            POP_ERRNO ;
            return (errno) ;
        }
    }

    va_end (ap) ;

    comxDestroy (mxchan) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    coliMakeIOR ()

    Make an IOR for an Object.


Purpose:

    The coliMakeIOR() function makes an Interoperable Object Reference (IOR)
    for an object.

    The IOR's TAG_INTERNET_IOP profile includes a TAG_CODE_SETS component
    specifying "ISO 8859-1:1987; Latin Alphabet No. 1" as the character
    Transmission Code Set (TCS) and "ISO/IEC 10646-1:1993; UTF-16, UCS
    Transformation Format 16-bit form" as the wide-character TCS (TCS-W).
    These code sets are compatible with the COMX marshaling functions
    for characters, strings, wide characters, and wide strings.


    Invocation:

        status = coliMakeIOR (object, host, port, version, typeID, &ior) ;

    where

        <object>	- I
            is the application's object key (represented as a sequence of
            octets).  A copy is made of the octet sequence for insertion
            in the IOR.
        <host>		- I
            is the computer hosting the object.  The local host is used if
            this argument is NULL.
        <port>		- I
            is the TCP/IP port number at which the object's server is
            listening for requests.
        <version>	- I
            is the IIOP version number for the IOR.
        <typeID>	- I
            is an optional typeID for the object.
        <ior>		- O
            returns the IOR for the object.  The dynamically allocated fields
            of the IOR can be deallocated using the gimxIOR() function in
            erase mode or using the comxErase() function.
        <status>	- O
            returns the status of making the IOR, zero if there were no errors
            and ERRNO otherwise.

*******************************************************************************/


errno_t  coliMakeIOR (

#    if PROTOTYPES
        ObjectKey  object,
        const  char  *host,
        unsigned  short  port,
        Version  version,
        const  char  *typeID,
        IOR  *ior)
#    else
        object, host, port, version, typeID, ior)

        ObjectKey  object ;
        char  *host ;
        unsigned  short  port ;
        Version  version ;
        char  *typeID ;
        IOR  *ior ;
#    endif

{    /* Local variables. */
    CodeSetComponentInfo  codeSets ;
    ProfileBody  *profile ;
    TaggedComponent  *component ;




/* Initialize the base IOR structure. */

    ior->type_id = NULL ;
    ior->profiles.count = 0 ;
    ior->profiles.elements = NULL ;

    if (typeID != NULL) {
        ior->type_id = strdup (typeID) ;
        if (ior->type_id == NULL) {
            LGE "(coliMakeIOR) Error duplicating type ID: \"%s\"\nstrdup: ",
                typeID) ;
            return (errno) ;
        }
    }

/* Create a TAG_INTERNET_IOP profile specifying the host and port of the
   object's server. */

    ior->profiles.elements = (TaggedProfile *) calloc (1,
                                                       sizeof (TaggedProfile)) ;
    if (ior->profiles.elements == NULL) {
        LGE "(coliMakeIOR) Error allocating tagged profile.\ncalloc: ") ;
        PUSH_ERRNO ;  comxErase ((ComxFunc) gimxIOR, ior) ;  POP_ERRNO ;
        return (errno) ;
    }
    ior->profiles.count = 1 ;

    ior->profiles.elements[0].which = IOP_TAG_INTERNET_IOP ;

    profile = &ior->profiles.elements[0].data.iiop_body ;

    profile->iiop_version = version ;
    profile->port = port ;
    profile->object_key.count = 0 ;
    profile->object_key.elements = NULL ;
    profile->components.count = 0 ;
    profile->components.elements = NULL ;

    if (host == NULL)  host = netHostOf (netAddrOf (NULL), false) ;
    profile->host = strdup (host) ;
    if (profile->host == NULL) {
        LGE "(coliMakeIOR) Error duplicating host name: \"%s\"\nstrdup: ",
            host) ;
        PUSH_ERRNO ;  comxErase ((ComxFunc) gimxIOR, ior) ;  POP_ERRNO ;
        return (errno) ;
    }

    profile->object_key.elements = (octet *) memdup (object.elements,
                                                     object.count) ;
    if (profile->object_key.elements == NULL) {
        LGE "(coliMakeIOR) Error duplicating %lu-octet object key for %u@%s.\nmemdup: ",
            object.count, (unsigned int) profile->port, profile->host) ;
        PUSH_ERRNO ;  comxErase ((ComxFunc) gimxIOR, ior) ;  POP_ERRNO ;
        return (errno) ;
    }
    profile->object_key.count = object.count ;

/* Add to the profile a TAG_CODE_SETS component specifying UTF-16 as the
   wide character set. */

    profile->components.elements =
        (TaggedComponent *) calloc (1, sizeof (TaggedComponent)) ;
    if (profile->components.elements == NULL) {
        LGE "(coliMakeIOR) Error allocating tagged component for %u@%s.\ncalloc: ",
            (unsigned int) profile->port, profile->host) ;
        PUSH_ERRNO ;  comxErase ((ComxFunc) gimxIOR, ior) ;  POP_ERRNO ;
        return (errno) ;
    }
    profile->components.count = 1 ;

    component = &profile->components.elements[0] ;
    component->tag = IOP_TAG_CODE_SETS ;
    component->component_data.count = 0 ;
    component->component_data.elements = NULL ;

/* Encapsulate the code set information in an octet sequence.  The layout of
   the fields in the octet sequence is according to the CodeSetComponentInfo
   structure (see section 13.10.2.4 in the CORBA specification). */

#ifdef __palmos__
	/* Lookup tables not working in PRC-TOOLS ... */
    HostFPrintF (HostLogFile(), "(coliMakeIOR) &CodeSetIdLUT = 0x%lX", (long) CodeSetIdLUT) ;
    codeSets.ForCharData.native_code_set = 0x00010001L ;
    codeSets.ForCharData.conversion_code_sets.count = 0 ;
    codeSets.ForWcharData.native_code_set = 0x00010109L ;
    codeSets.ForWcharData.conversion_code_sets.count = 0 ;
#else
				/* ISO 8859-1:1987; Latin Alphabet No. 1 */
    codeSets.ForCharData.native_code_set =
        coliToNumber (CodeSetIdLUT, "ISO 8859-1:1987", true) ;
    codeSets.ForCharData.conversion_code_sets.count = 0 ;
				/* ISO/IEC 10646-1:1993; UTF-16,
				   UCS Transformation Format 16-bit form */
    codeSets.ForWcharData.native_code_set =
        coliToNumber (CodeSetIdLUT, "UTF-16", true) ;
    codeSets.ForWcharData.conversion_code_sets.count = 0 ;
#endif

    if (comxEncapsule (version, MxENCODE, &component->component_data,
                       gimxCodeSetComponentInfo, &codeSets, NULL)) {
        LGE "(coliMakeIOR) Error encapsulating code information for %u@%s component.\ncomxEncapsule: ",
            profile->port, profile->host) ;
        PUSH_ERRNO ;  comxErase ((ComxFunc) gimxIOR, ior) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(coliMakeIOR) Made %u@%s IOR with %lu-octet object key.\n",
        (unsigned int) profile->port, profile->host, object.count) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    coliO2S ()

    Convert an IOR to a Stringified Reference.


Purpose:

    Function coliO2S() converts a binary Interoperable Object Reference (IOR)
    to a "stringified" (ASCII) object reference.


    Invocation:

        string = coliO2S (ior, dynamic) ;

    where

        <ior>		- I
            is the binary IOR to be converted.
        <dynamic>	- I
            controls the disposition of the result.  If this argument is
            true (non-zero), coliO2S() returns a dynamically allocated
            string that the caller is responsible for free(3)ing.  If this
            argument is false (zero), the string is stored in memory local
            to coliO2S() and it should be used or duplicated before calling
            coliO2S() again.
        <string>	- O
            returns the stringified object reference; NULL is returned in
            the event of an error.

*******************************************************************************/


char  *coliO2S (

#    if PROTOTYPES
        const  IOR  *ior,
        bool  dynamic)
#    else
        ior, dynamic)

        IOR  *ior ;
        bool  dynamic ;
#    endif

{    /* Local variables. */
    char  *iorString, *s ;
    OctetSeq  encapsulation ;
    size_t  length ;
    unsigned  long  i ;
    Version  version = { GIOP_VERSION_MAJOR, GIOP_VERSION_MINOR } ;
    static  char  *result = NULL ;
    static  size_t  resultMax = 0 ;



/* Encode the IOR as a CDR encapsulation (i.e., an octet sequence). */

    if (comxEncapsule (version, MxENCODE, &encapsulation,
                       gimxIOR, ior, NULL)) {
        LGE "(coliO2S) Error encoding IOR.\ncomxEncapsule: ") ;
        return (NULL) ;
    }

/* Dynamically allocate a string to hold the stringified reference. */

    length = strlen ("IOR:") + (encapsulation.count * 2) + 1 ;

    if (dynamic || (length > resultMax)) {
        iorString = (char *) malloc (length) ;
        if (iorString == NULL) {
            LGE "(coliO2S) Error allocating %d-byte string.\nmalloc: ",
                length) ;
            PUSH_ERRNO ;
            comxErase ((ComxFunc) comxOctetSeq, &encapsulation) ;
            POP_ERRNO ;
            return (NULL) ;
        }
        if (!dynamic) {
            if (result != NULL)  free (result) ;
            result = iorString ;  resultMax = length ;
        }
    } else {
        iorString = result ;
    }

/* Generate the stringified reference, which is simply a hex dump of
   the octet sequence encapsulation of the encoded IOR. */

    strcpy (iorString, "IOR:") ;
    s = iorString + strlen (iorString) ;

    for (i = 0 ;  i < encapsulation.count ;  i++, s += 2)
        sprintf (s, "%02X", (unsigned char) encapsulation.elements[i]) ;

    comxErase ((ComxFunc) comxOctetSeq, &encapsulation) ;

    return (iorString) ;

}

/*!*****************************************************************************

Procedure:

    coliO2URL ()

    Convert an IOR to a URL.


Purpose:

    Function coliO2URL() converts a binary Interoperable Object Reference (IOR)
    to a "corbaloc:" URL.  URLs look as follows:

        corbaloc:[iiop]:[<major>.<minor>@][<host>][:<port>][/<key>]

    Multiple IIOP addresses may be encoded in the URL, separated by commas:

        corbaloc:[iiop]:[<major>.<minor>@][<host>][:<port>],
                 [iiop]:[<major>.<minor>@][<host>][:<port>],
                 ...
                 [iiop]:[<major>.<minor>@][<host>][:<port>][/<key>]

    (The line breaks are for readibility only and are present in the actual
    URL.)

    Default fields are omitted in accordance with the CORBA specification
    (section 13.6.10.3):

            Version: 1.0
               Host: <localhost>
               Port: 2809


    Invocation:

        url = coliO2URL (ior, dynamic) ;

    where

        <ior>		- I
            is the binary IOR to be converted.
        <dynamic>	- I
            controls the disposition of the result.  If this argument is
            true (non-zero), coliO2URL() returns a dynamically allocated
            URL that the caller is responsible for free(3)ing.  If this
            argument is false (zero), the URL is stored in memory local
            to coliO2URL() and it should be used or duplicated before
            calling coliO2URL() again.
        <url>		- O
            returns the URL; NULL is returned in the event of an error.

*******************************************************************************/


char  *coliO2URL (

#    if PROTOTYPES
        const  IOR  *ior,
        bool  dynamic)
#    else
        ior, dynamic)

        IOR  *ior ;
        bool  dynamic ;
#    endif

{    /* Local variables. */
    char  character, *urlString, *s ;
    ProfileBody  *profile ;
    size_t  i, length ;
    static  char  *result = NULL ;
    static  size_t  resultMax = 0 ;



/* Estimate the length of the URL. */

    length = strlen ("corbaloc::/") ;
    for (i = 0 ;  (profile = (ProfileBody *) coliProfile (ior, i)) ;  i++) {
        length += strlen ("iiop:9.9@:99999/,") ;
        if (profile->host != NULL)  length += strlen (profile->host) ;
        if (i == 0)  length += (profile->object_key.count * 3) ;
    }
    length++ ;

/* Dynamically allocate a string to hold the URL. */

    if (dynamic || (length > resultMax)) {
        urlString = (char *) malloc (length) ;
        if (urlString == NULL) {
            LGE "(coliO2URL) Error allocating %d-byte URL.\nmalloc: ", length) ;
            return (NULL) ;
        }
        if (!dynamic) {
            if (result != NULL)  free (result) ;
            result = urlString ;  resultMax = length ;
        }
    } else {
        urlString = result ;
    }

/* Construct the URL. */

    strcpy (urlString, "corbaloc:") ;
    s = urlString + strlen (urlString) ;

/* Add each IIOP address to the URL. */

    for (i = 0 ;  (profile = (ProfileBody *) coliProfile (ior, i)) ;  i++) {

        if (i > 0)  *s++ = ',' ;

        strcpy (s, "iiop:") ;  s += strlen (s) ;

        if ((profile->iiop_version.major != 1) ||	/* Default is GIOP 1.0. */
            (profile->iiop_version.minor != 0)) {
            sprintf (s, "%d.%d@",
                     (int) profile->iiop_version.major,
                     (int) profile->iiop_version.minor) ;
            s += strlen (s) ;
        }

        if (profile->host != NULL) {
            strcpy (s, profile->host) ;  s += strlen (s) ;
        }

        if (profile->port != 2809) {
            sprintf (s, ":%u", (unsigned int) profile->port) ;
            s += strlen (s) ;
        }

    }

    if (i == 0)  *s++ = ':' ;			/* "[iiop]:" */

/* Add the object key to the URL. */

    profile = (ProfileBody *) coliProfile (ior, 0) ;

    if (profile->object_key.count > 0) {
        *s++ = '/' ;
        for (i = 0 ;  i < profile->object_key.count ;  i++) {
            character = profile->object_key.elements[i] ;
            if ((character != 0)  &&
                (isalnum ((unsigned char) character) ||
                 (strchr (";/:?@&=+$,-_!~*'()", character) != NULL))) {
                *s++ = character ;
            } else {
                sprintf (s, "%%%02X", (unsigned char) character) ;
                s += strlen (s) ;
            }
        }
    }

    *s = '\0' ;

    return (urlString) ;

}

/*!*****************************************************************************

Procedure:

    coliOpenIOR ()

    Open an IIOP Stream for an IOR.


Purpose:

    Function coliOpenIOR() opens a new IIOP stream, if necessary, for an IOR.
    If the IOR maps to an existing stream, the existing stream is returned.
    Otherwise, a new network connection is established with the host and port
    specified in the IOR.

    This function is useful when a client receives an object reference from
    a server over an existing IIOP stream.  If the object reference specifies
    a host or port different from that of the existing stream, the client must
    open a new connection to the object's server before calling methods of the
    object.  The application is responsible for distinguishing between the old
    and new stream and for closing the new stream when it is no longer needed.


    Invocation:

        status = coliOpenIOR (ior, oldStream, &newStream) ;

    where

        <ior>		- I
            is an object reference.
        <oldStream>	- I
            is an existing IIOP stream.
        <newStream>	- O
            returns an IIOP stream for the object.  If the object can be
            accessed through the existing stream, the existing stream is
            returned.  Otherwise, a new IIOP stream is established and
            returned.
        <status>	- O
            returns the status of opening the stream, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliOpenIOR (

#    if PROTOTYPES
        const  IOR  *ior,
        IiopStream  oldStream,
        IiopStream  *newStream)
#    else
        ior, oldStream, newStream)

        IOR  *ior ;
        IiopStream  oldStream ;
        IiopStream  *newStream ;
#    endif

{    /* Local variables. */
    char  serverName[256] ;
    ProfileBody  *profile ;
    TcpEndpoint  connection ;



/* Check if the existing stream can service the object. */

    profile = (ProfileBody *) coliProfile (ior, 0) ;

    sprintf (serverName, "%u@%s", (unsigned int) profile->port, profile->host) ;

    if ((oldStream != NULL) &&
        (strcmp (serverName, iiopName (oldStream)) == 0)) {
        *newStream = oldStream ;
        LGI "(coliOpenIOR) Using existing stream: %s\n", serverName) ;
        return (0) ;
    }

    *newStream = NULL ;

/* The IOR specifies a different port and/or host; establish a new network
   connection to the object's server. */

    if (tcpCall (serverName, 0, &connection)) {
        LGE "(coliOpenIOR) Error connecting to %s.\ntcpCall: ", serverName) ;
        return (errno) ;
    }

/* Create an IIOP stream on the TCP/IP connection. */

    if (iiopCreate (connection, newStream)) {
        LGE "(coliOpenIOR) Error creating IIOP stream for %s.\niiopCreate: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;  tcpDestroy (connection) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(coliOpenIOR) New IIOP stream: %s\n", iiopName (*newStream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    coliProfile ()

    Get an IOR's TAG_INTERNET_IOP Profile.


Purpose:

    Function coliProfile() returns the I-th TAG_INTERNET_IOP profile body
    from an Interoperable Object Reference (IOR).  The TAG_INTERNET_IOP
    profile contains the referenced object's server host, TCP/IP port,
    and key.


    Invocation:

        profile = coliProfile (ior, index) ;

    where

        <ior>		- I
            is the IOR for an object.
        <index>		- I
            specifies the index (0..N-1) of the desired TAG_INTERNET_IOP
            profile.  NOTE that the index is relative to the subset of
            TAG_INTERNET_IOP profiles and is NOT an absolute index into
            the IOR's sequence of tagged profiles (which may include, for
            example, TAG_MULTIPLE_COMPONENTS profiles here and there.)
        <profile>	- O
            returns the desired TAG_INTERNET_IOP profile body from the IOR;
            NULL is returned if the IOR has no TAG_INTERNET_IOP profile of
            the specified index.

*******************************************************************************/


const  ProfileBody  *coliProfile (

#    if PROTOTYPES
        const  IOR  *ior,
        int  index)
#    else
        ior, index)

        IOR  *ior ;
        int  index ;
#    endif

{    /* Local variables. */
    unsigned  long  i ;



    SET_ERRNO (EINVAL) ;

    if (ior == NULL)  return (NULL) ;

    for (i = 0 ;  i < ior->profiles.count ;  i++) {
        if ((ior->profiles.elements[i].which == IOP_TAG_INTERNET_IOP)  &&
            (index-- == 0)) {
            return (&(ior->profiles.elements[i].data.iiop_body)) ;
        }
    }

    return (NULL) ;

}

/*!*****************************************************************************

Procedure:

    coliReply ()

    Issue a Reply for a Prior CORBA Request.


Purpose:

    Function coliReply() issues a reply for a prior CORBA request.  A CORBA
    reply message specifying the request ID, reply status, and additional
    return parameters is constructed and sent to the CORBA peer.


    Invocation:

        status = coliReply (stream, requestID, replyStatus,
                            [marshalF1, value1,]
                            [marshalF2, value2,]
                            ...
                            NULL) ;

    where

        <stream>		- I
            is the IIOP stream.
        <requestID>		- I
            is the CORBA request ID from the previously received request.
        <replyStatus>		- I
            is the reply status; see the enumerated ReplyStatusType in
            "gimx_util.h".
        <marshalFn>, <valueN>	- I
            specify a marshaling function and value for each return parameter
            expected by the CORBA peer in response to its request.  <marshalFn>
            is a pointer to a marshaling function of type ComxFunc (declared
            in "comx_util.h"); <valueN> is a (void *) pointer to the value.
            A single NULL terminates the list of function/value pairs.
        <status>		- O
            returns the status of constructing and sending the reply message,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliReply (

#    if PROTOTYPES
        IiopStream  stream,
        unsigned  long  requestID,
        ReplyStatusType  replyStatus,
        ...)
#    else
        stream, requestID, replyStatus, va_alist)

        IiopStream  stream ;
        unsigned  long  requestID ;
        ReplyStatusType  replyStatus ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    ComxChannel  mxchan ;
    ComxFunc  marshalF ;
    IiopHeader  header ;




/* Construct a CORBA reply header. */

    comxCreate (coliVersion (NULL), false, 12, NULL, 0, &mxchan) ;

    if (GIOP_VERSION_GE (coli_version, 1, 2)) {

        ReplyHeader  rphdr ;				/* GIOP 1.2 and later */

        rphdr.request_id = requestID ;
        rphdr.reply_status = replyStatus ;
        rphdr.service_context.count = 0 ;
        rphdr.service_context.elements = NULL ;

        gimxReplyHeader (mxchan, &rphdr) ;

        comxSkip (mxchan, 0, 8) ;	/* 8-byte alignment following header
					   in GIOP 1.2 and later. */
    } else {

        ReplyHeader_1_0  rphdr ;			/* GIOP 1.0, 1.1 */

        rphdr.service_context.count = 0 ;
        rphdr.service_context.elements = NULL ;
        rphdr.request_id = requestID ;
        rphdr.reply_status = replyStatus ;

        gimxReplyHeader_1_0 (mxchan, &rphdr) ;

    }

/* Encode the arguments, if any, and append them to the reply header. */

#if HAVE_STDARG_H
    va_start (ap, replyStatus) ;
#else
    va_start (ap) ;
#endif

    while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
        if (marshalF (mxchan, va_arg (ap, void *))) {
            LGE "(coliReply) Error encoding parameter for reply %lu to %s.\n",
                requestID, iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  va_end (ap) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

    va_end (ap) ;

/* Build the IIOP message header. */

    header.GIOP_version = coli_version ;
    header.flags = 0 ;
    header.message_type = Reply ;
    header.message_size = comxSkip (mxchan, 0, 0) ;

/* Output the reply message to the peer. */

    if (iiopWrite (stream, -1.0, &header, comxBuffer (mxchan, 0))) {
        LGE "(coliReply) Error sending reply #%lu to %s.\niiopWrite: ",
            requestID, iiopName (stream)) ;
        PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
        return (errno) ;
    }

    comxDestroy (mxchan) ;

    LGI "(coliReply) Sent %s reply #%lu to %s.\n",
        coliToName (ReplyStatusTypeLUT, (long) replyStatus),
        requestID, iiopName (stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    coliRequest ()

    Issue a Request to a CORBA Server Object.


Purpose:

    Function coliRequest() issues a request to a CORBA server for an object
    to perform an operation.  A CORBA request message specifying the object,
    operation, and additional arguments is constructed and sent to the server.


    Invocation:

        status = coliRequest (stream, object, operation, contexts,
                              [marshalF1, value1,]
                              [marshalF2, value2,]
                              ...
                              NULL) ;

    where

        <stream>		- I
            is the IIOP stream to the object's server.
        <object>		- I
            is the CORBA object key that identifies the object that
            is to perform the operation.
        <operation>		- I
            is the operation to be performed.
        <contexts>		- I
            is an optional list of service contexts to be passed to the server.
        <marshalFn>, <valueN>	- I
            specify a marshaling function and value for each argument
            expected by the server object for the given operation.
            <marshalFn> is a pointer to a marshaling function of type
            ComxFunc (declared in "comx_util.h"); <valueN> is a (void *)
            pointer to the value.  A single NULL terminates the list of
            function/value pairs.
        <status>		- O
            returns the status of constructing and sending the request
            message, zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliRequest (

#    if PROTOTYPES
        IiopStream  stream,
        ObjectKey  object,
        const  char  *operation,
        const  ServiceContextList  *contexts,
        ...)
#    else
        stream, object, operation, contexts, va_alist)

        IiopStream  stream ;
        ObjectKey  object ;
        char  *operation ;
        ServiceContextList  *contexts ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;
    ComxChannel  mxchan ;
    ComxFunc  marshalF ;
    IiopHeader  header ;




/* Construct a CORBA request header for the operation. */

    comxCreate (coliVersion (NULL), false, 12, NULL, 0, &mxchan) ;

    if (GIOP_VERSION_GE (coli_version, 1, 2)) {

        RequestHeader  rqhdr ;				/* GIOP 1.2 and later */

        rqhdr.request_id = iiopRequestID (stream) ;
        rqhdr.response_flags = MESSAGING_SYNC_WITH_TARGET ;
        rqhdr.target.which = KeyAddr ;
        rqhdr.target.data.object_key = *((OctetSeq *) &object) ;
        rqhdr.operation = (char *) operation ;
        rqhdr.service_context.count = 0 ;
        rqhdr.service_context.elements = NULL ;
        if (contexts != NULL)  rqhdr.service_context = *contexts ;

        gimxRequestHeader (mxchan, &rqhdr) ;

        comxSkip (mxchan, 0, 8) ;	/* 8-byte alignment following header
					   in GIOP 1.2 and later. */

    } else if (GIOP_VERSION_GE (coli_version, 1, 1)) {

        RequestHeader_1_1  rqhdr ;			/* GIOP 1.1 */

        rqhdr.service_context.count = 0 ;
        rqhdr.service_context.elements = NULL ;
        if (contexts != NULL)  rqhdr.service_context = *contexts ;
        rqhdr.request_id = iiopRequestID (stream) ;
        rqhdr.response_expected = 0x01 ;
        rqhdr.object_key = *((OctetSeq *) &object) ;
        rqhdr.operation = (char *) operation ;
        rqhdr.requesting_principal.count = 0 ;
        rqhdr.requesting_principal.elements = NULL ;

        gimxRequestHeader_1_1 (mxchan, &rqhdr) ;

    } else {

        RequestHeader_1_0  rqhdr ;			/* GIOP 1.0 */

        rqhdr.service_context.count = 0 ;
        rqhdr.service_context.elements = NULL ;
        if (contexts != NULL)  rqhdr.service_context = *contexts ;
        rqhdr.request_id = iiopRequestID (stream) ;
        rqhdr.response_expected = 0x01 ;
        rqhdr.object_key = *((OctetSeq *) &object) ;
        rqhdr.operation = (char *) operation ;
        rqhdr.requesting_principal.count = 0 ;
        rqhdr.requesting_principal.elements = NULL ;

        gimxRequestHeader_1_0 (mxchan, &rqhdr) ;

    }

/* Encode the arguments, if any, and append them to the request header. */

#if HAVE_STDARG_H
    va_start (ap, contexts) ;
#else
    va_start (ap) ;
#endif

    while (NULL != (marshalF = va_arg (ap, ComxFunc))) {
        if (marshalF (mxchan, va_arg (ap, void *))) {
            LGE "(coliRequest) Error encoding %s argument for %s.\n",
                operation, iiopName (stream)) ;
            PUSH_ERRNO ;  comxDestroy (mxchan) ;  va_end (ap) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

    va_end (ap) ;

/* Build the IIOP message header. */

    header.GIOP_version = coli_version ;
    header.flags = 0 ;
    header.message_type = Request ;
    header.message_size = comxSkip (mxchan, 0, 0) ;

/* Output the request message to the server. */

    if (iiopWrite (stream, -1.0, &header, comxBuffer (mxchan, 0))) {
        LGE "(coliRequest) Error sending %s request to %s.\niiopWrite: ",
            operation, iiopName (stream)) ;
        PUSH_ERRNO ;  comxDestroy (mxchan) ;  POP_ERRNO ;
        return (errno) ;
    }

    comxDestroy (mxchan) ;

    LGI "(coliRequest) Sent %s operation to %s.\n",
        operation, iiopName (stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    coliS2O ()

    Convert a Stringified Reference to an IOR.


Purpose:

    Function coliS2O() converts a "stringified" (ASCII) object reference
    to its binary Interoperable Object Reference (IOR).


    Invocation:

        status = coliS2O (string, &ior) ;

    where

        <string>	- I
            is the stringified object reference to be converted.
        <ior>		- O
            returns the IOR in its binary form.  The caller is responsible for
            erasing the dynamically-allocated fields of the IOR using gimxIOR()
            in erase mode or using comxErase().
        <status>	- O
            returns the status of converting the string, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliS2O (

#    if PROTOTYPES
        const  char  *string,
        IOR  *ior)
#    else
        string, ior)

        char  *string ;
        IOR  *ior ;
#    endif

{    /* Local variables. */
    bool  byteOrder ;
    ComxChannel  mxchan ;
    IOR  convertedIOR ;
    OctetSeq  rawIOR ;
    unsigned  char  *s ;
    unsigned  long  i ;
    Version  version = { GIOP_VERSION_MAJOR, GIOP_VERSION_MINOR } ;




    if (ior == NULL)  ior = &convertedIOR ;
    ior->type_id = NULL ;
    ior->profiles.count = 0 ;  ior->profiles.elements = NULL ;
    mxchan = NULL ;
    rawIOR.count = 0 ;  rawIOR.elements = NULL ;

/* Skip the "IOR:" prefix in the stringified reference. */

    s = (unsigned char *) strchr (string, ':') ;
    if (s++ == NULL)  s = (unsigned char *) string ;

/* Convert the remainder of the string to a sequence of binary octets. */

    rawIOR.count = strlen ((char *) s) / 2 ;

    rawIOR.elements = (octet *) calloc (rawIOR.count, 1) ;
    if (rawIOR.elements == NULL) {
        LGE "(coliS2O) Error allocating %lu-octet sequence for \"%s\".\ncalloc: ",
            rawIOR.count, string) ;
        goto onError ;
    }

    for (i = 0 ;  i < rawIOR.count ;  i++, s += 2) {
        if (!isxdigit (s[0]) || !isxdigit (s[1])) {
            SET_ERRNO (EINVAL) ;
            LGE "(coliS2O) Invalid hexadecimal coding beginning near \"%s\".\n",
                s) ;
            goto onError ;
        }
        rawIOR.elements[i] = isdigit (s[0]) ? (s[0] - '0')
                                            : ((toupper (s[0]) - 'A') + 10) ;
        rawIOR.elements[i] = (rawIOR.elements[i] << 4)
                             |
                             (isdigit (s[1]) ? (s[1] - '0')
                                             : ((toupper (s[1]) - 'A') + 10)) ;
    }

/* Decode the binary octets as an IOR. */

    if (comxCreate (version, rawIOR.elements[0] & ENDIAN_MASK,
                    0, rawIOR.elements, rawIOR.count, &mxchan)) {
        LGE "(coliS2O) Error creating marshaling channel for \"%s\".\ncomxCreate: ",
            string) ;
        goto onError ;
    }

    if (comxBoolean (mxchan, &byteOrder) || gimxIOR (mxchan, ior)) {
        LGE "(coliS2O) Error decoding IOR in \"%s\".\ngimxIOR: ",
            string) ;
        goto onError ;
    }

    comxDestroy (mxchan) ;
    if (ior == &convertedIOR)  comxErase ((ComxFunc) gimxIOR, &convertedIOR) ;
    free (rawIOR.elements) ;

    return (0) ;


/*******************************************************************************
    Error Return - deallocate any allocated memory.
*******************************************************************************/

onError:
    PUSH_ERRNO ;
    comxErase ((ComxFunc) gimxIOR, ior) ;
    if (mxchan != NULL)  comxDestroy (mxchan) ;
    if (rawIOR.elements != NULL)  free (rawIOR.elements) ;
    POP_ERRNO ;

    return (errno) ;

}

/*!*****************************************************************************

Procedure:

    coliS2URL ()

    Convert a Stringified Object Reference to a URL.


Purpose:

    Function coliS2URL() converts a "stringified" (ASCII) object reference
    to a "corbaloc:" URL.

    Invocation:

        url = coliS2URL (string, dynamic) ;

    where

        <string>	- I
            is the stringified object reference to be converted.
        <dynamic>	- I
            controls the disposition of the result.  If this argument is
            true (non-zero), coliS2URL() returns a dynamically allocated
            string that the caller is responsible for free(3)ing.  If this
            argument is false (zero), the string is stored in memory local
            to coliS2URL() and it should be used or duplicated before calling
            coliS2URL() again.
        <url>		- O
            returns the URL; NULL is returned in the event of an error.

*******************************************************************************/


char  *coliS2URL (

#    if PROTOTYPES
        const  char  *string,
        bool  dynamic)
#    else
        string, dynamic)

        char  *string ;
        bool  dynamic ;
#    endif

{    /* Local variables. */
    char  *url ;
    IOR  ior ;



/* Convert the stringified reference to an object reference. */

    if (coliS2O (string, &ior)) {
        LGE "(coliS2URL) Error decoding IOR.\ncoliS2O: ") ;
        return (NULL) ;
    }

/* Convert the object reference to a URL. */

    url = coliO2URL (&ior, dynamic) ;

    PUSH_ERRNO ;  comxErase ((ComxFunc) gimxIOR, &ior) ;  POP_ERRNO ;

    return (url) ;

}

/*!*****************************************************************************

Procedure:

    coliToName ()

    Map a Number to a Name.


Purpose:

    Function coliToName() looks up a number in a lookup table and
    returns the corresponding name.  It is useful for converting
    CORBA enumerations to readable ASCII strings.


    Invocation:

        name = coliToName (table, number) ;

    where

        <table>		- I
            is the lookup table containing the desired mapping.
        <number>	- I
            is the number to be looked up.
        <name>		- O
            returns the ASCII string corresponding to the specified
            number; a formatted version of the number is returned if
            it is not found in the lookup table.  The name is stored
            in memory local to the IIOP utilities and it should not
            be modified or freed by the caller.

*******************************************************************************/


const  char  *coliToName (

#    if PROTOTYPES
        const  ColiMap  table[],
        long  number)
#    else
        table, number)

        ColiMap  table[] ;
        long  number ;
#    endif

{    /* Local variables. */
    int  i ;
    static  char  buffer[32] ;



#ifndef __palmos__
    for (i = 0 ;  table[i].name != NULL ;  i++) {
        if (table[i].number == number)  break ;
    }

    if (table[i].name == NULL) {		/* Number not found in table? */
#endif
        sprintf (buffer, "%ld", number) ;
        return (buffer) ;
#ifndef __palmos__
    } else {					/* Number found in table. */
        return (table[i].name) ;
    }
#endif

}

/*!*****************************************************************************

Procedure:

    coliToNumber ()

    Map a Name to a Number.


Purpose:

    Function coliToNumber() looks up a string in a lookup table and
    returns the corresponding number.  It is useful for converting
    ASCII strings to CORBA enumerations.


    Invocation:

        number = coliToNumber (table, name, partial) ;

    where

        <table>		- I
            is the lookup table containing the desired mapping.
        <name>		- I
            is the name to be looked up.
        <partial>	- I
            indicates if a partial match of the name is acceptable.  If this
            argument is false, a case-insensitive comparison of the name to
            the strings in the table is performed.  If this argument is true,
            a case-sensitive search of the name within the strings is performed.
            This last feature is primarily intended for looking up abbreviated
            names of code set IDs; e.g., a shortened "UTF-8" instead of the full
            "X/Open UTF-8; UCS Transformation Format 8 (UTF-8)"!
        <number>	- O
            returns the number corresponding to the specified name;
            -1 is returned if the name is not found in the lookup table.

*******************************************************************************/


long  coliToNumber (

#    if PROTOTYPES
        const  ColiMap  table[],
        const  char  *name,
        bool  partial)
#    else
        table, name, partial)

        ColiMap  table[] ;
        char  *name ;
        bool  partial ;
#    endif

{    /* Local variables. */
#ifndef __palmos__
    int  i ;



    for (i = 0 ;  table[i].name != NULL ;  i++) {
        if (partial && (strstr (table[i].name, name) != NULL))  break ;
        if (!partial && (strcasecmp (table[i].name, name) == 0))  break ;
    }

    return ((table[i].name == NULL) ? -1L : table[i].number) ;
#else
    return (-1L) ;
#endif

}

/*!*****************************************************************************

Procedure:

    coliURL2O ()

    Convert an IIOP URL to an IOR.


Purpose:

    Function coliURL2O() converts a "corbaloc:iiop:" URL to an Interoperable
    Object Reference (IOR).  URLs look as follows:

        corbaloc:[iiop]:[<major>.<minor>@][<host>][:<port>][/<key>]

    Multiple IIOP addresses can be specified in the URL, separated by
    commas:

        corbaloc:[iiop]:[<major>.<minor>@][<host>][:<port>],
                 [iiop]:[<major>.<minor>@][<host>][:<port>],
                 ...
                 [iiop]:[<major>.<minor>@][<host>][:<port>][/<key>]

    (The line breaks are for readibility only and should not be present
    in an actual URL.)

    If fields are missing from an address, the following substitutions are
    made in accordance with the CORBA specification (section 13.6.10.3):

            Version: 1.0
               Host: <localhost>
               Port: 2809


    Invocation:

        status = coliURL2O (url, &ior) ;

    where

        <url>		- I
            is the "corbaloc:" URL to be converted.
        <ior>		- O
            returns the IOR in binary form.  The caller is responsible
            for erasing the dynamically-allocated fields of the IOR
            using gimxIOR() in erase mode or using comxErase() after
            the IOR is no longer needed.
        <status>	- O
            returns the status of converting the URL, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  coliURL2O (

#    if PROTOTYPES
        const  char  *url,
        IOR  *ior)
#    else
        url, ior)

        char  *url ;
        IOR  *ior ;
#    endif

{    /* Local variables. */
    char  *address, *addresses, *key, *s ;
    octet  *t ;
    ProfileBody  *profile ;
    unsigned  long  i, numAddresses ;




    addresses = NULL ;
    key = NULL ;
    ior->type_id = NULL ;
    ior->profiles.count = 0 ;  ior->profiles.elements = NULL ;

    if (url == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(coliURL2O) NULL URL: ") ;
        goto onError ;
    }

    if (strncmp (url, "corbaloc:", strlen ("corbaloc:")) != 0) {
        SET_ERRNO (EINVAL) ;
        LGE "(coliURL2O) Invalid URL: \"%s\"\n", url) ;
        goto onError ;
    }

/* Position to the beginning of the first address. */

    addresses = strdup (url + strlen ("corbaloc:")) ;
    if (addresses == NULL) {
        LGE "(coliURL2O) Error duplicating URL: \"%s\"\nstrdup: ", url) ;
        goto onError ;
    }

/* Strip the key string from the end of the addresses. */

    s = strchr (addresses, '/') ;
    if (s != NULL) {
        *s++ = '\0' ;
        key = strdup (s) ;
        if (key == NULL) {
            LGE "(coliURL2O) Error duplicating key: \"%s\"\nstrdup: ", s) ;
            goto onError ;
        }
    }

/*******************************************************************************
    For each IIOP address in the URL, add a TAG_INTERNET_IOP profile to the IOR.
*******************************************************************************/

/* Count the number of IIOP addresses and NUL-terminate each one. */

    numAddresses = 1 ;
    s = strchr (addresses, ',') ;
    while (s++ != NULL) {
        numAddresses++ ;
        s = strchr (s, ',') ;
    }

/* Allocate and initialize a sequence of tagged profiles for the IIOP
   addresses. */

    ior->profiles.elements = (TaggedProfile *) calloc (numAddresses,
                                                       sizeof (TaggedProfile)) ;
    if (ior->profiles.elements == NULL) {
        LGE "(coliURL2O) Error allocating %lu profiles for URL \"%s\".\ncalloc: ",
            numAddresses, url) ;
        goto onError ;
    }
    ior->profiles.count = numAddresses ;

    for (i = 0 ;  i < numAddresses ;  i++) {
        ior->profiles.elements[i].which = IOP_TAG_INTERNET_IOP ;
        profile = &ior->profiles.elements[i].data.iiop_body ;
        profile->iiop_version.major = 1 ;  profile->iiop_version.minor = 0 ;
        profile->host = NULL ;
        profile->port = 2809 ;
        profile->object_key.count = 0 ;  profile->object_key.elements = NULL ;
        profile->components.count = 0 ;  profile->components.elements = NULL ;
    }

/* For each IIOP address in the URL, flesh out its profile body. */

    i = numAddresses ;

    while (i-- > 0) {

        profile = &ior->profiles.elements[i].data.iiop_body ;

/* Locate the rightmost address. */

        s = strrchr (addresses, ',') ;
        if (s == NULL) {
            address = addresses ;
        } else {
            *s++ = '\0' ;
            address = s ;
        }

/* Skip the protocol specification ("[iiop]:") in the address. */

        s = strchr (address, ':') ;
        if (s == NULL) {
            SET_ERRNO (EINVAL) ;
            LGE "(coliURL2O) Missing protocol ID in \"%s\"\n", address) ;
            goto onError ;
        }
        address = ++s ;

/* Grab the version number, if present at the beginning of the address. */

        s = strchr (address, '@') ;
        if (s != NULL) {
            int  major, minor ;
#if defined(HAVE_SSCANF) && !HAVE_SSCANF
            major = atoi (address) ;		/* Assume "N.N". */
            minor = atoi (address + 2) ;
#else
            if (sscanf (address, "%d.%d", &major, &minor) < 2) {
                SET_ERRNO (EINVAL) ;
                LGE "(coliURL2O) Invalid version number in \"%s\"\n", address) ;
                goto onError ;
            }
#endif
            profile->iiop_version.major = (octet) major ;
            profile->iiop_version.minor = (octet) minor ;
            address = ++s ;
        }

/* Get the port number, if present at the end of the address. */

        s = strchr (address, ':') ;
        if (s != NULL) {
            *s++ = '\0' ;
            profile->port = (unsigned short) atoi (s) ;
        }

/* Finally, the middle of the address, if present, is the host name. */

        if (strlen (address) > 0) {
            profile->host = strdup (address) ;
            if (profile->host == NULL) {
                LGE "(coliURL2O) Error duplicating host name: \"%s\"\nstrdup: ",
                    address) ;
                goto onError ;
            }
        } else {
            profile->host = strdup (netHostOf (netAddrOf (NULL), false)) ;
            if (profile->host == NULL) {
                LGE "(coliURL2O) Error duplicating local host name: \"%s\"\nstrdup: ",
                    netHostOf (netAddrOf (NULL), false)) ;
                goto onError ;
            }
        }

/* Add the object key common to all of the IIOP addresses. */

        if (key != NULL) {
            profile->object_key.elements = (octet *) strdup (key) ;
            if (profile->object_key.elements == NULL) {
                LGE "(coliURL2O) Error duplicating object key: \"%s\"\nstrdup: ",
                    key) ;
                goto onError ;
            }
            profile->object_key.count = strlen (key) ;
            s = key ;  t = profile->object_key.elements ;
            for ( ; ; ) {		/* Translate "%xx" escape sequences. */
                *t = (octet) *s ;
                if (*s == '\0')  break ;
                if (*s++ == '%') {
                    if (!isxdigit (s[0]) || !isxdigit (s[1])) {
                        SET_ERRNO (EINVAL) ;
                        LGE "(coliURL2O) Invalid hexadecimal coding beginning near \"%s\".\n",
                            s) ;
                        goto onError ;
                    }
                    *t = isdigit (s[0]) ? (s[0] - '0')
                                        : ((toupper (s[0]) - 'A') + 10) ;
                    *t = (*t << 4)
                         |
                         (isdigit (s[1]) ? (s[1] - '0')
                                         : ((toupper (s[1]) - 'A') + 10)) ;
                    s += 2 ;
                }
                t++ ;
            }
            profile->object_key.count = t - profile->object_key.elements ;
        }

    }     /* For each IIOP address in the URL */


    if (addresses != NULL)  free (addresses) ;
    if (key != NULL)  free (key) ;

    return (0) ;


/*******************************************************************************
    Error Return - deallocate any allocated memory.
*******************************************************************************/

onError:
    PUSH_ERRNO ;
    if (addresses != NULL)  free (addresses) ;
    if (key != NULL)  free (key) ;
    comxErase ((ComxFunc) gimxIOR, ior) ;
    POP_ERRNO ;

    return (errno) ;

}

/*!*****************************************************************************

Procedure:

    coliVersion ()

    Get/Set the GIOP Version.


Purpose:

    Function coliVersion() gets or sets the GIOP version used internally
    by the COLI_UTIL package for encoding and sending CORBA messages.


    Invocation:

        giopVersion = coliVersion (versionString) ;

    where

        <versionString>	- I
            is an ASCII string specifying the desired GIOP version (e.g.,
            "1.1", "1.2", "1.3").  If this argument is NULL or if an
            invalid version string was supplied, coliVersion() returns
            the COLI_UTIL package's existing GIOP version.
        <giopVersion>	- O
            returns the GIOP version that will be used by the COLI_UTIL
            package to encode and send CORBA messages.

*******************************************************************************/


Version  coliVersion (

#    if PROTOTYPES
        const  char  *versionString)
#    else
        versionString)

        char  *versionString ;
#    endif

{

/* If no version string is supplied on the first call to coliVersion(),
   use the contents of environment variable, GIOP_VERSION.  If the
   environment variable is not defined, use the default GIOP version
   (defined at the top of this source file). */

    if ((versionString == NULL) &&
        (coli_version.major == 0) &&
        (coli_version.minor == 0)) {
        versionString = getenv ("GIOP_VERSION") ;
        if (versionString == NULL)  versionString = GIOP_VERSION ;
    }

/* If a version string was supplied, then parse the string and set the
   new GIOP version. */

    if (versionString != NULL) {
        char  *s = strchr (versionString, '.') ;
        if (s != NULL) {
            coli_version.major = (octet) atoi (versionString) ;
            coli_version.minor = (octet) atoi (++s) ;
        }
    }

/* Return the new or old version to the caller. */

    return (coli_version) ;

}
