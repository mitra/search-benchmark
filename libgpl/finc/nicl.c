/* $Id: nicl.c,v 1.1 2009/08/08 04:00:21 alex Exp alex $ */
/*******************************************************************************

Program:

    nicl

    Network FICL Server.


Author:    Alex Measday


Purpose:

    NICL is a network server that provides each client with its own FICL
    virtual machine.  (The name NICL was originally used for a similar, I/O
    event dispatcher-based Tcl program that was later superseded by GENTLE.)
    Although each client has its own VM, word definitions, non-local variables,
    and constants, etc. are global to all clients' VMs; what's defined by one
    VM is known to all VMs.

    NICL creates its own I/O event dispatcher and assigns its handle to FORTH
    constant G-DISPATCHER.  Clients and scripts should use this dispatcher
    instead of creating their own via the IOX-CREATE word.

    FICL's default dictionary size is 12,288 cells, of which standard FICL
    uses about 7,500 cells.  The dictionary size can be changed by setting
    environment variable FICL_DICTIONARY_SIZE to the desired number of cells.


    Invocation:

        % nicl [-debug] [-Debug] [-evaluate <code>] [-listen <port>] [<file(s)>]

    where:

        "-debug"
        "-Debug"
            enables debug output (written to STDOUT).  Capital "-Debug"
            generates more voluminous debug.
        "-evaluate <code>"
            passes the argument string to the FORTH interpreter.
        "-listen <port>"
            specifies a network server port at which NICL will listen for and
            accept client connection requests.  A separate FICL virtual machine
            is created for each new client and I/O is redirected to the client.
        "<file(s)>"
            are one or more FORTH files to load and execute.  The "-evaluate"
            option can be used to push arguments on the stack for use by the
            code in the file.  If multiple files are being loaded, a different
            "-evaluate" option can be specified before each one.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <signal.h>			/* Signal definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "tpl_util.h"			/* Tuple utilities. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  newClientCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

static  void  outputText (
#    if PROTOTYPES
        ficlCallback  *callback,
        char  *text
#    endif
    ) ;

static  errno_t  readClientCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*******************************************************************************
    NICL's Main Program.
*******************************************************************************/

int  main (

#    if PROTOTYPES
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{    /* Local variables. */
    char  *argument ;
    ficlCell  cell ;
    ficlSystem  *sys ;
    ficlSystemInformation  fsi ;
    ficlVm  *vm ;
    int  errflg, option ;
    IoxDispatcher  dispatcher ;
    OptContext  scan ;
    TcpEndpoint  server ;
    Tuple  tuple ;

    const  char  *optionList[] = {	/* Command line options. */
        "{Debug}", "{debug}", "{evaluate:}", "{listen:}", NULL
    } ;




#if HAVE_SIGNAL && defined(SIGPIPE)
    signal (SIGPIPE, SIG_IGN) ;
#endif
    aperror_print = 1 ;


/*******************************************************************************
    Initialize FICL and create a virtual machine.
*******************************************************************************/

/* Create a Ficl system information structure. */

    ficlSystemInformationInitialize (&fsi) ;
    if (getenv ("FICL_DICTIONARY_SIZE") != NULL) {
        fsi.dictionarySize = atoi (getenv ("FICL_DICTIONARY_SIZE")) ;
    }

/* Initialize FICL. */

    sys = ficlSystemCreate (&fsi) ;
    if (sys == NULL) {
        LGE "[%s] Error initializing FICL.\n", argv[0]) ;
        exit (errno) ;
    }

/* Compile extra words. */

    ficlSystemCompileExtras (sys) ;

/* Compile FINC extensions. */

    buildWordsDRS (sys) ;
    buildWordsLFN (sys) ;
    buildWordsMISC (sys) ;
    buildWordsNET (sys) ;
    buildWordsIOX (sys) ;
    buildWordsSKT (sys) ;
    buildWordsTCP (sys) ;
    buildWordsTV (sys) ;

/* Create a virtual machine for interpreting files specified on the
   command line. */

    vm = ficlSystemCreateVm (sys) ;
    if (vm == NULL) {
        LGE "[%s] Error creating a virtual machine.\n", argv[0]) ;
        exit (errno) ;
    }

    ficlVmEvaluate (vm, ".ver .( Nicl " __DATE__ " ) cr quit") ;


/*******************************************************************************
    Create a global I/O event dispatcher and define it as constant G-DISPATCHER
    for Forth code that needs to use a dispatcher.
*******************************************************************************/

    if (ioxCreate (&dispatcher)) {
        LGE "[%s] Error creating I/O event dispatcher.\n", argv[0]) ;
        exit (errno) ;
    }

    cell.p = dispatcher ;
    ficlVmPush (vm, cell) ;

    ficlVmEvaluate (vm, "constant G-DISPATCHER") ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    server = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-Debug" */
            tcp_util_debug = 1 ;
        case 2:			/* "-debug" */
            iox_util_debug = 1 ;
            lfn_util_debug = 1 ;
            break ;
        case 3:			/* "-evaluate <code>" */
            ficlVmEvaluate (vm, argument) ;
            break ;
        case 4:			/* "-listen <port>" */
            if (tcpListen (argument, -1, &server))
                errflg++ ;
            else if (NULL == (tuple = tplCreate (2,
                                                 (void *) sys,
                                                 (void *) server)))
                errflg++ ;
            else if (NULL == ioxOnIO (dispatcher, newClientCB, tuple,
                                      IoxRead, tcpFd (server)))
                errflg++ ;
            break ;
        case NONOPT:		/* "<fileName>" */
            cell.p = argument ;
            ficlVmPush (vm, cell) ;
            cell.u = (unsigned long) strlen (argument) ;
            ficlVmPush (vm, cell) ;
            ficlVmEvaluate (vm, "included") ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg || (server == NULL)) {
        fprintf (stderr, "Usage:  finc [-debug] [-Debug] [-evaluate <code>] [-listen <port>] [<fileName>]\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    Loop forever, processing input events as they occur.
*******************************************************************************/

    ioxMonitor (dispatcher, -1.0) ;

    exit (errno) ;

}

/*!*****************************************************************************

Procedure:

    newClientCB ()

    Answer Connection Requests from New Clients.


Purpose:

    Function newClientCB() answers network connection requests from new
    clients.  When a server port is created, the port's listening socket
    is registered with the IOX dispatcher as an input source.  Thereafter,
    when a connection request is received at the listening socket, the IOX
    dispatcher automatically invokes newClientCB() to accept the request
    and set up the new client.  The client's data connection is registered
    as an input source with the IOX dispatcher and a FICL virtual machine
    is created for the client.


    Invocation:

        status = newClientCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the address of a 2-tuple containing the FICL system pointer
            and the TcpEndpoint for the listening socket.
        <status>	- O
            returns the status of answering the connection request, zero if
            there were no errors and ERRNO otherwise.  The status value is
            ignored by the IOX dispatcher, but it may be of use if the
            application calls newClientCB() directly.

*******************************************************************************/


static  errno_t  newClientCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        dispatcher, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    ficlSystem  *sys ;
    ficlVm  *vm ;
    LfnStream  stream ;
    TcpEndpoint  client, server ;
    Tuple  tuple ;



    tuple = (Tuple) userData ;
    sys = tplGet (tuple, 0) ;
    server = tplGet (tuple, 1) ;

/* Answer the connection request and create a LF-terminated network stream
   for the client. */

    if (tcpAnswer (server, -1.0, &client)) {
        LGE "(newClientCB) Error answering connection request: ") ;
        return (errno) ;
    }

    if (lfnCreate (client, NULL, &stream)) {
        LGE "(newClientCB) Error creating LF-terminated network stream: ") ;
        return (errno) ;
    }

/* Create a FICL virtual machine for the client and configure the VM to
   output text to the client's network connection. */

    vm = ficlSystemCreateVm (sys) ;
    if (vm == NULL) {
        LGE "(newClientCB) Error creating a virtual machine.\n") ;
        exit (errno) ;
    }

    vm->callback.context = stream ;
    vm->callback.textOut = outputText ;
    vm->callback.errorOut = outputText ;

    ficlVmEvaluate (vm, ".ver .( Nicl " __DATE__ " ) cr quit") ;

    ficlVmTextOut (vm, FICL_PROMPT) ;

/* Register the new client as an input source with the I/O event dispatcher. */

    if (NULL == ioxOnIO (ioxDispatcher (callback), readClientCB, vm,
                         IoxRead, tcpFd (client))) {
        LGE "(newClientCB) Error registering client with I/O event dispatcher.\n") ;
        return (errno) ;
    }


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    outputText ()

    Output Text to a Client's Network Connection.


Purpose:

    Function outputText() is called by a client's FICL virtual machine to
    output text to the client's network connection.  When the client's VM
    is created, a pointer to the outputText() function is stored in the VM's
    callback structure along with the handle for the client's LF-terminated
    network connection.


    Invocation:

        outputText (callback, text) ;

    where:

        <callback>	- I
            is a pointer to the client VM's callback structure.  The handle for
            the LF-terminated network connection to the client is stored in the
            context field of the callback structure.
        <text>		- I
            is the string of text to be output.

*******************************************************************************/


static  void  outputText (

#    if PROTOTYPES
        ficlCallback  *callback,
        char  *text)
#    else
        callback, text)

        ficlCallback  *callback ;
        char  *text ;
#    endif

{
    if ((text != NULL) && (strlen (text) > 0)) {
        lfnPutLine ((LfnStream) callback->context, -1.0, "%s", text) ;
    }
}

/*!*****************************************************************************

Procedure:

    readClientCB ()

    Read and Evaluate Input from a Client.


Purpose:

    Function readClientCB() reads lines of FORTH input from a client's
    LF-terminated network connection and passes the lines to the client's
    FICL virtual machine for evaluation.  When the data connection to a client
    is established, the connection's socket is registered with the I/O event
    dispatcher as an input source.  Thereafter, when input is detected on the
    socket, the dispatcher automatically invokes readClientCB() to read and
    process the input.


    Invocation:

        status = readClientCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by ioxOnIO().
        <reason>	- I
            is the reason, IoxRead, the callback is being invoked.
        <userData>	- I
            is the address of the client's FICL virtual machine.  The handle
            for the client's LF-terminated network connection is stored in the
            VM's callback structure.
        <status>	- O
            returns the status of reading and processing the input, zero if
            there were no errors and ERRNO otherwise.  The status value is
            ignored by the IOX dispatcher, but it may be of use if the
            application calls readClientCB() directly.

*******************************************************************************/


static  errno_t  readClientCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        dispatcher, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    char  *inbuf ;
    ficlVm  *vm ;
    int  exitCode ;
    LfnStream  stream ;




    vm = (ficlVm *) userData ;
    stream = vm->callback.context ;

/* While more input is available, read and process the next input line. */

    exitCode = 0 ;

    while (lfnIsReadable (stream)) {
					/* Read the next message. */
        if (lfnGetLine (stream, -1.0, &inbuf)) {
            LGE "(readClientCB) Error reading from %s.\nlfnGetLine: ",
                lfnName (stream)) ;
            break ;
        }

        exitCode = ficlVmEvaluate (vm, inbuf) ;
        if ((exitCode == FICL_VM_STATUS_USER_EXIT) ||
            (exitCode == FICL_VM_STATUS_QUIT)) {
            break ;
        }
        ficlVmTextOut (vm, FICL_PROMPT) ;

    }

/* Check to see if the stream's network connection has been broken.  If so,
   close the connection and destroy the client's FICL virtual machine. */

    if (!lfnIsUp (stream) ||
        (exitCode == FICL_VM_STATUS_USER_EXIT) ||
        (exitCode == FICL_VM_STATUS_QUIT)) {
        errno = EPIPE ;
        if ((exitCode != FICL_VM_STATUS_USER_EXIT) &&
            (exitCode != FICL_VM_STATUS_QUIT)) {
            LGE "(readClientCB) Broken connection to %s.\nlfnIsUp: ",
                lfnName (stream)) ;
        }
        PUSH_ERRNO ;
        ioxCancel (callback) ;
        ficlVmDestroy (vm) ;
        lfnDestroy (stream) ;
        POP_ERRNO ;
        return (errno) ;
    }

    return (0) ;

}
