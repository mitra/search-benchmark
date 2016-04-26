/* $Id: nft_util.c,v 1.18 2011/07/18 17:44:47 alex Exp alex $ */
/*******************************************************************************

File:

    nft_util.c

    FTP Utilities.


Author:    Alex Measday


Purpose:

    The NFT_UTIL package provides the basis for implementing a File Transfer
    Protocol (FTP) server.  The implementation of the NFT_UTIL package itself
    and its companion package of command processing functions, "nft_proc.c",
    was based on the following Request for Comments (available at a number of
    Internet sites):

        RFC 765 - "File Transfer Protocol" (obsolete)
            Although superseded by RFC 959, this RFC described the (defunct?)
            mail-related FTP commands: MAIL, MLFL, MSAM, MSOM, MSRQ, and MRCP.

        RFC 959 - "File Transfer Protocol (FTP)"
            The official FTP RFC, well-written, not dry.

        RFC 1123 - "Requirements for Internet Hosts -- Application and Support"
            This all-encompassing RFC clarified some remaining issues in the
            FTP standard and took into account existing practice.

    as well as some empirical testing with the SunOS 4.1.3 and HP/UX 9.05
    FTP servers.


    FTP Sessions
    ------------

    An FTP server listens for and accepts network connection requests from
    clients who wish to transfer files.  A "session" for a particular client
    begins when that client first connects to the FTP server and ends when
    the client is disconnected from the server; an FTP server with multiple
    clients would have multiple sessions active simultaneously.  Associated
    with each session are two network connections:

        Control - is the connection over which commands are sent to the
            FTP server and replies returned to the client.  The control
            connection stays open for the life of the session.

        Data - is a connection over which files and other data are sent
            and received.  A new data connection is established for each
            data transfer (e.g., the sending of a single file) and closed
            when the transfer completes.

    FTP commands are CR/LF-terminated, ASCII strings consisting of an upper
    case, 3- or 4-character keyword followed by zero or more, space-separated
    arguments; for example, the following command requests the retrieval of a
    file:

        "RETR thisFile"

    Some of the more common FTP commands are:

        "USER <userName>"	(Automatically issued by the FTP client)
        "PASS <password>"
            - log a user onto the FTP server's host.

        "CWD <newDirectory>"	(FTP Client Command: "cd <newDirectory>")
        "CDUP"			(FTP Client Command: "cd ..")
            - change the current working directory.  CDUP moves
              up to the parent of the current working directory.

        "PWD"			(FTP Client Command: "pwd")
            - returns the current working directory.

        "NLST [<directory>]"	(FTP Client Command: "ls")
            - returns a list of the files in the specified directory (which
              defaults to the current working directory).

        "TYPE A|I"		(FTP Client Command: "ascii" or "binary")
            - specifies the type of data being transferred, "A" for ASCII
              and "I" (Image) for binary.

        "PORT <hostAndPort>"	(Automatically issued by the FTP client)
            - specifies a port on the client's host to which the FTP
              server should connect for data transfers.  This command
              is usually issued anew prior to each file transfer or
              directory listing.

        "RETR <fileName>"	(FTP Client Command: "get <fileName>")
            - retrieves the specified file from the FTP server's host.

        "STOR <fileName>"	(FTP Client Command: "put <fileName>")
            - store the to-be-transferred data on the FTP server's host
              under the specified file name.

        "QUIT"			(FTP Client Command: "bye" or "quit")
            - terminates the session.

    FTP replies consist of a 3-digit, numeric status code followed by
    descriptive text.  The status codes are enumerated in RFC 959 (but see
    RFC 1123 for some updates).  For example, the RETR command typically
    results in two replies being returned to the client over the control
    connection:

        "150 Data connection opened: thisFile (98765 bytes)"

        ... the contents of "thisFile" are sent over the data connection ...

        "226 Transfer complete: thisFile (98765 bytes)

    RFCs 959 and 1123 specify what status codes should be used in reply to
    which commands.  Although there are a few exceptions, the implementor
    is free to choose the format and contents of the reply text.


    The NFT Package
    ---------------

    The NFT package provides a server implementation with a high-level
    means of conducting an FTP session.  The server is responsible for
    listening for and answering a network connection request from a
    client:

        TcpEndpoint  client, server ;
        ...
        tcpListen ("ftp", -1, &server) ;
        tcpAnswer (server, -1.0, &client) ;

    Once a client connection has been established, an FTP session can
    be created:

        NftSession  session ;
        ...
        nftCreate (client, NULL, NULL, NULL, &session) ;

    The server is now ready to read and process FTP commands from the client:

        char  *command ;
        ...
        nftGetLine (session, &command) ;
        nftEvaluate (session, command) ;

    When the client connection is broken or an FTP QUIT command is received,
    the server should terminate the FTP session:

        nftDestroy (session) ;

    Putting all of the above together yields a simple - but working - FTP
    server:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "tcp_util.h"			-- TCP/IP networking utilities.
        #include  "nft_util.h"			-- FTP utilities.

        int  main (int argc, char *argv[])
        {
            char  *command ;
            TcpEndpoint  client, server ;
            NftSession  session ;
						- Listen at port 21.
            tcpListen ((argc > 1) ? argv[1] : "21", -1, &server) ;

            for ( ; ; ) {			-- Answer next client.
                tcpAnswer (server, -1.0, &client) ;
                nftCreate (client, NULL, NULL, NULL, &session) ;
                nftPutLine (session, "220 Service is ready.\n") ;
                for ( ; ; ) {			-- Service connected client.
                    if (nftGetLine (session, &command))  break ;
                    nftEvaluate (session, command) ;
                    if ((nftInfo (session))->logout)  break ;
                }
                nftDestroy (session) ;		-- Lost client.
            }

        }

    The server's name is specified as the first argument on the command line
    (i.e., "argv[1]") and defaults to port 21.  If a client connection is
    broken, the server loops back to wait for the next client.


    Extending an NFT Server
    -----------------------

    The NFT_UTIL package provides a means for modifying or extending the
    functionality of an NFT-based FTP server; it does this by maintaining
    a table that maps FTP command keywords to the C functions that process
    those commands.  When called to evaluate a command string, nftEvaluate()
    parses the command line into an ARGC/ARGV array of arguments.  It then
    looks up the command keyword (ARGV[0]) in the table and calls the command
    processing function bound to that keyword, passing the ARGC/ARGV array of
    arguments to the function.

    nftCreate() initializes a session's keyword-function map with default
    entries for the commands called for in the RFCs.  These default entries
    are defined in the "defaultCommands[]" and "defaultCallbacks[]" arrays
    below.  The default command processing functions - except for those for
    PASV, PORT, and QUIT - are found in the companion package, "nft_proc.c".
    An application can modify the processing of an existing command by
    registering a new command processing function for the command.  The
    following example replaces the default nftRETR() retrieval function
    with myRetrieve():

        extern  int  myRetrieve (NftSession session,
                                 int argc, const char *argv[],
                                 void *userData) ;
        ...
        nftRegister (session, "RETR", myRetrieve) ;

    nftRegister() can also be used to add entirely new FTP commands to the
    keyword-function map.  When replacing an existing command's callback,
    study the RFCs and the default implementation of the command so as to
    ensure that your implementation covers all the bases.


    Command Processing Functions
    ----------------------------

    Application-specific command processing functions registered with
    nftRegister() and invoked by nftEvaluate() should be declared as
    follows:

        int  myFunction (NftSession  session,
                         const  char  *command,
                         char  *arguments,
                         void  *userData)
        {
            ... body of function ...
        }

    ARGC is the number of arguments (plus the command keyword) in the
    command string being evaluated; ARGV[] is an array of character strings,
    each one being one of the arguments.  For example, an FTP ALLO command,

        "ALLO 123 456"

    would be parsed into an ARGC/ARGV array as shown here:

        argc = 3    argv[0] = "ALLO"
                    argv[1] = "123"	-- File size
                    argv[2] = "456"	-- Maximum record/page size

    The command processing function is responsible for verifying the number
    and validity of a command's arguments.  The USERDATA argument is an
    arbitrary (VOID *) pointer specified when the session was created.

    A number of NFT functions are available for use in a command processing
    function.  A pointer to the public information in a session structure
    can be obtained with a call to nftInfo():

        NftSessionInfo  *info = nftInfo (session) ;

    nftPutLine() should be used to format and send a reply message to the
    client over the session's control connection; the following example
    returns a response for the PWD command:

        nftPutLine (session, "257 \"%s\"\n", info->currentDirectory) ;

    Commands such as RETR, STOR, and LIST must establish a separate network
    connection for transferring data back and forth.  A basic implementation
    of the RETR command illustrates the use of the nftOpen(), nftWrite(),
    and nftClose() functions to transfer data:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  <string.h>			-- C Library string functions.
        #include  "nft_util.h"			-- FTP utilities.

        int  myRetrieve (NftSession  session,
                         const  char  *command,
                         char  *arguments,
                         void  *userData)
        {
            char  buffer[1024], fileName[1024] ;
            FILE  *file ;
            int  length ;
            NftSessionInfo  *info = nftInfo (session) ;

		        -- Append the file name to the current working
			-- directory and open the file for reading.
            strcpy (fileName, info->currentDirectory) ;
            strcat (fileName, argv[1]) ;
            file = fopen (fileName, "rb") ;

			-- Establish a network connection with the client
			-- for the purpose of transferring data.
            nftOpen (session) ;
            nftPutLine (session, "150 Transferring: %s\n", fileName) ;

			-- Send the contents of the file to the client.
            while ((length = fread (buffer, 1, sizeof buffer, file)) > 0)
                nftWrite (session, length, buffer, NULL) ;

			-- End the data transfer.
            nftPutLine (session, "226 Transfer complete.\n") ;
            nftClose (session) ;
            fclose (file) ;

            return (0) ;

        }

    The STOR command is implemented in a similar fashion, with the calls to
    fread(3) and nftWrite() replaced by calls to nftRead() and fwrite(3),
    respectively.  The NFT_UTIL package handles the PORT and PASV commands
    that precede a data transfer command, so nftOpen() will establish the
    appropriate type of data connection with the application (or its
    programmer) being none the wiser.

    The myRetrieve() function above does not strictly conform to the FTP
    standard in that it only supports binary transfers ("TYPE I") of
    arbitrary files.  Text files should be transferred according to the
    Telnet protocol.  In particular, each end-of-line marker (e.g., "\n")
    in a text file must be transmitted as a Telnet end-of-line sequence:
    a carriage return followed by a line feed ("\r\n").  The FTP client
    and the server are responsible for making the appropriate conversions
    to and from their hosts' end-of-line conventions when sending or
    receiving ASCII data.

    nftRead() and nftWrite() do NOT perform these conversions for you.
    However, the CR/LF utilities (see "crlf_util.c") simplify the handling
    of ASCII text.  Using these conversion utilities, the myRetrieve()
    function needs to be modified in only two places in order to support
    ASCII text transfers:

        #include  "crlf_util.h"			-- CR/LF utilities.

        int  myRetrieve (...)
        {
            ...
			-- Open the file for reading.
            if (info->representation[0] == 'A')	-- ASCII transfer?
                file = fopen (fileName, "r") ;	-- Open ASCII file.
            else
                file = fopen (fileName, "rb") ;	-- Open binary file.
            ...
			-- Send the contents of the file to the client.
            while ((length = fread (buffer, 1, (sizeof buffer)/2, file)) > 0) {
                if (info->representation[0] == 'A') {
                    nl2crlf (buffer, length, sizeof buffer) ;
                    length = strlen (buffer) ;
                }
                if (nftWrite (session, length, buffer, NULL))  break ;
            }

            ...

        }

    Within the send loop, nl2crlf() is called to convert newline characters
    in the file to the carriage return/line feed sequence.  Note that only
    half of the buffer is used for reading, thus allowing for the worst case
    scenario of a string consisting entirely of newlines being expanded to
    twice its size.


Public Procedures:

    nftClose() - closes a session's data connection.
    nftCreate() - creates an FTP session.
    nftDestroy() - deletes an FTP session.
    nftEvaluate() - evaluates an FTP command.
    nftFd() - returns a session's control or data socket number.
    nftGetLine() - reads the next line from a session's control connection.
    nftIgnoreCmd() - ignores an FTP command.
    nftInfo() - returns a pointer to the session's public information.
    nftIsReadable() - checks if input is waiting to be read from a session's
        control or data connection.
    nftIsUp() - checks if a session's control or data connection is up.
    nftIsWriteable() - checks if data can be written to a session's control
        or data connection.
    nftName() - returns the name of a session's control or data connection.
    nftNextCommand() - reads and evaluates the next FTP command.
    nftOpen() - opens a session's data connection.
    nftPASV() - processes the FTP PASV command.
    nftPeer() - returns the name of the session's peer.
    nftPORT() - processes the FTP PORT command.
    nftPutLine() - writes a line of output to a session's control connection.
    nftQUIT() - processes the FTP QUIT command.
    nftRead() - reads input from a session's data connection.
    nftRegister() - maps an FTP command to a user-supplied function.
    nftSyntax() - returns the syntax of an FTP command.
    nftWrite() - writes output to a session's data connection.

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
#include  "crlf_util.h"			/* CR/LF utilities. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "hash_util.h"			/* Hash table definitions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "net_util.h"			/* Networking utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */
#include  "nft_util.h"			/* FTP utilties. */
#include  "nft_proc.h"			/* FTP command processing functions. */


/*******************************************************************************
    FTP Session - contains information about an FTP session, including the
        session's control and data network connections, etc.
*******************************************************************************/

typedef  struct  _NftSession {
    LfnStream  controlStream ;		/* Control/status connection with peer. */
    HashTable  commandProcs ;		/* Maps FTP commands to functions. */
    TcpEndpoint  dataStream ;		/* Data transfer connection with peer. */
    TcpEndpoint  listeningPort ;	/* Listening port for PASV data connections. */
    char  *command ;			/* Current command; NUL-terminated. */
    char  *arguments ;			/* Arguments following command. */
#define  MAXOUTPUT  1023
    char  *outputString ;		/* Formatted string to be output. */
    NftSessionInfo  info ;		/* Public information about the session. */
}  _NftSession ;


#define  TIMEOUT  120.0			/* Timeout in seconds for establishing a data connection. */


int  nft_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  nft_util_debug

/*******************************************************************************
    Default mappings of FTP commands to processing functions.
*******************************************************************************/

static  char  *defaultCommands[]  OCD ("nft_util")  = {
    "ABOR",
    "ACCT",
    "ALLO",
    "APPE",
    "CDUP",
    "CWD",
    "DELE",
    "HELP",
    "LIST",
    "MAIL",
    "MDTM",
    "MKD",
    "MLFL",
    "MODE",
    "MRCP",
    "MRSQ",
    "MSAM",
    "MSND",
    "MSOM",
    "NLST",
    "NOOP",
    "PASS",
    "PASV",
    "PORT",
    "PWD",
    "QUIT",
    "REIN",
    "REST",
    "RETR",
    "RMD",
    "RNFR",
    "RNTO",
    "SITE",
    "SIZE",
    "SMNT",
    "STAT",
    "STOR",
    "STOU",
    "STRU",
    "SYST",
    "TYPE",
    "USER",
    "XCUP",
    "XCWD",
    "XMKD",
    "XPWD",
    "XRMD",
    NULL
} ;

static  NftCommandProc  *defaultCallbacks[]  OCD ("nft_util")  = {
    NULL,					/* ABOR */
    nftIgnoreCmd,				/* ACCT <account> */
    nftIgnoreCmd,				/* ALLO <numBytes> */
    nftStoreCmds,				/* APPE <pathname> */
    nftAccessCmds,				/* CDUP */
    nftCWD,					/* CWD <pathname> */
    nftFileCmds,				/* DELE <pathname> */
    nftHELP,					/* HELP [<keyword>] */
    nftListCmds,				/* LIST [<pathname>] */
    NULL,					/* MAIL [<recipient>] */
    nftFileCmds,				/* MDTM <pathname> */
    nftFileCmds,				/* MKD <pathname> */
    NULL,					/* MLFL [<recipient>] */
    nftMODE,					/* MODE <code> */
    NULL,					/* MRCP <recipient> */
    NULL,					/* MRSQ [<scheme>] */
    NULL,					/* MSAM [<recipient>] */
    NULL,					/* MSND [<recipient>] */
    NULL,					/* MSOM [<recipient>] */
    nftListCmds,				/* NLST [<pathname>] */
    nftServiceCmds,				/* NOOP */
    nftPASS,					/* PASS <password> */
    nftPASV,					/* PASV */
    nftPORT,					/* PORT <h1,h2,h3,h4,p1,p2> */
    nftServiceCmds,				/* PWD */
    nftQUIT,					/* QUIT */
    NULL,					/* REIN */
    NULL,					/* REST <marker> */
    nftRETR,					/* RETR <pathname> */
    nftFileCmds,				/* RMD <pathname> */
    nftFileCmds,				/* RNFR <pathname> */
    nftFileCmds,				/* RNTO <pathname> */
    NULL,					/* SITE <string> */
    nftFileCmds,				/* SIZE <pathname> */
    NULL,					/* SMNT <pathname> */
    nftSTAT,					/* STAT */
    nftStoreCmds,				/* STOR <pathname> */
    nftStoreCmds,				/* STOU */
    nftSTRU,					/* STRU <code> */
    nftServiceCmds,				/* SYST */
    nftTYPE,					/* TYPE <representation> */
    nftUSER,					/* USER <user> */
    nftAccessCmds,				/* XCUP */
    nftCWD,					/* XCWD <pathname> */
    nftFileCmds,				/* XMKD <pathname> */
    nftServiceCmds,				/* XPWD */
    nftFileCmds,				/* XRMD <pathname> */
    NULL
} ;

static  char  *defaultSyntax[]  OCD ("nft_util")  = {
    "ABOR",
    "ACCT <account>",
    "ALLO <numBytes> [<maxRecordSize>]",
    "APPE <pathname>",
    "CDUP",
    "CWD <pathname>",
    "DELE <pathname>",
    "HELP [<keyword>]",
    "LIST [<pathname>]",
    "MAIL [<recipient>]",
    "MDTM <pathname>",
    "MKD <pathname>",
    "MLFL [<recipient>]",
    "MODE S|B|C",
    "MRCP <recipient>",
    "MRSQ [<scheme>]",
    "MSAM [<recipient>]",
    "MSND [<recipient>]",
    "MSOM [<recipient>]",
    "NLST [<pathname>]",
    "NOOP",
    "PASS <password>",
    "PASV",
    "PORT <h1,h2,h3,h4,p1,p2>",
    "PWD",
    "QUIT",
    "REIN",
    "REST <marker>",
    "RETR <pathname>",
    "RMD <pathname>",
    "RNFR <pathname>",
    "RNTO <pathname>",
    "SITE <string>",
    "SIZE <pathname>",
    "SMNT <pathname>",
    "STAT",
    "STOR <pathname>",
    "STOU [<pathname>]",
    "STRU F|R|P",
    "SYST",
    "TYPE <representation>",
    "USER <name>",
    "XCUP",
    "XCWD <pathname>",
    "XMKD <pathname>",
    "XPWD",
    "XRMD <pathname>",
    NULL
} ;

/*!*****************************************************************************

Procedure:

    nftClose ()

    Close the Data Connection with an FTP Client.


Purpose:

    Function nftClose() closes the data transfer connection with a client
    that was previously established by a call to nftOpen().


    Invocation:

        status = nftClose (session) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <status>	- O
            returns the status of closing the data connection,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftClose (

#    if PROTOTYPES
        NftSession  session)
#    else
        session)

        NftSession  session ;
#    endif

{    /* Local variables. */
    TcpEndpoint  connection ;



    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftClose) NULL session handle:") ;
        return (errno) ;
    }
    if (session->dataStream == NULL)  return (0) ;

    LGI "(nftClose) Closing data connection with %s.\n",
        tcpName (session->dataStream)) ;

/* Close the session's data transfer connection. */

    if ((session->listeningPort != NULL) &&
        (session->info.dataPortName != NULL)) {
        free (session->info.dataPortName) ;
        session->info.dataPortName = NULL ;
    }

    connection = session->dataStream ;
    session->dataStream = NULL ;

    return (tcpDestroy (connection)) ;

}

/*!*****************************************************************************

Procedure:

    nftCreate ()

    Create an FTP Session.


Purpose:

    Function nftCreate() creates an FTP session on top of a previously-created
    network connection that will be used to receive commands and return status
    to the session peer.


    Invocation:

        status = nftCreate (controlPoint, commands, callbacks, userData,
                            &session) ;

    where

        <controlPoint>	- I
            is the previously-created TCP/IP endpoint for the network
            connection that will be used to exchange commands and status
            with the session's peer.  (See "tcp_util.c" for more information
            about network endpoints.)  NOTE that the "controlPoint" endpoint
            is automatically destroyed (i.e., the socket is closed) when the
            FTP session is destroyed.
        <commands>	- I
            is an array of FTP command names (e.g., "RETR", "STOR"); the
            last entry in the array must be NULL.  This argument may be NULL.
        <callbacks>	- I
            is an array of pointers to functions that process the FTP
            commands whose names are found in the corresponding entries
            in the COMMANDS array.  This argument may be NULL.
        <userData>	- I
            is an arbitrary (VOID *) pointer that will be passed to the
            callbacks defined in the CALLBACKS array.
        <session>	- O
            returns a handle for the FTP session.  This handle is used in
            calls to the other NFT functions.
        <status>	- O
            returns the status of creating the session, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftCreate (

#    if PROTOTYPES
        TcpEndpoint  controlPoint,
        const  char  *commands[],
        NftCommandProc  *callbacks[],
        void  *userData,
        NftSession  *session)
#    else
        controlPoint, commands, callbacks, userData, session)

        TcpEndpoint  controlPoint ;
        const  char  *commands[] ;
        NftCommandProc  *callbacks[] ;
        void  *userData ;
        NftSession  *session ;
#    endif

{    /* Local variables. */
    char  *peer ;
    int  i ;




    *session = NULL ;

/* Create and initialize an FTP session structure. */

    *session = (_NftSession *) malloc (sizeof (_NftSession)) ;
    if (*session == NULL) {
        LGE "(nftCreate) Error allocating session structure for \"%s\".\nmalloc: ",
            tcpName (controlPoint)) ;
        return (errno) ;
    }

    (*session)->controlStream = NULL ;
    (*session)->commandProcs = NULL ;
    (*session)->dataStream = NULL ;
    (*session)->listeningPort = NULL ;
    (*session)->command = NULL ;
    (*session)->arguments = NULL ;
    (*session)->outputString = NULL ;
    (*session)->info.userData = userData ;
    (*session)->info.userName = NULL ;
    (*session)->info.currentDirectory = NULL ;
    (*session)->info.timeout = 5.0 * 60.0 ;	/* 5-minute timeout when idle. */
    (*session)->info.dataPortName = NULL ;
    (*session)->info.representation[0] = 'A' ;
    (*session)->info.representation[1] = 'N' ;
    (*session)->info.logout = 0 ;
    (*session)->info.oldPathname = NULL ;

/* Create a LF-terminated network stream on top of the control connection. */

    if (lfnCreate (controlPoint, NULL, &(*session)->controlStream)) {
        LGE "(nftCreate) Error creating LF-terminated network stream for %s.\nlfnCreate: ",
            tcpName (controlPoint)) ;
        PUSH_ERRNO ;  nftDestroy (*session) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Create a mapping between FTP command names and processing functions. */

    if (hashCreate (32, &(*session)->commandProcs)) {
        LGE "(nftCreate) Error creating FTP command-function mapping for %s.\nhashCreate: ",
            nftName (*session, 0)) ;
        PUSH_ERRNO ;  nftDestroy (*session) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Register the default command processing functions. */

    for (i = 0 ;  defaultCommands[i] != NULL ;  i++)
        nftRegister (*session, defaultCommands[i], defaultCallbacks[i]) ;

/* Register any caller-defined processing functions, which may override
   the defaults. */

    if (commands != NULL) {
        for (i = 0 ;  commands[i] != NULL ;  i++)
            nftRegister (*session, commands[i], callbacks[i]) ;
    }

/* Remember the current directory. */

    (*session)->info.currentDirectory = strdup (fnmBuild (FnmPath, NULL)) ;
    if ((*session)->info.currentDirectory == NULL) {
        LGE "(nftCreate) Error duplicating current directory name for %s.\nstrdup: ",
            nftName (*session, 0)) ;
        PUSH_ERRNO ;  nftDestroy (*session) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Construct the default server address for data ports. */

    peer = (char *) sktPeer (tcpFd (controlPoint)) ;
    (*session)->info.dataPortName = strndup ("ftp-data",
                                             sizeof ("ftp-data") +
                                             ((peer == NULL) ? 0 :
                                              (1 + strlen (peer)))) ;
    if ((*session)->info.dataPortName == NULL) {
        LGE "(nftCreate) Error allocating default data port name for %s.\nstrndup: ",
            nftName (*session, 0)) ;
        PUSH_ERRNO ;  nftDestroy (*session) ;  POP_ERRNO ;
        return (errno) ;
    }
    if (peer != NULL) {
        strcat ((*session)->info.dataPortName, "@") ;
        strcat ((*session)->info.dataPortName, peer) ;
    }

/* Allocate a buffer in which to format text to be output. */

    (*session)->outputString = malloc (MAXOUTPUT+1) ;
    if ((*session)->outputString == NULL) {
        LGE "(nftCreate) Error allocating %d-byte output string for %s.\nmalloc: ",
            MAXOUTPUT, nftName (*session, 0)) ;
        PUSH_ERRNO ;  nftDestroy (*session) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(nftCreate) Created FTP session %s.\n", nftName (*session, 0)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftDestroy ()

    Delete an FTP Session.


Purpose:

    Function nftDestroy() destroys an FTP session; the control and data
    network connections, if open, are closed.


    Invocation:

        status = nftDestroy (session) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <status>	- O
            returns the status of deleting the session, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftDestroy (

#    if PROTOTYPES
        NftSession  session)
#    else
        session)

        NftSession  session ;
#    endif

{

    if (session == NULL)  return (0) ;

    LGI "(nftDestroy) Closing session %s ...\n", nftName (session, 0)) ;

/* Close the control and data network connections. */

    if (session->controlStream != NULL)  lfnDestroy (session->controlStream) ;
    if (session->dataStream != NULL)  tcpDestroy (session->dataStream) ;
    if (session->listeningPort != NULL)  tcpDestroy (session->listeningPort) ;

/* Delete the hash table used to map commands to functions. */

    if (session->commandProcs != NULL)  hashDestroy (session->commandProcs) ;

/* Deallocate the FTP session structure. */

    if (session->command != NULL)
        free (session->command) ;	/* "arguments" is part of command string. */
    if (session->outputString != NULL)
        free (session->outputString) ;
    if (session->info.userName != NULL)
        free (session->info.userName) ;
    if (session->info.currentDirectory != NULL)
        free (session->info.currentDirectory) ;
    if (session->info.dataPortName != NULL)
        free (session->info.dataPortName) ;
    if (session->info.oldPathname != NULL)
        free (session->info.oldPathname) ;
    free ((char *) session) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftEvaluate ()

    Evaluate an FTP Command.


Purpose:

    Function nftEvaluate() parses an FTP command and calls the processing
    function defined for the command.


    Invocation:

        status = nftEvaluate (session, command) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the FTP command; e.g., "RETR <pathname>",
            "STOR <pathname>", etc.
        <status>	- O
            returns the status of evaluating the command,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftEvaluate (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command)
#    else
        session, command)

        NftSession  session ;
        char  *command ;
#    endif

{    /* Local variables. */
    void  *commandProc ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftEvaluate) NULL session handle:") ;
        return (errno) ;
    }

/* Save and parse the command string. */

    if (session->command != NULL) {
        free (session->command) ;
        session->arguments = NULL ;
    }

    session->command = strdup (command) ;
    if (session->command == NULL) {
        LGE "(nftEvaluate) Error duplicating command string: \"%s\".\nstrdup: ",
            command) ;
        return (errno) ;
    }

    session->arguments = session->command + strcspn (session->command, " \t") ;
    if (*session->arguments == '\0') {
        session->arguments = NULL ;
    } else {
        *session->arguments++ = '\0' ;
        session->arguments += strspn (session->arguments, " \t") ;
        strTrim (session->arguments, -1) ;
    }

    strToUpper (session->command, -1) ;

    if (strcmp (session->command, "PASS") != 0)
        LGI "(nftEvaluate) %s\n", session->command) ;

/* Lookup and execute the processing function for the command. */

    if (hashSearch (session->commandProcs, session->command, &commandProc) &&
        (commandProc != NULL)) {

        return (((NftCommandProc *) commandProc) (session,
                                                  session->command,
                                                  session->arguments,
                                                  session->info.userData)) ;

    } else {

        nftPutLine (session, "502 %s not implemented.\n", session->command) ;
        SET_ERRNO (EINVAL) ;
        return (errno) ;

    }

}

/*!*****************************************************************************

Procedure:

    nftFd ()

    Get an FTP Session's Control or Data Socket.


Purpose:

    Function nftFd() returns the Unix file descriptor for an FTP session's
    control or data socket connection.


    Invocation:

        fd = nftFd (session, which) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <which>		- I
            specifies which file descriptor to return, the control
            connection's socket (WHICH == 0) or the data connection's
            socket (WHICH != 0).
        <fd>		- O
            returns the UNIX file descriptor for the specified socket;
            -1 is returned if there is no connection of the desired type.

*******************************************************************************/


IoFd  nftFd (

#    if PROTOTYPES
        NftSession  session,
        int  which)
#    else
        session, which)

        NftSession  session ;
        int  which ;
#    endif

{
    if (session == NULL)  return (INVALID_SOCKET) ;
    return (which ? tcpFd (session->dataStream)
                  : lfnFd (session->controlStream)) ;
}

/*!*****************************************************************************

Procedure:

    nftGetLine ()

    Get the Next Line of Input from an FTP Session's Control Connection.


Purpose:

    Function nftGetLine() reads the next line of input from an FTP session's
    control connection.


    Invocation:

        status = nftGetLine (session, &string) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <string>	- O
            returns a pointer to the null-terminated string that was read;
            the string does NOT include the trailing CR/LF.  The string is
            stored in memory private to the NFT session and, although the
            caller can modify the string, it should be used or duplicated
            before calling nftGetLine() again.
        <status>
            returns the status of reading the input line, zero if there
            were no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  nftGetLine (

#    if PROTOTYPES
        NftSession  session,
        char  **string)
#    else
        session, string)

        NftSession  session ;
        char  **string ;
#    endif

{

    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftGetLine) NULL session handle:") ;
        return (errno) ;
    }

    return (lfnGetLine (session->controlStream, session->info.timeout, string)) ;

}

/*!*****************************************************************************

Procedure:

    nftIgnoreCmd ()

    Ignore an FTP Command.


Purpose:

    Function nftIgnoreCmd() is a command processing function that ignores
    its command; i.e., a "202 Command not implemented, superflous at this
    site" message is returned to the client.

*******************************************************************************/


errno_t  nftIgnoreCmd (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command,
        char  *arguments,
        void  *userData)
#    else
        session, command, arguments, userData)

        NftSession  session ;
        const  char  *command ;
        char  *arguments ;
        void  *userData ;
#    endif

{

    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftIgnoreCmd) NULL session handle:") ;
        return (errno) ;
    }

    LGI "(nftIgnoreCmd) Ignoring %s command.\n", command) ;

    nftPutLine (session, "202 %s not implemented, superflous at this site.\n",
                command) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftInfo ()

    Get an FTP Session's Public Information.


Purpose:

    Function nftInfo() returns a pointer to the NftSessionInfo structure
    (see "nft_util.h") containing an FTP session's public information.


    Invocation:

        info = nftInfo (session) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <info>		- O
            returns a pointer to a session's public information structure;
            NULL is returned in the event of an error.

*******************************************************************************/


NftSessionInfo  *nftInfo (

#    if PROTOTYPES
        NftSession  session)
#    else
        session)

        NftSession  session ;
#    endif

{
    return ((session == NULL) ? NULL : &session->info) ;
}

/*!*****************************************************************************

Procedure:

    nftIsReadable ()

    Check if Input is Available on an FTP Session's Control or Data Socket.


Purpose:

    Function nftIsReadable() checks to see if input is available for reading
    on an FTP session's control or data socket connection.


    Invocation:

        isReadable = nftIsReadable (session, which) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <which>		- I
            specifies which connection to check, the control connection
            (WHICH == 0) or the data connection (WHICH != 0).
        <isReadable>	- O
            returns true (a non-zero value) if input is available on the
            specified connection and false (zero) otherwise.

*******************************************************************************/


bool  nftIsReadable (

#    if PROTOTYPES
        NftSession  session,
        int  which)
#    else
        session, which)

        NftSession  session ;
        int  which ;
#    endif

{
    if (session == NULL)  return (false) ;
    return (which ? tcpIsReadable (session->dataStream)
                  : lfnIsReadable (session->controlStream)) ;
}

/*!*****************************************************************************

Procedure:

    nftIsUp ()

    Check if an FTP Session's Control or Data Socket is Up.


Purpose:

    Function nftIsUp() checks to see if an FTP session's control or
    data connection is still up.


    Invocation:

        isUp = nftIsUp (session, which) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <which>		- I
            specifies which connection to check, the control connection
            (WHICH == 0) or the data connection (WHICH != 0).
        <isUp>	- O
            returns true (a non-zero value) if the specified connection
            is up and false (zero) otherwise.

*******************************************************************************/


bool  nftIsUp (

#    if PROTOTYPES
        NftSession  session,
        int  which)
#    else
        session, which)

        NftSession  session ;
        int  which ;
#    endif

{
    if (session == NULL)  return (false) ;
    return (which ? tcpIsUp (session->dataStream)
                  : lfnIsUp (session->controlStream)) ;
}

/*!*****************************************************************************

Procedure:

    nftIsWriteable ()

    Check if an FTP Session's Control or Data Socket is Ready for Writing.


Purpose:

    Function nftIsWriteable() checks to see if data can be written to an
    FTP session's control or data connection.


    Invocation:

        isWriteable = nftIsWriteable (session, which) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <which>		- I
            specifies which connection to check, the control connection
            (WHICH == 0) or the data connection (WHICH != 0).
        <isWriteable>	- O
            returns true (a non-zero value) if the specified connection
            is ready for writing and false (zero) otherwise.

*******************************************************************************/


bool  nftIsWriteable (

#    if PROTOTYPES
        NftSession  session,
        int  which)
#    else
        session, which)

        NftSession  session ;
        int  which ;
#    endif

{
    if (session == NULL)  return (false) ;
    return (which ? tcpIsWriteable (session->dataStream)
                  : lfnIsWriteable (session->controlStream)) ;
}

/*!*****************************************************************************

Procedure:

    nftName ()

    Get the Name of an FTP Session's Control or Data Connection.


Purpose:

    Function nftName() returns the name of an FTP session's control or
    data connection.


    Invocation:

        name = nftName (session, which) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <which>		- I
            specifies which connection to check, the control connection
            (WHICH == 0) or the data connection (WHICH != 0).
        <name>		- O
            returns the session's name.  The name is stored in memory local
            to the NFT utilities and it should not be modified or freed by
            the caller.

*******************************************************************************/


const  char  *nftName (

#    if PROTOTYPES
        NftSession  session,
        int  which)
#    else
        session, which)

        NftSession  session ;
        int  which ;
#    endif

{
    if (session == NULL)  return ("") ;
    return (which ? tcpName (session->dataStream)
                  : lfnName (session->controlStream)) ;
}

/*!*****************************************************************************

Procedure:

    nftOpen ()

    Establish a Data Connection with an FTP Client.


Purpose:

    Function nftOpen() establishes a network connection with a session's
    client for the purpose of transferring data.  This connection is
    separate from the command connection and is taken up and down for
    each FTP command that uses the data channel.  nftRead() and nftWrite()
    should be used to perform I/O on the data channel.


    Invocation:

        status = nftOpen (session) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <status>	- O
            returns the status of opening the data connection, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftOpen (

#    if PROTOTYPES
        NftSession  session)
#    else
        session)

        NftSession  session ;
#    endif

{    /* Local variables. */
    char  *dataPortName ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftOpen) NULL session handle:") ;
        return (errno) ;
    }

    if (session->dataStream != NULL) {
        SET_ERRNO (EEXIST) ;
        LGE "(nftOpen) Data connection is already open.\n") ;
        return (0) ;
    }

/* In active mode, connect to the client's data transfer port. */

    if (session->listeningPort == NULL) {
        dataPortName = (session->info.dataPortName == NULL)
                       ? "20" : session->info.dataPortName ;
        if (tcpCall (dataPortName, 1, &session->dataStream) ||
            tcpComplete (session->dataStream, TIMEOUT, 1)) {
            LGE "(nftOpen) Error establishing connection with port %s.\ntcpCall: ",
                dataPortName) ;
            PUSH_ERRNO ;  session->dataStream = NULL ;  POP_ERRNO ;
            return (errno) ;
        }
    }

/* In passive mode, wait for the client to connect to your data transfer port. */

    else {
        if (tcpAnswer (session->listeningPort, session->info.timeout,
                       &session->dataStream)) {
            LGE "(nftOpen) Error establishing connection on port %s.\ntcpAnswer: ",
                tcpName (session->listeningPort)) ;
            return (errno) ;
        }
        session->info.dataPortName = strdup (tcpName (session->dataStream)) ;
        if (session->info.dataPortName == NULL) {
            LGE "(nftOpen) Error duplicating data stream name: %s\nstrdup: ",
                tcpName (session->dataStream)) ;
            return (errno) ;
        }
    }

    LGI "(nftOpen) Data connection established with port %s.\n",
        tcpName (session->dataStream)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftPASV ()

    Process the FTP PASV Command.


Purpose:

    Function nftPASV() processes the FTP PASV command:

        PASV

    which instructs the session to passively accept data transfer connections
    on a port of its own choosing rather than actively initiating a connection
    with the client.  The dynamically-allocated network port at which the
    session will listen for data transfer connection requests is returned
    to the client in the FTP reply message.


    Invocation:

        status = nftPASV (session, command, arguments, userData) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the command keyword, "PASV".
        <arguments>	- I
            is a string of one or more arguments to the command.  The called
            function can modify the argument string in place for the purpose
            of separating multiple arguments.  NULL is passed in if no
            arguments were present in the command.
        <userData>	- I
            is an arbitrary (VOID *) pointer passed by the application
            to nftCreate() when the session was created.
        <status>	- O
            returns the status of processing the FTP command, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftPASV (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command,
        char  *arguments,
        void  *userData)
#    else
        session, command, arguments, userData)

        NftSession  session ;
        const  char  *command ;
        char  *arguments ;
        void  *userData ;
#    endif

{    /* Local variables. */
    int  h1, h2, h3, h4, portNumber ;
    long  hostAddress ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftPASV) NULL session handle:") ;
        return (errno) ;
    }

    LGI "(nftPASV) PASV\n") ;
    if (session->listeningPort != NULL) {
        tcpDestroy (session->listeningPort) ;
        session->listeningPort = NULL ;
    }
    if (tcpListen (NULL, 1, &session->listeningPort)) {
        LGE "(nftPASV) Error listening on data port.\ntcpListen: ") ;
        return (errno) ;
    }
    hostAddress = netAddrOf (NULL) ;
    hostAddress = htonl (hostAddress) ;
    h1 = (hostAddress >> 24) & 0x0FF ;
    h2 = (hostAddress >> 16) & 0x0FF ;
    h3 = (hostAddress >>  8) & 0x0FF ;
    h4 = (hostAddress >>  0) & 0x0FF ;
    portNumber = sktPort (tcpFd (session->listeningPort)) ;
    nftPutLine (session, "227 Entering Passive Mode (%d,%d,%d,%d,%u,%u)\n",
                h1, h2, h3, h4, portNumber / 256U, portNumber % 256U) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftPORT ()

    Process the FTP PORT Command.


Purpose:

    Function nftPORT() processes the FTP PORT command:

        PORT <h1>,<h2>,<h3>,<h4>,<p1>,<p2>

    which defines the "<port>@<host>" address to which the session will
    connect for data transfers.


    Invocation:

        status = nftPORT (session, command, arguments, userData) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the command keyword, "PORT".
        <arguments>	- I
            is a string of one or more arguments to the command.  The called
            function can modify the argument string in place for the purpose
            of separating multiple arguments.  NULL is passed in if no
            arguments were present in the command.
        <userData>	- I
            is an arbitrary (VOID *) pointer passed by the application
            to nftCreate() when the session was created.
        <status>	- O
            returns the status of processing the FTP command, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftPORT (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command,
        char  *arguments,
        void  *userData)
#    else
        session, command, arguments, userData)

        NftSession  session ;
        const  char  *command ;
        char  *arguments ;
        void  *userData ;
#    endif

{    /* Local variables. */
    int  h1, h2, h3, h4, p1, p2 ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftPORT) NULL session handle:") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    } else if (sscanf (arguments, "%3d,%3d,%3d,%3d,%3d,%3d",
                       &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        nftPutLine (session, "501 Invalid PORT argument: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    LGI "(nftPORT) PORT %s\n", arguments) ;

    if (session->info.dataPortName != NULL) {
        free (session->info.dataPortName) ;
    }
    session->info.dataPortName = (char *) malloc (5 + 1 + (3*4) + 1) ;
    if (session->info.dataPortName == NULL) {
        LGE "(nftPORT) Error duplicating data port name.\nmalloc: ") ;
        return (errno) ;
    }
    sprintf (session->info.dataPortName, "%d@%d.%d.%d.%d",
             p1*256 + p2, h1, h2, h3, h4) ;
    nftPutLine (session, "200 PORT: %s\n", session->info.dataPortName) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftPutLine ()

    Output a Line to an FTP Session's Control Connection.


Purpose:

    Function nftPutLine() formats an output line and writes it to an
    FTP session's control connection.  The caller is responsible for
    ensuring that the length of the formatted output line does not
    exceed 1023 bytes.  nftPutLine() takes care of converting newline
    characters to the carriage return/line feed sequence (Telnet
    end-of-line) required by the FTP protocol.


    Invocation:

        status = nftPutLine (session, format, arg1, arg2, ...) ;

    where

        <stream>		- I
            is the session handle returned by nftCreate().
        <format>		- I
            is a normal PRINTF(3)-style format string.
        <arg1, ..., argN>	- I
            are the arguments expected by the format string.
        <status>
            returns the status of writing the line, zero if there were no
            errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  nftPutLine (

#    if PROTOTYPES
        NftSession  session,
        const  char  *format,
        ...)
#    else
        session, format, va_alist)

        NftSession  session ;
        char  *format ;
        va_dcl
#    endif

{    /* Local variables. */
    va_list  ap ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftPutLine) NULL session handle: ") ;
        return (errno) ;
    }

/* Format the output line. */

#if HAVE_STDARG_H
    va_start (ap, format) ;
#else
    va_start (ap) ;
#endif
    (void) vsprintf (session->outputString, format, ap) ;
    va_end (ap) ;

/* Replace each newline character by the Telnet end-of-line sequence
   (a carriage return followed by a line feed). */

    nl2crlf (session->outputString, -1, MAXOUTPUT+1) ;

/* Write the output line to the control connection. */

    return (lfnPutLine (session->controlStream, session->info.timeout,
                        "%s", session->outputString)) ;

}

/*!*****************************************************************************

Procedure:

    nftQUIT ()

    Process the FTP QUIT Command.


Purpose:

    Function nftQUIT() processes the FTP QUIT command:

        QUIT

    which terminates the user by closing the control connection.


    Invocation:

        status = nftQUIT (session, command, arguments, userData) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the command keyword, "QUIT".
        <arguments>	- I
            is a string of one or more arguments to the command.  The called
            function can modify the argument string in place for the purpose
            of separating multiple arguments.  NULL is passed in if no
            arguments were present in the command.
        <userData>	- I
            is an arbitrary (VOID *) pointer passed by the application
            to nftCreate() when the session was created.
        <status>	- O
            returns the status of processing the FTP command, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftQUIT (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command,
        char  *arguments,
        void  *userData)
#    else
        session, command, arguments, userData)

        NftSession  session ;
        const  char  *command ;
        char  *arguments ;
        void  *userData ;
#    endif

{

    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftQUIT) NULL session handle:") ;
        return (errno) ;
    }

    LGI "(nftQUIT) QUIT\n") ;

    nftPutLine (session, "221 Closing connection %s.\n",
                lfnName (session->controlStream)) ;

    session->info.logout = 1 ;
    if (session->dataStream == NULL) {		/* Transfer not in progress? */
        lfnDestroy (session->controlStream) ;
        session->controlStream = NULL ;
    }

    return (0) ;

}

/*!******************************************************************************

Procedure:

    nftRead ()

    Read Input from a Session's Data Transfer Connection.


Purpose:

    Function nftRead() reads a specified amount of unformatted data
    from an FTP client via the session's data transfer connection.


    Invocation:

        status = nftRead (session, numBytesToRead, buffer, &numBytesRead) ;

    where

        <session>		- I
            is the session handle returned by nftCreate().
        <numBytesToRead>	- I
            has two different meanings depending on its sign.  (1) If the
            number of bytes to read is positive, nftRead() will continue
            to read input until it has accumulated the exact number of bytes
            requested.  If the session's timeout interval expires before the
            requested number of bytes has been read, then nftRead() returns
            with an EWOULDBLOCK status.  (2) If the number of bytes to read
            is negative, nftRead() returns after reading the first "chunk"
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
            was specified for the session's timeout interval, then this
            number should equal (the absolute value of) numBytesToRead.
            If a finite wait was specified, the number of bytes read may
            be less than the number requested.
        <status>		- O
            returns the status of reading the input, zero if there were
            no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  nftRead (

#    if PROTOTYPES
        NftSession  session,
        ssize_t  numBytesToRead,
        char  *buffer,
        size_t  *numBytesRead)
#    else
        session, numBytesToRead, buffer, numBytesRead)

        NftSession  session ;
        ssize_t  numBytesToRead ;
        char  *buffer ;
        size_t  *numBytesRead ;
#    endif

{    /* Local variables. */
    size_t  numBytes ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftRead) NULL session handle: ") ;
        return (errno) ;
    }

    if (numBytesRead == NULL)  numBytesRead = &numBytes ;

/* Input the data from the network. */

    if (tcpRead (session->dataStream, session->info.timeout, numBytesToRead,
                 buffer, numBytesRead)) {
        LGE "(nftRead) Error reading %d bytes from %s.\ntcpRead: ",
            numBytesToRead, nftName (session, 1)) ;
        return (errno) ;
    }

    LGI "(nftRead) From %s: %lu bytes of unformatted data\n",
        nftName (session, 1), (unsigned long) *numBytesRead) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftRegister ()

    Register a Function to Process an FTP Command.


Purpose:

    Function nftRegister() defines a mapping between an FTP command and
    a function to process the command.


    Invocation:

        status = nftRegister (session, command, callback) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the FTP command; e.g., "RETR", "STOR", etc.
        <callback>	- I
            is a pointer to a function used to process the specified
            FTP command; NULL may be used to indicate that the command
            is not supported.
        <status>	- O
            returns the status of registering the processing function,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nftRegister (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command,
        NftCommandProc  *callback)
#    else
        session, command, callback)

        NftSession  session ;
        char  *command ;
        NftCommandProc  *callback ;
#    endif

{

    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftRegister) NULL session handle:") ;
        return (errno) ;
    }

    LGI "(nftRegister) %s\t%p\n", command, (void *) callback) ;

    if (callback == NULL) {
        if (hashSearch (session->commandProcs, command, NULL))
            return (hashDelete (session->commandProcs, command)) ;
        else
            return (0) ;
    } else {
        return (hashAdd (session->commandProcs, command, (void *) callback)) ;
    }

}

/*!*****************************************************************************

Procedure:

    nftSyntax ()

    Get the Syntax of an FTP Command.


Purpose:

    Function nftSyntax() returns a character string containing the syntax
    of an FTP command.


    Invocation:

        syntax = nftSyntax (session, command) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the command keyword.
        <syntax>	- O
            returns the syntax of the specified command.  The syntax string
            is stored in memory local to the NFT utilities and it should not
            be modified or freed by the caller.

*******************************************************************************/


const  char  *nftSyntax (

#    if PROTOTYPES
        NftSession  session,
        const  char  *command)
#    else
        session, command)

        NftSession  session ;
        char  *command ;
#    endif

{    /* Local variables. */
    int  i ;



    for (i = 0 ;  defaultSyntax[i] != NULL ;  i++) {
        if ((command[0] == defaultSyntax[i][0]) &&
            (strncmp (command, defaultSyntax[i], strlen (command)) == 0))
            return (defaultSyntax[i]) ;
    }

    return ("<unknown>") ;

}

/*!*****************************************************************************

Procedure:

    nftWrite ()

    Write Output to a Session's Data Transfer Connection.


Purpose:

    Function nftWrite() writes a specified amount of unformatted data
    to an FTP client through the session's data transfer connection.


    Invocation:

        status = nftWrite (session, numBytesToWrite, buffer, &numBytesWritten) ;

    where

        <session>		- I
            is the session handle returned by nftCreate().
        <numBytesToWrite>	- I
            specifies how much data to write.  If the session's timeout
            interval expires before the requested number of bytes has been
            written, then nftWrite() returns with an EWOULDBLOCK status.
        <buffer>		- I
            is the data to be output.
        <numBytesWritten>	- O
            returns the actual number of bytes written.  If an infinite wait
            was specified for the session's timeout interval, then this number
            should equal numBytesToWrite.  If a finite wait was specified, the
            number of bytes written may be less than the number requested.
        <status>		- O
            returns the status of outputting the data, zero if there were
            no errors and ERRNO if an error occurred.

*******************************************************************************/


errno_t  nftWrite (

#    if PROTOTYPES
        NftSession  session,
        size_t  numBytesToWrite,
        const  char  *buffer,
        size_t  *numBytesWritten)
#    else
        session, numBytesToWrite, buffer, numBytesWritten)

        NftSession  session ;
        size_t  numBytesToWrite ;
        char  *buffer ;
        size_t  *numBytesWritten ;
#    endif

{    /* Local variables. */
    size_t  numBytes ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftWrite) NULL session handle: ") ;
        return (errno) ;
    }

    if (numBytesWritten == NULL)  numBytesWritten = &numBytes ;

/* Output the data to the network. */

    if (tcpWrite (session->dataStream, session->info.timeout, numBytesToWrite,
                  buffer, numBytesWritten)) {
        LGE "(nftWrite) Error writing %d bytes to %s.\ntcpWrite: ",
            numBytesToWrite, nftName (session, 1)) ;
        return (errno) ;
    }

    LGI "(nftWrite) To %s: %lu bytes of unformatted data\n",
        nftName (session, 1), (unsigned long) *numBytesWritten) ;

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the NFT_UTIL functions.

    Under UNIX:
        Compile, link, and run as follows:
            % cc -DTEST nft_util.c <libraries> -o nft_test
            % nft_test <port>

    Under VxWorks:
        Compile and link as follows:
            % cc -DTEST nft_util.c <libraries> -o nft_test.vx.o
        Load and run the server with the following commands:
            -> ld <nft_test.vx.o
            -> sp nft_test, "<port>"

    The test program is a basic FTP server that listens for clients
    at the specified network port.  Try connecting to it from within
    ftp(1):

        % ftp
        ftp> open <host> <port>
        ... enter username and password ...
        ftp> pwd
        ... see current directory ...
        ftp> ls
        ... list current directory ...
        ftp> close
        ... connection to server is closed ...
        ftp>

*******************************************************************************/

#ifdef VXWORKS

    void  nft_test (
        char  *commandLine)

#else

    main (argc, argv)
        int  argc ;
        char  *argv[] ;

#endif

{    /* Local variables. */
    char  *string ;
    TcpEndpoint  connection, listeningPoint ;
    NftSession  session ;




#ifdef VXWORKS
    char  **argv ;
    const  char  *command ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("nft_test", commandLine, &argc, &argv) ;
#endif

    vperror_print = 1 ;
    nft_util_debug = 1 ;

    if (argc < 2) {
        fprintf (stderr, "Usage:  nft_test <port>\n") ;
        exit (EINVAL) ;
    }

    if (tcpListen (argv[1], -1, &listeningPoint))  exit (errno) ;

    for ( ; ; ) {
        if (tcpAnswer (listeningPoint, -1.0, &connection))  break ;
        if (nftCreate (connection, NULL, NULL, NULL, &session))  break ;
        nftPutLine (session, "220 Service is ready.\n") ;
        for ( ; ; ) {
            if (nftGetLine (session, &string))  break ;
            nftEvaluate (session, string) ;
        }
        nftDestroy (session) ;
    }

    tcpDestroy (listeningPoint) ;

    exit (0) ;

}

#endif  /* TEST */
