/* $Id: words_tcp.c,v 1.1 2005/01/10 19:31:55 alex Exp alex $ */
/*******************************************************************************

File:

    words_tcp.c

    TCP/IP Utilities.


Author:    Alex Measday


Purpose:

    The WORDS_TCP package defines words for establishing and communicating
    over TCP/IP network connections:

        <endpoint> <timeout> TCP-ANSWER
        "<service>[@<host>]" <noWait?> TCP-CALL
        <endpoint> <timeout> <destroy?> TCP-COMPLETE
        <value> TCP-DEBUG
        <endpoint> TCP-DESTROY
        <endpoint> TCP-FD
        <port> <backlog> TCP-LISTEN
        <endpoint> TCP-NAME
        <endpoint> TCP-PENDING?
        <buffer> <length> <endpoint> <timeout> TCP-READ
        <endpoint> TCP-READABLE?
        <endpoint> TCP-UP?
        <buffer> <length> <endpoint> <timeout> TCP-WRITE
        <endpoint> TCP-WRITEABLE?


Public Procedures:

    buildWordsTCP() - registers the words with the FICL system.

Private Procedures:

    word_TCP_ANSWER() - implements the TCP-ANSWER word.
    word_TCP_CALL() - implements the TCP-CALL word.
    word_TCP_COMPLETE() - implements the TCP-COMPLETE word.
    word_TCP_DEBUG() - implements the TCP-DEBUG word.
    word_TCP_DESTROY() - implements the TCP-DESTROY word.
    word_TCP_FD() - implements the TCP-FD word.
    word_TCP_LISTEN() - implements the TCP-LISTEN word.
    word_TCP_NAME() - implements the TCP-NAME word.
    word_TCP_PENDINGq() - implements the TCP-PENDING? word.
    word_TCP_READ() - implements the TCP-READ word.
    word_TCP_READABLEq() - implements the TCP-READABLE? word.
    word_TCP_UPq() - implements the TCP-UP? word.
    word_TCP_WRITE() - implements the TCP-WRITE word.
    word_TCP_WRITEABLEq() - implements the TCP-WRITEABLE? word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_TCP_ANSWER P_((ficlVm *vm)) ;
static  void  word_TCP_CALL P_((ficlVm *vm)) ;
static  void  word_TCP_COMPLETE P_((ficlVm *vm)) ;
static  void  word_TCP_DEBUG P_((ficlVm *vm)) ;
static  void  word_TCP_DESTROY P_((ficlVm *vm)) ;
static  void  word_TCP_FD P_((ficlVm *vm)) ;
static  void  word_TCP_LISTEN P_((ficlVm *vm)) ;
static  void  word_TCP_NAME P_((ficlVm *vm)) ;
static  void  word_TCP_PENDINGq P_((ficlVm *vm)) ;
static  void  word_TCP_READ P_((ficlVm *vm)) ;
static  void  word_TCP_READABLEq P_((ficlVm *vm)) ;
static  void  word_TCP_UPq P_((ficlVm *vm)) ;
static  void  word_TCP_WRITE P_((ficlVm *vm)) ;
static  void  word_TCP_WRITEABLEq P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsTCP ()

    Enter the TCP Words into the Dictionary.


Purpose:

    Function buildWordsTCP() enters the TCP words into the system dictionary.


    Invocation:

        buildWordsTCP (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsTCP (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-ANSWER", word_TCP_ANSWER,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-CALL", word_TCP_CALL,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-COMPLETE", word_TCP_COMPLETE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-DEBUG", word_TCP_DEBUG,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-DESTROY", word_TCP_DESTROY,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-FD", word_TCP_FD,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-LISTEN", word_TCP_LISTEN,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-NAME", word_TCP_NAME,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-PENDING?", word_TCP_PENDINGq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-READ", word_TCP_READ,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-READABLE?", word_TCP_READABLEq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-UP?", word_TCP_UPq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-WRITE", word_TCP_WRITE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TCP-WRITEABLE?", word_TCP_WRITEABLEq,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_ANSWER ()

    Answer a Client's Connection Request.


Purpose:

    Function word_TCP_ANSWER() implements the TCP-ANSWER word, which waits for
    and answers a client's request for a network connection.

        TCP-ANSWER

            ( ep1 r -- ep2 0 | ior )

        Wait at most r seconds for a connection request to be received on
        listening endpoint ep1.  If a request is received, accept the request.
        The operating system automatically creates a new endpoint ep2 (the
        "data" endpoint) through which the server can talk to the client.
        The I/O result indicates the status of answering the connection
        request.


    Invocation:

        word_TCP_ANSWER (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_ANSWER (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    double  timeout ;
    ficlCell  cell ;
    int  ior ;
    TcpEndpoint  dataPoint, listeningPoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 2) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Listening endpoint. */
    listeningPoint = (TcpEndpoint) cell.p ;

/* Wait for and answer the next connection request from a client. */

    ior = tcpAnswer (listeningPoint, timeout, &dataPoint) ;

/* Return the data endpoint and I/O result on the stack. */

    if (ior == 0) {		/* Successfully answered connection request? */
        cell.p = dataPoint ;
        ficlVmPush (vm, cell) ;
    }
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_CALL ()

    Request a Network Connection to a Server.


Purpose:

    Function word_TCP_CALL() implements the TCP-CALL word, which is used by
    a client task to "call" a server task and request a network connection
    to the server.

        TCP-CALL

            ( c-addr u f -- ep 0 | ior )

        Request a network connection to the server at c-addr/u
        ("<service>[@<host>]").  If the no-wait flag f is false,
        TCP-CALL waits until the connection is established (or
        refused) before returning.  If the no-wait flag f is
        true, TCP-CALL initiates the connection attempt and
        returns immediately; the application should subsequently
        invoke TCP-COMPLETE to complete the connection.  In all
        cases, the data endpoint is returned on the stack, along
        with the I/O result of the connection attempt.


    Invocation:

        word_TCP_CALL (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_CALL (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  noWait ;
    char  *server ;
    ficlCell  cell ;
    int  ior ;
    TcpEndpoint  dataPoint ;
    unsigned  long  length ;



    FICL_STACK_CHECK (vm->dataStack, 3, 2) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Don't wait for completion? */
    noWait = (cell.i != 0) ;
    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* "<service>[@<host>]" */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        server = NULL ;
    } else {
        server = (char *) cell.p ;
        if (server[length] != '\0')  server = strndup (server, length) ;
    }

/* Attempt to establish a connection to the server. */

    ior = tcpCall (server, noWait, &dataPoint) ;

    if (server != cell.p)  free (server) ;

/* Return the data endpoint and I/O result on the stack. */

    if (ior == 0) {	/* Successfully established or initiated connection? */
        cell.p = dataPoint ;
        ficlVmPush (vm, cell) ;
    }
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_COMPLETE ()

    Complete a Call to a Server.


Purpose:

    Function word_TCP_COMPLETE() implements the TCP-COMPLETE word, which waits
    for an asynchronous, network connection attempt to complete.

        TCP-COMPLETE

            ( ep r f -- ior )

        Wait for an asynchronous, network connection attempt to complete.
        Invoking TCP-CALL in no-wait mode initiates an attempt to connect
        to a network server.  At some later time, the application must call
        TCP-COMPLETE to complete the connection attempt (if it is fated to
        complete).

        Timeout r specifies the maximum amount of time (in seconds) that the
        caller wishes to wait for the call to complete.  A negative timeout
        (e.g., -1E0) causes an infinite wait; a zero timeout (0E0) causes an
        immediate return if the connection is not yet established.

        If the connection attempt times out or otherwise fails and flag f is
        true, TCP-COMPLETE will automatically destroy the endpoint.  This mode
        is useful when the application plans to make a single go/no-go call to
        TCP-COMPLETE.

        If, under the same circumstances, flag f is false, TCP-COMPLETE will
        NOT destroy the endpoint; the application is responsible for executing
        TCP-DESTROY explicitly.  This mode is useful when the application plans
        to periodically call TCP-COMPLETE (perhaps with a timeout of 0E0) until
        the connection is successfully established.

        In all cases, the I/O result is returned on the stack: 0 if the
        connection was established, EWOULDBLOCK if the timeout period
        expired, and ERRNO otherwise.


    Invocation:

        word_TCP_COMPLETE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_COMPLETE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  destroyOnError ;
    double  timeout ;
    ficlCell  cell ;
    int  ior ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 2, 1) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Destroy endpoint on error? */\
    destroyOnError = (cell.i != 0) ;
						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Pending endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Wait for the connection to be established. */

    ior = tcpComplete (endpoint, timeout, destroyOnError) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_DEBUG ()

    Enable/Disable Networking Debug Output.


Purpose:

    Function word_TCP_DEBUG() implements the TCP-DEBUG word, which enables
    or disables TCP/IP networking debug.

        TCP-DEBUG

            ( n -- )

        Set the TCP/IP networking debug flag to n.  A value of 0 disables
        debug; a non-zero value enables debug.  Debug is written to standard
        output.


    Invocation:

        word_TCP_DEBUG (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_DEBUG (

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
    tcp_util_debug = cell.i ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_DESTROY ()

    Close a Network Endpoint.


Purpose:

    Function word_TCP_DESTROY() implements the TCP-DESTROY word, which
    deletes listening endpoints created by TCP-LISTEN and data endpoints
    created by TCP-ANSWER or TCP-CALL.

        TCP-DESTROY

            ( ep -- ior )

        Close a listening or data endpoint; the endpoint should no longer
        be referenced.


    Invocation:

        word_TCP_DESTROY (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_DESTROY (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Close the endpoint. */

    ior = tcpDestroy (endpoint) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_FD ()

    Get an Endpoint's Socket.


Purpose:

    Function word_TCP_FD() implements the TCP-FD word, which returns
    a listening or data endpoint's socket.

        TCP-FD

            ( ep -- fd )

        Get a listening or data endpoint's socket.


    Invocation:

        word_TCP_FD (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_FD (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    IoFd  fd ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Get the endpoint's socket. */

    fd = tcpFd (endpoint) ;

/* Return the socket on the stack. */

    cell.i = fd ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_LISTEN ()

    Listen for Network Connection Requests from Clients.


Purpose:

    Function word_TCP_LISTEN() implements the TCP-LISTEN word, which creates
    a "listening" endpoint on which a network server can listen for connection
    requests from clients.

        TCP-LISTEN

            ( n1 n2 -- ep 0 | ior )

        Create a "listening" endpoint bound to port n1 at which the application
        will listen for connection requests from clients; at most n2 requests
        may be pending.  The listening endpoint ep and the I/O result of
        creating the endpoint are returned on the stack.  The application
        uses the TCP-ANSWER word to accept incoming connection requests.


    Invocation:

        word_TCP_LISTEN (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_LISTEN (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  service[16] ;
    ficlCell  cell ;
    int  backlog, ior, port ;
    TcpEndpoint  listeningPoint ;



    FICL_STACK_CHECK (vm->dataStack, 2, 2) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Maximum pending requests. */
    backlog = cell.i ;
    if (backlog < 0)  backlog = 5 ;
    cell = ficlVmPop (vm) ;			/* Server port. */
    port = cell.i ;

/* Create the listening endpoint. */

    sprintf (service, "%d", port) ;

    ior = tcpListen (service, backlog, &listeningPoint) ;

/* Return the endpoint and I/O result on the stack. */

    if (ior == 0) {		/* Successfully created listening port? */
        cell.p = listeningPoint ;
        ficlVmPush (vm, cell) ;
    }
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_NAME ()

    Get an Endpoint's Name.


Purpose:

    Function word_TCP_NAME() implements the TCP-NAME word, which returns
    a listening or data endpoint's name.

        TCP-NAME

            ( ep -- c-addr u )

        Get a listening or data endpoint's name and return it as c-addr/u.
        The string is NUL-terminated and stored internally; it should be
        used or duplicated before calling TCP-NAME again.  An address of
        NULL and a length of zero are returned in the event of an error.

    Invocation:

        word_TCP_NAME (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_NAME (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *name ;
    ficlCell  cell ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Get the endpoint's name. */

    name = (char *) tcpName (endpoint) ;

/* Return the endpoint's name on the stack. */

    cell.p = name ;
    ficlVmPush (vm, cell) ;
    cell.u = (name == NULL) ? 0 : strlen (name) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_PENDINGq ()

    Check a Listening Port for Pending Connection Requests.


Purpose:

    Function word_TCP_PENDINGq() implements the TCP-PENDING? word, which
    checks to see if any connection requests are waiting to be answered
    on a listening endpoint.

        TCP-PENDING?

            ( ep -- f )

        Check if any connection requests from potential clients are waiting
        to be answered on listening endpoint ep; return true if requests are
        pending and false otherwise.  This word should only be applied to
        listening endpoints created by TCP-LISTEN.


    Invocation:

        word_TCP_PENDINGq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_PENDINGq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* TCP/IP endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Check if the endpoint is readable. */

    flag = tcpRequestPending (endpoint) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_READ ()

    Read Data from a Network Connection.


Purpose:

    Function word_TCP_READ() implements the TCP-READ word, which reads data
    from a network connection.

        TCP-READ

            ( c-addr n ep r -- u ior )

        Read n bytes of data into buffer c-addr from network connection ep.
        The actual number of bytes read, u, and the I/O result are returned
        on the stack.

        Because of the way network I/O works, a single record written to a
        connection by one task may be read in multiple "chunks" by the task
        at the other end of the connection.  TCP-READ takes this into account
        and, if you ask it for 100 bytes, it will automatically perform however
        many network reads are necessary to collect the 100 bytes.

        If n is negative, TCP-READ returns after reading the first "chunk"
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

        word_TCP_READ (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_READ (

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
    TcpEndpoint  dataPoint ;
    ssize_t  numBytesToRead ;
    size_t  numBytesRead ;



    FICL_STACK_CHECK (vm->dataStack, 3, 2) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Data endpoint. */
    dataPoint = (TcpEndpoint) cell.p ;
    cell = ficlVmPop (vm) ;			/* # of bytes to read. */
    numBytesToRead = (ssize_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Input buffer. */
    buffer = cell.p ;

/* Read the data from the network connection. */

    ior = tcpRead (dataPoint, timeout, numBytesToRead, buffer, &numBytesRead) ;

/* Return the number of bytes read and the I/O result on the stack. */

    cell.u = (unsigned long) numBytesRead ;
    ficlVmPush (vm, cell) ;
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_READABLEq ()

    Check if Data is Waiting to be Read from a Network Connection.


Purpose:

    Function word_TCP_READABLEq() implements the TCP-READABLE? word, which
    checks to see if data is waiting to be read from a network connection.

        TCP-READABLE?

            ( ep -- f )

        Check if data is waiting to be read from network connection ep;
        return true if the connection is readable and false otherwise.
        This word is equivalent to "<endpoint> TCP-FD SKT-READABLE?".


    Invocation:

        word_TCP_READABLEq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_READABLEq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* TCP/IP endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Check if the endpoint is readable. */

    flag = tcpIsReadable (endpoint) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_UPq ()

    Check if a Network Connection is Up.


Purpose:

    Function word_TCP_UPq() implements the TCP-UP? word, which checks
    to see if a network connection is still up.

        TCP-UP?

            ( ep -- f )

        Check if network connection ep is still up; return true if the
        connection is up and false otherwise.  This word is equivalent to
        "<endpoint> TCP-FD SKT-UP?".


    Invocation:

        word_TCP_UPq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_UPq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* TCP/IP endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Check if the connection is up. */

    flag = tcpIsUp (endpoint) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TCP_WRITE ()

    Write Data to a Network Connection.


Purpose:

    Function word_TCP_WRITE() implements the TCP-WRITE word, which writes data
    to a network connection.

        TCP-WRITE

            ( c-addr u1 ep r -- u2 ior )

        Write u1 bytes of data from buffer c-addr to network connection ep.
        The actual number of bytes written, u2, and the I/O result are returned
        on the stack.

        Because of the way network I/O works, attempting to output a given
        amount of data to a network connection may require multiple system
        WRITE(2)s.  TCP-WRITE takes this into account and, if you ask it to
        output 100 bytes, it will call WRITE(2) as many times as necessary
        to output the full 100 bytes of data to the connection.

        Timeout r specifies the maximum amount of time (in seconds) that the
        application wishes to wait for the data to be output.  A negative
        timeout (e.g., -1E0) causes an infinite wait; TCP-WRITE will wait
        as long as necessary to output all of the data.  A zero timeout (0E0)
        specifies no wait: if the connection is not ready for writing,
        TCP-WRITE returns immediately; if the connection is ready for
        writing, TCP-WRITE returns after outputting whatever it can.

        If the timeout expires before the requested amount of data has been
        written, the actual number of bytes written is returned on the stack
        as u2, along with an I/O result of EWOULDBLOCK.


    Invocation:

        word_TCP_WRITE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_WRITE (

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
    TcpEndpoint  dataPoint ;
    size_t  numBytesToWrite, numBytesWritten ;



    FICL_STACK_CHECK (vm->dataStack, 3, 2) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Data endpoint. */
    dataPoint = (TcpEndpoint) cell.p ;
    cell = ficlVmPop (vm) ;			/* # of bytes to write. */
    numBytesToWrite = (size_t) cell.u ;
    cell = ficlVmPop (vm) ;			/* Output buffer. */
    buffer = cell.p ;

/* Write the data to the network connection. */

    ior = tcpWrite (dataPoint, timeout, numBytesToWrite, buffer,
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

    word_TCP_WRITEABLEq ()

    Check if a Network Connection is Ready for Writing.


Purpose:

    Function word_TCP_WRITEABLEq() implements the TCP-WRITEABLE? word,
    which checks to see if data can be written to a network connection.
    stream.

        TCP-WRITEABLE?

            ( st -- f )

        Check if data can be written to network connection ep; return true
        if the connection is writeable and false otherwise.  This word is
        equivalent to "<endpoint> TCP-FD SKT-WRITEABLE?".


    Invocation:

        word_TCP_WRITEABLEq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TCP_WRITEABLEq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    TcpEndpoint  endpoint ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* TCP/IP endpoint. */
    endpoint = (TcpEndpoint) cell.p ;

/* Check if the connection is writeable. */

    flag = tcpIsWriteable (endpoint) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}
