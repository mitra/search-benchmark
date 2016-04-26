/* $Id: words_lfn.c,v 1.1 2005/01/10 19:31:55 alex Exp alex $ */
/*******************************************************************************

File:

    words_lfn.c

    Line Feed-Terminated Networking Utilities.


Author:    Alex Measday


Purpose:

    The WORDS_LFN package defines words for sending and receiving
    LF-terminated text over a network connection.

        <endpoint> "<options>" LFN-CREATE
        <value> LFN-DEBUG
        <stream> LFN-DESTROY
        <stream> LFN-FD
        <stream> <timeout> LFN-GETLINE
        <stream> LFN-NAME
        <string> <stream> <crlf> <timeout> LFN-PUTLINE
        <buffer> <length> <stream> <timeout> LFN-READ
        <stream> LFN-READABLE?
        <stream> LFN-UP?
        <buffer> <length> <stream> <timeout> LFN-WRITE
        <stream> LFN-WRITEABLE?


Public Procedures:

    buildWordsLFN() - registers the words with the FICL system.

Private Procedures:

    word_LFN_CREATE() - implements the LFN-CREATE word.
    word_LFN_DEBUG() - implements the LFN-DEBUG word.
    word_LFN_DESTROY() - implements the LFN-DESTROY word.
    word_LFN_FD() - implements the LFN-FD word.
    word_LFN_GETLINE() - implements the LFN-GETLINE word.
    word_LFN_NAME() - implements the LFN-NAME word.
    word_LFN_PUTLINE() - implements the LFN-PUTLINE word.
    word_LFN_READ() - implements the LFN-READ word.
    word_LFN_READABLEq() - implements the LFN-READABLE? word.
    word_LFN_UPq() - implements the LFN-UP? word.
    word_LFN_WRITE() - implements the LFN-WRITE word.
    word_LFN_WRITEABLEq() - implements the LFN-WRITEABLE? word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_LFN_CREATE P_((ficlVm *vm)) ;
static  void  word_LFN_DEBUG P_((ficlVm *vm)) ;
static  void  word_LFN_DESTROY P_((ficlVm *vm)) ;
static  void  word_LFN_FD P_((ficlVm *vm)) ;
static  void  word_LFN_GETLINE P_((ficlVm *vm)) ;
static  void  word_LFN_NAME P_((ficlVm *vm)) ;
static  void  word_LFN_PUTLINE P_((ficlVm *vm)) ;
static  void  word_LFN_READ P_((ficlVm *vm)) ;
static  void  word_LFN_READABLEq P_((ficlVm *vm)) ;
static  void  word_LFN_UPq P_((ficlVm *vm)) ;
static  void  word_LFN_WRITE P_((ficlVm *vm)) ;
static  void  word_LFN_WRITEABLEq P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsLFN ()

    Enter the LFN Words into the Dictionary.


Purpose:

    Function buildWordsLFN() enters the LFN words into the system dictionary.


    Invocation:

        buildWordsLFN (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsLFN (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-CREATE", word_LFN_CREATE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-DEBUG", word_LFN_DEBUG,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-DESTROY", word_LFN_DESTROY,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-FD", word_LFN_FD,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-GETLINE", word_LFN_GETLINE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-NAME", word_LFN_NAME,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-PUTLINE", word_LFN_PUTLINE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-READ", word_LFN_READ,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-READABLE?", word_LFN_READABLEq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-UP?", word_LFN_UPq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-WRITE", word_LFN_WRITE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "LFN-WRITEABLE?", word_LFN_WRITEABLEq,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_CREATE ()

    Create a LF-Terminated Network Stream.


Purpose:

    Function word_LFN_CREATE() implements the LFN-CREATE word, which
    creates a LF-terminated network stream.

        LFN-CREATE

            ( ep c-addr u -- st 0 | ior )

        Create a LF-terminated network stream on top of previously-created
        network endpoint ep (i.e., using TCP-ANSWER or TCP-CALL).  The stream
        takes ownership of the endpoint; the endpoint will automatically be
        destroyed when the stream is destroyed.  The stream is returned on
        the stack as st, along with the I/O result ior.

        The c-addr/u string contains zero or more of the following UNIX
        command line-style options:

            "-input <size>"
                specifies the size of the stream's internal input buffer;
                the default is 2048 bytes.  NOTE that this is only a limit
                on the input buffer, not on incoming strings.
            "-output <length>"
                specifies the maximum output message size for the stream;
                the default is 2047 bytes.


    Invocation:

        word_LFN_CREATE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_CREATE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *options ;
    ficlCell  cell ;
    int  ior ;
    LfnStream  stream ;
    TcpEndpoint  dataPoint ;
    unsigned  long  length ;



    FICL_STACK_CHECK (vm->dataStack, 3, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Options. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        options = NULL ;
    } else {
        options = (char *) cell.p ;
        if (options[length] != '\0')  options = strndup (options, length) ;
    }
    cell = ficlVmPop (vm) ;			/* Data endpoint. */
    dataPoint = (TcpEndpoint) cell.p ;

/* Create the stream. */

    ior = lfnCreate (dataPoint, options, &stream) ;

    if (options != cell.p)  free (options) ;

/* Return the stream and the I/O result on the stack. */

    if (ior == 0) {			/* Successfully created stream? */
        cell.p = stream ;
        ficlVmPush (vm, cell) ;
    }
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_DEBUG ()

    Enable/Disable LF-Terminated Networking Debug Output.


Purpose:

    Function word_LFN_DEBUG() implements the LFN-DEBUG word, which enables
    or disables LF-terminated networking debug.

        LFN-DEBUG

            ( n -- )

        Set the LF-terminated networking debug flag to n.  A value of 0
        disables debug; a non-zero value enables debug.  Debug is written
        to standard output.


    Invocation:

        word_LFN_DEBUG (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_DEBUG (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;



    FICL_STACK_CHECK (vm->dataStack, 1, 0) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Debug flag. */
    lfn_util_debug = cell.i ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_DESTROY ()

    Close a LF-Terminated Stream.


Purpose:

    Function word_LFN_DESTROY() implements the LFN-DESTROY word, which
    destroys a LF-terminated network stream.

        LFN-DESTROY

            ( st -- ior )

        Close LF-terminated network stream st and its underlying TCP/IP
        endpoint; the stream should no longer be referenced.


    Invocation:

        word_LFN_DESTROY (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_DESTROY (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Close the stream. */

    ior = lfnDestroy (stream) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_FD ()

    Get a LF-Terminated Stream's Socket.


Purpose:

    Function word_LFN_FD() implements the LFN-FD word, which returns
    the socket associated with a LF-terminated network stream.

        LFN-FD

            ( st -- fd )

        Get LF-terminated stream st's socket.


    Invocation:

        word_LFN_FD (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_FD (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    IoFd  fd ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Get the stream's socket. */

    fd = lfnFd (stream) ;

/* Return the socket on the stack. */

    cell.i = fd ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_GETLINE ()

    Read a Line of Input.


Purpose:

    Function word_LFN_GETLINE() implements the LFN-GETLINE word,
    which reads the next, CR/LF-delimited line of input from a stream.

        LFN-GETLINE

            ( st r -- c-addr u ior )

        Read the next CR/LF-delimited line of input from LF-terminated
        network stream st and return it as c-addr/u.  The string is
        NUL-terminated and stored internally; it should be used or
        duplicated before calling LFN-GETLINE on this stream again.
        An address of NULL and a length of zero are returned in the
        event of an error.

        Timeout r specifies the maximum amount of time (in seconds)
        that the application wishes to wait for the next line to be
        read.  A negative timeout (e.g., -1E0) causes an infinite wait;
        a zero timeout (0E0) allows a read only if input is immediately
        available.  If the timeout expires before a line of input has
        been read, the I/O result is EWOULDBLOCK.


    Invocation:

        word_LFN_GETLINE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_GETLINE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *string ;
    double  timeout ;
    ficlCell  cell ;
    int  ior ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 3) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */
						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Read the next line from the stream. */

    ior = lfnGetLine (stream, timeout, &string) ;

    if (ior != 0)  string = NULL ;

/* Return the input line and the I/O result on the stack. */

    cell.p = string ;
    ficlVmPush (vm, cell) ;
    cell.u = (string == NULL) ? 0 : strlen (string) ;
    ficlVmPush (vm, cell) ;
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_NAME ()

    Get a LF-Terminated Stream's Name.


Purpose:

    Function word_LFN_NAME() implements the LFN-NAME word, which returns
    the name of a LF-terminated network stream.

        LFN-NAME

            ( st -- c-addr u )

        Get LF-terminated network stream st's name and return it as c-addr/u.
        The string is NUL-terminated and stored internally; it should be used
        or duplicated before calling LFN-NAME again.  An address of NULL and
        a length of zero are returned in the event of an error.


    Invocation:

        word_LFN_NAME (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_NAME (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *name ;
    ficlCell  cell ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Get the stream's name. */

    name = (char *) lfnName (stream) ;

/* Return the stream's name on the stack. */

    cell.p = name ;
    ficlVmPush (vm, cell) ;
    cell.u = (name == NULL) ? 0 : strlen (name) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_PUTLINE ()

    Write a Line of Output.


Purpose:

    Function word_LFN_PUTLINE() implements the LFN-PUTLINE word,
    which writes output to a LF-terminated network stream.

        LFN-PUTLINE

            ( c-addr u st n r -- ior )

        Write the string c-addr/u to LF-terminated network stream st.
        The I/O result is returned on the stack.

        Bit mask n specifies line terminators to be appended to the
        output string: 0 = no terminator, 1 = LF only, 2 = CR only,
        and 3 = CR/LF.  Zero is typically used if the application
        explicitly puts the line terminators in the output string.

        Timeout r specifies the maximum amount of time (in seconds) that the
        application wishes to wait for the line to be output.  A negative
        timeout (e.g., -1E0) causes an infinite wait.  A zero timeout (0E0)
        specifies no wait: if the connection is not ready for writing,
        LFN-PUTLINE returns immediately; if the connection is ready for
        writing, LFN-PUTLINE returns after outputting whatever it can.
        If the timeout expires before the output line has been written,
        an I/O result of EWOULDBLOCK is returned on the stack.


    Invocation:

        word_LFN_PUTLINE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_PUTLINE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *string ;
    double  timeout ;
    ficlCell  cell ;
    int  ior, terminators ;
    LfnStream  stream ;
    unsigned  long  length ;
    size_t  numBytesToWrite, numBytesWritten ;



    FICL_STACK_CHECK (vm->dataStack, 3, 1) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */
						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Line terminator mask. */
    terminators = cell.i ;
    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;
    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Output line. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        string = NULL ;
        length = 0 ;
    }

/* Append the line terminators, if any. */

    string = strndup ((char *) cell.p, length+2) ;
    if (terminators & 0x02)  string[length++] = '\r' ;
    if (terminators & 0x01)  string[length++] = '\n' ;
    string[length] = '\0' ;

/* Output the line to the stream. */

    numBytesToWrite = length ;
    ior = lfnWrite (stream, timeout, numBytesToWrite, string,
                    &numBytesWritten) ;

    free (string) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_READ ()

    Read Unformatted Data.


Purpose:

    Function word_LFN_READ() implements the LFN-READ word, which reads
    unformatted data from a LF-terminated network stream.

        LFN-READ

            ( c-addr n st r -- u ior )

        Read n bytes of data into buffer c-addr from LF-terminated network
        stream st.  The actual number of bytes read, u, and the I/O result
        are returned on the stack.

        Because of the way network I/O works, a single record written to a
        connection by one task may be read in multiple "chunks" by the task
        at the other end of the connection.  LFN-READ takes this into account
        and, if you ask it for 100 bytes, it will automatically perform however
        many network reads are necessary to collect the 100 bytes.

        If n is negative, LFN-READ returns after reading the first "chunk"
        of input received; the number of bytes read from that first "chunk"
        is limited to the absolute value of n.  The actual number of bytes
        read is returned as u on the stack.

        Timeout r specifies the maximum amount of time (in seconds) that the
        application wishes to wait for the first data to arrive.  A negative
        timeout (e.g., -1E0) causes an infinite wait; a zero timeout (0E0)
        allows a read only if input is immediately available.  If the timeout
        expires before the requested amount of data has been read, the actual
        number of bytes read is returned on the stack as u, along with an
        I/O result of EWOULDBLOCK.


    Invocation:

        word_LFN_READ (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_READ (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *buffer ;
    double  timeout ;
    ficlCell  cell ;
    int  ior ;
    LfnStream  stream ;
    ssize_t  numBytesToRead ;
    size_t  numBytesRead ;



    FICL_STACK_CHECK (vm->dataStack, 3, 2) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;
    cell = ficlVmPop (vm) ;			/* # of bytes to read. */
    numBytesToRead = (ssize_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Input buffer. */
    buffer = cell.p ;

/* Read the data from the network stream. */

    ior = lfnRead (stream, timeout, numBytesToRead, buffer, &numBytesRead) ;

/* Return the number of bytes read and the I/O result on the stack. */

    cell.u = (unsigned long) numBytesRead ;
    ficlVmPush (vm, cell) ;
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_READABLEq ()

    Check if Data is Waiting to be Read from a LF-Terminated Stream.


Purpose:

    Function word_LFN_READABLEq() implements the LFN-READABLE? word,
    which checks to see if data is waiting to be read from a LF-termianted
    network stream.

        LFN-READABLE?

            ( st -- f )

        Check if data is waiting to be read from LF-terminated network
        stream st; return true if the stream is readable and false
        otherwise.  Because input is buffered, LFN-READABLE? is not
        equivalent to "<stream> LFN-FD SKT-READABLE?".  LFN-ISREADBLE
        first checks for currently buffered input; if there is none,
        LFN-READABLE? then checks the socket.


    Invocation:

        word_LFN_READABLEq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_READABLEq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Check if the stream is readable. */

    flag = lfnIsReadable (stream) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_UPq ()

    Check if a Stream's Network Connection is Up.


Purpose:

    Function word_LFN_UPq() implements the LFN-UP? word, which checks
    to see if a LF-terminated stream's underlying network connection is
    still up.

        LFN-UP?

            ( st -- f )

        Check LF-terminated network stream st to see if its network connection
        is still up; return true if the connection is up and false otherwise.


    Invocation:

        word_LFN_UPq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_UPq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Check if the connection is up. */

    flag = lfnIsUp (stream) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_WRITE ()

    Write Unformatted Data.


Purpose:

    Function word_LFN_WRITE() implements the LFN-WRITE word, which writes
    unformatted data to a LF-terminated network stream.

        LFN-WRITE

            ( c-addr u1 st r -- u2 ior )

        Write u1 bytes of data from buffer c-addr to LF-terminated network
        stream st.  The actual number of bytes written, u2, and the I/O result
        are returned on the stack.

        Because of the way network I/O works, attempting to output a given
        amount of data to a network connection may require multiple system
        WRITE(2)s.  LFN-WRITE takes this into account and, if you ask it to
        output 100 bytes, it will call WRITE(2) as many times as necessary
        to output the full 100 bytes of data to the connection.

        Timeout r specifies the maximum amount of time (in seconds) that the
        application wishes to wait for the data to be output.  A negative
        timeout (e.g., -1E0) causes an infinite wait; LFN-WRITE will wait
        as long as necessary to output all of the data.  A zero timeout (0E0)
        specifies no wait: if the connection is not ready for writing,
        LFN-WRITE returns immediately; if the connection is ready for
        writing, LFN-WRITE returns after outputting whatever it can.

        If the timeout expires before the requested amount of data has been
        written, the actual number of bytes written is returned on the stack
        as u2, along with an I/O result of EWOULDBLOCK.


    Invocation:

        word_LFN_WRITE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_WRITE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *buffer ;
    double  timeout ;
    ficlCell  cell ;
    int  ior ;
    LfnStream  stream ;
    size_t  numBytesToWrite, numBytesWritten ;



    FICL_STACK_CHECK (vm->dataStack, 3, 2) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;
    cell = ficlVmPop (vm) ;			/* # of bytes to write. */
    numBytesToWrite = (size_t) cell.u ;
    cell = ficlVmPop (vm) ;			/* Output buffer. */
    buffer = cell.p ;

/* Write the data to the network stream. */

    ior = lfnWrite (stream, timeout, numBytesToWrite, buffer,
                    &numBytesWritten) ;

/* Return the number of bytes written and the I/O result on the stack. */

    cell.u = (unsigned long) numBytesWritten ;
    ficlVmPush (vm, cell) ;
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_LFN_WRITEABLEq ()

    Check if a Stream is Ready for Writing.


Purpose:

    Function word_LFN_WRITEABLEq() implements the LFN-WRITEABLE? word,
    which checks to see if data can be written to a LF-terminated network
    stream.

        LFN-WRITEABLE?

            ( st -- f )

        Check if data can be written to LF-terminated network stream st;
        return true if the stream is writeable and false otherwise.


    Invocation:

        word_LFN_WRITEABLEq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_LFN_WRITEABLEq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    LfnStream  stream ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Stream. */
    stream = (LfnStream) cell.p ;

/* Check if the stream is writeable. */

    flag = lfnIsWriteable (stream) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}
