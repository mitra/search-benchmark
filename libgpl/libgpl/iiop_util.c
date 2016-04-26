/* $Id: iiop_util.c,v 1.15 2011/07/18 17:52:01 alex Exp $ */
/*******************************************************************************

File:

    iiop_util.c

    Internet Inter-ORB Protocol (IIOP) Streams.


Author:    Alex Measday


Purpose:

    The IIOP utilities are used to send and receive Internet Inter-ORB
    Protocol (IIOP) messages over TCP/IP network connections.  An IIOP
    message consists of a 12-byte header followed by a message body.
    The contents of the header specify the message type and the length
    of the message body.

    A IIOP stream is created on a previously established network connection.
    The following program implements a simple IIOP server that reads messages
    from a client:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  "tcp_util.h"		-- TCP/IP networking utilities.
        #include  "iiop_util.h"		-- IIOP streams.

        int  main (int argc, char *argv[])
        {
            IiopStream  stream ;
            IiopHeader  header ;
            octet  *body ;
            TcpEndpoint  client, server ;

            tcpListen (argv[1], -1, &server) ;	-- Create listening endpoint.

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                iiopCreate (client, &stream) ;
                for ( ; ; ) {			-- Read messages from client.
                    if (iiopRead (stream, -1.0, &header, &body))  break ;
                    ... do something with the message ...
                }
                iiopDestroy (stream) ;		-- Lost client.
            }

        }

    The server's name is specified as the first argument on the command line
    (i.e., "argv[1]").  If a client connection is broken, the server loops
    back to wait for another client.

    The client program below sends a request to an IIOP server:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  "tcp_util.h"		-- TCP/IP networking utilities.
        #include  "iiop_util.h"		-- IIOP streams.

        int  main (int argc, char *argv[])
        {
            IiopStream  stream ;
            IiopHeader  header ;
            octet  *body ;
            TcpEndpoint  connection ;

            tcpCall (argv[1], 0, &connection) ;	-- Call server.
            iiopCreate (connection, &stream) ;
            ... set up message header and body ...
            if (iiopWrite (stream, -1.0, &header, body)) {
                ... error writing to server ...
            }
            free (body) ;
            if (iiopRead (stream, -1.0, &header, &body)) {
                ... error reading from server ...
            }
            iiopDestroy (stream) ;		-- Drop server.

        }

    Every IIOP message, as transferred over a network connection,
    consists of a 12-byte message header and, optionally, a message
    body encoded using CORBA's Common Data Representation (CDR).
    The message header includes the following fields:

        'G', 'I', 'O', 'P' - in the first 4 bytes.
        Version - specifies the GIOP version number (e.g., 1.0, 1.1, 1.2)
            used in the construction of the message.
        Flags - most importantly, a byte-order flag that is set if the
            encoded data in the message is little-endian.
        Type - is the GIOP::MsgType: Request, Reply, etc.
        Size - is the size in bytes of the message body that follows
            the message header.

    The IiopHeader structure returned by iiopRead() and passed into
    iiopWrite() is a logical structure.  In the case of iiopRead(), the
    fields in the header are set based on the contents of the physical
    12-byte header.  GIOP 1.0- and 1.1-specific fields are converted to
    their 1.2 counterparts and the message size is converted from the
    message byte order to host byte order.  (GIOP 1.2 was the latest
    GIOP version at the time this software was written.)  In the case
    of iiopWrite(), the physical 12-byte header is constructed from the
    logical fields; the host byte order is assumed for message byte
    order and GIOP 1.2-specific fields are converted to their earlier
    counterparts depending on the version number specified in the
    logical header.

    The message body contains data encoded in CDR according to the
    GIOP version number in the header and in the byte order (big-
    or little-endian) indicated by the flag.  See the CORBA Marshaling
    utilities (COMX_UTIL) for CDR decoding and encoding functions.
    As an example of a message body, consider the Request message.
    The message body consists of:

        Request Header - specifying the target object of the request
            the operation to be performed.
        Arguments - are the values, encoded in CDR, of the input and
            input/output parameters expected by the operation.

    The similar Reply message body consists of:

        Reply Header - specifying the completion status of the
            requested operation.
        Return Values - are the return values, encoded in CDR, of
            the input/output and output parameters resulting from
            successful completion of the operation.
        Function Value - is the returned function value, if any,
            from the operation.

    Both iiopRead() and iiopWrite() take a timeout argument that allows
    the application to limit the amount of time these routines wait to
    perform their respective functions.

    In event-driven applications (e.g., those based on the X Toolkit or
    the "nix_util.c" dispatcher), the socket connection underlying the
    IIOP stream, returned by iiopFd(), can be monitored for input by your
    event dispatcher.  Because input is buffered, the input callback must
    repeatedly call iiopRead() while iiopIsReadable() is true.

    When a IIOP stream is no longer needed, a single call will close the
    network connection and discard the stream:

        iiopDestroy (stream) ;


Public Procedures:

    iiopCreate() - creates an IIOP stream.
    iiopDestroy() - deletes an IIOP network stream.
    iiopFd() - returns an IIOP stream's socket number.
    iiopGetContexts() - returns a stream's service contexts.
    iiopIsReadable() - checks if input is waiting to be read from a stream.
    iiopIsUp() - checks if an IIOP stream is up.
    iiopIsWriteable() - checks if data can be written to a stream.
    iiopName() - returns the name of an IIOP stream.
    iiopRead() - reads the next message from an IIOP stream.
    iiopRequestID() - gets the next request ID.
    iiopSetContexts() - sets a stream's service contexts.
    iiopWrite() - writes a message to an IIOP stream.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "bio_util.h"			/* Buffered I/O streams. */
#include  "coli_util.h"			/* CORBA-Lite utilities. */
#include  "net_util.h"			/* Networking utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "iiop_util.h"			/* Internet Inter-ORB Protocol streams. */


/*******************************************************************************
    Internet Inter-ORB Protocol Stream - contains information about
        an IIOP stream, including the underlying network connection
        and other attributes.
*******************************************************************************/

typedef  struct  _IiopStream {
    TcpEndpoint  connection ;		/* TCP/IP connection. */
    BioStream  input ;			/* Buffered input stream. */
    char  *inbuf ;			/* Current input message. */
    size_t  insize ;			/* Allocated size of input buffer. */
    char  *outbuf ;			/* Current output message. */
    size_t  outsize ;			/* Allocated size of output buffer. */
    unsigned  long  requestID ;		/* Incremented for each message sent. */
    ServiceContextList  *contexts ;	/* CORBA service contexts. */
}  _IiopStream ;


int  iiop_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  iiop_util_debug


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
#    define  FIRST_BYTE(value)  (*((unsigned char *) &(value)))
#    define  __LITTLE_ENDIAN  1234
#    define  __BIG_ENDIAN  4321
#    define  __PDP_ENDIAN  3412
#    define  __BYTE_ORDER						\
         ((FIRST_BYTE (endian_value) == 0x11) ? __BIG_ENDIAN :		\
          (FIRST_BYTE (endian_value) == 0x44) ? __LITTLE_ENDIAN :	\
          (FIRST_BYTE (endian_value) == 0x22) ? __PDP_ENDIAN : 0)
#endif

/*!*****************************************************************************

Procedure:

    iiopCreate ()

    Create an IIOP Stream.


Purpose:

    Function iiopCreate() creates an Internet Inter-ORB Protocol stream
    on top of a previously-created network connection.


    Invocation:

        status = iiopCreate (connection, &stream) ;

    where

        <connection>	- I
            is the previously-created network endpoint for the underlying
            network connection.  (See "tcp_util.c" for more information
            about network endpoints.)  NOTE that the "connection" endpoint
            is automatically destroyed (i.e., the socket is closed) when
            the IIOP stream is destroyed.
        <stream>	- O
            returns a handle for the new IIOP stream.  This handle is used
            in calls to the other IIOP functions.
        <status>	- O
            returns the status of creating the stream, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  iiopCreate (

#    if PROTOTYPES
        TcpEndpoint  connection,
        IiopStream  *stream)
#    else
        connection, stream)

        TcpEndpoint  connection ;
        IiopStream  *stream ;
#    endif

{

/* Create and initialize an IIOP stream structure for the network connection. */

    *stream = (_IiopStream *) malloc (sizeof (_IiopStream)) ;
    if (*stream == NULL) {
        LGE "(iiopCreate) Error allocating stream structure for \"%s\".\nmalloc: ",
            tcpName (connection)) ;
        return (errno) ;
    }

    (*stream)->connection = connection ;
    (*stream)->input = NULL ;
    (*stream)->inbuf = NULL ;
    (*stream)->insize = 0 ;
    (*stream)->outbuf = NULL ;
    (*stream)->outsize = 0 ;
    (*stream)->requestID = 0 ;
    (*stream)->contexts = NULL ;

/* Buffer input on the network connection. */

    if (bioCreate (connection, (BioInputF) tcpRead, 0, NULL, 0,
                   &(*stream)->input)) {
        LGE "(iiopCreate) Error creating buffered input stream for \"%s\".\nbioCreate: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;  iiopDestroy (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(iiopCreate) Created IIOP network stream %s, socket %d\n",
        iiopName (*stream), iiopFd (*stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    iiopDestroy ()

    Delete an IIOP Stream.


Purpose:

    Function iiopDestroy() destroys an IIOP stream.  The underlying network
    connection is closed and the IIOP stream structure is deallocated.


    Invocation:

        status = iiopDestroy (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <status>	- O
            returns the status of deleting the stream, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  iiopDestroy (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{

    if (stream == NULL)  return (0) ;

    LGI "(iiopDestroy) Closing %s stream ...\n", iiopName (stream)) ;

/* Close the buffered input stream. */

    bioDestroy (stream->input) ;

/* Close the underlying network connection. */

    tcpDestroy (stream->connection) ;

/* Deallocate the IIOP stream structure. */

    if (stream->inbuf != NULL) {
        free (stream->inbuf) ;
        stream->inbuf = NULL ;
    }
    if (stream->outbuf != NULL) {
        free (stream->outbuf) ;
        stream->outbuf = NULL ;
    }
    iiopSetContexts (stream, NULL) ;
    free (stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    iiopFd ()

    Get an IIOP Stream's Socket.


Purpose:

    Function iiopFd() returns the Unix file descriptor for the socket
    connection associated with an IIOP stream.


    Invocation:

        fd = iiopFd (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <fd>		- O
            returns the UNIX file descriptor for the stream's socket.

*******************************************************************************/


IoFd  iiopFd (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{
    return ((stream == NULL) ? INVALID_SOCKET : tcpFd (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    iiopGetContexts ()

    Get an IIOP Stream's Service Context List.


Purpose:

    Function iiopGetContexts() returns the list of CORBA service contexts
    currently associated with outgoing requests on this stream.


    Invocation:

        list = iiopGetContexts (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <list>		- O
            returns the address of the service context list.  The caller
            should NOT deallocate the list using comxErase().  NULL is
            returned if no context list is bound to the stream.

*******************************************************************************/


ServiceContextList  *iiopGetContexts (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{

    return ((stream == NULL) ? NULL : stream->contexts) ;

}

/*!*****************************************************************************

Procedure:

    iiopIsReadable ()

    Check if Data is Waiting to be Read.


Purpose:

    The iiopIsReadable() function checks to see if data is waiting to
    be read from an IIOP stream.


    Invocation:

        isReadable = iiopIsReadable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  iiopIsReadable (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{
    if (stream == NULL)
        return (false) ;
    else if (bioPendingInput (stream->input))		/* Buffered input? */
        return (true) ;
    else
        return (tcpIsReadable (stream->connection)) ;	/* Real input? */
}

/*!*****************************************************************************

Procedure:

    iiopIsUp ()

    Check if a Stream's Network Connection is Up.


Purpose:

    The iiopIsUp() function checks to see if an IIOP stream's underlying
    network connection is still up.


    Invocation:

        isUp = iiopIsUp (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <isUp>		- O
            returns true (a non-zero value) if the stream's network
            connection is up and false (zero) otherwise.

*******************************************************************************/


bool  iiopIsUp (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsUp (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    iiopIsWriteable ()

    Check if a Stream is Ready for Writing.


Purpose:

    The iiopIsWriteable() function checks to see if data can be written to
    an IIOP stream.


    Invocation:

        isWriteable = iiopIsWriteable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <isWriteable>	- O
            returns true (a non-zero value) if the IIOP stream is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  iiopIsWriteable (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsWriteable (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    iiopName ()

    Get an IIOP Stream's Name.


Purpose:

    Function iiopName() returns an IIOP stream's name.


    Invocation:

        name = iiopName (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <name>		- O
            returns the stream's name.  The name is stored in memory local
            to the IIOP utilities and it should not be modified or freed by
            the caller.

*******************************************************************************/


const  char  *iiopName (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{
    return ((stream == NULL) ? "" : tcpName (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    iiopRead ()

    Read the Next Message from an IIOP Stream.


Purpose:

    Function iiopRead() reads the next message from an IIOP network stream.
    The timeout argument provides a way of limiting how long iiopRead()
    waits to *begin* reading a message.


    Invocation:

        status = iiopRead (stream, timeout, &header, &body) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the next message to be read.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) allows a read only if input is
            immediately available.
        <header>	- O
            returns the message header.  The numeric fields in the header
            are in host-byte order, as opposed to the network-byte order
            in which they were transferred over the network.
        <body>		- O
            returns a pointer to the message body; NULL is returned if the
            message has no body.  The message body must be used or duplicated
            before calling iiopRead() again.  If the caller is not interested
            in the message body, specify this argument as NULL, in which case
            the message body will be discarded.
        <status>	- O
            returns the status of reading a message, zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired with
            no input, and ERRNO otherwise.

*******************************************************************************/


errno_t  iiopRead (

#    if PROTOTYPES
        IiopStream  stream,
        double  timeout,
        IiopHeader  *header,
        octet  **body)
#    else
        stream, timeout, header, body)

        IiopStream  stream ;
        double  timeout ;
        IiopHeader  *header ;
        octet  **body ;
#    endif

{    /* Local variables. */
    unsigned  char  buffer[12] ;




    if (body != NULL)  *body = NULL ;

    if ((stream == NULL) || (header == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(iiopRead) NULL stream handle or header: ") ;
        return (errno) ;
    }

/* Read the message header. */

    if (bioRead (stream->input, timeout, sizeof buffer,
                 (char *) buffer, NULL)) {
        if (errno != EWOULDBLOCK) {
            LGE "(iiopRead) Error reading message header from %s.\nbioRead: ",
                iiopName (stream)) ;
        }
        return (errno) ;
    }

/* Transfer the fields to the caller's header structure.  If the message's
   endianness differs from that of the host CPU, the size field in the
   message's header is converted to the host CPU's endianness. */

    header->GIOP_version.major = (octet) buffer[4] ;
    header->GIOP_version.minor = (octet) buffer[5] ;
    header->flags = (unsigned short) buffer[6] ;
    header->message_type = (GIOPMsgType) buffer[7] ;

    if (header->flags & ENDIAN_MASK) {
        header->message_size = ((unsigned long) buffer[11] << 24) |
                               ((unsigned long) buffer[10] << 16) |
                               ((unsigned long) buffer[9] << 8) |
                               ((unsigned long) buffer[8]) ;
    } else {
        header->message_size = ((unsigned long) buffer[8] << 24) |
                               ((unsigned long) buffer[9] << 16) |
                               ((unsigned long) buffer[10] << 8) |
                               ((unsigned long) buffer[11]) ;
    }

    LGI "(iiopRead) %s header - Version: %d.%d  Flags: 0x%02X  Type: %s  Size: %lu\n",
        iiopName (stream), (int) header->GIOP_version.major,
        (int) header->GIOP_version.minor,  (int) header->flags,
        coliToName (GIOPMsgTypeLUT, (long) header->message_type),
        header->message_size) ;

/* Check the header for "GIOP". */

    if ((buffer[0] != 'G') || (buffer[1] != 'I') ||
        (buffer[2] != 'O') || (buffer[3] != 'P')) {
        SET_ERRNO (EINVAL) ;
        LGE "(iiopRead) Invalid GIOP header: 0x%02X%02X%02X%02X\n",
            (int) buffer[0], (int) buffer[1],
            (int) buffer[2], (int) buffer[3]) ;
        return (errno) ;
    }

/* If necessary, allocate a buffer for the message body. */

    if ((size_t) header->message_size > stream->insize) {
        if (stream->inbuf != NULL)  free (stream->inbuf) ;
        stream->insize = 0 ;
        stream->inbuf = malloc (header->message_size) ;
        if (stream->inbuf == NULL) {
            LGE "(iiopRead) Unable to allocate %lu-byte input buffer for %s.\n",
                header->message_size, iiopName (stream)) ;
            return (errno) ;
        }
        stream->insize = header->message_size ;
    }

/* Read the message body (if there is one). */

    if (header->message_size > 0) {
        if (bioRead (stream->input, -1.0, header->message_size,
                     stream->inbuf, NULL)) {
            if (errno != EWOULDBLOCK) {
                LGE "(iiopRead) Error reading %lu-byte message body from %s.\nbioRead: ",
                    header->message_size, iiopName (stream)) ;
            }
            return (errno) ;
        }
        if (body != NULL)  *body = (octet *) stream->inbuf ;
    }

    LGI "(iiopRead) %lu-byte (%ld) message from %s.\n",
        header->message_size, (long) header->message_type, iiopName (stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    iiopRequestID ()

    Get an IIOP Stream's Next Request ID.


Purpose:

    Function iiopRequestID() returns a GIOP request ID that clients
    can use to tag requests.  The request ID is initialized to zero
    when the IIOP stream is created and is incremented after each
    call to iiopWrite().


    Invocation:

        id = iiopRequestID (stream) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <id>		- O
            returns a GIOP request ID.

*******************************************************************************/


unsigned  long  iiopRequestID (

#    if PROTOTYPES
        IiopStream  stream)
#    else
        stream)

        IiopStream  stream ;
#    endif

{
    return ((stream == NULL) ? 0 : stream->requestID) ;
}

/*!*****************************************************************************

Procedure:

    iiopSetContexts ()

    Set an IIOP Stream's Service Context List.


Purpose:

    Function iiopSetContexts() binds a list of CORBA service contexts to an
    IIOP stream; the prior list, if any, is discarded.  Service contexts are
    additional (and optional) information that can be sent along in requests
    to a CORBA service; see coliRequest().  Service contexts don't have
    anything to do with IIOP especially, but the IiopStream seemed to be
    a useful holder for the service context list.  An application or library
    can create an IIOP stream, create a service context list, bind the list
    to the stream, and then retrieve it whenever needed using iiopGetContexts().
    The list will be automatically deallocated when the stream is destroyed.


    Invocation:

        status = iiopSetContexts (stream, list) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <list>		- I
            is the address of the new service context list.  The existing list
            is deallocated and the IIOP stream takes possession of the new list.
            NULL is an acceptable argument.
        <status>	- O
            returns the status of binding the list to the stream, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  iiopSetContexts (

#    if PROTOTYPES
        IiopStream  stream,
        ServiceContextList  *contexts)
#    else
        stream, contexts)

        IiopStream  stream ;
        ServiceContextList  *contexts ;
#    endif

{

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(iiopSetContexts) NULL stream handle: ") ;
        return (errno) ;
    }

/* Deallocate the existing list of service contexts. */

    if (stream->contexts != NULL)
        comxErase ((ComxFunc) gimxServiceContextList, stream->contexts) ;

/* Bind the new list to the stream. */

    stream->contexts = contexts ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    iiopWrite ()

    Write a Message to an IIOP Stream.


Purpose:

    Function iiopWrite() writes a message to an IIOP network stream.  The
    timeout argument provides a way of limiting how long iiopWrite() waits
    to *begin* outputting the message.


    Invocation:

        status = iiopWrite (stream, timeout, header, body) ;

    where

        <stream>	- I
            is the stream handle returned by iiopCreate().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the message to be written.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) only writes the message if it can
            be output immediately.
        <header>	- I
            is a pointer to the IIOP header of the message being written.
            The big-/little-endian flag bit in the outgoing header is set
            automatically for the host CPU's architecture.
        <body>		- I
            is the message body, the length of which is specified in the
            message header.  NULL can be specified if there is no message
            body.
        <status>	- O
            returns the status of writing the message, zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired with
            no output, and ERRNO otherwise.

*******************************************************************************/


errno_t  iiopWrite (

#    if PROTOTYPES
        IiopStream  stream,
        double  timeout,
        const  IiopHeader  *header,
        const  octet  *body)
#    else
        stream, timeout, header, body)

        IiopStream  stream ;
        double  timeout ;
        IiopHeader  *header ;
        octet  *body ;
#    endif

{    /* Local variables. */
    size_t  length ;




    if ((stream == NULL) || (header == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(iiopWrite) NULL stream handle or header: ") ;
        return (errno) ;
    }

/* Increment the GIOP request ID for this IIOP stream.  Client applications
   can use the request ID to tag the requests they send to servers. */

    stream->requestID++ ;

/* Compute the length of the full message (header + body) and allocate
   a buffer to hold it. */

    length = 12  +  ((body == NULL) ? 0 : header->message_size) ;

    if (length > stream->outsize) {
        if (stream->outbuf != NULL)  free (stream->outbuf) ;
        stream->outsize = 0 ;
        stream->outbuf = malloc (length) ;
        if (stream->outbuf == NULL) {
            LGE "(iiopWrite) Unable to allocate %ld-byte output buffer for %s.\n",
                (long) length, iiopName (stream)) ;
            return (errno) ;
        }
        stream->outsize = length ;
    }

/* Construct the GIOP message header in the output buffer.  The enumerated
   message type is narrowed to a byte and the big-/little-endian flag bit
   is set appropriately. */

    (void) memcpy (stream->outbuf, "GIOP", 4) ;
    stream->outbuf[4] = header->GIOP_version.major ;
    stream->outbuf[5] = header->GIOP_version.minor ;
    stream->outbuf[6] =
        (octet) ((header->flags & ~ENDIAN_MASK) |
                 ((__BYTE_ORDER == __LITTLE_ENDIAN) ? ENDIAN_MASK : 0)) ;
    stream->outbuf[7] = (octet) header->message_type ;
    if (__BYTE_ORDER == __LITTLE_ENDIAN) {
        stream->outbuf[8] = (octet) (header->message_size & 0x0FF) ;
        stream->outbuf[9] = (octet) ((header->message_size >> 8) & 0x0FF) ;
        stream->outbuf[10] = (octet) ((header->message_size >> 16) & 0x0FF) ;
        stream->outbuf[11] = (octet) ((header->message_size >> 24) & 0x0FF) ;
    } else {
        stream->outbuf[8] = (octet) ((header->message_size >> 24) & 0x0FF) ;
        stream->outbuf[9] = (octet) ((header->message_size >> 16) & 0x0FF) ;
        stream->outbuf[10] = (octet) ((header->message_size >> 8) & 0x0FF) ;
        stream->outbuf[11] = (octet) (header->message_size & 0x0FF) ;
    }

/* Append the message body to the header. */

    if ((header->message_size > 0) && (body != NULL)) {
        (void) memcpy (&stream->outbuf[sizeof (MessageHeader)],
                       body, header->message_size) ;
    }

/* Output the complete message to the network. */

    if (tcpWrite (stream->connection, timeout, length, stream->outbuf, NULL)) {
        LGE "(iiopWrite) Error writing %d-byte message to %s.\ntcpWrite: ",
            length, iiopName (stream)) ;
        return (errno) ;
    }

    LGI "(iiopWrite) %lu-byte message (%s) to %s.\n",
        (unsigned long) length,
        coliToName (GIOPMsgTypeLUT, (long) header->message_type),
        iiopName (stream)) ;

    return (0) ;

}
