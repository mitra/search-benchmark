/* $Id: finc.c,v 1.4 2009/09/16 12:42:08 alex Exp alex $ */
/*******************************************************************************

Program:

    finc

    Forth-Inspired Network Commands.


Author:    Alex Measday


Purpose:

    FINC is FICL with networking extensions.

    FICL's default dictionary size is 12,288 cells, of which standard FICL
    uses about 7,500 cells.  The dictionary size can be changed by setting
    environment variable FICL_DICTIONARY_SIZE to the desired number of cells.


        NOTE:  The command line is scanned twice.  The first scan looks for
            and only processes the "-listen <port>" option that indicates
            a remote user connection is to be used for interacting with the
            Ficl interpreter.  Once a client connection is established, Ficl
            is initialized with the network connection for its I/O streams.
            Next, a second scan of the command line is performed, this time
            ignoring the "-listen <port>" option and processing all others.


    Invocation:

        % finc [-debug] [-Debug] [-evaluate <code>] [-listen <port>] [<file(s)>]

    where:

        "-debug"
        "-Debug"
            enables debug output (written to STDOUT).  Capital "-Debug"
            generates more voluminous debug.
        "-evaluate <code>"
            passes the argument string to the FORTH interpreter.
        "-listen <port>"
            specifies a network server port at which FINC will listen for and
            accept the first client connection request.  The network connection
            is then used for interacting with the Ficl interpreter, instead of
            the default standard input and standard output.  Telnet(1) can be
            used in line-by-line mode as the remote client.
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
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*!******************************************************************************
    outputText() - outputs a text string to a network connection.
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

/*******************************************************************************
    FINC's Main Program.
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
    char  *argument, buffer[1024], *fileName, *inbuf ;
    ficlCell  cell ;
    ficlSystem  *sys ;
    ficlSystemInformation  fsi ;
    ficlVm  *vm ;
    int  errflg, option, status ;
    LfnStream  stream ;
    OptContext  scan ;
    TcpEndpoint  client, server ;

    const  char  *optionList[] = {	/* Command line options. */
        "{Debug}", "{debug}", "{evaluate:}", "{listen:}", NULL
    } ;




#if HAVE_SIGNAL && defined(SIGPIPE)
    signal (SIGPIPE, SIG_IGN) ;
#endif
    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options and only process the "-listen <port>" option
    for a remote user I/O connection.
*******************************************************************************/

    server = NULL ;  stream = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 4:			/* "-listen <port>" */
            if (tcpListen (argument, -1, &server))  errflg++ ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  finc [-debug] [-Debug] [-evaluate <code>] [-listen <port>] [<fileName>]\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    If a network listening port was created for remote user I/O, then wait for
    and accept a connection request from a client.
*******************************************************************************/

    if (server != NULL) {
        printf ("(%s) Waiting for remote user connection ...\n", argv[0]) ;
        if (tcpAnswer (server, -1.0, &client) ||
            lfnCreate (client, NULL, &stream)) {
            LGE "(%s) Error answering connection request for remote user I/O: ",
                argv[0]) ;
            exit (errno) ;
        }
        tcpDestroy (server) ;
    }


/*******************************************************************************
    Initialize FICL and create a virtual machine.
*******************************************************************************/

/* Create a Ficl system information structure. */

    ficlSystemInformationInitialize (&fsi) ;
    if (getenv ("FICL_DICTIONARY_SIZE") != NULL) {
        fsi.dictionarySize = atoi (getenv ("FICL_DICTIONARY_SIZE")) ;
    }

/* If a network connection for remote user I/O was established, then configure
   Ficl to use the connection in place of standard input and standard output. */

    if (stream != NULL) {
        fsi.context = stream ;
        fsi.textOut = outputText ;
        fsi.errorOut = outputText ;
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

/* Create a virtual machine. */

    vm = ficlSystemCreateVm (sys) ;
    if (vm == NULL) {
        LGE "[%s] Error creating a virtual machine.\n", argv[0]) ;
        exit (errno) ;
    }

    status = ficlVmEvaluate (vm, ".ver .( Finc " __DATE__ " ) cr quit") ;


/*******************************************************************************
    Scan the command line options again, ignoring any "-listen <port>" option.
*******************************************************************************/

    fileName = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-Debug" */
            tcp_util_debug = 1 ;
        case 2:			/* "-debug" */
            break ;
        case 3:			/* "-evaluate <code>" */
            status = ficlVmEvaluate (vm, argument) ;
            break ;
        case 4:			/* "-listen <port>" */
            break ;
        case NONOPT:		/* "<fileName>" */
            cell.p = argument ;
            ficlVmPush (vm, cell) ;
            cell.u = (unsigned long) strlen (argument) ;
            ficlVmPush (vm, cell) ;
            status = ficlVmEvaluate (vm, "included") ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  finc [-debug] [-Debug] [-evaluate <code>] [-listen <port>] [<fileName>]\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    Read and execute commands from the user.
*******************************************************************************/

    while (status != FICL_VM_STATUS_USER_EXIT) {
        ficlVmTextOut (vm, FICL_PROMPT) ;  ficlVmTextOut (vm, "") ;
        if (stream == NULL) {
            if (fgets (buffer, sizeof buffer, stdin) == NULL)  break ;
            inbuf = buffer ;
        } else {
            if (lfnGetLine (stream, -1.0, &inbuf))  break ;
        }
        status = ficlVmEvaluate (vm, inbuf) ;
    }


    exit (0) ;

}
