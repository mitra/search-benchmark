/* $Id: bio_util.c,v 1.11 2011/07/18 17:37:46 alex Exp $ */
/*******************************************************************************

File:

    bio_util.c

    Buffered Input/Output Utilities.


Author:    Alex Measday


Purpose:

    The BIO_UTIL package ...


Public Procedures:

    bioCreate() - creates a buffered I/O stream.
    bioDestroy() - deletes a buffered I/O stream.
    bioFlush() - flushes buffered output.
    bioPendingInput() - returns a byte count of buffered input.
    bioPendingOutput() - returns a byte count of buffered output.
    bioRead() - reads from a buffered input stream.
    bioWrite() - writes to a buffered output stream.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "meo_util.h"			/* Memory operations. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "bio_util.h"			/* Buffered I/O streams. */


/*******************************************************************************
    Buffered Input/Output Stream - contains information about a buffered I/O
        stream, including its input and output buffers and its input and output
        functions.  There is one input buffer that is filled as needed.  As
        many output buffers as needed are created and linked in a queue; the
        pointer in the BioStream structure points to the last buffer in the
        queue, which, in turn, is linked to the first buffer in the queue.
*******************************************************************************/

#define  MAX_BUF_SIZE  (32UL*1024UL)	/* Arbitrarily chosen! */
#if MAX_BUF_SIZE > INT_MAX
#    undef  MAX_BUF_SIZE
#    define  MAX_BUF_SIZE  INT_MAX	/* For PalmOS NetLibReceive(). */
#endif

typedef  struct  BioBuffer {
    size_t  numBytes ;			/* # of bytes of actual data in buffer. */
    size_t  nextByte ;			/* Index of next byte in buffer. */
    char  *data ;			/* Buffer of data. */
    struct  BioBuffer  *next ;		/* Link to next buffer. */
}  BioBuffer ;

typedef  struct  _BioStream {
    void  *ioStream ;			/* Underlying stream being buffered. */
    BioInputF  inputF ;			/* Reads data from underlying stream. */
    size_t  inputBufferSize ;		/* Input buffer size in bytes. */
    BioBuffer  *input ;			/* Input buffer. */
    BioOutputF  outputF ;		/* Writes data to underlying stream. */
    size_t  outputBufferSize ;		/* Output buffer size in bytes. */
    BioBuffer  *output ;		/* Queue of output buffers. */
}  _BioStream ;


int  bio_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  bio_util_debug
int  bio_timing_debug = 0 ;		/* I/O timing debug switch (1/0 = yes/no). */


#define  DESTROY(stream) \
    (bioDestroy (stream), stream = NULL)

/*!*****************************************************************************

Procedure:

    bioCreate ()

    Create a Buffered Input/Output Stream.


Purpose:

    Function bioCreate() creates a buffered I/O stream on top of a
    previously-created data stream (e.g., a network connection).


    Invocation:

        status = bioCreate (ioStream, inputF, inputBufferSize,
                            outputF, outputBufferSize, &stream) ;

    where

        <ioStream>		- I
            is the underlying data stream to be buffered, cast as a (VOID *)
            pointer.  NOTE that the data stream is NOT destroyed by the
            BIO_UTIL package when the buffered I/O stream itself is destroyed;
            the application is responsible for destroying the underlying data
            stream.
        <inputF>		- I
            is the function that bioRead() calls to actually read data from the
            underlying data stream.  The function should be declared as follows:
                errno_t  input_function (void *ioStream,
                                         double timeout,
                                         size_t numBytesToRead,
                                         char *buffer,
                                         size_t *numBytesRead) ;
            where "ioStream" is the underlying data stream handle, "timeout"
            specifies how long (in seconds) the input function should wait for
            input, "numBytesToRead" specifies how many bytes to read, "buffer"
            is the address of the buffer into which the data should be read,
            and "numBytesRead" returns the actual number of bytes read.  The
            function's return value indicates if the read was successful (zero
            is returned) or if it failed (ERRNO is returned).
        <inputBufferSize>	- I
            is the size in bytes of the input buffer; if a value of zero is
            specified, the default is 32K bytes.
        <outputF>		- I
            is the function that bioFlush() calls to actually write data to the
            underlying data stream.  The function should be declared as follows:
                errno_t  output_function (void *ioStream,
                                          double timeout,
                                          size_t numBytesToWrite,
                                          const char *buffer,
                                          size_t *numBytesWritten) ;
            where "ioStream" is the underlying data stream handle, "timeout"
            specifies how long (in seconds) the output function should wait to
            output data, "numBytesToWrite" specifies how many bytes to write,
            "buffer" contains the data to be written, and "numBytesWritten"
            returns the actual number of bytes written.  The function's return
            value indicates if the write was successful (zero is returned) or
            if it failed (ERRNO is returned).
        <outputBufferSize>	- I
            is the size in bytes of each output buffer; if a value of zero is
            specified, the default is 32K bytes.
        <stream>		- O
            returns a handle for the new, buffered I/O stream.  This handle is
            used in calls to the other BIO_UTIL functions.
        <status>		- O
            returns the status of creating the stream, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  bioCreate (

#    if PROTOTYPES
        void  *ioStream,
        BioInputF  inputF,
        size_t  inputBufferSize,
        BioOutputF  outputF,
        size_t  outputBufferSize,
        BioStream  *stream)
#    else
        ioStream, inputF, inputBufferSize, outputF, outputBufferSize, stream)

        void  *ioStream ;
        BioInputF  inputF ;
        size_t  inputBufferSize ;
        BioOutputF  outputF ;
        size_t  outputBufferSize ;
        BioStream  *stream ;
#    endif

{

/* Create and initialize a buffered I/O stream structure for the data stream. */

    *stream = (_BioStream *) malloc (sizeof (_BioStream)) ;
    if (*stream == NULL) {
        LGE "(bioCreate) Error allocating stream structure.\nmalloc: ") ;
        return (errno) ;
    }

    (*stream)->ioStream = ioStream ;
    (*stream)->inputF = inputF ;
    (*stream)->inputBufferSize = (inputBufferSize > 0) ? inputBufferSize
                                                       : MAX_BUF_SIZE ;
    (*stream)->input = NULL ;
    (*stream)->outputF = outputF ;
    (*stream)->outputBufferSize = (outputBufferSize > 0) ? outputBufferSize
                                                         : MAX_BUF_SIZE ;
    (*stream)->output = NULL ;

    LGI "(bioCreate) Created buffer I/O stream %p for data stream %p.\n",
        (void *) *stream, ioStream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    bioDestroy ()

    Delete a Buffered I/O Stream.


Purpose:

    Function bioDestroy() destroys a buffered I/O stream.  NOTE that the
    underlying data stream is NOT destroyed; the application is responsible
    for doing so.


    Invocation:

        status = bioDestroy (stream) ;

    where

        <stream>	- I
            is the stream handle returned by bioCreate().
        <status>	- O
            returns the status of deleting the stream, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  bioDestroy (

#    if PROTOTYPES
        BioStream  stream)
#    else
        stream)

        BioStream  stream ;
#    endif

{

    if (stream == NULL)  return (0) ;

    LGI "(bioDestroy) Closing buffered I/O stream %p ...\n", (void *) stream) ;

/* Deallocate the input buffer. */

    if (stream->input != NULL) {
        if (stream->input->data != NULL) {
            free (stream->input->data) ;  stream->input->data = NULL ;
        }
        free (stream->input) ;  stream->input = NULL ;
    }

/* Deallocate the output buffers. */

    if (stream->output != NULL) {
        BioBuffer  *firstBuffer, *lastBuffer = stream->output ;
        do {
            firstBuffer = lastBuffer->next ;
            lastBuffer->next = firstBuffer->next ;
            if (firstBuffer->data != NULL) {
                free (firstBuffer->data) ;  firstBuffer->data = NULL ;
            }
            free (firstBuffer) ;
        } while (firstBuffer != lastBuffer) ;
        stream->output = NULL ;
    }

/* Deallocate the buffered I/O stream structure. */

    free (stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    bioFlush ()

    Flushes Buffered Output Data.


Purpose:

    Function bioFlush() attempts to write any buffered output data to a
    buffered I/O stream's underlying data stream.


    Invocation:

        status = bioFlush (stream) ;

    where

        <stream>		- I
            is the stream handle returned by bioCreate().
        <status>		- O
            returns the status of attempting to flush the output,
            zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  bioFlush (

#    if PROTOTYPES
        BioStream  stream)
#    else
        stream)

        BioStream  stream ;
#    endif

{    /* Local variables. */
    BioBuffer  *firstBuffer, *lastBuffer ;
    size_t  numBytesWritten ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(bioFlush) NULL stream handle: ") ;
        return (errno) ;
    }


/*******************************************************************************
    Beginning with the first buffer in the output queue, attempt to output as
    much data as possible to the underlying data stream.
*******************************************************************************/

    lastBuffer = stream->output ;
    if (lastBuffer == NULL)  return (0) ;
    firstBuffer = lastBuffer->next ;

    for ( ; ; ) {

/* Attempt to output the next batch of buffered data. */

        if (stream->outputF (stream->ioStream, 0.0,
                             firstBuffer->numBytes - firstBuffer->nextByte,
                             &firstBuffer->data[firstBuffer->nextByte],
                             &numBytesWritten)) {
            if (errno == EWOULDBLOCK) {
                break ;					/* No data flushed. */
            } else {
                LGE "(bioFlush) Error flushing %lu bytes to stream %p.\n",
                    firstBuffer->numBytes - firstBuffer->nextByte, stream) ;
                return (errno) ;
            }
        }

        firstBuffer->nextByte += numBytesWritten ;

        LGI "(bioFlush) Wrote %lu bytes to stream %p.\n",
            (unsigned long) numBytesWritten, (void *) stream) ;

/* If the current buffer has been completely flushed, then advance to the
   next buffer. */

        if (firstBuffer->nextByte == firstBuffer->numBytes) {
            lastBuffer->next = firstBuffer->next ;	/* Unlink queue head. */
            free (firstBuffer->data) ;
            free (firstBuffer) ;
            if (firstBuffer == lastBuffer) {
                stream->output = NULL ;
                break ;
            } else {
                firstBuffer = lastBuffer->next ;
            }
        }

    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    bioPendingInput ()

    Count the Number of Bytes of Buffered Input.


Purpose:

    Function bioPendingInput() returns the number of bytes of data remaining
    in the stream's input buffer.


    Invocation:

        numBytes = bioPendingInput (stream) ;

    where

        <stream>	- I
            is the stream handle returned by bioCreate().
        <numBytes>	- O
            returns the number of bytes of data remaining in the stream's
            input buffer.

*******************************************************************************/


size_t  bioPendingInput (

#    if PROTOTYPES
        BioStream  stream)
#    else
        stream)

        BioStream  stream ;
#    endif

{

    if (stream == NULL)
        return (0) ;
    else if (stream->input == NULL)
        return (0) ;
    else
        return ((stream->input)->numBytes - (stream->input)->nextByte) ;

}

/*!*****************************************************************************

Procedure:

    bioPendingOutput ()

    Count the Number of Bytes of Buffered Output.


Purpose:

    Function bioPendingOutput() returns the number of bytes of data remaining
    in the stream's output buffers, waiting to be flushed to the underlying
    data stream.


    Invocation:

        numBytes = bioPendingOutput (stream) ;

    where

        <stream>	- I
            is the stream handle returned by bioCreate().
        <numBytes>	- O
            returns the number of bytes of data remaining in the stream's
            output buffers.

*******************************************************************************/


size_t  bioPendingOutput (

#    if PROTOTYPES
        BioStream  stream)
#    else
        stream)

        BioStream  stream ;
#    endif

{    /* Local variables. */
    BioBuffer  *buffer ;
    size_t  count ;



    if (stream == NULL)  return (0) ;

/* Count the number of bytes in each output buffer. */

    count = 0 ;
    for (buffer = stream->output ;  buffer != NULL ;  buffer = buffer->next) {
        count += (buffer->numBytes - buffer->nextByte) ;
    }

    return (count) ;

}

/*!*****************************************************************************

Procedure:

    bioRead ()

    Read Data from a Buffered I/O Stream.


Purpose:

    Function bioRead() reads data from a buffered I/O stream ...

    A timeout can be specified that limits how long bioRead() waits for the
    first data to arrive.  bioRead() will then wait as long as necessary for
    the remainder of the desired amount of data to be received.  This ensures
    that a partial record will NOT be returned at the end of the timeout
    interval.


    Invocation:

        status = bioRead (stream, timeout, numBytesToRead,
                          buffer, &numBytesRead) ;

    where

        <stream>		- I
            is the stream handle returned by bioCreate().
        <timeout>		- I
            specifies the maximum amount of time (in seconds) that the caller
            wishes to wait for the first data to arrive.  A fractional time can
            be specified; e.g., 2.5 seconds.  A negative timeout (e.g., -1.0)
            causes an infinite wait; a zero timeout (0.0) allows a read only
            if input is immediately available.
        <numBytesToRead>	- I
            specifies the number of bytes to read.  If the timeout interval
            expires before ANY data has been read, then bioRead() returns with
            an EWOULDBLOCK status.  If some of the data is read before the
            timeout interval expires, bioRead() will wait as long as necessary
            to read the remaining data.  This ensures that a partial record is
            not returned to the caller at the end of the timeout interval.
        <buffer>		- O
            receives the input data.  This buffer should be at least
            numBytesToRead in size.
        <numBytesRead>		- O
            returns the actual number of bytes read.
        <status>		- O
            returns the status of reading from the stream: zero if no errors
            occurred, EWOULDBLOCK if the timeout interval expired before the
            requested amount of data was input, and ERRNO otherwise.

*******************************************************************************/


errno_t  bioRead (

#    if PROTOTYPES
        BioStream  stream,
        double  timeout,
        size_t  numBytesToRead,
        char  *buffer,
        size_t  *numBytesRead)
#    else
        stream, timeout, numBytesToRead, buffer, numBytesRead)

        BioStream  stream ;
        double  timeout ;
        size_t  numBytesToRead ;
        char  *buffer ;
        size_t  *numBytesRead ;
#    endif

{    /* Local variables. */
    BioBuffer  *input ;
    size_t  numBytesToCopy, numBytesToGo ;




    if (numBytesRead != NULL)  *numBytesRead = 0 ;

    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(bioRead) NULL stream handle: ") ;
        return (errno) ;
    }


/*******************************************************************************
    If an input buffer hasn't been allocated yet, then do so.
*******************************************************************************/

    if (stream->input == NULL) {
        stream->input = (BioBuffer *) malloc (sizeof (BioBuffer)) ;
        if (stream->input == NULL) {
            LGE "(bioRead) Error allocating input buffer header for stream %p.\nmalloc: ",
                stream) ;
            return (errno) ;
        }
        stream->input->numBytes = 0 ;
        stream->input->nextByte = 0 ;
        stream->input->data = NULL ;
        stream->input->next = NULL ;
    }

    if (stream->input->data == NULL) {
        stream->input->data = malloc (stream->inputBufferSize) ;
        if (stream->input->data == NULL) {
            LGE "(bioRead) Error allocating %lu-byte input buffer for stream %p.\nmalloc: ",
                (unsigned long) stream->inputBufferSize, stream) ;
            return (errno) ;
        }
    }


/*******************************************************************************
    Read the requested amount of data.
*******************************************************************************/

    input = stream->input ;
    numBytesToGo = numBytesToRead ;

    while (numBytesToGo > 0) {

/* If the input buffer is exhausted, then replenish it. */

        if (input->nextByte >= input->numBytes) {
            if (stream->inputF (stream->ioStream, timeout,
                                - ((ssize_t) stream->inputBufferSize),
                                input->data, &input->numBytes)) {
                if (errno != EWOULDBLOCK) {
                    LGE "(bioRead) Error reading data from %p's underlying I/O stream, %p.\n",
                        stream, stream->ioStream) ;
                }
                return (errno) ;
            }
            if (bio_timing_debug) {
                I_DEFAULT_HANDLER (I_DEFAULT_PARAMS "%s Read %lu bytes ...\n",
                                   tvShow (tvTOD (), false, NULL),
                                   (unsigned long) input->numBytes) ;
            }
            LGI "(bioRead) Read %lu bytes from %p's underlying I/O stream, %p.\n",
                (unsigned long) input->numBytes, (void *) stream,
                stream->ioStream) ;
            input->nextByte = 0 ;
        }

/* The timeout only applies to reading the very first byte of the
   requested amount of data.  Reset the timeout so that subsequent
   reads have an infinite timeout. */

        timeout = -1.0 ;

/* Copy the available number of bytes from the stream's input buffer
   into the caller's input buffer. */

        numBytesToCopy = input->numBytes - input->nextByte ;
        if (numBytesToCopy > numBytesToGo)  numBytesToCopy = numBytesToGo ;
        (void) memcpy (buffer + numBytesToRead - numBytesToGo,
                       &input->data[input->nextByte], numBytesToCopy) ;
        numBytesToGo -= numBytesToCopy ;  input->nextByte += numBytesToCopy ;

    }


    LGI "(bioRead) Returning %lu bytes from stream %p.\n",
        (unsigned long) numBytesToRead, (void *) stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    bioWrite ()

    Write Data to a Buffered I/O Stream.


Purpose:

    Function bioWrite() writes data to a buffered I/O stream ...


    Invocation:

        status = bioWrite (stream, numBytesToWrite, data) ;

    where

        <stream>		- I
            is the stream handle returned by bioCreate().
        <numBytesToWrite>	- I
            specifies the number of bytes to write.
        <data>			- O
            is the data to be output.
        <status>		- O
            returns the status of queuing the data for output,
            zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  bioWrite (

#    if PROTOTYPES
        BioStream  stream,
        size_t  numBytesToWrite,
        const  char  *data)
#    else
        stream, numBytesToWrite, data)

        BioStream  stream ;
        size_t  numBytesToWrite ;
        char  *data ;
#    endif

{    /* Local variables. */
    BioBuffer  *lastBuffer ;
    size_t  length ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(bioWrite) NULL stream handle: ") ;
        return (errno) ;
    }


/*******************************************************************************
    Copy the caller's data to buffers in the output queue.
*******************************************************************************/

    lastBuffer = stream->output ;

    while (numBytesToWrite > 0) {

/* If the last buffer is full (or non-existent), create a new one and add it
   to the output queue. */

        if ((lastBuffer == NULL) ||
            (lastBuffer->numBytes == stream->outputBufferSize)) {

            lastBuffer = (BioBuffer *) malloc (sizeof (BioBuffer)) ;
            if (lastBuffer == NULL) {
                LGE "(bioWrite) Error allocating output buffer header for stream %p.\nmalloc: ",
                    stream) ;
                return (errno) ;
            }
            lastBuffer->numBytes = 0 ;
            lastBuffer->nextByte = 0 ;
            lastBuffer->data = NULL ;
            lastBuffer->next = (stream->output)->next ;	/* Link to queue head. */
            stream->output = lastBuffer ;		/* Append to queue. */

        }

        if (lastBuffer->data == NULL) {
            lastBuffer->data = malloc (stream->outputBufferSize) ;
            if (lastBuffer->data == NULL) {
                LGE "(bioWrite) Error allocating %lu-byte output buffer for stream %p.\nmalloc: ",
                    (unsigned long) stream->outputBufferSize, stream) ;
                return (errno) ;
            }
        }

/* Add as much data as possible to the last buffer in the queue. */

        length = stream->outputBufferSize - lastBuffer->numBytes ;
        if (length > numBytesToWrite)  length = numBytesToWrite ;

        (void) memcpy (&lastBuffer->data[lastBuffer->numBytes], data, length) ;

        lastBuffer->numBytes += length ;
        numBytesToWrite -= length ;
        data += length ;

        LGI "(bioWrite) Added %lu bytes to buffer %p, stream %p.\n",
            (unsigned long) length, (void *) lastBuffer, (void *) stream) ;

    }


    return (0) ;

}
