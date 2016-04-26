/* $Id: words_skt.c,v 1.1 2005/01/10 19:31:55 alex Exp alex $ */
/*******************************************************************************

File:

    words_skt.c

    Socket Utilities.


Author:    Alex Measday


Purpose:

    The WORDS_SKT package defines words for monitoring sockets:

        SKT-CLEANUP
        <fd> SKT-PEER
        <fd> SKT-PORT
        <fd> SKT-READABLE?
        <fd> SKT-SETBUF
        SKT-STARTUP
        <fd> SKT-UP?
        <fd> SKT-WRITEABLE?


Public Procedures:

    buildWordsSKT() - registers the words with the FICL system.

Private Procedures:

    word_SKT_CLEANUP() - implements the SKT-CLEANUP word.
    word_SKT_PEER() - implements the SKT-PEER word.
    word_SKT_PORT() - implements the SKT-PORT word.
    word_SKT_READABLEq() - implements the SKT-READABLE? word.
    word_SKT_SETBUF() - implements the SKT-SETBUF word.
    word_SKT_STARTUP() - implements the SKT-STARTUP word.
    word_SKT_UPq() - implements the SKT-UP? word.
    word_SKT_WRITEABLEq() - implements the SKT-WRITEABLE? word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "net_util.h"			/* Networking utilities. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_SKT_CLEANUP P_((ficlVm *vm)) ;
static  void  word_SKT_PEER P_((ficlVm *vm)) ;
static  void  word_SKT_PORT P_((ficlVm *vm)) ;
static  void  word_SKT_READABLEq P_((ficlVm *vm)) ;
static  void  word_SKT_SETBUF P_((ficlVm *vm)) ;
static  void  word_SKT_STARTUP P_((ficlVm *vm)) ;
static  void  word_SKT_UPq P_((ficlVm *vm)) ;
static  void  word_SKT_WRITEABLEq P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsSKT ()

    Enter the SKT Words into the Dictionary.


Purpose:

    Function buildWordsSKT() enters the SKT words into the system dictionary.


    Invocation:

        buildWordsSKT (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsSKT (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-CLEANUP", word_SKT_CLEANUP,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-PEER", word_SKT_PEER,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-PORT", word_SKT_PORT,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-READABLE?", word_SKT_READABLEq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-SETBUF", word_SKT_SETBUF,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-STARTUP", word_SKT_STARTUP,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-UP?", word_SKT_UPq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "SKT-WRITEABLE?", word_SKT_WRITEABLEq,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_CLEANUP ()

    Shut Down the Socket Library.


Purpose:

    Function word_SKT_CLEANUP() implements the SKT-CLEANUP word, which shuts
    down the socket library on platforms that require it (e.g., Windows).

        SKT-CLEANUP

            ( -- ior )

        Shut down the socket library; ior returns an error indication.


    Invocation:

        word_SKT_CLEANUP (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_CLEANUP (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;



    FICL_STACK_CHECK (vm->dataStack, 0, 1) ;

/* Shut down the socket library. */

    ior = sktCleanup () ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_PEER ()

    Get the IP address of a Socket's Peer.


Purpose:

    Function word_SKT_PEER() implements the SKT-PEER word, which returns
    the IP address of the host at the other end of a network conection.

        SKT-PEER

            ( fd -- u )

        Determine the host at the other end of network socket connection fd
        and returns its IP address in u in network-byte-order.


    Invocation:

        word_SKT_PEER (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_PEER (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *peer ;
    ficlCell  cell ;
    IoFd  fd ;
    unsigned  long  ipAddress ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Socket. */
    fd = (IoFd) cell.i ;

/* Get the socket's peer name. */

    peer = (char *) sktPeer (fd) ;

/* Convert the peer name to an IP address. */

    ipAddress = (peer == NULL) ? 0 : netAddrOf (peer) ;

/* Return the IP address on the stack. */

    cell.u = ipAddress ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_PORT ()

    Get a Socket's Port Number.


Purpose:

    Function word_SKT_PORT() implements the SKT-PORT word, which returns
    the number of the port to which a socket (either listening or data)
    is bound.

        SKT-PORT

            ( fd -- n )

        Get the number of the port to which socket fd is bound; -1 is
        returned in the event of an error.


    Invocation:

        word_SKT_PORT (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_PORT (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  port ;
    IoFd  fd ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Socket. */
    fd = (IoFd) cell.i ;

/* Get the socket's port number. */

    port = sktPort (fd) ;

/* Return the port number on the stack. */

    cell.i = port ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_READABLEq ()

    Check if Data is Waiting to be Read from a Socket.


Purpose:

    Function word_SKT_READABLEq() implements the SKT-READABLE? word,
    which checks to see if data is waiting to be read from a socket.

        SKT-READABLE?

            ( fd -- f )

        Check if data is waiting to be read from socket fd; return true
        if the socket is readable and false otherwise.


    Invocation:

        word_SKT_READABLEq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_READABLEq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    IoFd  fd ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Socket. */
    fd = (IoFd) cell.i ;

/* Check if the socket is readable. */

    flag = sktIsReadable (fd) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_SETBUF ()

    Change the Sizes of a Socket's Receive and Send Buffers.


Purpose:

    Function word_SKT_SETBUF() implements the SKT-SETBUF word, which changes
    the sizes of a socket's receive and/or send buffers.

        SKT-SETBUF

            ( fd n1 n2 -- ior )

        Set the size of socket fd's receive buffer to n1 bytes and the size
        of its send buffer to n2 bytes; ior returns an error indication.
        If a buffer size is less than zero, the respective buffer retains
        its current size.


    Invocation:

        word_SKT_SETBUF (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_SETBUF (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior, receiveSize, sendSize ;
    IoFd  fd ;



    FICL_STACK_CHECK (vm->dataStack, 3, 1) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Size of send buffer. */
    sendSize = cell.i ;
    cell = ficlVmPop (vm) ;			/* Size of receive buffer. */
    receiveSize = cell.i ;
    cell = ficlVmPop (vm) ;			/* Socket. */
    fd = (IoFd) cell.i ;

/* Set the socket's buffer sizes. */

    ior = sktSetBuf (fd, receiveSize, sendSize) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_STARTUP ()

    Start Up the Socket Library.


Purpose:

    Function word_SKT_STARTUP() implements the SKT-STARTUP word, which starts
    up the socket library on platforms that require it (e.g., Windows).

        SKT-STARTUP

            ( -- ior )

        Start up the socket library; ior returns an error indication.


    Invocation:

        word_SKT_STARTUP (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_STARTUP (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;



    FICL_STACK_CHECK (vm->dataStack, 0, 1) ;

/* Start up the socket library. */

    ior = sktStartup () ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_UPq ()

    Check if a Connection is Up.


Purpose:

    Function word_SKT_UPq() implements the SKT-UP? word, which checks
    to see if a network connection is still up.

        SKT-UP?

            ( fd -- f )

        Check socket fd to see if its network connection is still up;
        return true if the connection is up and false otherwise.


    Invocation:

        word_SKT_UPq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_UPq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    IoFd  fd ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Socket. */
    fd = (IoFd) cell.i ;

/* Check if the connection is up. */

    flag = sktIsUp (fd) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_SKT_WRITEABLEq ()

    Check if Data can be Written to a Socket.


Purpose:

    Function word_SKT_WRITEABLEq() implements the SKT-WRITEABLE? word,
    which checks to see if data can be written to a socket.

        SKT-WRITEABLE?

            ( fd -- f )

        Check if data can be written to socket fd; return true if the
        socket is writeable and false otherwise.


    Invocation:

        word_SKT_WRITEABLEq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_SKT_WRITEABLEq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  flag ;
    ficlCell  cell ;
    IoFd  fd ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Socket. */
    fd = (IoFd) cell.i ;

/* Check if the socket is writeable. */

    flag = sktIsWriteable (fd) ;

/* Return the flag on the stack. */

    cell.i = flag ? ~0 : 0 ;
    ficlVmPush (vm, cell) ;

    return ;

}
