/* $Id: lfn_util.c,v 1.17 2011/07/18 17:51:24 alex Exp alex $ */
/*******************************************************************************

File:

    lfn_util.c

    Line Feed-Terminated Networking Utilities.


Author:    Alex Measday


Purpose:

    The LFN_UTIL functions provide a simple means of sending and receiving
    LF-terminated text over a network connection.  The LFN_UTIL package is
    layered on top of the lower-level TCP_UTIL functions.  Network connections
    can be established between clients and servers and LFN streams are built
    on these connections.

    A simple server process that reads and displays the ASCII text messages
    it receives could be as brief as the following program:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        #include  "lfn_util.h"			-- LF-terminated network I/O.

        int  main (int argc, char *argv[])
        {
            char  *message ;
            TcpEndpoint  client, server ;
            LfnStream  stream ;

            tcpListen (argv[1], -1, &server) ;	-- Create listening endpoint.

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                lfnCreate (client, NULL, &stream) ;
                for ( ; ; ) {			-- Service connected client.
                    if (lfnGetLine (stream, -1.0, &message))  break ;
                    printf ("Message: %s\n", message) ;
                }
                lfnDestroy (stream) ;		-- Lost client.
            }

        }

    The server's name is specified as the first argument on the command line
    (i.e., "argv[1]").  If a client connection is broken, the server loops
    back to wait for another client.

    A simple client process that reads its user's input and forwards it to
    the server process would look as follows:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        #include  "lfn_util.h"			-- LF-terminated network I/O.

        int  main (int argc, char *argv[])
        {
            char  buffer[128] ;
            TcpEndpoint  connection ;
            LfnStream  stream ;

            tcpCall (argv[1], 0, &connection) ;	-- Call server.
            lfnCreate (connection, NULL, &stream) ;
            for ( ; ; ) {			-- Forward input to server.
                if (gets (buffer) == NULL)  break ;
                lfnPutLine (stream, -1.0, "%s\n", buffer) ;
            }
            lfnDestroy (stream) ;		-- Lost user!

        }


Notes:

    These functions are reentrant under VxWorks (except for the global
    debug flag).


Public Procedures:

    lfnCreate() - creates a LF-terminated network stream.
    lfnDestroy() - deletes a LF-terminated network stream.
    lfnFd() - returns an LFN stream's socket number.
    lfnGetLine() - reads a line of input from an LFN stream.
    lfnIsReadable() - checks if input is waiting to be read from a stream.
    lfnIsUp() - checks if an LFN stream is up.
    lfnIsWriteable() - checks if data can be written to a stream.
    lfnPutLine() - writes a line of output to an LFN stream.
    lfnRead() - reads unformatted data from an LFN stream.
    lfnWrite() - writes unformatted data to an LFN stream.

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
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */


/*******************************************************************************
    LFN Stream - contains information about a LF-terminated, data stream,
        including the underlying network connection, the input buffer, and
        other attributes.
*******************************************************************************/

typedef  struct  _LfnStream {
    TcpEndpoint  connection ;		/* TCP/IP connection. */
    int  terminator ;			/* 0 = none, 1 = LF, 3 = CR/LF. */
    size_t  maxInput ;			/* Size of input buffer. */
    size_t  nextChar ;			/* Index of next character in buffer. */
    size_t  lastChar ;			/* Index of last character in buffer. */
    char  *inputBuffer ;		/* Buffered input. */
    char  *inputString ;		/* Last string read. */
    size_t  maxOutput ;			/* Maximum output message length. */
    char  *outputString ;		/* Formatted string to be output. */
}  _LfnStream ;


int  lfn_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  lfn_util_debug


#define  MAX_INPUT_BUFFER  2048
#define  MAX_OUTPUT_STRING  2047

/*!*****************************************************************************

Procedure:

    lfnCreate ()

    Create an LFN Stream.


Purpose:

    Function lfnCreate() creates a LF-terminated network stream on top of
    a previously-created network connection.

    The options argument passed into this function is a string containing
    zero or more of the following UNIX command line-style options:

        "-crlf"
            specifies that lfnPutLine() should automatically append a CR/LF
            terminator to its output text.  By default, lfnPutLine() will
            not automatically append a line terminator.
        "-input <size>"
            specifies the size of the internal input buffer; the default
            is 2048 bytes.  NOTE that this is only a limit on the input
            buffer, not on incoming strings.  Function lfnGetLine() may
            load the buffer multiple times in order to construct an input
            string, which can grow to any length.
        "-lf"
            specifies that lfnPutLine() should automatically append a LF
            terminator to its output text.  By default, lfnPutLine() will
            not automatically append a line terminator.
        "-output <size>"
            specifies the maximum output message size; the default is
            2047 bytes.


    Invocation:

        status = lfnCreate (dataPoint, options, &stream) ;

    where

        <dataPoint>	- I
            is the previously-created network endpoint for the underlying
            network connection.  (See "tcp_util.c" for more information
            about network endpoints.)  NOTE that the "dataPoint" endpoint
            is automatically destroyed (i.e., the socket is closed) when
            the LFN stream is destroyed.
        <options>	- I
            is a string containing zero or more of the UNIX command line-style
            options described above.
        <stream>	- O
            returns a handle for the new LFN stream.  This handle is used
            in calls to the other LFN functions.
        <status>	- O
            returns the status of creating the stream, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  lfnCreate (

#    if PROTOTYPES
        TcpEndpoint  dataPoint,
        const  char  *options,
        LfnStream  *stream)
#    else
        dataPoint, options, stream)

        TcpEndpoint  dataPoint ;
        char  *options ;
        LfnStream  *stream ;
#    endif

{    /* Local variables. */
    char  *argument, **argv ;
    int  argc, errflg, option, terminator ;
    size_t  maxInput, maxOutput ;
    OptContext  context ;

    static  const  char  *optionList[] = {
        "{crlf}", "{input:}", "{lf}", "{output:}", NULL
    } ;




    *stream = NULL ;


/*******************************************************************************
    Convert the options string into an ARGC/ARGV array and scan the arguments.
*******************************************************************************/

    maxInput = MAX_INPUT_BUFFER ;
    maxOutput = MAX_OUTPUT_STRING ;
    terminator = 0 ;

    if (options != NULL) {

        opt_create_argv ("lfnCreate", options, &argc, &argv) ;
        opt_init (argc, argv, NULL, optionList, &context) ;
        opt_errors (context, false) ;

        errflg = 0 ;
        while ((option = opt_get (context, &argument))) {
            switch (option) {
            case 1:			/* "-crlf" */
                terminator = 3 ;
                break ;
            case 2:			/* "-input <size>" */
                maxInput = (size_t) atol (argument) ;
                break ;
            case 3:			/* "-lf" */
                terminator = 1 ;
                break ;
            case 4:			/* "-output <size>" */
                maxOutput = (size_t) atol (argument) ;
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
            LGE "(lfnCreate) Invalid option/argument in %s's options string: \"%s\"\n",
                tcpName (dataPoint), options) ;
            return (errno) ;
        }

    }

/*******************************************************************************
    Create and initialize an LFN stream structure for the network connection.
*******************************************************************************/

    *stream = (_LfnStream *) malloc (sizeof (_LfnStream)) ;
    if (*stream == NULL) {
        LGE "(lfnCreate) Error allocating stream structure for \"%s\".\nmalloc: ",
            tcpName (dataPoint)) ;
        return (errno) ;
    }

    (*stream)->connection = dataPoint ;
    (*stream)->terminator = terminator ;
    (*stream)->maxInput = maxInput ;
    (*stream)->nextChar = 1 ;		/* "last < next" indicates empty buffer. */
    (*stream)->lastChar = 0 ;
    (*stream)->inputBuffer = NULL ;
    (*stream)->inputString = NULL ;
    (*stream)->maxOutput = maxOutput ;
    (*stream)->outputString = NULL ;

    (*stream)->inputBuffer = malloc (maxInput+1) ;
    if ((*stream)->inputBuffer == NULL) {
        LGE "(lfnCreate) Error allocating %d-byte input buffer for %s.\nmalloc: ",
            maxInput+1, lfnName (*stream)) ;
        lfnDestroy (*stream) ;  return (errno) ;
    }

    (*stream)->outputString = malloc (maxOutput+1) ;
    if ((*stream)->outputString == NULL) {
        LGE "(lfnCreate) Error allocating %d-byte output string for %s.\nmalloc: ",
            maxOutput+1, lfnName (*stream)) ;
        lfnDestroy (*stream) ;  return (errno) ;
    }


    LGI "(lfnCreate) Created formatted network stream %s, socket %d\n",
        lfnName (*stream), lfnFd (*stream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    lfnDestroy ()

    Delete an LFN Stream.


Purpose:

    Function lfnDestroy() destroys a LF-terminated network stream.  The
    underlying network connection is closed and the LFN stream structure
    is deallocated.


    Invocation:

        status = lfnDestroy (stream) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <status>	- O
            returns the status of deleting the stream, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  lfnDestroy (

#    if PROTOTYPES
        LfnStream  stream)
#    else
        stream)

        LfnStream  stream ;
#    endif

{

    if (stream == NULL)  return (0) ;

    LGI "(lfnDestroy) Closing %s(%ld) stream ...\n",
        lfnName (stream), (long) lfnFd (stream)) ;

/* Close the underlying network connection. */

    tcpDestroy (stream->connection) ;

/* Deallocate the LFN stream structure. */

    if (stream->inputBuffer != NULL)  free (stream->inputBuffer) ;
    if (stream->inputString != NULL)  free (stream->inputString) ;
    if (stream->outputString != NULL)  free (stream->outputString) ;
    free ((char *) stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    lfnFd ()

    Get an LFN Stream's Socket.


Purpose:

    Function lfnFd() returns the Unix file descriptor for the socket
    connection associated with a LF-terminated network stream.


    Invocation:

        fd = lfnFd (stream) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <fd>		- O
            returns the UNIX file descriptor for the stream's socket.

*******************************************************************************/


IoFd  lfnFd (

#    if PROTOTYPES
        LfnStream  stream)
#    else
        stream)

        LfnStream  stream ;
#    endif

{
    return ((stream == NULL) ? INVALID_SOCKET : tcpFd (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    lfnGetLine ()

    Read a Line of Input.


Purpose:

    Function lfnGetLine() reads the next, CR/LF-delimited line of input from
    the stream.


    Invocation:

        status = lfnGetLine (stream, timeout, &string) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the next line to be read.  A
            fractional time can be specified; e.g., 2.5 seconds.  A
            negative timeout (e.g., -1.0) causes an infinite wait; a
            zero timeout (0.0) allows a read only if input is immediately
            available.
        <string>	- O
            returns a pointer to the null-terminated string that was read;
            the string does NOT include the trailing CR/LF.  The string is
            stored in memory private to the LFN stream and, although the
            caller can modify the string, it should be used or duplicated
            before calling lfnGetLine() again.
        <status>
            returns the status of reading the input line, zero if there
            were no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  lfnGetLine (

#    if PROTOTYPES
        LfnStream  stream,
        double  timeout,
        char  **string)
#    else
        stream, timeout, string)

        LfnStream  stream ;
        double  timeout ;
        char  **string ;
#    endif

{    /* Local variables. */
    char  *buf, *line, *s ;
    size_t  last, length, newLength, next ;
    struct  timeval  expirationTime ;




    if (string != NULL)  *string = NULL ;

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(lfnGetLine) NULL stream handle: ") ;
        return (errno) ;
    }


/* If a timeout interval was specified, then compute the expiration time
   of the interval as the current time plus the interval. */

    if (timeout >= 0.0) {
        expirationTime = tvAdd (tvTOD (), tvCreateF (timeout)) ;
    }


/*******************************************************************************
    Construct the next line of input.
*******************************************************************************/

    buf = stream->inputBuffer ;
    next = stream->nextChar ;  last = stream->lastChar ;

    if (stream->inputString != NULL)  free (stream->inputString) ;
    line = stream->inputString = NULL ;  length = 0 ;

    for ( ; ; ) {

/* Copy buffered input to the input string until the LF or CR/LF terminator
   is reached. */

        if (next <= last) {
            s = strchr (&buf[next], '\n') ;
            if (s == NULL)
                newLength = length + strlen (&buf[next]) ;
            else
                newLength = length + (s - &buf[next]) ;
            if (stream->inputString == NULL)
                line = malloc (newLength+1) ;
            else
                line = realloc (stream->inputString, newLength+1) ;
            if (line == NULL) {
                LGE "(lfnGetLine) Error allocating %d-byte input string for %s.\nmalloc: ",
                    newLength+1, lfnName (stream)) ;
                return (errno) ;
            }
            stream->inputString = line ;
            strncpy (&line[length], &buf[next], newLength - length) ;
            next += newLength - length ;
            length = newLength ;
            line[length] = '\0' ;
            if (s != NULL) {			/* LF reached? */
                next++ ;			/* Advance past LF. */
                if (line[length-1] == '\r')  line[--length] = '\0' ;
                break ;				/* Strip CR if present. */
            }
        }

/* If the buffered input has been exhausted before completing the input line,
   then read more data from the socket connection.  NOTE that the input buffer
   is sized so that a NUL character can always be appended to the actual
   input data - so the strchr(3) above doesn't run past the end of the data. */

        if (timeout > 0.0)
            timeout = tvFloat (tvSubtract (expirationTime, tvTOD ())) ;

        if (tcpRead (stream->connection, timeout,
                     - ((ssize_t) stream->maxInput),
                     stream->inputBuffer, &last)) {
            LGE "(lfnGetLine) Error reading %d bytes from %s(%ld) stream.\ntcpRead: ",
                stream->maxInput, lfnName (stream), (long) lfnFd (stream)) ;
            return (errno) ;
        }

        next = 0 ;				/* Reset the indices. */
        stream->inputBuffer[last--] = '\0' ;

    }

    stream->nextChar = next ;  stream->lastChar = last ;


/* Return the input string to the caller. */

    *string = stream->inputString ;

    LGI "(lfnGetLine) From %s(%ld): \"%s\"\n",
        lfnName (stream), (long) lfnFd (stream), stream->inputString) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    lfnIsReadable ()

    Check if Data is Waiting to be Read.


Purpose:

    The lfnIsReadable() function checks to see if data is waiting to
    be read from a LF-terminated network stream.


    Invocation:

        isReadable = lfnIsReadable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <isReadable>	- O
            returns true (a non-zero value) if data is available for
            reading and false (zero) otherwise.

*******************************************************************************/


bool  lfnIsReadable (

#    if PROTOTYPES
        LfnStream  stream)
#    else
        stream)

        LfnStream  stream ;
#    endif

{
    if (stream == NULL)
        return (false) ;
    else if (stream->nextChar <= stream->lastChar)	/* Buffered input? */
        return (true) ;
    else
        return (tcpIsReadable (stream->connection)) ;	/* Real input? */
}

/*!*****************************************************************************

Procedure:

    lfnIsUp ()

    Check if a Stream's Network Connection is Up.


Purpose:

    The lfnIsUp() function checks to see if an LFN stream's underlying
    network connection is still up.


    Invocation:

        isUp = lfnIsUp (stream) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <isUp>		- O
            returns true (a non-zero value) if the stream's network
            connection is up and false (zero) otherwise.

*******************************************************************************/


bool  lfnIsUp (

#    if PROTOTYPES
        LfnStream  stream)
#    else
        stream)

        LfnStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsUp (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    lfnIsWriteable ()

    Check if a Stream is Ready for Writing.


Purpose:

    The lfnIsWriteable() function checks to see if data can be written to
    a LF-terminated network stream.


    Invocation:

        isWriteable = lfnIsWriteable (stream) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <isWriteable>	- O
            returns true (a non-zero value) if the LFN stream is ready
            for writing and false (zero) otherwise.

*******************************************************************************/


bool  lfnIsWriteable (

#    if PROTOTYPES
        LfnStream  stream)
#    else
        stream)

        LfnStream  stream ;
#    endif

{
    return ((stream == NULL) ? false : tcpIsWriteable (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    lfnName ()

    Get an LFN Stream's Name.


Purpose:

    Function lfnName() returns the name of a LF-terminated network stream.


    Invocation:

        name = lfnName (stream) ;

    where

        <stream>	- I
            is the stream handle returned by lfnCreate().
        <name>		- O
            returns the stream's name.  The name is stored in memory local
            to the LFN utilities and it should not be modified or freed
            by the caller.

*******************************************************************************/


const  char  *lfnName (

#    if PROTOTYPES
        LfnStream  stream)
#    else
        stream)

        LfnStream  stream ;
#    endif

{
    return ((stream == NULL) ? "" : tcpName (stream->connection)) ;
}

/*!*****************************************************************************

Procedure:

    lfnPutLine ()


Purpose:

    Function lfnPutLine() formats an output line and writes it to a network
    connection.  The caller is responsible for ensuring that the length of
    the formatted output line does not exceed the maximum message length
    specified in the call to lfnCreate().

        NOTE that, if a line terminator was not specified in the call
        to lfnCreate() (see the "-crlf" and "-lf" options), the caller
        must explicitly specify the desired line terminator in the format
        string.


    Invocation:

        status = lfnPutLine (stream, timeout, format, arg1, arg2, ...) ;

    where

        <stream>		- I
            is the stream handle returned by lfnCreate().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the line to be output.  A fractional
            time can be specified; e.g., 2.5 seconds.  A negative timeout
            (e.g., -1.0) causes an infinite wait.  A zero timeout (0.0)
            specifies no wait: if the connection is not ready for writing,
            lfnPutLine() returns immediately; if the connection is ready
            for writing, lfnPutLine() returns after outputting whatever it
            can.  A zero timeout is strongly discouraged.
        <format>		- I
            is a normal PRINTF(3)-style format string.
        <arg1, ..., argN>	- I
            are the arguments expected by the format string.
        <status>
            returns the status of writing the line, zero if there were no
            errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  lfnPutLine (

#    if PROTOTYPES
        LfnStream  stream,
        double  timeout,
        const  char  *format,
        ...)
#    else
        stream, timeout, format, va_alist)

        LfnStream  stream ;
        double  timeout ;
        char  *format ;
        va_dcl
#    endif

{    /* Local variables. */
    char  *text ;
    va_list  ap ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(lfnPutLine) NULL stream handle: ") ;
        return (errno) ;
    }

/* Format the output line, if it needs formatting. */

    if (strchr (format, '%') == NULL) {		/* Any formatting characters? */

        text = (char *) format ;

    } else {

#if HAVE_STDARG_H
        va_start (ap, format) ;
#else
        va_start (ap) ;
#endif
        (void) vsprintf (stream->outputString, format, ap) ;
        va_end (ap) ;

        text = stream->outputString ;

    }

/* Write the output line to the network connection. */

    if (tcpWrite (stream->connection, timeout, strlen (text), text, NULL)) {
        LGE "(lfnPutLine) Error writing %d-byte output line to %s(%ld).\ntcpWrite: ",
            strlen (text), lfnName (stream), (long) lfnFd (stream)) ;
        return (errno) ;
    }

    LGI "(lfnPutLine) To %s(%ld): \"%s\"\n",
        lfnName (stream), (long) lfnFd (stream), text) ;

/* Append a line terminator, if requested. */

    if (stream->terminator == 1)
        text = (char *) "\n" ;
    else if (stream->terminator == 3)
        text = (char *) "\r\n" ;
    else
        text = NULL ;

    if ((text != NULL) &&
        tcpWrite (stream->connection, timeout, strlen (text), text, NULL)) {
        LGE "(lfnPutLine) Error writing line terminator to %s(%ld).\ntcpWrite: ",
            lfnName (stream), (long) lfnFd (stream)) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    lfnRead ()

    Read Unformatted Data.


Purpose:

    Function lfnRead() reads a specified amount of unformatted data from
    a stream.


    Invocation:

        status = lfnRead (stream, timeout, numBytesToRead,
                          buffer, &numBytesRead) ;

    where

        <stream>		- I
            is the stream handle returned by lfnCreate().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the desired amount of input to be
            read.  A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) allows a read only if input is
            immediately available.
        <numBytesToRead>	- I
            has two different meanings depending on its sign.  (1) If the
            number of bytes to read is positive, lfnRead() will continue
            to read input until it has accumulated the exact number of bytes
            requested.  If the timeout interval expires before the requested
            number of bytes has been read, then lfnRead() returns with an
            EWOULDBLOCK status.  (2) If the number of bytes to read is
            negative, lfnRead() returns after reading the first "chunk"
            of input received; the number of bytes read from that first
            "chunk" is limited by the absolute value of numBytesToRead.
            A normal status (0) is returned if the first "chunk" of input
            is received before the timeout interval expires; EWOULDBLOCK
            is returned if no input is received within that interval.
        <buffer>		- O
            receives the input data.  This buffer should be at least
            numBytesToRead in size.
        <numBytesRead>	- O
            returns the actual number of bytes read.  If an infinite wait
            was specified (TIMEOUT < 0.0), then this number should equal
            (the absolute value of) numBytesToRead.  If a finite wait
            was specified, the number of bytes read may be less than the
            number requested.
        <status>		- O
            returns the status of reading the input, zero if there were
            no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  lfnRead (

#    if PROTOTYPES
        LfnStream  stream,
        double  timeout,
        ssize_t  numBytesToRead,
        char  *buffer,
        size_t  *numBytesRead)
#    else
        stream, timeout, numBytesToRead, buffer, numBytesRead)

        LfnStream  stream ;
        double  timeout ;
        ssize_t  numBytesToRead ;
        char  *buffer ;
        size_t  *numBytesRead ;
#    endif

{    /* Local variables. */
    bool  fixedAmount ;
    size_t  bufferedInput, numBytes ;
    ssize_t  length ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(lfnRead) NULL stream handle: ") ;
        return (errno) ;
    }

    fixedAmount = (numBytesToRead >= 0) ;
    if (!fixedAmount)  numBytesToRead = -numBytesToRead ;

    if (numBytesRead == NULL)  numBytesRead = &numBytes ;

/* Copy any buffered input to the caller's buffer. */

    length = stream->lastChar - stream->nextChar + 1 ;
    if (length > numBytesToRead)  length = numBytesToRead ;
    if (length > 0) {
        (void) memcpy (buffer, &stream->inputBuffer[stream->nextChar], length) ;
        stream->nextChar += length ;
        bufferedInput = (size_t) length ;
    } else {
        bufferedInput = 0 ;
    }

/* Read the rest of the data from the network. */

    length = numBytesToRead - bufferedInput ;
    if ((length > 0) && tcpRead (stream->connection, timeout,
                                 fixedAmount ? length : -length,
                                 &buffer[bufferedInput], numBytesRead)) {
        LGE "(lfnRead) Error reading %d bytes from %s(%ld).\ntcpRead: ",
            length, lfnName (stream), (long) lfnFd (stream)) ;
        return (errno) ;
    }

    *numBytesRead += bufferedInput ;

    LGI "(lfnRead) From %s(%ld): %lu bytes of unformatted data\n",
        lfnName (stream), (long) lfnFd (stream),
        (unsigned long) *numBytesRead) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    lfnWrite ()

    Write Unformatted Data.


Purpose:

    Function lfnWrite() writes a specified amount of unformatted data to
    a stream.


    Invocation:

        status = lfnWrite (stream, timeout, numBytesToWrite, buffer,
                           &numBytesWritten) ;

    where

        <stream>		- I
            is the stream handle returned by lfnCreate().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the data to be output.  A fractional
            time can be specified; e.g., 2.5 seconds.   A negative timeout
            (e.g., -1.0) causes an infinite wait; lfnWrite() will wait as
            long as necessary to output all of the data.  A zero timeout
            (0.0) specifies no wait: if the socket is not ready for writing,
            lfnWrite() returns immediately; if the socket is ready for
            writing, lfnWrite() returns after outputting whatever it can.
        <numBytesToWrite>	- I
            specifies how much data to write.  If the timeout interval
            expires before the requested number of bytes has been written,
            then lfnWrite() returns with an EWOULDBLOCK status.
        <buffer>		- I
            is the data to be output.
        <numBytesWritten>	- O
            returns the actual number of bytes written.  If an infinite wait
            was specified (TIMEOUT < 0.0), then this number should equal
            numBytesToWrite.  If a finite wait was specified, the number
            of bytes written may be less than the number requested.
        <status>		- O
            returns the status of outputting the data, zero if there were
            no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  lfnWrite (

#    if PROTOTYPES
        LfnStream  stream,
        double  timeout,
        size_t  numBytesToWrite,
        const  char  *buffer,
        size_t  *numBytesWritten)
#    else
        stream, timeout, numBytesToWrite, buffer, numBytesWritten)

        LfnStream  stream ;
        double  timeout ;
        size_t  numBytesToWrite ;
        char  *buffer ;
        size_t  *numBytesWritten ;
#    endif

{    /* Local variables. */
    size_t  numBytes ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(lfnWrite) NULL stream handle: ") ;
        return (errno) ;
    }

    if (numBytesWritten == NULL)  numBytesWritten = &numBytes ;

/* Output the data to the network. */

    if (tcpWrite (stream->connection, timeout, numBytesToWrite, buffer,
                  numBytesWritten)) {
        LGE "(lfnWrite) Error writing %d bytes to %s(%ld).\ntcpWrite: ",
            numBytesToWrite, lfnName (stream), (long) lfnFd (stream)) ;
        return (errno) ;
    }

    LGI "(lfnWrite) To %s(%ld): %lu bytes of unformatted data\n",
        lfnName (stream), (long) lfnFd (stream),
        (unsigned long) *numBytesWritten) ;

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the LFN_UTIL functions.

    Under UNIX:
        Compile and link as follows:
            % cc -DTEST lfn_util.c <libraries> -o lfn_test
        First run the server with the following command line:
            % lfn_test server <name> &
        Then, run the client (preferably in a separate window):
            % lfn_test client <name>
        The client sends 16 messages to the server and the server
        reads them.

    Under VxWorks:
        Compile and link as follows:
            % cc -DTEST lfn_util.c <libraries> -o lfn_test.vx.o
        First, load and run the server with the following commands:
            -> ld <lfn_test.vx.o
            -> sp lfn_test, "server <name>"
        Then, load (to be on the safe side) and run the client:
            -> ld <lfn_test.vx.o
            -> sp lfn_test, "client <name>"
        The client sends 16 messages to the server and the server
        reads them.

*******************************************************************************/

#ifdef VXWORKS

    void  lfn_test (
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
    LfnStream  stream ;




#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("lfn_test", commandLine, &argc, &argv) ;
#endif

    aperror_print = 1 ;
    tcp_util_debug = 1 ;
    lfn_util_debug = 1 ;

    if (argc < 3) {
        fprintf (stderr, "Usage:  lfn_test client|server <name>\n") ;
        exit (EINVAL) ;
    }

    if (strcmp (argv[1], "client") == 0) {	/* The Client? */

        tcpCall (argv[2], 0, &connection) ;
        lfnCreate (connection, "-buffer 1024", &stream) ;
        for (i = 0 ;  i < 16 ;  i++) {
            lfnPutLine (stream, -1.0, "Message #%d\n", i) ;
        }
        lfnDestroy (stream) ;

    } else {					/* The Server. */

        tcpListen (argv[2], -1, &listeningPoint) ;
        tcpAnswer (listeningPoint, -1.0, &connection) ;
        lfnCreate (connection, "-buffer 1024", &stream) ;
        for ( ; ; ) {
            if (lfnGetLine (stream, -1.0, &string)) {
                LGE "Error reading from connection on %s.\nlfnGetLine: ",
                    argv[2]) ;
                break ;
            }
            printf ("[SERVER] Input: \"%s\"\n", string) ;
        }
        lfnDestroy (stream) ;

    }

}

#endif  /* TEST */
