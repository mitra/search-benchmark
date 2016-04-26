/* $Id: vim_util.c,v 1.11 2012/07/03 17:21:31 alex Exp $ */
/*******************************************************************************

File:

    vim_util.c

    Version-Independent Message Streams.


Author:    Alex Measday


Purpose:

    The VIM utilities are used to send and receive "version-independent
    messages" over TCP/IP network connections.  A VIM message consists
    of a 12-byte header followed by a list of zero or more, XDR-encoded
    name/value pairs (see "nvp_util.c").  The contents of the header
    specify a message ID and the length of the message body.

    A VIM stream is created on a previously established network connection.
    The following program implements a simple VIM server that periodically
    sends the time-of-day to a client:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  <unistd.h>		-- sleep(3) definition.
        #include  "tcp_util.h"		-- TCP/IP networking utilities.
        #include  "tv_util.h"		-- "timeval" manipulation functions.
        #include  "vim_util.h"		-- Version-independent message streams.

        int  main (int argc, char *argv[])
        {
            VimStream  stream ;
            VimHeader  header ;
            NVList  list ;
            TcpEndpoint  client, server ;

            tcpListen (argv[1], -1, &server) ;	-- Create listening endpoint.

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                vimCreate (client, &stream) ;
                while (vimIsUp (client)) {	-- Send times to client.
                    nvlCreate (NULL, &list) ;
                    nvlAdd (list, nvpNew ("TIME", NvpTime, tvTOD ())) ;
                    header.ID = 0 ;  header.parameter = NULL ;
                    vimWriteList (stream, -1.0, &header, list) ;
                    nvlDestroy (list) ;
                    sleep (1) ;
                }
                vimDestroy (stream) ;		-- Lost client.
            }

        }

    The server's name is specified as the first argument on the command line
    (i.e., "argv[1]").  If a client connection is broken, the server loops
    back to wait for another client.

    The client program below reads and displays the time-of-day messages from
    the VIM server:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  "tcp_util.h"		-- TCP/IP networking utilities.
        #include  "vim_util.h"		-- Version-independent message streams.

        int  main (int argc, char *argv[])
        {
            VimStream  stream ;
            VimHeader  header ;
            NVList  list ;
            TcpEndpoint  connection ;

            tcpCall (argv[1], 0, &connection) ;	-- Call server.
            vimCreate (connection, &stream) ;
            for ( ; ; ) {			-- Read times from server.
                if (vimReadList (stream, -1.0, &header, &list))  break ;
                printf ("Server's time = %s\n", nvpString (nvlFind ("TIME"))) ;
                nvlDestroy (list) ;
            }
            vimDestroy (stream) ;		-- Lost server.

        }

    The VIM message header contains an integer ID field and a (void *)
    parameter field that can be used by clients and servers for message
    identification and tagging; the VIM_UTIL package does not use these
    fields.  Both vimRead() and vimWrite() take a timeout argument that
    allows the application to limit the amount of time these routines
    wait to perform their respective functions.

    In event-driven applications (e.g., those based on the X Toolkit or
    the "iox_util.c" dispatcher), the socket connection underlying the
    VIM stream, returned by vimFd(), can be monitored for input by your
    event dispatcher.  Because input is buffered, the input callback must
    repeatedly call vimRead() while vimIsReadable() is true.

    When a VIM stream is no longer needed, a single call will close the
    network connection and discard the stream:

        vimDestroy (stream) ;


History:

    The name/value pair, name/value list, and version-independent message
    stream packages were inspired by Mike Maloney's C++ implementations of
    "named variables" and "named variable sets", and by Robert Martin's
    "attributed data trees" (see "Version-Independent Messages" in Appendix B
    of his DESIGNING OBJECTED-ORIENTED C++ APPLICATIONS USING THE BOOCH METHOD).


Public Procedures:

    vimCreate() - creates a version-independent message stream.
    vimDecode() - decodes a list of name/value pairs from a VIM message.
    vimDestroy() - deletes a VIM network stream.
    vimFd() - returns a VIM stream's socket number.
    vimIsReadable() - checks if input is waiting to be read from a stream.
    vimIsUp() - checks if a VIM stream is up.
    vimIsWriteable() - checks if data can be written to a stream.
    vimName() - returns the name of a VIM stream.
    vimRead() - reads the next message from a VIM stream.
    vimReadList() - reads an NVList message from a VIM stream.
    vimWrite() - writes a message to a VIM stream.
    vimWriteList() - writes an NVList message to a VIM stream.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "bio_util.h"			/* Buffered I/O streams. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "vim_util.h"			/* Version-independent message streams. */


/*******************************************************************************
    Version-Independent Message Stream - contains information about
        a message stream, including the underlying network connection
        and other attributes.
*******************************************************************************/

#define  MAX_MSG_SIZE  16384UL

typedef  struct  _VimStream {
    TcpEndpoint  connection ;		/* TCP/IP connection. */
    BioStream  input ;			/* Buffered input stream. */
    char  *inputMessage ;		/* Working storage for vimRead(). */
    size_t  inputLength ;		/* Allocated size of input message. */
    char  *outputMessage ;		/* Working storage for vimWrite(). */
    size_t  outputLength ;		/* Allocated size of output message. */
}  _VimStream ;


int  vim_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  vim_util_debug

/*!*****************************************************************************

Procedure:

    vimCreate ()

    Create a VIM Stream.


Purpose:

    Function vimCreate() creates a version-independent message stream on top
    of a previously-created network connection.


    Invocation:

        status = vimCreate (connection, &stream) ;

    where

        <connection>	- I
            is the previously-created network endpoint for the underlying
            network connection.  (See "tcp_util.c" for more information
            about network endpoints.)  NOTE that the "connection" endpoint
            is automatically destroyed (i.e., the socket is closed) when
            the VIM stream is destroyed.
        <stream>	- O
            returns a handle for the new VIM stream.  This handle is used
            in calls to the other VIM functions.
        <status>	- O
            returns the status of creating the stream, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  vimCreate (

#    if PROTOTYPES
        TcpEndpoint  connection,
        VimStream  *stream)
#    else
        connection, stream)

        TcpEndpoint  connection ;
        VimStream  *stream ;
#    endif

{

/* Create and initialize a VIM stream structure for the network connection. */

    *stream = (_VimStream *) malloc (sizeof (_VimStream)) ;
    if (*stream == NULL) {
        LGE "(vimCreate) Error allocating stream structure for \"%s\".\nmalloc: ",
            tcpName (connection)) ;
        return (errno) ;
    }

    (*stream)->connection = connection ;
    (*stream)->inputMessage = NULL ;
    (*stream)->inputLength = 0 ;
    (*stream)->outputMessage = NULL ;
    (*stream)->outputLength = 0 ;

/* Buffer input on the network connection. */

    if (bioCreate (connection, (BioInputF) tcpRead, 0, NULL, 0,
                   &(*stream)->input)) {
        LGE "(vimCreate) Error creating buffered input stream for \"%s\".\nbioCreate: ",
            tcpName (connection)) ;
        PUSH_ERRNO ;  vimDestroy (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(vimCreate) Created VIM network stream %s, socket %d\n",
        vimName (*stream), vimFd (*stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    vimDecode ()

    Decode a Name/Value Pair List from a VIM Message.


Purpose:

    Function vimDecode() decodes a list of name/value pairs from a
    previouly-read VIM message.


    Invocation:

        list = vimDecode (header, body) ;

    where

        <header>	- I
            is a pointer to the VIM header of the message.
        <body>		- I
            is the message body, the length of which is specified in the
            message header.  NULL can be specified if there is no message
            body.
        <list>		- O
            returns the list of name/value pairs extracted from the body
            of the message.  An empty list is returned if the message has
            no body and NULL is returned in the event of an error.  The
            caller is responsible for calling nvlDestroy() to delete the
            list, if any, when it is no longer needed.

*******************************************************************************/


NVList  vimDecode (

#    if PROTOTYPES
        VimHeader  *header,
        const  char  *body)
#    else
        header, body)

        VimHeader  *header ;
        char  *body ;
#    endif

{    /* Local variables. */
    NVList  list ;
    XDR  memoryStream ;




    if (header == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(vimDecode) NULL header: ") ;
        return (NULL) ;
    }

/* Create an empty list of name/value pairs. */

    if (nvlCreate (NULL, &list)) {
        LGE "(vimDecode) Error creating name/value list.\nnvlCreate: ") ;
        return (NULL) ;
    }

    if ((body == NULL) || (header->length <= 0))  return (list) ;

/* Create an XDR stream based on the message body. */

    xdrmem_create (&memoryStream, (char *) body, header->length, XDR_DECODE) ;

/* Decode the list of name/value pairs in the message body. */

    if (!xdr_NVList (&memoryStream, &list)) {
        if (errno == 0)  SET_ERRNO (EINVAL) ;
        LGE "(vimDecode) Error decoding name/value list.\nxdr_NVList: ") ;
        PUSH_ERRNO ;  nvlDestroy (list) ;  POP_ERRNO ;
        return (NULL) ;
    }

    return (list) ;

}

/*!*****************************************************************************

Procedure:

    vimDestroy ()

    Delete a VIM Stream.


Purpose:

    Function vimDestroy() destroys a VIM stream.  The underlying network
    connection is closed and the VIM stream structure is deallocated.


    Invocation:

        status = vimDestroy (stream) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <status>	- O
            returns the status of deleting the stream, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  vimDestroy (

#    if PROTOTYPES
        VimStream  stream)
#    else
        stream)

        VimStream  stream ;
#    endif

{

    if (stream == NULL)  return (0) ;

    LGI "(vimDestroy) Closing %s stream ...\n", vimName (stream)) ;

/* Close the buffered input stream. */

    bioDestroy (stream->input) ;

/* Close the underlying network connection. */

    tcpDestroy (stream->connection) ;

/* Deallocate the I/O buffers. */

    if (stream->inputMessage != NULL) {
        free (stream->inputMessage) ;
        stream->inputMessage = NULL ;
    }

    if (stream->outputMessage != NULL) {
        free (stream->outputMessage) ;
        stream->outputMessage = NULL ;
    }

/* Deallocate the VIM stream structure. */

    free (stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    vimFd ()

    Get a VIM Stream's Socket.


Purpose:

    Function vimFd() returns the Unix file descriptor for the socket
    connection associated with a VIM stream.


    Invocation:

        fd = vimFd (stream) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <fd>		- O
            returns the UNIX file descriptor for the stream's socket.

*******************************************************************************/


IoFd  vimFd (

#    if PROTOTYPES
        VimStream  stream)
#    else
        stream)

        VimStream  stream ;
#    endif

{
    return ((stream == NULL) ? INVALID_SOCKET : tcpFd (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    vimIsReadable ()

    Check if Data is Waiting to be Read.


Purpose:

    The vimIsReadable() function checks to see if data is waiting to
    be read from a VIM stream.


    Invocation:

        isReadable = vimIsReadable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  vimIsReadable (

#    if PROTOTYPES
        VimStream  stream)
#    else
        stream)

        VimStream  stream ;
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

    vimIsUp ()

    Check if a Stream's Network Connection is Up.


Purpose:

    The vimIsUp() function checks to see if a VIM stream's underlying
    network connection is still up.


    Invocation:

        isUp = vimIsUp (stream) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <isUp>		- O
            returns true (a non-zero value) if the stream's network
            connection is up and false (zero) otherwise.

*******************************************************************************/


bool  vimIsUp (

#    if PROTOTYPES
        VimStream  stream)
#    else
        stream)

        VimStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsUp (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    vimIsWriteable ()

    Check if a Stream is Ready for Writing.


Purpose:

    The vimIsWriteable() function checks to see if data can be written to
    a VIM stream.


    Invocation:

        isWriteable = vimIsWriteable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <isWriteable>	- O
            returns true (a non-zero value) if the VIM stream is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  vimIsWriteable (

#    if PROTOTYPES
        VimStream  stream)
#    else
        stream)

        VimStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsWriteable (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    vimName ()

    Get a VIM Stream's Name.


Purpose:

    Function vimName() returns a VIM stream's name.


    Invocation:

        name = vimName (stream) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <name>		- O
            returns the stream's name.  The name is stored in memory local
            to the VIM utilities and it should not be modified or freed by
            the caller.

*******************************************************************************/


const  char  *vimName (

#    if PROTOTYPES
        VimStream  stream)
#    else
        stream)

        VimStream  stream ;
#    endif

{
    return ((stream == NULL) ? "" : tcpName (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    vimRead ()

    Read the Next Message from a VIM Stream.


Purpose:

    Function vimRead() reads the next message from a VIM network stream.
    The timeout argument provides a way of limiting how long vimRead()
    waits to *begin* reading a message.


    Invocation:

        status = vimRead (stream, timeout, &header, &body) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
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
            message has no body (e.g., a simple command with no accompanying
            data).  The message body must be used or duplicated before calling
            vimRead() again.  If the caller is not interested in the message
            body, specify this argument as NULL, in which case the message
            body will be discarded.
        <status>	- O
            returns the status of reading a message, zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired with
            no input, and ERRNO otherwise.

*******************************************************************************/


errno_t  vimRead (

#    if PROTOTYPES
        VimStream  stream,
        double  timeout,
        VimHeader  *header,
        char  **body)
#    else
        stream, timeout, header, body)

        VimStream  stream ;
        double  timeout ;
        VimHeader  *header ;
        char  **body ;
#    endif

{

    if (body != NULL)  *body = NULL ;

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(vimRead) NULL stream handle: ") ;
        return (errno) ;
    }

/* Read the message header. */

    if (bioRead (stream->input, timeout, sizeof (VimHeader),
                 (char *) header, NULL)) {
        if (errno != EWOULDBLOCK) {
            LGE "(vimRead) Error reading message header from %s.\nbioRead: ",
                vimName (stream)) ;
        }
        return (errno) ;
    }

/* Convert the fields in the message header to host-byte order. */

    header->ID = ntohl (header->ID) ;
    header->parameter = (void *) ntohl ((long) header->parameter) ;
    header->length = ntohl (header->length) ;

/* Verify that the message body will fit in the input buffer. */

    if (header->length > MAX_MSG_SIZE) {
        SET_ERRNO (ENOMEM) ;
        LGE "(vimRead) %ld-byte message would overflow %s's input buffer.\n",
            header->length, vimName (stream)) ;
        return (errno) ;
    }

/* Allocate a buffer for the incoming message. */

    if ((stream->inputMessage == NULL) ||
        (header->length > (long) stream->inputLength)) {

        if (stream->inputMessage != NULL) {
            free (stream->inputMessage) ;
            stream->inputMessage = NULL ;
            stream->inputLength = 0 ;
        }

        stream->inputMessage = malloc (header->length) ;
        if (stream->inputMessage == NULL) {
            LGE "(vimRead) Error allocating %ld-byte buffer for incoming messsage from %s.\n",
                header->length, vimName (stream)) ;
            return (errno) ;
        }

        stream->inputLength = header->length ;

    }

/* Read the message body. */

    if (bioRead (stream->input, -1.0, header->length,
                 stream->inputMessage, NULL)) {
        if (errno != EWOULDBLOCK) {
            LGE "(vimRead) Error reading message body from %s.\nbioRead: ",
                vimName (stream)) ;
        }
        return (errno) ;
    }

    if (body != NULL)  *body = stream->inputMessage ;

    LGI "(vimRead) %ld-byte message (%ld,%p) from %s.\n",
        header->length, header->ID, header->parameter, vimName (stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    vimReadList ()

    Read an NVList Message from a VIM Stream.


Purpose:

    Function vimReadList() reads the next message from a VIM network stream
    and interprets the message body as a list of name/value pairs.  The timeout
    argument provides a way of limiting how long vimReadList() waits to *begin*
    reading a message.


    Invocation:

        status = vimReadList (stream, timeout, &header, &list) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
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
        <list>		- O
            returns the body of the message as a list of name/value pairs.
            NULL is returned if the message has no body (e.g., a simple
            command with no accompanying data).  The caller is responsible
            for calling nvlDestroy() to delete the list, if any, when it
            is no longer needed.  If the caller is not interested in the
            message body, specify this argument as NULL, in which case the
            message body will be discarded.
        <status>	- O
            returns the status of reading a message, zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired with
            no input, and ERRNO otherwise.

*******************************************************************************/


errno_t  vimReadList (

#    if PROTOTYPES
        VimStream  stream,
        double  timeout,
        VimHeader  *header,
        NVList  *list)
#    else
        stream, timeout, header, list)

        VimStream  stream ;
        double  timeout ;
        VimHeader  *header ;
        NVList  *list ;
#    endif

{    /* Local variables. */
    char  *body ;



    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(vimReadList) NULL stream handle: ") ;
        return (errno) ;
    }

/* Read the message. */

    if (vimRead (stream, timeout, header, &body)) {
        LGE "(vimReadList) Error reading message from %s.\nvimRead: ",
            vimName (stream)) ;
        return (errno) ;
    }

/* If the caller doesn't want to see the message body or there is no message
   body, return immediately. */

    if (list == NULL) {
        return (0) ;
    } else if ((body == NULL) || (header->length <= 0)) {
        *list = NULL ;
        return (0) ;
    }

/* Otherwise, decode the list of name/value pairs in the message body. */

    *list = vimDecode (header, body) ;
    if (*list == NULL) {
        LGE "(vimReadList) Error decoding name/value list for %s.\nvimDecode: ",
            vimName (stream)) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    vimWrite ()

    Write a Message to a VIM Stream.


Purpose:

    Function vimWrite() writes a message to a VIM network stream.  The timeout
    argument provides a way of limiting how long vimWrite() waits to *begin*
    outputting the message.


    Invocation:

        status = vimWrite (stream, timeout, header, body) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the message to be written.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) only writes the message if it can
            be output immediately.
        <header>	- I
            is a pointer to the VIM header of the message being written.
            The numeric fields in the header should be in host-byte order;
            they will be automatically converted to network-byte order when
            transferred over the network.
        <body>		- I
            is the message body, the length of which is specified in the
            message header.  NULL can be specified if there is no message
            body.
        <status>	- O
            returns the status of writing the message, zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired with
            no output, and ERRNO otherwise.

*******************************************************************************/


errno_t  vimWrite (

#    if PROTOTYPES
        VimStream  stream,
        double  timeout,
        const  VimHeader  *header,
        const  char  *body)
#    else
        stream, timeout, header, body)

        VimStream  stream ;
        double  timeout ;
        VimHeader  *header ;
        char  *body ;
#    endif

{    /* Local variables. */
    size_t  length ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(vimWrite) NULL stream handle: ") ;
        return (errno) ;
    }

    length = sizeof (VimHeader) + ((body == NULL) ? 0 : header->length) ;
    if (length > MAX_MSG_SIZE) {
        SET_ERRNO (EINVAL) ;
        LGE "(vimWrite) %ld-byte message is too large for %s.\n",
            length, vimName (stream)) ;
        return (errno) ;
    }

/* Allocate a buffer for the outgoing message. */

    if ((stream->outputMessage == NULL) ||  (length > stream->outputLength)) {

        if (stream->outputMessage != NULL) {
            free (stream->outputMessage) ;
            stream->outputMessage = NULL ;
            stream->outputLength = 0 ;
        }

        stream->outputMessage = malloc (length) ;
        if (stream->outputMessage == NULL) {
            LGE "(vimWrite) Error allocating %ld-byte buffer for outgoing message to %s.\n",
                length, vimName (stream)) ;
            return (errno) ;
        }

        stream->outputLength = length ;

    }

/* Place the message header in the output buffer.  Convert numeric fields in
   the header from host-byte-order to network-byte-order. */

    ((VimHeader *) stream->outputMessage)->ID =
        htonl (header->ID) ;
    ((VimHeader *) stream->outputMessage)->parameter =
        (void *) htonl ((long) header->parameter) ;
    ((VimHeader *) stream->outputMessage)->length =
        htonl (length - sizeof (VimHeader)) ;

/* Append the message body to the header. */

    if ((length > sizeof (VimHeader)) && (body != NULL)) {
        (void) memcpy (&stream->outputMessage[sizeof (VimHeader)], body,
                       length - sizeof (VimHeader)) ;
    }

/* Output the complete message to the network. */

    if (tcpWrite (stream->connection, timeout, length, stream->outputMessage, NULL)) {
        LGE "(vimWrite) Error writing %ld-byte message to %s.\ntcpWrite: ",
            (long) length, vimName (stream)) ;
        return (errno) ;
    }

    LGI "(vimWrite) %ld-byte message (%ld,%p) to %s.\n",
        (long) length, header->ID, header->parameter, vimName (stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    vimWriteList ()

    Write an NVList Message to a VIM Stream.


Purpose:

    Function vimWriteList() writes a message whose body is a list of
    name/value pairs to a network stream.  The timeout argument provides
    a way of limiting how long vimWriteList() waits to *begin* outputting
    the message.


    Invocation:

        status = vimWriteList (stream, timeout, header, list) ;

    where

        <stream>	- I
            is the stream handle returned by vimCreate().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the message to be written.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) only writes the message if it can
            be output immediately.
        <header>	- I
            is a pointer to the VIM header of the message being written.
            The numeric fields in the header should be in host-byte order;
            they will be automatically converted to network-byte order when
            transferred over the network.
        <list>		- I
            is the body of the message, represented as a list of name/value
            pairs.  Either an empty list or a NULL list handle can be
            specified if there is no message body.
        <status>	- O
            returns the status of writing the message, zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired with
            no output, and ERRNO otherwise.

*******************************************************************************/


errno_t  vimWriteList (

#    if PROTOTYPES
        VimStream  stream,
        double  timeout,
        VimHeader  *header,
        NVList  list)
#    else
        stream, timeout, header, list)

        VimStream  stream ;
        double  timeout ;
        VimHeader  *header ;
        NVList  list ;
#    endif

{    /* Local variables. */
    char  *buffer ;
    long  length ;
    XDR  memoryStream ;



    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(vimWriteList) NULL stream handle: ") ;
        return (errno) ;
    }

/* Allocate a buffer for the encoded name/value pairs. */

    length = MAX_MSG_SIZE ;
    buffer = malloc (length) ;
    if (buffer == NULL) {
        LGE "(vimWriteList) Error allocating %ld-byte buffer for encoding name/value pairs for %s.\n",
            length, vimName (stream)) ;
        return (errno) ;
    }

/* Create an XDR stream based on the output buffer. */

    xdrmem_create (&memoryStream, buffer, sizeof buffer, XDR_ENCODE) ;

/* Encode the list of name/value pairs into the output buffer. */

    if (!xdr_NVList (&memoryStream, &list)) {
        LGE "(vimWriteList) Error encoding name/value list %s on %s.\nxdr_NVList: ",
            nvlName (list), vimName (stream)) ;
        PUSH_ERRNO ;  free (buffer) ;  POP_ERRNO ;
        return (errno) ;
    }

    header->length = xdr_getpos (&memoryStream) ;

/* Output the message header and body to the network. */

    if (vimWrite (stream, timeout, header, buffer)) {
        LGE "(vimWriteList) Error writing %ld-byte message to %s.\nvimWrite: ",
            header->length, vimName (stream)) ;
        PUSH_ERRNO ;  free (buffer) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(vimWriteList) %ld-byte message (%ld,%p) to %s.\n",
        header->length, header->ID, header->parameter, vimName (stream)) ;

    free (buffer) ;

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the VIM_UTIL functions.

    Under UNIX:
        Compile and link as follows:
            % cc -DTEST vim_util.c <libraries> -o vim_test
        First run the server with the following command line:
            % vim_test server <name> &
        Then, run the client (preferably in a separate window):
            % vim_test client <name>
        The client sends 16 messages to the server and the server
        reads them.

*******************************************************************************/

int  main (argc, argv)
    int  argc ;
    char  *argv[] ;

{    /* Local variables. */
    int  i ;
    VimHeader  header ;
    VimStream  stream ;
    NVPair  messageTime, realNumber, pair ;
    NVList  list ;
    TcpEndpoint  connection, listeningPoint ;




    aperror_print = 1 ;
    vim_util_debug = 1 ;
    nvp_util_debug = 1 ;
    nvl_util_debug = 1 ;
    tcp_util_debug = 1 ;

    if (argc < 3) {
        fprintf (stderr, "Usage:  vim_test client|server <name>\n") ;
        exit (EINVAL) ;
    }

    if (strcmp (argv[1], "client") == 0) {	/* The Client? */

        nvlCreate (NULL, &list) ;
        nvpCreate ("INFO", &pair) ;  nvlAdd (list, pair) ;
        nvpCreate ("REAL", &realNumber) ;  nvlAdd (list, realNumber) ;
        nvpAssign (realNumber, 1, NvpDouble, 123.456) ;
        nvpCreate ("TIME", &messageTime) ;  nvlAdd (list, messageTime) ;

        tcpCall (argv[2], 0, &connection) ;
        vimCreate (connection, &stream) ;
        for (i = 0 ;  i < 16 ;  i++) {
            nvpAssign (pair, 1, NvpLong, i) ;
            nvpAssign (messageTime, 1, NvpTime, tvTOD ()) ;
            memset (&header, 0, sizeof header) ;
            vimWriteList (stream, -1.0, &header, list) ;
        }
        vimDestroy (stream) ;

    } else {					/* The Server. */

        tcpListen (argv[2], -1, &listeningPoint) ;
        tcpAnswer (listeningPoint, -1.0, &connection) ;
        vimCreate (connection, &stream) ;
        for ( ; ; ) {
            if (vimReadList (stream, -1.0, &header, &list)) {
                LGE "Error reading from connection on %s.\nvimReadList: ",
                    argv[2]) ;
                break ;
            }
            for (i = 0 ;  i < nvlCount (list) ;  i++) {
                pair = nvlGet (list, i) ;
                printf ("[SERVER] %s = %s\n",
                        nvpName (pair), nvpString (pair)) ;
            }
            nvlDestroy (list) ;
        }
        vimDestroy (stream) ;

    }

    exit (0) ;

}

#endif  /* TEST */
