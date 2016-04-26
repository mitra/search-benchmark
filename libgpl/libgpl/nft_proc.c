/* $Id: nft_proc.c,v 1.22 2011/07/18 17:44:47 alex Exp $ */
/*******************************************************************************

File:

    nft_proc.c

    FTP Command Processing Functions.


Author:    Alex Measday


Purpose:

    The NFT_PROC package is a collection of default and sample implementations
    of FTP command processing functions to be used with the NFT_UTIL package.

    When nftEvaluate() is called to evaluate an FTP command received from
    an FTP client, it parses the command line into a command keyword and
    a single string containing the arguments, if any.  The command keyword
    (e.g., RETR, STOR, etc.) is mapped to the command function registered
    for the keyword and the command function is called as follows:

        status = commandFunction (session, command, arguments, userData) ;

    where

        <session>	- I
            is the session handle returned by nftCreate().
        <command>	- I
            is the command keyword (e.g., FILE, RETR, and STOR).
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

    The command processing function can get access to the session's
    public information (e.g., user name, current directory, etc.) by
    calling nftInfo().  If the function modifies a field in the public
    information, it is responsible for "garbage collecting" the old
    value of the field.

    The command processing functions are responsible for replying to the
    client using the nftPutLine() and nftWrite() output functions.  The
    sequencing and content of replies are detailed (more or less!) in
    RFC 959, "File Transfer Protocol", which is available from a number
    of sources on the Internet.  I say "more or less" because I had to
    run "ftp(1)" with debug on and connected to Sun's FTP server in order
    to get a better grasp of some of the command and reply sequences.


Public Procedures:

    nftAccessCmds() - processes FTP access control commands.
    nftCWD() - processes the FTP CWD command.
    nftFileCmds() - processes the FTP file management commands.
    nftHELP() - processes the FTP HELP command.
    nftListCmds() - processes the FTP LIST and NLST commands.
    nftMODE() - processes the FTP MODE command.
    nftPASS() - processes the FTP PASS command.
    nftRETR() - processes the FTP RETR command.
    nftServiceCmds() - processes FTP service commands.
    nftSTAT() - processes the FTP STAT command.
    nftStoreCmds() - processes the FTP APPE and STOR commands.
    nftSTRU() - processes the FTP STRU command.
    nftTYPE() - processes the FTP TYPE command.
    nftUSER() - processes the FTP USER command.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#if !STDC_HEADERS
    extern  int  unlink () ;
#    define  remove(path)  unlink (path)
#endif
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <time.h>			/* Time definitions. */

#ifndef AUTHENTICATE
#    define  AUTHENTICATE  0
#endif
#define  CD_UP  "CWD .."		/* Change directory up one level. */
#define  MAXBUF  8192			/* Receive/send buffer size. */
#define  WILDCARD  "*"			/* Wildcard file name specification. */

#if defined(NDS)
#    define  OPERATING_SYSTEM  "NDS"
#elif defined(VMS)
#    define  OPERATING_SYSTEM  "VMS"
#    undef  CD_UP
#    define  CD_UP  "CWD [-]"
#    undef  WILDCARD
#    define  WILDCARD  "*.*;*"
#    include  <file.h>			/* File definitions. */
#    include  <unixio.h>		/* VMS-emulation of UNIX I/O. */
#elif defined(VXWORKS)
#    define  OPERATING_SYSTEM  "VXWORKS"
#    include  <ioLib.h>			/* I/O library definitions. */
#    include  <loginLib.h>		/* Login library definitions. */
#    define  exit  return
#elif defined(_WIN32)
#    define  OPERATING_SYSTEM  "WIN32"
#    include  <direct.h>		/* Directory manipulation functions. */
#else
#    if defined(LYNX)
#        define  OPERATING_SYSTEM  "LYNX"
#    else
#        define  OPERATING_SYSTEM  "UNIX"
#    endif
#    if defined(solaris)
#        include  <crypt.h>		/* Password encryption. */
#    else
        extern  char  *crypt () ;
#    endif
#    include  <fcntl.h>			/* File control definitions. */
#    include  <pwd.h>			/* Password definitions. */
#    include  <unistd.h>		/* UNIX I/O definitions. */
#endif

#include  <sys/stat.h>			/* File status definitions. */

#include  "crlf_util.h"			/* CR/LF utilities. */
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "nft_util.h"			/* FTP utilties. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "nft_proc.h"			/* FTP command processing definitions. */

#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  nft_util_debug

#define  NO_STORE_IF_EXISTS  1

/*!*****************************************************************************

Procedure:

    nftAccessCmds ()

    Process the FTP Access Control Commands.


Purpose:

    Function nftAccessCmds() processes the FTP Access Control commands;
    e.g., USER, PASS, ACCT, CWD, etc.

*******************************************************************************/


errno_t  nftAccessCmds (

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
        LGE "(nftAccessCmds) NULL session handle: ") ;
        return (errno) ;
    }

    LGI "(nftAccessCmds) Processing %s command.\n", command) ;

    if ((strcmp (command, "CDUP") == 0) || (strcmp (command, "XCUP") == 0)) {
        return (nftEvaluate (session, CD_UP)) ;
    } else {
        return (nftIgnoreCmd (session, command, arguments, userData)) ;
    }

}

/*!*****************************************************************************

Procedure:

    nftCWD ()

    Process the FTP CWD Command.


Purpose:

    Function nftCWD() processes the FTP CWD command:

        CWD <pathname>

    which changes the session's current directory to the specified pathname.
    The pathname as stored in the session structure ALWAYS has a trailing "/".

*******************************************************************************/


errno_t  nftCWD (

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
    char  *argument ;
    const  char  *newDirectory ;
    NftSessionInfo  *info = nftInfo (session) ;



    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftCWD) NULL session handle: ") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

/* Append a "/" to the user-specified pathname to ensure that the FNM_UTIL
   functions treat it as a directory name. */

    argument = malloc (strlen (arguments) + 1 + 1) ;
    if (argument == NULL) {
        LGE "(nftCWD) Error duplicating %s argument.\nmalloc: ", arguments) ;
        return (errno) ;
    }
    strcpy (argument, arguments) ;
#ifndef VMS
    if (argument[strlen (argument) - 1] != '/')  strcat (argument, "/") ;
#endif

/* Build and save the new directory pathname. */

    newDirectory = fnmBuild (FnmPath, argument, info->currentDirectory, NULL) ;
    free (argument) ;
    free (info->currentDirectory) ;
    info->currentDirectory = strdup (newDirectory) ;
    if (info->currentDirectory == NULL) {
        LGE "(nftCWD) Error duplicating directory name.\nstrdup: ") ;
        return (errno) ;
    }

    nftPutLine (session, "200 %s\n", info->currentDirectory) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftFileCmds ()

    Process the FTP File Management Commands.


Purpose:

    Function nftFileCmds() processes the following FTP file management
    commands:

        DELE <pathname>		- deletes a file.
      * MDTM <pathname>		- gets the time a file was last modified.
        MKD <pathname>		- makes a new directory.
        RMD <pathname>		- removes a directory.
        RNFR <oldName>		- specifies a file to be renamed.
        RNTO <newName>		- renames an RNFRed file.
      * SIZE <pathname>		- gets the size of a file.
        XMKD <pathname>		- experimental MKD.
        XRMD <pathname>		- experimental RMD.

    The X commands are simply aliases for the normal commands, an allowed
    implementation according to RFC 1123.  Commands annotated with an
    asterisk are expected (according to the HP/UX documentation) to be
    incorporated into the FTP protocol in a future RFC.

*******************************************************************************/


errno_t  nftFileCmds (

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
    char  *fileName ;
    NftSessionInfo  *info = nftInfo (session) ;
    struct  stat  fileInfo ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftFileCmds) NULL session handle: ") ;
        return (errno) ;
    }

    LGI "(nftFileCmds) Processing %s command.\n", command) ;

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    fileName = (char *) fnmBuild (FnmPath, arguments, info->currentDirectory,
                                  NULL) ;

    if (strcmp (command, "DELE") == 0) {
        if (remove (fileName))
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        else
            nftPutLine (session, "250 Deleted: %s\n", fileName) ;
    } else if (strcmp (command, "MDTM") == 0) {
        if (stat (fileName, &fileInfo))
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        else
            nftPutLine (session, "213 %s", ctime (&fileInfo.st_mtime)) ;
    } else if ((strcmp (command, "MKD") == 0) ||
               (strcmp (command, "XMKD") == 0)) {
        if (MKDIR (fileName, 0))
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        else
            nftPutLine (session, "257 \"%s\" directory created.\n", fileName) ;
    } else if ((strcmp (command, "RMD") == 0) ||
               (strcmp (command, "XRMD") == 0)) {
#if defined(HAVE_RMDIR) && !HAVE_RMDIR
        nftPutLine (session, "502 %s not implemented.\n", command) ;
#else
        if (rmdir (fileName))
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        else
            nftPutLine (session, "250 Directory deleted: %s\n", fileName) ;
#endif
    } else if (strcmp (command, "RNFR") == 0) {
        if (info->oldPathname != NULL)  free (info->oldPathname) ;
        info->oldPathname = strdup (fileName) ;
        if (info->oldPathname == NULL) {
            LGE "(nftFileCmds) Error duplicating RNFR pathname: %s\nstrdup: ",
                fileName) ;
            return (errno) ;
        }
        if (stat (fileName, &fileInfo))
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        else
            nftPutLine (session, "350 %s found; awaiting new pathname.\n",
                        fileName) ;
    } else if (strcmp (command, "RNTO") == 0) {
        if (info->oldPathname == NULL) {
            nftPutLine (session, "503 Old pathname not specified for %s.\n",
                        fileName) ;
            return (0) ;
        }
        fileName = (char *) fnmBuild (FnmPath, arguments, info->oldPathname,
                                      NULL) ;
        if (rename (info->oldPathname, fileName))
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        else
            nftPutLine (session, "250 Renamed: %s\n", fileName) ;
        free (info->oldPathname) ;  info->oldPathname = NULL ;
    } else if (strcmp (command, "SIZE") == 0) {
        if (stat (fileName, &fileInfo)) {
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        } else {
            nftPutLine (session, "213 %lu\n",
                        (unsigned long) fileInfo.st_size) ;
        }
    } else {
        return (nftIgnoreCmd (session, command, arguments, userData)) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftHELP ()

    Process the FTP HELP Command.


Purpose:

    Function nftHELP() processes the FTP HELP command:

        HELP [<keyword>]

    which returns helpful information to the client.  If a command keyword is
    specified, the syntax of that command is returned to the client; otherwise,
    a list of all the FTP commands is returned to the client.

*******************************************************************************/


errno_t  nftHELP (

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
    static  char  *helpText[2]  OCD ("nft_util")  = {
"214-The following commands are recognized (* = unimplemented; + = ignored):\n\
    Access Commands:\n\
        USER    PASS   *ACCT    CWD     CDUP    SMNT   *REIN    QUIT\n\
    Transfer Parameter Commands:\n\
        PORT    PASV    TYPE    STRU    MODE\n\
    FTP Service Commands:\n\
        RETR    STOR    STOU    APPE   +ALLO   *REST    RNFR    RNTO\n\
       *ABOR    DELE    RMD     MKD     PWD     LIST    NLST   *SITE\n\
        SYST    STAT    HELP    NOOP\n",
"    Mail Transfer Commands (obsolete):\n\
       *MLFL   *MAIL   *MSND   *MSOM   *MSAM   *MSRQ   *MRCP\n\
    Experimental FTP Commands:\n\
        XCUP    XCWD    XMKD    XPWD    XRMD\n\
    Future FTP Commands:\n\
        MDTM    SIZE\n\
214 Send comments to c.a.measday@ieee.org"
    } ;



    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftHELP) NULL session handle: ") ;
        return (errno) ;
    }

    LGI "(nftHELP) Are you being served?\n") ;

    if (arguments == NULL) {
        nftPutLine (session, "%s\n", helpText[0]) ;
        nftPutLine (session, "%s\n", helpText[1]) ;
    } else {
        nftPutLine (session, "214 %s\n", nftSyntax (session, arguments)) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftListCmds ()

    Process the FTP LIST and NLST Commands.


Purpose:

    Function nftListCmds() processes the following FTP commands:

        LIST [<pathname>]
        NLST [<pathname>]

    which generate a list of files matching PATHNAME or, if no pathname
    was specified, a list of files in the current directory.

*******************************************************************************/


errno_t  nftListCmds (

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
    bool  fullList ;
    char  *buffer, *fileName, *nextFile, *pathname ;
    DirectoryScan  scan ;
    int  length, otherInfo ;
    NftSessionInfo  *info = nftInfo (session) ;
    struct  stat  fileInfo ;
    struct  tm  calendarTime ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftListCmds) NULL session handle: ") ;
        return (errno) ;
    }

    LGI "(nftListCmds) %s %s\n",
        command, (arguments == NULL) ? "" : arguments) ;

/* Construct the wildcard pathname. */

    if (arguments == NULL)
        pathname = strdup (fnmBuild (FnmPath, WILDCARD,
                                     info->currentDirectory, NULL)) ;
    else
        pathname = strdup (fnmBuild (FnmPath, arguments, WILDCARD,
                                     info->currentDirectory, NULL)) ;

/* Generate the directory listing. */

    if (drsCreate (pathname, &scan)) {
        nftPutLine (session, "550 %s: %s\n", pathname, STRERROR (errno)) ;
        free (pathname) ;
        return (0) ;
    }

/* Establish a data connection with the client. */

    if (nftIsUp (session, 1)) {
        nftPutLine (session, "125 Data connection open: %s\n",
                    info->dataPortName) ;
    } else if (nftOpen (session)) {
        nftPutLine (session, "425 Can't open data connection: %s\n",
                    info->dataPortName) ;
        drsDestroy (scan) ;  free (pathname) ;
        return (0) ;
    } else {
        nftPutLine (session, "150 Data connection opened: %s\n",
                    info->dataPortName) ;
    }

/* Transmit the directory listing.  The output is buffered to increase
   the speed of the transfer. */

    fullList = (strcmp (command, "LIST") == 0) ;
    otherInfo = (fullList ? (12+2+16+2) : 0) ;
    buffer = malloc (MAXBUF) ;  buffer[0] = '\0' ;  length = 0 ;
    nextFile = (char *) drsFirst (scan) ;

    while (nextFile != NULL) {
#if defined(NDS) || defined(FAST_ON_UNIX)
        fileName = strrchr (nextFile, '/') ;
        if (fileName == NULL)
            fileName = nextFile ;
        else
            ++fileName ;
#else
        fileName = (char *) fnmBuild (FnmFile, nextFile, NULL) ;
#endif
        if ((length + otherInfo + strlen (fileName) + 3) > MAXBUF) {
            if (nftWrite (session, length, buffer, NULL))  break ;
            buffer[0] = '\0' ;  length = 0 ;
        }
        if (fullList) {
            stat (fileName, &fileInfo) ;
            sprintf (&buffer[length], "%12lu  ",
                     (unsigned long) fileInfo.st_size) ;
            length = strlen (buffer) ;
#ifdef VXWORKS
            localtime_r ((time_t *) &fileInfo.st_mtime, &calendarTime) ;
#else
            calendarTime = *(localtime ((time_t *) &fileInfo.st_mtime)) ;
#endif
            strftime (&buffer[length], MAXBUF - length,
                      "%Y/%m/%d %H:%M  ", &calendarTime) ;
        }
        strcat (buffer, fileName) ;
        strcat (buffer, "\r\n") ;
        length = strlen (buffer) ;
        nextFile = (char *) drsNext (scan) ;
    }

    if (length > 0)  nftWrite (session, length, buffer, NULL) ;

    free (buffer) ;

/* Close the data connection. */

    nftPutLine (session, "226 %s complete: %s (%d files)\n",
                command, pathname, drsCount (scan)) ;
    nftClose (session) ;
    drsDestroy (scan) ;
    free (pathname) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftMODE ()

    Process the FTP MODE Command.


Purpose:

    Function nftMODE() processes the FTP MODE command:

        MODE S|B|C

    which specifies the data transfer mode for a session: "S" for stream,
    "B" for block, or "C" for compressed.  Like most FTP servers apparently,
    the NFT_UTIL package only supports stream mode.

*******************************************************************************/


errno_t  nftMODE (

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
        LGE "(nftMODE) NULL session handle: ") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    } if (strlen (arguments) != 1) {
        nftPutLine (session, "501 Invalid MODE argument: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    *arguments = toupper (*arguments) ;

    switch (*arguments) {
    case 'S':
        nftPutLine (session, "200 Date transfer mode: S (stream)\n") ;
        break ;
    case 'B':
    case 'C':
        nftPutLine (session, "504 Unimplemented transfer mode: %s\n",
                    (*arguments == 'B') ? "B (block)" : "C (compressed)") ;
        break ;
    default:
        nftPutLine (session, "501 Invalid MODE argument: %s\n",
                    nftSyntax (session, command)) ;
        break ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftPASS ()

    Process the FTP PASS Command.


Purpose:

    Function nftPASS() processes the FTP PASS command:

        PASS <password>

    which may or may not log the user into the FTP session.  On operating
    systems which support it, passwords are verified and the session's
    current working directory is changed to the user's home directory.
    For security reasons, you shouldn't turn network debug on when an
    NFT-based FTP server is run in a production environment.

*******************************************************************************/


errno_t  nftPASS (

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
    NftSessionInfo  *info = nftInfo (session) ;
#if AUTHENTICATE && !defined(VXWORKS)
    char  *encryptedPassword ;
    struct  passwd  *userEntry ;
#endif




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftPASS) NULL session handle: ") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

/* The user's name must be specified before his/her password is entered. */

    if (info->userName == NULL) {
        nftPutLine (session, "503 No user name; login first.\n") ;
        return (0) ;
    }

/* Disallow anonymous FTP access. */

    if (strcmp (info->userName, "anonymous") == 0) {
        nftPutLine (session, "530 %s is denied anonymous access.\n",
                    arguments) ;
        free (info->userName) ;  info->userName = NULL ;
        return (0) ;
    }

/* Verify the user's name and password. */

#if AUTHENTICATE
#ifdef VXWORKS
    if (loginUserVerify (info->userName, arguments) != OK) {
#else
    userEntry = getpwnam (info->userName) ;
    encryptedPassword = crypt (arguments, userEntry->pw_passwd) ;
    if (strcmp (encryptedPassword, userEntry->pw_passwd) != 0) {
#endif
        nftPutLine (session, "530 Login failed.\n") ;
        free (info->userName) ;  info->userName = NULL ;
        return (nftQUIT (session, "QUIT", NULL, userData)) ;
    }
#endif

/* Make the user's home directory the session's current working directory. */

#if AUTHENTICATE && !defined(VXWORKS)
    if (info->currentDirectory != NULL)  free (info->currentDirectory) ;
    info->currentDirectory = strndup (userEntry->pw_dir,
                                      strlen (userEntry->pw_dir) + 1) ;
    if (info->currentDirectory == NULL) {
        LGE "(nftUSER) Error duplicating home directory: %s\nstrndup: ",
            userEntry->pw_dir) ;
        return (errno) ;
    }
    strcat (info->currentDirectory, "/") ;
#endif

/* Logged in! */

    nftPutLine (session, "230 User %s logged in.\n", info->userName) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftRETR ()

    Process the FTP RETR Command.


Purpose:

    Function nftRETR() processes the FTP RETR command:

        RETR <pathname>

    which retrieves the specified file.  If the transfer type is ASCII
    ("TYPE A"), newline characters in the file text are converted to
    Telnet end-of-lines ("\r\n") before being sent to the client.  If
    the transfer type is NOT ASCII, a straight binary transfer is done.

*******************************************************************************/


errno_t  nftRETR (

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
    char  *buffer ;
    const  char  *fileName ;
    FILE  *file ;
    long  totalBytes ;
    size_t  numBytes ;
    NftSessionInfo  *info = nftInfo (session) ;
    struct  stat  fileInfo ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftRETR) NULL session handle: ") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    LGI "(nftRETR) %s %s\n", command, arguments) ;

/* Open the file being retrieved. */

    fileName = fnmBuild (FnmPath, arguments, info->currentDirectory, NULL) ;
    if (info->representation[0] == 'A')
        file = fopen (fileName, "r") ;
    else
        file = fopen (fileName, "rb") ;
    if (file == NULL) {
        nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        return (0) ;
    }

    fstat (fileno (file), &fileInfo) ;
    fileName = fnmBuild (FnmFile, fileName, NULL) ;

/* Establish a data connection with the client. */

    if (nftIsUp (session, 1)) {
        nftPutLine (session, "125 FILE: %s (%lu bytes) on %s [%c]\n",
                    fileName, fileInfo.st_size, info->dataPortName,
                    info->representation[0]) ;
    } else if (nftOpen (session)) {
        nftPutLine (session, "425 Can't open data connection: %s\n",
                    info->dataPortName) ;
        fclose (file) ;
        return (0) ;
    } else {
        nftPutLine (session, "150 FILE: %s (%lu bytes) on %s [%c]\n",
                    fileName, fileInfo.st_size, info->dataPortName,
                    info->representation[0]) ;
    }

/* Send the requested file. */

    buffer = malloc (MAXBUF) ;
    for (totalBytes = 0 ;  ;  totalBytes += numBytes) {
        if (info->representation[0] == 'A') {
            if ((numBytes = fread (buffer, 1, MAXBUF/2, file)) == 0)  break ;
            nl2crlf (buffer, numBytes, MAXBUF) ;
            numBytes = strlen (buffer) ;
        } else {
            if ((numBytes = fread (buffer, 1, MAXBUF, file)) == 0)  break ;
        }
        if (nftWrite (session, numBytes, buffer, NULL))  break ;
    }
    free (buffer) ;

/* Close the data connection. */

    nftPutLine (session, "226 RETR complete: %s (%ld bytes)\n",
                fileName, totalBytes) ;
    nftClose (session) ;
    fclose (file) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftServiceCmds ()

    Process the FTP Service Commands.


Purpose:

    Function nftServiceCmds() processes the following FTP Service commands:

        NOOP			- performs no operation.
        PWD			- gets the session's current working directory.
        SYST			- gets the server's operating system.
        XPWD			- experimental PWD.

    XPWD is simply an alias for PWD, an allowed implementation according to
    RFC 1123.

*******************************************************************************/


errno_t  nftServiceCmds (

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
    NftSessionInfo  *info = nftInfo (session) ;



    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftServiceCmds) NULL session handle: ") ;
        return (errno) ;
    }

    LGI "(nftServiceCmds) Processing %s command.\n", command) ;

    if (strcmp (command, "NOOP") == 0) {
        nftPutLine (session, "200 Command okay.\n") ;
    } else if ((strcmp (command, "PWD") == 0) ||
               (strcmp (command, "XPWD") == 0)) {
        nftPutLine (session, "257 \"%s\"\n", info->currentDirectory) ;
    } else if (strcmp (command, "SYST") == 0) {
        nftPutLine (session, "215 %s\n", OPERATING_SYSTEM) ;
    } else {
        return (nftIgnoreCmd (session, command, arguments, userData)) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftSTAT ()

    Process the FTP STAT Command.


Purpose:

    Function nftSTAT() processes the FTP STAT command, which returns
    the current session status to the client.

*******************************************************************************/


errno_t  nftSTAT (

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
    NftSessionInfo  *info = nftInfo (session) ;



    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftSTAT) NULL session handle: ") ;
        return (errno) ;
    }

    LGI "(nftSTAT) Up and running ...\n") ;

    nftPutLine (session, "211-FTP Session Status:\n") ;
    nftPutLine (session, "    Serving %s@%s.\n",
                (info->userName == NULL) ? "?" : info->userName,
                nftName (session, 0)) ;
    nftPutLine (session, "    Representation type is %c%c.\n",
                info->representation[0], info->representation[1]) ;
    if (nftFd (session, 1) >= 0)
        nftPutLine (session, "    Transferring data on %s.\n",
                    nftName (session, 1)) ;
    if (info->logout)
        nftPutLine (session, "    Logging out.\n") ;
    nftPutLine (session, "211 When I grow up, I want to be an HTTP server!\n") ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftStoreCmds ()

    Process the FTP APPE and STOR Commands.


Purpose:

    Function nftStoreCmds() processes the following FTP commands:

        APPE <pathname>		- appends received data to a file.
        STOR <pathname>		- stores received data in a file.
        STOU [<pathname>]	- stores received data in system-named file.

    which receive and store the specified file, appending to the existing
    file in the case of APPE.  The STOU command stores the incoming data
    in the optional, specified file if the file doesn't already exist; if
    the file does exist, a unique file name is generated and returned to
    the client.

    If the transfer type is ASCII ("TYPE A"), Telnet end-of-lines ("\r\n")
    in the received text are converted to newline characters before being
    written to the file.  If the transfer type is NOT ASCII, a straight
    binary transfer is done.

*******************************************************************************/


errno_t  nftStoreCmds (

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
    char  *buffer, lastChar ;
    const  char  *fileName ;
    FILE  *file ;
    int  append, storeUnique ;
    long  totalBytes ;
    NftSessionInfo  *info = nftInfo (session) ;
    size_t  numBytes ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftStoreCmds) NULL session handle: ") ;
        return (errno) ;
    }

    append = (strcmp (command, "APPE") == 0) ;
    storeUnique = (strcmp (command, "STOU") == 0) ;

    if (!storeUnique && (arguments == NULL)) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    LGI "(nftStoreCmds) %s %s\n", command, arguments) ;


/* In the case of the Store Unique (STOU) command, generate a unique file
   name for the file being stored. */

    if (storeUnique) {
        FileName  fname ;

        fname = fnmCreate (info->currentDirectory,
                           (arguments == NULL) ? tmpnam (NULL) : arguments) ;
        for ( ; ; ) {
            if (!fnmExists (fname))  break ;
            fnmDestroy (fname) ;
            fname = fnmCreate (info->currentDirectory, tmpnam (NULL), NULL) ;
        }
        fileName = fnmBuild (FnmPath, fnmPath (fname), NULL) ;
        fnmDestroy (fname) ;
    } else {
        fileName = fnmBuild (FnmPath, arguments, info->currentDirectory, NULL) ;
    }


/* Open the file being received. */

#ifdef NO_STORE_IF_EXISTS
    if (!append) {		/* This is contrary to the FTP standard. */
        FileName  fname = fnmCreate (fileName, NULL) ;

        if (fnmExists (fname)) {
            nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (EEXIST)) ;
            fnmDestroy (fname) ;
            return (0) ;
        }
        fnmDestroy (fname) ;
    }
#endif
    if (info->representation[0] == 'A')
        file = fopen (fileName, append ? "a" : "w") ;
    else
        file = fopen (fileName, append ? "ab" : "wb") ;
    if (file == NULL) {
        nftPutLine (session, "550 %s: %s\n", fileName, STRERROR (errno)) ;
        return (0) ;
    }
    fileName = fnmBuild (FnmFile, fileName, NULL) ;


/* Establish a data connection with the client. */

    if (nftIsUp (session, 1)) {
        nftPutLine (session, "125 FILE: %s on %s [%c]\n",
                    fileName, info->dataPortName, info->representation[0]) ;
    } else if (nftOpen (session)) {
        nftPutLine (session, "425 Can't open data connection: %s\n",
                    info->dataPortName) ;
        fclose (file) ;
        return (0) ;
    } else {
        nftPutLine (session, "150 FILE: %s on %s [%c]\n",
                    fileName, info->dataPortName, info->representation[0]) ;
    }


/* Read and store the data in the file. */

    buffer = malloc (MAXBUF) ;

    lastChar = '\0' ;

    for (totalBytes = 0 ;  ;  totalBytes += numBytes) {
        if (info->representation[0] == 'A') {
            if (nftRead (session, -(MAXBUF/2), buffer, &numBytes))  break ;
            crlf2nl (buffer, numBytes, &lastChar) ;
            numBytes = strlen (buffer) ;
        } else {
            if (nftRead (session, -MAXBUF, buffer, &numBytes))  break ;
        }
        if (fwrite (buffer, 1, numBytes, file) < numBytes)  break ;
    }

    free (buffer) ;

/* Close the data connection. */

    nftPutLine (session, "226 %s complete: %s (%ld bytes)\n",
                command, fileName, totalBytes) ;
    nftClose (session) ;
    fclose (file) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftSTRU ()

    Process the FTP STRU Command.


Purpose:

    Function nftSTRU() processes the FTP STRU command:

        STRU F|R|P

    which specifies the file structure: "F" for file (no record structure),
    "R" for record structure, or "P" for page structure.  The NFT_UTIL
    package only supports the unstructured "F" file structure.

*******************************************************************************/


errno_t  nftSTRU (

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
        LGE "(nftSTRU) NULL session handle: ") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    } else if (strlen (arguments) != 1) {
        nftPutLine (session, "501 Invalid argument: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    *arguments = toupper (*arguments) ;

    switch (*arguments) {
    case 'F':
        nftPutLine (session, "200 File structure: F (file)\n") ;
        break ;
    case 'R':
    case 'P':
        nftPutLine (session, "504 Unimplemented file structure: %s\n",
                    (*arguments == 'R') ? "R (record)" : "P (page)") ;
        break ;
    default:
        nftPutLine (session, "501 Invalid STRU argument: %s\n", arguments) ;
        break ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftTYPE ()

    Process the FTP TYPE Command.


Purpose:

    Function nftTYPE() processes the FTP TYPE command:

        TYPE A|E N|T|C
        TYPE I
        TYPE L <byteSize>

    which defines the data representation type for a session:

        "A" - ASCII
        "E" - EBCDIC
        "I" - Image
        "L" - Local byte

    The allowable format codes for ASCII and EBCDIC are:

        "N" - Non-print
        "T" - Telnet format
        "C" - Carriage control (ASA)

    Only the "AN" and "I" representation types are currently supported,
    although "L8" should also be supported according to RFC 1123.

*******************************************************************************/


errno_t  nftTYPE (

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
    char  *format ;
    NftSessionInfo  *info = nftInfo (session) ;




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftTYPE) NULL session handle: ") ;
        return (errno) ;
    }

/* Examine the first argument, the representation type. */

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    format = arguments + strcspn (arguments, " \t") ;
    if (*format == '\0') {
        format = NULL ;
    } else {
        *format++ = '\0' ;
        format = format + strspn (format, " \t") ;
    }

    if (strlen (arguments) != 1) {
        nftPutLine (session, "501 Invalid representation type: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    *arguments = toupper (*arguments) ;

    switch (*arguments) {
    case 'A':
    case 'I':
        info->representation[0] = *arguments ;
        info->representation[1] = 'N' ;
        break ;
    case 'E':
    case 'L':
        nftPutLine (session, "504 Unimplemented representation type: %s\n",
                    (*arguments == 'E') ? "E (EBCDIC)" : "L (local byte)") ;
        return (0) ;
    default:
        nftPutLine (session, "501 Invalid representation type: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    if (format == NULL) {
        nftPutLine (session, "200 Representation type: %s\n",
                    (info->representation[0] == 'A') ? "A (ASCII)"
                                                     : "I (image)") ;
        return (0) ;
    }

/* Examine the second argument, the format parameter. */

    if (strlen (format) != 1) {
        nftPutLine (session, "501 Invalid format parameter: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    *format = toupper (*format) ;

    switch (*format) {
    case 'N':
        info->representation[1] = 'N' ;
        break ;
    case 'T':
    case 'C':
        nftPutLine (session, "504 Unimplemented format: %s\n",
                    (*format == 'T') ? "T (Telnet)" : "C (carriage control)") ;
        return (0) ;
    default:
        nftPutLine (session, "501 Invalid format parameter: %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nftUSER ()

    Process the FTP USER Command.


Purpose:

    Function nftUSER() processes the FTP USER command:

        USER <name>

    which logs the user into the FTP session.

*******************************************************************************/


errno_t  nftUSER (

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
    NftSessionInfo  *info = nftInfo (session) ;
#if AUTHENTICATE && !defined(VXWORKS)
    struct  passwd  *userEntry ;
#endif




    if (session == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nftUSER) NULL session handle: ") ;
        return (errno) ;
    }

    if (arguments == NULL) {
        nftPutLine (session, "501 Missing argument(s): %s\n",
                    nftSyntax (session, command)) ;
        return (0) ;
    }

/* Verify the user's name. */

#if AUTHENTICATE && !defined(VXWORKS)
    userEntry = getpwnam (arguments) ;
    if (userEntry == NULL) {
        nftPutLine (session, "530 Login failed.\n") ;
        return (nftQUIT (session, "QUIT", NULL, userData)) ;
    }

    LGI "User %s -\n", userEntry->pw_name) ;
    LGI "    Password: %s\n", userEntry->pw_passwd) ;
    LGI "         UID: %ld\n", (long) userEntry->pw_uid) ;
    LGI "         GID: %ld\n", (long) userEntry->pw_gid) ;
#    ifdef AGE_AND_COMMENT
    LGI "         Age: %s\n", userEntry->pw_age) ;
    LGI "     Comment: %s\n", userEntry->pw_comment) ;
#    endif
    LGI "       GECOS: %s\n", userEntry->pw_gecos) ;
    LGI "   Directory: %s\n", userEntry->pw_dir) ;
    LGI "       Shell: %s\n", userEntry->pw_shell) ;
#endif

/* Save the user's name for future reference. */

    if (info->userName != NULL)  free (info->userName) ;
    info->userName = strdup (arguments) ;
    if (info->userName == NULL) {
        LGE "(nftUSER) Error duplicating user name: %s\nstrdup: ", arguments) ;
        return (errno) ;
    }

/* Check for anonymous FTP. */

    if (strcmp (info->userName, "anonymous") == 0) {
        nftPutLine (session, "331 Enter your E-Mail address as the password.\n") ;
        return (0) ;
    }

/* If a password is required, then prompt the user for it. */

#if AUTHENTICATE
#ifdef VXWORKS
    {    /* Check user name in nftPASS(). */
#else
    if ((userEntry->pw_passwd != NULL) && (userEntry->pw_passwd[0] != '\0')) {
#endif
        nftPutLine (session, "331 Password required for %s.\n", arguments) ;
        return (0) ;
    }
#endif

/* Make the user's home directory the session's current working directory. */

#if AUTHENTICATE && !defined(VXWORKS)
    if (info->currentDirectory != NULL)  free (info->currentDirectory) ;
    info->currentDirectory = strndup (userEntry->pw_dir,
                                      strlen (userEntry->pw_dir) + 1) ;
    if (info->currentDirectory == NULL) {
        LGE "(nftUSER) Error duplicating home directory: %s\nstrndup: ",
            userEntry->pw_dir) ;
        return (errno) ;
    }
    strcat (info->currentDirectory, "/") ;
#endif

    nftPutLine (session, "230 User %s logged in.\n", arguments) ;

    return (0) ;

}
