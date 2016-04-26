/* $Id: xqt_util.c,v 1.12 2004/02/16 20:39:56 alex Exp $ */
/*******************************************************************************

File:

    xqt_util.c

    Shell Execution Utilities.


Author:    Alex Measday


Purpose:

    The Shell Execution Utilities provide a means for a program to execute
    host system commands, e.g., shell commands under UNIX and DCL commands
    under VMS.  The utilities create a subprocess (by forking under UNIX
    and spawning under VMS) to execute the commands; pipes (mailboxes under
    VMS) are used to connect the parent program to the subprocess's standard
    input and output streams.

    To execute a single shell command and to read the output of the command,
    the following code could be used:

        #include  "xqt_util.h"		-- Shell execution utilities.
        char  buffer[132] ;
        XqtStream  stream ;
        ...
        xqtOpen (NULL, &stream) ;
        ...
        xqtWrite (stream, "%s", "<shell command>") ;
        xqtRead (stream, sizeof buffer, buffer) ;
        ... process result string in buffer ...
        xqtClose (stream) ;

    The code above executes only one command; you can actually have any
    number of xqtWrite()s and xqtRead()s between the opening and the
    closing of the execution stream.

    Note that you must have some way of knowing how many lines of output
    to read from an executed command.  One means of ensuring that the
    correct number of lines is read is as follows:

        (1) Issue the command to be executed.
        (2) Issue a command that generates known output.
        (3) Read each line of output until the known output is encountered.

    Under VMS, for example, the following code fragment reads a variable-
    length directory listing:

        char  filename[256] ;
        ...
        xqtWrite (stream, "%s", "DIRECTORY/NOHEADING/NOTRAILING") ;
        xqtWrite (stream, "%s", "SHOW SYMBOL $STATUS") ;
        while (!xqtRead (stream, sizeof filename, filename)) {
            if (strncmp (filename, "  $STATUS ==", 12) == 0)  break ;
            ... process file name ...
        }
        ... end of directory listing ...

    DCL's $STATUS variable holds the completion status of the last command
    executed (the directory command, in this case).  The "SHOW SYMBOL $STATUS"
    command outputs the completion status in the following format:

                      $STATUS == "%X<completion_status>"

    The completion status is a VMS error code (SS$_, etc.) displayed in
    hexadecimal form.  A useful thing to do is to examine the value of the
    $STATUS variable after every command executed.  UNIX shells generally
    have a similar means of determining the completion status of a command.


Procedures:

    xqtClose() - closes a shell execution stream.
    xqtFd() - returns the file descriptor for input from the stream.
    xqtOpen - opens a shell execution stream.
    xqtPoll() - checks if any output from a shell execution stream is
        waiting to be read.
    xqtRead() - reads a line of output from a shell execution stream.
    xqtWrite() - writes a line of input to a shell execution stream.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* C Library string functions. */
#if defined(VMS)
#    include  <clidef.h>		/* VMS CLI definitions. */
#    include  <descrip.h>		/* Descriptor definitions. */
#    include  <dvidef.h>		/* VMS device information definitions. */
#    include  <nam.h>			/* RMS name block (NAM) definitions. */
#    define  MAXPATHLEN  NAM$C_MAXRSS
#    include  <stsdef.h>		/* VMS status code structure. */
#    include  <unixio.h>		/* VMS-emulation of UNIX I/O. */
#elif defined(VXWORKS)
#    include  <ioLib.h>			/* I/O library definitions. */
#    include  <unistd.h>		/* UNIX I/O definitions. */
#else
#    include  <unistd.h>		/* UNIX I/O definitions. */
#    include  <sys/ioctl.h>		/* I/O control definitions. */
#endif

#include  "str_util.h"			/* String manipulation functions. */
#include  "xqt_util.h"			/* Shell execution utilities. */


/*******************************************************************************
    Shell Execution Stream Descriptor - describes an established execution
        stream.  In particular, a stream's descriptor identifies the inter-
        process communications channels connected to the shell subprocess's
        standard input and output streams.
*******************************************************************************/

typedef  struct  _XqtStream {
    char  *shellName ;			/* Name of shell executing commands. */
    int  processID ;			/* Subprocess's process ID. */
    int  inputPipe[2] ;			/* Subprocess's standard input stream. */
    FILE  *input ;			/* File pointer for input pipe. */
    int  outputPipe[2] ;		/* Subprocess's standard output stream. */
    FILE  *output ;			/* File pointer for output pipe. */
#if defined(vms)
    int  channel ;			/* VMS channel assigned to output pipe's mailbox. */
#endif
    char  *lastCommand ;		/* Last command written to subprocess. */
}  _XqtStream ;


int  xqt_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  xqt_util_debug


#if defined(VMS)			/* C Preprocessor macro definitions. */
#    define  ASSIGN(desc, string) \
        { desc.dsc$w_length = (string == NULL) ? 0 : strlen (string) ; \
          desc.dsc$b_dtype = DSC$K_DTYPE_T ; \
          desc.dsc$b_class = DSC$K_CLASS_S ; \
          desc.dsc$a_pointer = (string == NULL) ? "" : string ; \
        }
#endif

/*!*****************************************************************************

Procedure:

    xqtClose ()

    Close a Shell Execution Stream.


Purpose:

    Function xqtClose() terminates a shell execution stream.


    Invocation:

        status = xqtClose (stream) ;

    where:

        <stream>	- I
            is the stream handle returned by xqtOpen().
        <status>	- O
            always returns zero.

*******************************************************************************/


int  xqtClose (

#    if PROTOTYPES
        XqtStream  stream)
#    else
        stream)

        XqtStream  stream ;
#    endif

{    /* Local variables. */
    int  i ;



    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xqtClose) NULL shell stream: ") ;
        return (errno) ;
    }

/* Close the input and output pipes and deallocate the execution stream
   structure. */

    LGI "(xqtClose) Closing shell (\"%s\") execution stream %p.\n",
        stream->shellName, (void *) stream) ;
    for (i = 0 ;  i < 2 ;  i++) {
        if (stream->inputPipe[i] >= 0)  close (stream->inputPipe[i]) ;
        if (stream->outputPipe[i] >= 0)  close (stream->outputPipe[i]) ;
    }
    if (stream->input != NULL)  fclose (stream->input) ;
    if (stream->output != NULL)  fclose (stream->output) ;
    if (stream->shellName != NULL)  free (stream->shellName) ;
    if (stream->lastCommand != NULL)  free (stream->lastCommand) ;
#if defined(vms)
    if (stream->channel > 0)  SYS$DASSGN (stream->channel) ;
#endif
    free ((void *) stream) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xqtFd ()

    Get a Shell Execution Stream's File Descriptor.


Purpose:

    Function xqtFd() returns the file descriptor used by xqtRead() to read the
    output from the shell.


    Invocation:

        fd = xqtFd (stream) ;

    where

        <stream>	- I
            is the stream handle returned by xqtOpen().
        <fd>		- O
            returns the UNIX file descriptor used to read output from the shell.

*******************************************************************************/


int  xqtFd (

#    if PROTOTYPES
        XqtStream  stream)
#    else
        stream)

        XqtStream  stream ;
#    endif

{

#if defined(vms)
    return ((stream == NULL) ? -1 : stream->outputPipe[0]) ;
#else
    return ((stream == NULL) ? -1 : fileno (stream->output)) ;
#endif

}

/*!*****************************************************************************

Procedure:

    xqtOpen ()

    Open a Shell Execution Stream.


Purpose:

    Function xqtOpen() opens a shell execution stream so that a program can
    execute host system shell commands.  Under UNIX, xqtOpen() forks a
    subprocess, which then EXEC(2)s the specified UNIX shell; pipes connect
    the parent program to the shell subprocess's standard input and output
    channels.  Under VMS, xqtOpen() spawns a subprocess running the VMS
    DCL interpreter; mailboxes connect the parent program to the DCL
    subprocess's standard input and output channels.

    The parent program can write commands to the shell subprocess using
    xqtWrite() and read output from the shell subprocess using xqtRead().


    Invocation:

        status = xqtOpen (shell, &stream) ;

    where:

        <shell>		- I
            is the name of the shell that will execute commands.  If this
            argument is NULL, the default shell of the host operating system
            is used (e.g., "/bin/csh" under UNIX and DCL under VMS).
        <stream>	- O
            returns a handle for the opened shell execution stream.
        <status>	- O
            returns the status of opening the shell execution stream, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  xqtOpen (

#    if PROTOTYPES
        const  char  *shell,
        XqtStream  *stream)
#    else
        shell, stream)

        char  *shell ;
        XqtStream  *stream ;
#    endif

{    /* Local variables. */
    int  i, status ;
#if defined(vms)
    char  inputName[MAXPATHLEN], outputName[MAXPATHLEN] ;
    long  flags ;
    struct  dsc$descriptor_s  inputNameDsc, outputNameDsc ;
#endif




#if defined(vms)
    if (shell == NULL)  shell = "DCL" ;
#else
    if (shell == NULL)  shell = "/bin/csh" ;
#endif


/* Create and initialize a shell execution stream structure. */

    *stream = (XqtStream) malloc (sizeof (_XqtStream)) ;
    if (*stream == NULL) {
        LGE "(xqtOpen) Error allocating a shell execution stream structure.\nmalloc: ") ;
        return (errno) ;
    }
    (*stream)->shellName = strdup (shell) ;
    (*stream)->processID = 0 ;
    for (i = 0 ;  i < 2 ;  i++) {
        (*stream)->inputPipe[i] = -1 ;
        (*stream)->outputPipe[i] = -1 ;
    }
    (*stream)->input = NULL ;
    (*stream)->output = NULL ;
#if defined(vms)
    (*stream)->channel = 0 ;
#endif
    (*stream)->lastCommand = NULL ;


/* Create the interprocess communication entities used to communicate with the
   shell subprocess. */

    status = pipe ((*stream)->inputPipe) ;
    if (status) {
        LGE "(xqtOpen) Error creating shell's input pipe.\npipe: ") ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

    status = pipe ((*stream)->outputPipe) ;
    if (status) {
        LGE "(xqtOpen) Error creating shell's output pipe.\npipe: ") ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

/*******************************************************************************
    Create a subprocess running the specified shell.
*******************************************************************************/


#if defined(vms)

/* Under VMS, spawn the subprocess, passing it the names of the input and
   output pipes for its SYS$INPUT and SYS$OUTPUT files. */

    getname ((*stream)->inputPipe[1], inputName) ;
    ASSIGN (inputNameDsc, inputName) ;
    getname ((*stream)->outputPipe[0], outputName) ;
    ASSIGN (outputNameDsc, outputName) ;
    flags = CLI$M_NOWAIT ;
    status = LIB$SPAWN (NULL, &inputNameDsc, &outputNameDsc, &flags,
                        NULL, &(*stream)->processID, NULL, NULL, NULL,
                        0, NULL, NULL) ;
    if (!(status & STS$M_SUCCESS)) {
        SET_ERRNO (EVMSERR) ;  vaxc$errno = status ;
        LGE "(xqtOpen) Error spawning the %s shell subprocess.\nLIB$SPAWN: ",
                 (*stream)->shellName) ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Assign a VMS channel to the output pipe's mailbox.  This channel is used
   by xqtPoll() to check for input from the shell subprocess. */

    status = SYS$ASSIGN (&outputNameDsc, &(*stream)->channel, 0, NULL) ;
    if (!(status & STS$M_SUCCESS)) {
        SET_ERRNO (EVMSERR) ;  vaxc$errno = status ;
        LGE "(xqtOpen) Error assigning a channel to mailbox \"%s\".\nSYS$ASSIGN: ",
                 outputName) ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

#else

/* Under UNIX, fork a subprocess.  In the subprocess, close the unused pipe
   descriptors (the "write" side of my input pipe and the "read" side of my
   output pipe), reassign standard input and standard output to the remaining
   pipe descriptors, and start up the specified shell. */

    (*stream)->processID = fork () ;

    if ((*stream)->processID == 0) {		/* Child process? */

        close ((*stream)->inputPipe[1]) ;	/* Write side of input pipe. */
        close ((*stream)->outputPipe[0]) ;	/* Read side of output pipe. */
						/* Reassign standard input. */
        if (dup2 ((*stream)->inputPipe[0], fileno (stdin)) < 0) {
            LGE "(xqtOpen) Error redirecting shell's standard input.\ndup2: ") ;
            exit (errno) ;
        }					/* Reassign standard output. */
        if (dup2 ((*stream)->outputPipe[1], fileno (stdout)) < 0) {
            LGE "(xqtOpen) Error redirecting shell's standard output.\ndup2: ") ;
            exit (errno) ;
        }					/* Run the shell. */
        execl ((*stream)->shellName, (*stream)->shellName, NULL) ;
        LGE "(xqtOpen) Error running %s shell.\nexecl: ", (*stream)->shellName) ;
        exit (errno) ;

    } else if ((*stream)->processID < 0) {	/* Error? */

        LGE "(xqtOpen) Error creating shell subprocess.\nfork: ") ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;

    } else {					/* Parent process? */

        /* <null> */ ;				/* No error. */

    }

#endif


/* No matter what operating system we're running under, I'm the parent
   process when I get down here.  Close the unused pipe descriptors,
   build (FILE *) pointers for the remaining pipe descriptors, and
   return a handle for the shell execution stream to the calling routine. */

    close ((*stream)->inputPipe[0]) ;		/* Read side of input pipe. */
    (*stream)->inputPipe[0] = -1 ;
						/* Write side of input pipe. */
    (*stream)->input = fdopen ((*stream)->inputPipe[1], "w") ;
    if ((*stream)->input == NULL) {
        LGE "(xqtOpen) Error opening the input pipe as a file.\nfdopen: ") ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }

						/* Read side of output pipe. */
    (*stream)->output = fdopen ((*stream)->outputPipe[0], "r") ;
    if ((*stream)->output == NULL) {
        LGE "(xqtOpen) Error opening the output pipe as a file.\nfdopen: ") ;
        PUSH_ERRNO ;  xqtClose (*stream) ;  POP_ERRNO ;
        return (errno) ;
    }
		/* Don't close the output pipe under VMS.  Doing so sends an
		   end-of-file message to the pipe's mailbox, which xqtRead()
		   will read first, before any output from the shell! */
#if !defined(vms)
    close ((*stream)->outputPipe[1]) ;		/* Write side of output pipe. */
    (*stream)->outputPipe[1] = -1 ;
#endif

    LGI "(xqtOpen) Opened shell (\"%s\") execution stream %p.\n",
        (*stream)->shellName, (void *) *stream) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xqtPoll ()

    Check if Output from a Shell Execution Stream is Waiting to be Read.


Purpose:

    Function xqtPoll() checks to see if output from a shell execution
    stream is available for reading by xqtRead().


    Invocation:

        num_chars = xqtPoll (stream) ;

    where:

        <stream>	- I
            is the stream handle returned by xqtOpen().
        <num_chars>	- O
            returns the number of characters waiting to be read from the
            shell execution stream.  Because of operating system dependencies,
            this may or may not be a true reflection of how many characters
            can be read.  The calling routine can assume that a positive
            number indicates outstanding input, zero indicates no available
            input, and a negative number indicates an error.

*******************************************************************************/


int  xqtPoll (

#    if PROTOTYPES
        XqtStream  stream)
#    else
        stream)

        XqtStream  stream ;
#    endif

{    /* Local variables. */
#if defined(vms)
    int  status ;
    short  iosb[4], length ;
    long  numMessages ;
    struct  ItemList {		/* Item list structure. */
        short  size ;		/* Size of buffer. */
        short  itemCode ;	/* DVI parameter. */
        long  *buffer ;		/* Buffer to receive information. */
        short  *length ;	/* Returned length of information. */
        int  endOfList ;	/* Indicates end of item list. */
    }  itemList ;
#else
    long  length ;
#endif



    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xqtPoll) NULL shell stream: ") ;
        return (-1) ;
    }

#if defined(vms)

/* Find out how many messages are sitting in the pipe's mailbox. */

    itemList.size = sizeof numMessages ;
    itemList.itemCode = DVI$_DEVDEPEND ;
    itemList.buffer = &numMessages ;
    itemList.length = &length ;
    itemList.endOfList = 0 ;

    status = SYS$GETDVIW (0, stream->channel, NULL, &itemList, iosb,
                          NULL, 0, NULL) ;
    if (status & STS$M_SUCCESS)  status = iosb[0] ;
    if (!(status & STS$M_SUCCESS)) {
        SET_ERRNO (EVMSERR) ;  vaxc$errno = status ;
        LGE "(xqtPoll) Error polling stream %p.\nSYS$GETDVIW: ", stream) ;
        length = -1 ;
    } else {
        length = numMessages & 0x0FFFF ;	/* Ignore high 16 bits. */
    }

#else

/* Find out how many characters of input are waiting to be read from
   the pipe. */

    while (ioctl (stream->outputPipe[0], FIONREAD, &length) == -1) {
        if (errno == EINTR)  continue ;		/* Retry on signal interrupt. */
        LGE "(xqtPoll) Error polling shell %p.\nioctl: ", stream) ;
        length = -1 ;
    }

#endif

    LGI "(xqtPoll) %ld characters from shell %p.\n",
        (long) length, (void *) stream) ;

    return (length) ;

}

/*!*****************************************************************************

Procedure:

    xqtRead ()

    Read a Line of Output from a Shell Execution Stream.


Purpose:

    Function xqtRead() reads the next line of output from a shell subprocess.


    Invocation:

        status = xqtRead (stream, maxLength, buffer) ;

    where:

        <stream>	- I
            is the stream handle returned by xqtOpen().
        <maxLength>	- I
            is the maximum number of characters to read, i.e., the size of
            the buffer (see the next argument).
        <buffer>	- O
            is a buffer to receive the next line of text read from the shell.
            At most maxLength-1 characters will be stored in the buffer,
            followed by a null character.
        <status>	- O
            returns the status of reading the next line of output from the
            shell subprocess, zero if no errors occurred and ERRNO otherwise.
            -1 is returned if end-of-input is detected (or is that end-of-
            output?!).

*******************************************************************************/


int  xqtRead (

#    if PROTOTYPES
        XqtStream  stream,
        int  maxLength,
        char  *buffer)
#    else
        stream, maxLength, buffer)

        XqtStream  stream ;
        int  maxLength ;
        char  *buffer ;
#    endif

{    /* Local variables. */
    int  length ;



    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xqtRead) NULL shell stream: ") ;
        return (errno) ;
    }

/* Input the next line of output from the shell subprocess. */

#if defined(vms)
    length = read (stream->outputPipe[0], buffer, maxLength-1) ;
    if (length <= 0) {
        if (length == 0) {
            LGE "(xqtRead) End-of-input on stream %p.\n", stream) ;
            return (-1) ;
        }
        LGE "(xqtRead) Error reading from stream %p.\nread: ", stream) ;
        return (errno) ;
    }
#else
    if (fgets (buffer, maxLength, stream->output) == NULL) {
        if (feof (stream->output)) {
            LGE "(xqtRead) End-of-input on stream %p.\n", stream) ;
            return (-1) ;
        }
        LGE "(xqtRead) Error reading from stream %p.\nfgets: ", stream) ;
        return (errno) ;
    }
    length = strlen (buffer) ;
#endif

    buffer[length] = '\0' ;
						/* Trim trailing newline. */
    if (buffer[length-1] == '\n')  buffer[length-1] = '\0' ;

    LGI "(xqtRead) Shell %p, %d bytes:\n%s\n",
        (void *) stream, length, buffer) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    xqtWrite ()

    Write a Command to a Shell Execution Stream.


Purpose:

    Function xqtWrite() outputs a command for execution to a shell subprocess.


    Invocation:

        status = xqtWrite (stream, format, arg1, ..., argN) ;

    where:

        <stream>	- I
            is the stream handle returned by xqtOpen().
        <format>	- I
            is a PRINTF(3)-style format specification for the command.  Do
            NOT add a trailing newline character to the command; the need
            for a newline is operating system-dependent and xqtWrite() will
            take care of appending it if needed.  If the FORMAT argument is
            NULL, xqtWrite() sends an end-of-input indication to the shell
            subprocess, thus terminating the subprocess.
        <arg1> ...
        <argN>		- I
            are the arguments expected by the format statement.
        <status>	- O
            returns the status of formatting and writing the command to the
            shell subprocess, zero if no errors occurred and ERRNO otherwise.
            This status does NOT indicate the completion status of the command.

*******************************************************************************/


int  xqtWrite (

#    if PROTOTYPES
        XqtStream  stream,
        const  char  *format,
        ...)
#    else
        stream, format, va_alist)

        XqtStream  stream ;
        char  *format ;
        va_dcl
#    endif

{    /* Local variables. */
    char  command[1024] ;
    va_list  ap ;




    if (stream == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(xqtWrite) NULL shell stream: ") ;
        return (errno) ;
    }


/* If a format string was NOT specified, then notify the shell subprocess
   of end-of-input by closing its input pipe. */

    if (format == NULL) {
        LGI "(xqtWrite) Shell %p: <end-of-input>\n", (void *) stream) ;
        if (close (stream->inputPipe[1])) {
            LGE "(xqtWrite) Error signalling end-of-input to the shell subprocess.\nclose: ") ;
            return (errno) ;
        }
        stream->inputPipe[1] = -1 ;
        return (0) ;
    }


/* Format the command.  (VSPRINTF(3) is used rather than VFPRINTF(3) so that
   debug output can be generated.) */

#if HAVE_STDARG_H
    va_start (ap, format) ;
#else
    va_start (ap) ;
#endif

    if (vsprintf (command, format, ap) == EOF) {
        LGE "(xqtWrite) Error formatting command: \"%s\"\nvsprintf: ",
                 format) ;
        return (errno) ;
    }


/* Output the command to the shell subprocess. */

    LGI "(xqtWrite) Shell %p: %s\n", (void *) stream, command) ;

#if defined(vms)

    if (write (stream->inputPipe[1], command, strlen (command)) < 0) {
        LGE "(xqtWrite) Error writing to stream %p.\nwrite: ", stream) ;
        return (errno) ;
    }

#else

    if (fprintf (stream->input, "%s\n", command) < 0) {
        LGE "(xqtWrite) Error writing to stream %p.\nfprintf: ", stream) ;
        return (errno) ;
    }

    if (fflush (stream->input) == EOF) {
        LGE "(xqtWrite) Error flushing output to stream %p.\nfflush: ",
                 stream) ;
        return (errno) ;
    }

#endif


    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the XQT_UTIL functions.  This test program executes a
    series of commands and writes the output from the commands to the user's
    screen.

    Under VMS,
        compile and link as follows:
            $ CC/DEFINE=TEST xqt_util.c
            $ LINK xqt_util, <libraries>
        and run with the following command line:
            % xqt_util

*******************************************************************************/

main (argc, argv)
    int  argc ;
    char  *argv[] ;
{    /* Local variables. */
    char  buffer[1024] ;
    int  i, status ;
    XqtStream  stream ;

    static  char  *command_list[] = {
#if defined(vms)
        "SHOW DEFAULT",
        "DIRECTORY",
        "SHOW SYSTEM/SUBPROCESS",
        "WRITE SYS$OUTPUT \"$STATUS = \", $STATUS",
#else
        "pwd",
        "ls",
        "ps",
        "echo '$STATUS =' $status",
#endif
        NULL
    } ;



    xqt_util_debug = 1 ;

    if (xqtOpen (NULL, &stream)) {
        LGE "[XQT_TEST] Error opening shell execution stream.\nxqtOpen: ") ;
        exit (errno) ;
    }

    for (i = 0 ;  command_list[i] != NULL ;  i++) {
        printf ("Command: \"%s\"\n", command_list[i]) ;
        if (xqtWrite (stream, "%s", command_list[i])) {
            LGE "[XQT_TEST] Error writing command to shell.\nxqtWrite: ") ;
            exit (errno) ;
        }
    }

    xqtWrite (stream, NULL) ;		/* End-of-input. */

    for ( ; ; ) {
        while (xqtPoll (stream) == 0)
            sleep (1) ;
        if (xqtPoll (stream) < 0)  break ;
        if (xqtRead (stream, sizeof buffer, buffer))  break ;
        if (strncmp (buffer, "$STATUS", 7) == 0)  break ;
    }

    xqtClose (stream) ;

}

#endif  /* TEST */
