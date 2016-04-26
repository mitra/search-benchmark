/* $Id: xnet_util.c,v 1.24 2012/07/03 17:21:00 alex Exp $ */
/*******************************************************************************

File:

    xnet_util.c

    XDR Networking Utilities.


Author:    Alex Measday


Purpose:

    The XNET utilities provide a high-level interface to the Sun XDR routines
    that perform record-oriented communications across a network connection.
    The XNET package is layered on top of the lower-level TCP_UTIL functions.
    Network connections can be established between clients and servers and
    XDR streams are built on these connections. The read and write functions
    make it easy to send ASCII text records back and forth between processes.

    A simple server process that reads and displays the ASCII text messages
    it receives could be as brief as the following program:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        #include  "xnet_util.h"			-- XNET definitions.

        int  main (int argc, char *argv[])
        {
            char  *message ;
            TcpEndpoint  client, server ;
            XnetStream  stream ;

            tcpListen (argv[1], -1, &server) ;	-- Create listening endpoint.

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                xnetCreate (client, NULL, &stream) ;
                for ( ; ; ) {			-- Service connected client.
                    if (xnetRead (stream, -1.0, &message))  break ;
                    printf ("Message: %s\n", message) ;
                }
                xnetDestroy (stream) ;		-- Lost client.
            }

        }

    The server's name is specified as the first argument on the command line
    (i.e., "argv[1]").  If a client connection is broken, the server loops
    back to wait for another client.

    A simple client process that reads its user's input and forwards it to
    the server process would look as follows:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        #include  "xnet_util.h"			-- XNET definitions.

        int  main (int argc, char *argv[])
        {
            char  buffer[128] ;
            TcpEndpoint  connection ;
            XnetStream  stream ;

            tcpCall (argv[1], 0, &connection) ;	-- Call server.
            xnetCreate (connection, NULL, &stream) ;
            for ( ; ; ) {			-- Forward input to server.
                if (gets (buffer) == NULL)  break ;
                xnetWrite (stream, -1.0, "%s", buffer) ;
            }
            xnetDestroy (stream) ;		-- Lost user!

        }

    Although the XNET functions are especially suited to exchanging
    ASCII messages between processes, an application can still access
    the lower-level XDR routines.  xnetHandle() returns the address of
    the XDR stream structure that is needed for the system XDR calls.
    The following server periodically sends its client the current time
    (in a record containing a UNIX "timeval" structure):

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  <rpc/rpc.h>			-- RPC/XDR definitions.
        #include  <sys/time.h>			-- System time definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        #include  "xnet_util.h"			-- XNET definitions.

        int  main (int argc, char *argv[])
        {
            struct  timeval  currentTime ;
            TcpEndpoint  client, server ;
            XDR  *xdrStream ;
            XnetStream  stream ;

            tcpListen (argv[1], -1, &server) ;	-- Create listening endpoint.

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                xnetCreate (client, NULL, &stream) ;
                xdrStream = xnetHandle (stream) ;
                for ( ; ; ) {			-- Service connected client.
                    gettimeofday (&currentTime, NULL) ;
                    if (!xdr_timeval (xdrStream, &currentTime) ||
                        xnetEndRecord (stream))  break ;
                    sleep (1) ;
                }
                xnetDestroy (stream) ;		-- Lost client.
            }

        }

    After manually constructing an output record using low-level XDR functions,
    the program must terminate the record with a call to xnetEndRecord(), as
    the example above shows.  Likewise, after decoding an input record using
    low-level XDR functions, the program must "close" the current record and
    advance to the beginning of the next record with a single call to
    xnetNextRecord().  [xnetWrite() and xnetRead(), respectively, call these
    functions automatically.]

    In event-driven applications (e.g., those based on the X Toolkit or the
    "iox_util.c" dispatcher), the socket connection underlying the XDR stream,
    returned by xnetFd(), can be monitored for input by your event dispatcher.
    Because input is buffered by the XDR library, the input callback must
    continue to read XDR records while xnetIsReadable() is true.  When reading
    string records, calls to xnetRead() can simply be interleaved with calls to
    xnetIsReadable():

        while (xnetIsReadable (stream)) {
            if (xnetRead (stream, -1.0, &message))  break ;
            ... process message ...
        }

    When manually decoding input records, xnetIsReadable() should only be
    checked in between records, not in the middle of a record:

        while (xnetIsReadable (stream)) {
            if (!xdr_int (xdrStream, &value1) ||
                !xdr_int (xdrStream, &value2) ||
                xnetEndRecord (stream))  break ;
            ... process values ...
        }


Notes:

    If you're using the XNET package in an event-driven application, read
    the paragraphs above about XNET input in event-driven applications.

    These functions are reentrant under VxWorks (except for the global
    debug flag).


Public Procedures:

    xnetCreate() - creates an XDR-based network stream.
    xnetDestroy() - deletes an XDR-based network stream.
    xnetEndRecord() - outputs the contents of the current output record
        to an XDR stream and begins a new record.
    xnetFd() - returns an XNET stream's socket number.
    xnetHandle() - returns an XNET stream's XDR handle.
    xnetIsReadable() - checks if input is waiting to be read from a stream.
    xnetIsUp() - checks if an XNET stream is up.
    xnetIsWriteable() - checks if data can be written to a stream.
    xnetNextRecord() - positions to the beginning of the next input record.
    xnetRead() - reads a record containing an XDR string from an XNET stream.
    xnetSetTimeout() - sets the I/O timeout on an XNET stream.
    xnetWrite() - writes a record containing an XDR string to an XNET stream.

Private Procedures:

    xnetReadStream() - reads data from a network connection on behalf of
        an XNET stream.
    xnetWriteStream() - writes data to a network connection on behalf of
        an XNET stream.

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
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "xnet_util.h"			/* XNET definitions. */


/*******************************************************************************
    XNET Stream - contains information about an XNET data stream, including
        the underlying network connection, the XDR stream structure, and other
        attributes.
*******************************************************************************/

typedef  struct  _XnetStream {
    TcpEndpoint  connection ;		/* TCP/IP connection. */
    XDR  xdrStream ;			/* XDR stream structure. */
    double  timeout ;			/* Timeout on completion of operation. */
    bool  moreInput ;			/* Buffered data after current record? */
    char  *inputString ;		/* Last string read. */
    char  *outputString ;		/* Formatted string to be output. */
}  _XnetStream ;


int  xnet_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  xnet_util_debug


#define  MAX_STRING_LENGTH  1024


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  int  xnetReadStream (		/* See xdrrec_create(3)'s readit(). */
#    if PROTOTYPES
        XnetStream  stream,
        char  *buffer,
        int  numBytesToRead
#    endif
    ) ;

static  int  xnetWriteStream (		/* See xdrrec_create(3)'s writeit(). */
#    if PROTOTYPES
        XnetStream  stream,
        const  char  *buffer,
        int  numBytesToWrite
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    xnetCreate ()

    Create an XNET Stream.


Purpose:

    Function xnetCreate() creates an XDR-based network stream on top of
    a previously-created network connection.

    The options argument passed into this function is a string containing
    zero or more of the following UNIX command line-style options:

        "-buffer <size>"
            specifies the size of the internal XDR send/receive buffers;
            the default is the system default (4096, I believe).
        "-maximum <length>"
            specifies the maximum message size; the default is 1024 bytes.


    Invocation:

        status = xnetCreate (dataPoint, options, &stream) ;

    where

        <dataPoint>	- I
            is the previously-created network endpoint for the underlying
            network connection.  (See "tcp_util.c" for more information
            about network endpoints.)  NOTE that the "dataPoint" endpoint
            is automatically destroyed (i.e., the socket is closed) when
            the XNET stream is destroyed.
        <options>	- I
            is a string containing zero or more of the UNIX command line-style
            options described above.
        <stream>	- O
            returns a handle for the new XNET stream.  This handle is used
            in calls to the other XNET functions.
        <status>	- O
            returns the status of creating the stream, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  xnetCreate (

#    if PROTOTYPES
        TcpEndpoint  dataPoint,
        const  char  *options,
        XnetStream  *stream)
#    else
        dataPoint, options, stream)

        TcpEndpoint  dataPoint ;
        char  *options ;
        XnetStream  *stream ;
#    endif

{    /* Local variables. */
    char  *argument, **argv ;
    int  argc, errflg, option ;
    int  bufferSize, maxLength ;
    OptContext  context ;

    static  const  char  *optionList[] = {
        "{buffer:}", "{maximum:}", NULL
    } ;





    *stream = NULL ;


/*******************************************************************************
    Convert the options string into an ARGC/ARGV array and scan the arguments.
*******************************************************************************/

    bufferSize = 0 ;  maxLength = 1024 ;

    if (options != NULL) {

        opt_create_argv ("xnetCreate", options, &argc, &argv) ;
        opt_init (argc, (char **) argv, NULL, optionList, &context) ;
        opt_errors (context, false) ;

        errflg = 0 ;
        while ((option = opt_get (context, &argument))) {
            switch (option) {
            case 1:			/* "-buffer <size>" */
                bufferSize = atoi (argument) ;
                break ;
            case 2:			/* "-maximum <length>" */
                maxLength = atoi (argument) ;
                break ;
            case NONOPT:
            case OPTERR:
            default:
                errflg++ ;  break ;
            }
        }

        opt_term (context) ;
        opt_delete_argv (argc, argv) ;

        if (errflg) {
            SET_ERRNO (EINVAL) ;
            LGE "(xnetCreate) Invalid option/argument in %s's options string: \"%s\"\n",
                tcpName (dataPoint), options) ;
            return (errno) ;
        }

    }

/*******************************************************************************
    Create and initialize an XNET stream structure for the server connection.
*******************************************************************************/

    *stream = (_XnetStream *) malloc (sizeof (_XnetStream)) ;
    if (*stream == NULL) {
        LGE "(xnetCreate) Error allocating stream structure for \"%s\".\nmalloc: ",
            tcpName (dataPoint)) ;
        return (errno) ;
    }

    (*stream)->connection = dataPoint ;
    xdrrec_create (&(*stream)->xdrStream, bufferSize, bufferSize,
                   (caddr_t) *stream,
                   (int (*)P_((char *, char *, int))) xnetReadStream,
                   (int (*)P_((char *, char *, int))) xnetWriteStream) ;
    (*stream)->xdrStream.x_op = XDR_DECODE ;
    (*stream)->timeout = -1.0 ;
    (*stream)->moreInput = false ;
    (*stream)->inputString = NULL ;
    (*stream)->outputString = NULL ;

    if (xnetNextRecord (*stream)) {
        LGE "(xnetCreate) Error positioning to start of %s stream.\nxnetNextRecord: ",
            xnetName (*stream)) ;
        PUSH_ERRNO ;  xnetDestroy (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

    (*stream)->outputString = malloc (maxLength+1) ;
    if ((*stream)->outputString == NULL) {
        LGE "(xnetCreate) Error allocating %d-byte output string for %s.\nmalloc: ",
            maxLength, xnetName (*stream)) ;
        PUSH_ERRNO ;  xnetDestroy (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(xnetCreate) Created XDR-based network stream %s, socket %d\n",
        xnetName (*stream), xnetFd (*stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetDestroy ()

    Delete an XNET Stream.


Purpose:

    Function xnetDestroy() destroys an XNET stream.  The underlying network
    connection is closed and the XNET stream structure is deallocated.

    xnetDestroy() does NOT flush out any data remaining in the stream's
    output buffer.


    Invocation:

        status = xnetDestroy (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <status>	- O
            returns the status of deleting the stream, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  xnetDestroy (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{

    if (stream == NULL)  return (0) ;

    LGI "(xnetDestroy) Closing %s stream ...\n", xnetName (stream)) ;

/* Close the underlying network connection. */

    if (stream->connection != NULL) {
        tcpDestroy (stream->connection) ;  stream->connection = NULL ;
    }

/* Deallocate the XNET stream structure. */

    if (stream->inputString != NULL) {
        stream->xdrStream.x_op = XDR_FREE ;
        xdr_string (&stream->xdrStream, &stream->inputString, INT_MAX) ;
        stream->inputString = NULL ;
    }
    xdr_destroy (&stream->xdrStream) ;
    if (stream->outputString != NULL) {
        free (stream->outputString) ;  stream->outputString = NULL ;
    }
    free ((char *) stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetEndRecord ()

    Flush the Current Record.


Purpose:

    Function xnetEndRecord() outputs the contents of the current output
    record to the network and begins a new record.


    Invocation:

        status = xnetEndRecord (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <status>
            returns the status of outputting the record, zero if there
            were no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  xnetEndRecord (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xnetEndRecord) NULL stream handle: ") ;
        return (errno) ;
    }

/* Flush the output record. */

    if (!xdrrec_endofrecord (&stream->xdrStream, TRUE)) {
        if (errno == 0)  SET_ERRNO (EIO) ;
        LGE "(xnetEndRecord) Error flushing record on %s stream.\nxdrrec_endofrecord: ",
            xnetName (stream)) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetFd ()

    Get an XNET Stream's Socket.


Purpose:

    Function xnetFd() returns the Unix file descriptor for the socket
    connection associated with an XNET stream.


    Invocation:

        fd = xnetFd (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <fd>		- O
            returns the UNIX file descriptor for the stream's socket.

*******************************************************************************/


IoFd  xnetFd (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{
    return ((stream == NULL) ? -1 : tcpFd (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    xnetHandle ()

    Get an XNET Stream's XDR Handle.


Purpose:

    Function xnetHandle() returns the XDR handle (i.e., the address of an
    XDR stream structure) from an XNET stream.  The XDR handle can be used
    for direct XDR function calls (as opposed to the higher-level XNET
    function calls).


    Invocation:

        handle = xnetHandle (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <handle>	- O
            returns the XDR handle (i.e., the address of the XDR structure)
            for the stream.  NULL is returned if the stream hasn't been
            created yet.

*******************************************************************************/


XDR  *xnetHandle (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{
    return ((stream == NULL) ? NULL : &stream->xdrStream) ;
}

/*!*****************************************************************************

Procedure:

    xnetIsReadable ()

    Check if Data is Waiting to be Read.


Purpose:

    The xnetIsReadable() function checks to see if data is waiting to
    be read from an XNET stream.


    Invocation:

        isReadable = xnetIsReadable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  xnetIsReadable (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{
    if (stream == NULL)
        return (false) ;
    else if (stream->moreInput)				/* Buffered input? */
        return (true) ;
    else
        return (tcpIsReadable (stream->connection)) ;	/* Real input? */
}

/*!*****************************************************************************

Procedure:

    xnetIsUp ()

    Check if a Stream's Network Connection is Up.


Purpose:

    The xnetIsUp() function checks to see if an XNET stream's underlying
    network connection is still up.


    Invocation:

        isUp = xnetIsUp (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <isUp>		- O
            returns true (a non-zero value) if the stream's network
            connection is up and false (zero) otherwise.

*******************************************************************************/


bool  xnetIsUp (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsUp (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    xnetIsWriteable ()

    Check if Stream is Ready for Writing.


Purpose:

    The xnetIsWriteable() function checks to see if data can be written to
    an XNET stream.


    Invocation:

        isWriteable = xnetIsWriteable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <isWriteable>	- O
            returns true (a non-zero value) if the XNET stream is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  xnetIsWriteable (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsWriteable (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    xnetName ()

    Get an XNET Stream's Name.


Purpose:

    Function xnetName() returns an XNET stream's name.


    Invocation:

        name = xnetName (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <name>		- O
            returns the stream's name.  The name is stored in memory local
            to the XNET utilities and it should not be modified or freed
            by the caller.

*******************************************************************************/


const  char  *xnetName (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{
    return ((stream == NULL) ? "" : tcpName (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    xnetNextRecord ()

    Begin the Next Input Record.


Purpose:

    Function xnetNextRecord() discards the current input record, reads the
    next record from the network, and positions the input pointer to the
    start of the new record.


    Invocation:

        status = xnetNextRecord (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <status>
            returns the status of positioning to the next record in the input
            stream, zero if there were no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  xnetNextRecord (

#    if PROTOTYPES
        XnetStream  stream)
#    else
        stream)

        XnetStream  stream ;
#    endif

{

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xnetNextRecord) NULL stream handle: ") ;
        return (errno) ;
    }

/* Check if there is data in the input buffer following the current input
   record.  An irrational side effect of the call to XDRREC_EOF() is that
   the contents of the current input record are discarded. */

    stream->moreInput = !xdrrec_eof (&stream->xdrStream) ;

/* After XDRREC_EOF() discards the current input record, call
   XDRREC_SKIPRECORD() to position to the start of the next input record. */

    if (!xdrrec_skiprecord (&stream->xdrStream)) {
        if (errno == 0)  SET_ERRNO (EIO) ;
        LGE "(xnetNextRecord) Error positioning to next input record on %s stream.\nxdrrec_skiprecord: ",
            xnetName (stream)) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetRead ()

    Read an XDR String Record.


Purpose:

    Function xnetRead() retrieves the next XDR string from the current
    input record and discards the remainder of the record.  In Sun XDR
    terms, this is equivalent to performing an XDR_STRING() decode,
    followed by an XDRREC_SKIPRECORD().  If each record only contains
    a single XDR string, repeated calls to xnetRead() will read each
    record in the input stream and extract the strings.


    Invocation:

        status = xnetRead (stream, timeout, &string) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the next text record to be read.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) allows a read only if input is
            immediately available.
        <string>	- O
            returns a pointer to the null-terminated string that was read.
            The string is stored in memory private to the XNET stream and,
            although the caller can modify the string, it should be used
            or duplicated before calling xnetRead() again.
        <status>
            returns the status of reading the text record, zero if there
            were no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  xnetRead (

#    if PROTOTYPES
        XnetStream  stream,
        double  timeout,
        char  **string)
#    else
        stream, timeout, string)

        XnetStream  stream ;
        double  timeout ;
        char  **string ;
#    endif

{

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xnetRead) NULL stream handle: ") ;
        return (errno) ;
    }

    stream->timeout = timeout ;

/* Free the last string read. */

    if (stream->inputString != NULL) {
        stream->xdrStream.x_op = XDR_FREE ;
        if (!xdr_string (&stream->xdrStream, &stream->inputString, INT_MAX)) {
            if (errno == 0)  SET_ERRNO (EIO) ;
            LGE "(xnetRead) Error freeing input string for %s stream.\nxdr_string: ",
                xnetName (stream)) ;
        }
        stream->inputString = NULL ;
    }

/* Get a string from the input record. */

    stream->xdrStream.x_op = XDR_DECODE ;
    if (!xdr_string (&stream->xdrStream, &stream->inputString, INT_MAX)) {
        if (errno == 0)  SET_ERRNO (EIO) ;
        LGE "(xnetRead) Error reading from %s stream.\nxdr_string: ",
            xnetName (stream)) ;
        return (errno) ;
    }

/* Discard the remainder of the record and prepare for a new record. */

    if (xnetNextRecord (stream)) {
        LGE "(xnetRead) Error skipping record on %s stream.\nxnetNextRecord: ",
            xnetName (stream)) ;
        return (errno) ;
    }

/* Return the input string to the caller. */

    *string = stream->inputString ;

    LGI "(xnetRead) From %s: \"%s\"\n",
        xnetName (stream), stream->inputString) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetSetTimeout ()

    Set the I/O Timeout on an XNET Stream.


Purpose:

    Function xnetSetTimeout() sets the timeout for the next I/O operation
    on an XNET stream.  xnetSetTimeout() should only be called if you're
    making direct XDR calls; it will have no effect on xnetRead() and
    xnetWrite(), since they have their own timeout arguments.


    Invocation:

        status = xnetSetTimeout (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xnetCreate().
        <status>	- O
            returns the status of setting the timeout, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  xnetSetTimeout (

#    if PROTOTYPES
        XnetStream  stream,
        double  timeout)
#    else
        stream, timeout)

        XnetStream  stream ;
        double  timeout ;
#    endif

{
    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xnetSetTimeout) NULL stream handle: ") ;
        return (errno) ;
    }

    stream->timeout = timeout ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetWrite ()


Purpose:

    Function xnetWrite() formats and adds a string to the current output
    record and flushes the record out to a network connection.  In Sun
    XDR terms, this is equivalent to performing an XDR_STRING() encode,
    followed by an XDRREC_ENDOFRECORD().


    Invocation:

        status = xnetWrite (stream, timeout, format, arg1, arg2, ...) ;

    where

        <stream>		- I
            is the stream handle returned by xnetCreate().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the text record to be written.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait.
            A zero timeout (0.0) specifies no wait: if the connection
            is not ready for writing, xnetWrite() returns immediately;
            if the connection is ready for writing, xnetWrite() returns
            after outputting whatever it can.  A zero timeout is strongly
            discouraged.
        <format>		- I
            is a normal PRINTF(3)-style format string.
        <arg1, ..., argN>	- I
            are the arguments expected by the format string.
        <status>
            returns the status of writing the text record, zero if there
            were no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  xnetWrite (

#    if PROTOTYPES
        XnetStream  stream,
        double  timeout,
        const  char  *format,
        ...)
#    else
        stream, timeout, format, va_alist)

        XnetStream  stream ;
        double  timeout ;
        char  *format ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xnetWrite) NULL stream handle: ") ;
        return (errno) ;
    }

    stream->timeout = timeout ;

/* Format the output text. */

#if HAVE_STDARG_H
    va_start (ap, format) ;
#else
    va_start (ap) ;
#endif
    (void) vsprintf (stream->outputString, format, ap) ;
    va_end (ap) ;

/* Add the string to the current output record. */

    stream->xdrStream.x_op = XDR_ENCODE ;
    if (!xdr_string (&stream->xdrStream, &stream->outputString, INT_MAX)) {
        if (errno == 0)  SET_ERRNO (EIO) ;
        LGE "(xnetWrite) Error writing to %s stream.\nxdr_string: ",
            xnetName (stream)) ;
        return (errno) ;
    }

/* Output the record to the network connection and begin a new record. */

    if (xnetEndRecord (stream)) {
        LGE "(xnetWrite) Error ending record on %s stream.\nxnetEndRecord: ",
            xnetName (stream)) ;
        return (errno) ;
    }

    LGI "(xnetWrite) To %s: \"%s\"\n",
        xnetName (stream), stream->outputString) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xnetReadStream ()


Purpose:

    Function xnetReadStream() is the low-level read function called by
    the XDR functions to actually read data from a network connection.
    xnetReadStream() is specified as the read function for a stream
    when the stream is created by XDRREC_CREATE(), which is called in
    xnetCreate().


    Invocation:

        numBytesRead = xnetReadStream (stream, &buffer, numBytesToRead) ;

    where

        <stream>		- I
            is the stream handle returned by xnetCreate().
        <buffer>		- O
            returns the data read from the stream.  Up to numBytesToRead
            of data will be read and placed in the buffer.
        <numBytesToRead>	- I
            specifies the maximum number of bytes of data to read.
        <numBytesRead>		- O
            returns the actual number of bytes read, which may be less
            than the number requested.  -1 is returned in the event of
            an error (e.g., a broken connection) and the error code is
            stored in ERRNO.

*******************************************************************************/


static  int  xnetReadStream (

#    if PROTOTYPES
        XnetStream  stream,
        char  *buffer,
        int  numBytesToRead)
#    else
        stream, buffer, numBytesToRead)

        XnetStream  stream ;
        char  *buffer ;
        int  numBytesToRead ;
#    endif

{    /* Local variables. */
    size_t  numBytesRead ;



    if (tcpRead (stream->connection, stream->timeout,
                 - ((ssize_t) numBytesToRead),
                 buffer, &numBytesRead)) {
        LGE "(xnetReadStream) Error reading %d bytes from %s stream.\ntcpRead: ",
            numBytesToRead, xnetName (stream)) ;
        return (-1) ;
    }

    return ((int) numBytesRead) ;

}

/*!*****************************************************************************

Procedure:

    xnetWriteStream ()


Purpose:

    Function xnetWriteStream() is the low-level write function called by
    the XDR functions to actually write data to a network connection.
    xnetWriteStream() is specified as the write function for a stream
    when the stream is created by XDRREC_CREATE(), which is called in
    xnetCreate().


    Invocation:

        numBytesWritten = xnet_write_stream (stream, buffer,
                                             numBytesToWrite) ;

    where

        <stream>		- I
            is the stream handle returned by xnetCreate().
        <buffer>		- I
            contains the data to be written to the network connection.
        <numBytesToWrite>	- I
            specifies the number of bytes of data to write to the connection.
        <numBytesWritten>	- O
            returns the actual number of bytes written, which may be less
            than the number requested.  -1 is returned in the event of
            an error and the error code is stored in ERRNO.

*******************************************************************************/


static  int  xnetWriteStream (

#    if PROTOTYPES
        XnetStream  stream,
        const  char  *buffer,
        int  numBytesToWrite)
#    else
        stream, buffer, numBytesToWrite)

        XnetStream  stream ;
        char  *buffer ;
        int  numBytesToWrite ;
#    endif

{    /* Local variables. */
    size_t  numBytesWritten ;




/* Write the data directly to the network connection. */

    if (tcpWrite (stream->connection, stream->timeout,
                  (ssize_t) numBytesToWrite, buffer, &numBytesWritten)) {
        LGE "(xnetWriteStream) Error writing %d bytes to %s stream.\ntcpWrite: ",
            numBytesToWrite, xnetName (stream)) ;
        return (-1) ;
    }

    return ((int) numBytesWritten) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the NET_UTIL functions.

    Under UNIX:
        Compile and link as follows:
            % cc -DTEST xnet_util.c <libraries> -o xnet_test
        First run the server with the following command line:
            % xnet_test server <name> &
        Then, run the client (preferably in a separate window):
            % xnet_test client <name>
        The client sends 16 messages to the server and the server
        reads them.

    Under VxWorks:
        Compile and link as follows:
            % cc -DTEST xnet_util.c <libraries> -o xnet_test.vx.o
        First, load and run the server with the following commands:
            -> ld <xnet_test.vx.o
            -> sp xnet_test, "server <name>"
        Then, load (to be on the safe side) and run the client:
            -> ld <xnet_test.vx.o
            -> sp xnet_test, "client <name>"
        The client sends 16 messages to the server and the server
        reads them.

*******************************************************************************/

#ifdef VXWORKS

    void  xnet_test (
        char  *commandLine)

#else

    main (argc, argv)
        int  argc ;
        char  *argv[] ;

#endif

{    /* Local variables. */
    char  *string ;
    int  i ;
    TcpEndpoint  connection, listeningPoint ;
    XnetStream  stream ;




#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("xnet_test", commandLine, &argc, &argv) ;
#endif

    tcp_util_debug = 1 ;
    xnet_util_debug = 1 ;

    if (argc < 3) {
        fprintf (stderr, "Usage:  xnet_test client|server <name>\n") ;
        exit (EINVAL) ;
    }

    if (strcmp (argv[1], "client") == 0) {	/* The Client? */

        tcpCall (argv[2], 0, &connection) ;
        xnetCreate (connection, "-buffer 1024", &stream) ;
        for (i = 0 ;  i < 16 ;  i++) {
            xnetWrite (stream, -1.0, "Message #%d", i) ;
        }
        xnetDestroy (stream) ;

    } else {					/* The Server. */

        tcpListen (argv[2], -1, &listeningPoint) ;
        tcpAnswer (listeningPoint, -1.0, &connection) ;
        xnetCreate (connection, "-buffer 1024", &stream) ;
        for ( ; ; ) {
            if (xnetRead (stream, -1.0, &string)) {
                LGE "Error reading from connection on %s.\nxnetRead: ",
                    argv[2]) ;
                break ;
            }
            printf ("[SERVER] Input: \"%s\"\n", string) ;
        }
        xnetDestroy (stream) ;

    }

}

#endif  /* TEST */
