/* $Id: log_util.c,v 1.7 2011/07/18 17:38:34 alex Exp $ */
/*******************************************************************************

File:

    log_util.c

    Logging Package.


Author:    Alex Measday


Purpose:

    The LOG_UTIL utilities provide a simple interface for logging ASCII text
    to a file.


Public Procedures:

    logClose() - closes a log file.
    logFlush() - flushes buffered output to a log file.
    logName() - returns the name of a log file.
    logOpen() - opens a log file.
    logWrite() - writes text to a log file.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "log_util.h"			/* Event logging utilities. */


/*******************************************************************************
    Log File - is the log file!
*******************************************************************************/

typedef  struct  _LogFile {
    char  *name ;			/* Full pathname of log file. */
    int  interval ;			/* # of writes between reopenings. */
    FILE  *file ;			/* Open file handle. */
    int  numWrites ;			/* # of writes since last reopen. */
}  _LogFile ;


int  log_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  log_util_debug

/*******************************************************************************

Procedure:

    logClose ()

    Close a Log File.


Purpose:

    Function logClose() closes a log file.


    Invocation:

        status = logClose (log) ;

    where:

        <log>		- I
            is the log file handle returned by logOpen().
        <status>	- O
            returns the status of closing the log file, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


int  logClose (

#    if PROTOTYPES
        LogFile  log)
#    else
        log)

        LogFile  log ;
#    endif

{

    if (log == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(logClose) NULL log handle: ") ;
        return (errno) ;
    }

    LGI "(logClose) Closing: %s\n", (log->name == NULL) ? "<nil>" : log->name) ;

/* Close the log file. */

    if ((log->file != NULL) && fclose (log->file)) {
        LGE "(logClose) Error closing: %s\nfclose: ", log->name) ;
        return (errno) ;
    } ;

/* Deallocate the log file object. */

    if (log->name != NULL)  free (log->name) ;
    free (log) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    logFlush ()

    Flush Buffered Output to a Log File.


Purpose:

    Function logFlush() flushes any buffered output to a log file.


    Invocation:

        status = logFlush (log) ;

    where:

        <log>		- I
            is the log file handle returned by logOpen().
        <status>	- O
            returns the status of flushing the buffered output:
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  logFlush (

#    if PROTOTYPES
        LogFile  log)
#    else
        log)

        LogFile  log ;
#    endif

{

    if (log == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(logFlush) NULL log file handle: ") ;
        return (errno) ;
    }

    if (log->file == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(logFlush) %s is not open: ", log->name) ;
        return (errno) ;
    }

    if (fflush (log->file)) {
        LGE "(logFlush) Error flushing output to %s.\nfflush: ", log->name) ;
        return (errno) ;
    }

    return (0) ;

}

/*******************************************************************************

Procedure:

    logName ()

    Get the Name of a Log File.


Purpose:

    Function logName() returns the fully-qualified pathname of a log file.


    Invocation:

        name = logName (log) ;

    where:

        <log>	- I
            is the log file handle returned by logOpen().
        <name>	- O
            returns the full pathname of the log file; NULL is returned in the
            event of an error.

*******************************************************************************/


const  char  *logName (

#    if PROTOTYPES
        LogFile  log)
#    else
        log)

        LogFile  log ;
#    endif

{
    return ((log == NULL) ? NULL : log->name) ;
}

/*******************************************************************************

Procedure:

    logOpen ()

    Open a Log File.


Purpose:

    Function logOpen() opens a log file for writing.

    The options argument passed into this function is a string containing
    zero or more of the following UNIX command line-style options:

        "-reopen <interval>"
            specifies that the log file is to be closed and reopened after
            <interval> writes.


    Invocation:

        status = logOpen (name, options, &log) ;

    where:

        <name>		- I
            is the name of the log file.
        <options>	- I
            is a string containing zero or more of the UNIX command line-style
            options described above.
        <log>		- O
            returns a handle for the opened log file.
        <status>	- O
            returns the status of opening the log file, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


int  logOpen (

#    if PROTOTYPES
        const  char  *name,
        const  char  *options,
        LogFile  *log)
#    else
        name, options, log)

        char  *name ;
        char  *options ;
        LogFile  *log ;
#    endif

{    /* Local variables. */
    char  *argument, **argv ;
    int  argc, errflg, interval, option ;
    OptContext  context ;

    static  const  char  *optionList[] = {
        "{reopen:}", NULL
    } ;




/*******************************************************************************
    Convert the options string into an ARGC/ARGV array and scan the arguments.
*******************************************************************************/

    interval = -1 ;

    if (options != NULL) {

        opt_create_argv ("logOpen", options, &argc, &argv) ;
        opt_init (argc, argv, NULL, optionList, &context) ;
        opt_errors (context, false) ;

        errflg = 0 ;
        while ((option = opt_get (context, &argument))) {
            switch (option) {
            case 1:			/* "-reopen <interval>" */
                interval = atoi (argument) ;
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
            LGE "(logWrite) Invalid option/argument in %s's options string: \"%s\"\n",
                name, options) ;
            return (errno) ;
        }

    }


/*******************************************************************************
    Open the log file.
*******************************************************************************/

/* Create a log file object. */

    *log = (LogFile) malloc (sizeof (_LogFile)) ;
    if (*log == NULL) {
        LGE "(logOpen) Error allocating log object for %s.\nmalloc: ", name) ;
        return (errno) ;
    }

    (*log)->file = NULL ;
    (*log)->name = strdup (fnmBuild (FnmPath, name, NULL)) ;
    if ((*log)->name == NULL) {
        LGE "(logOpen) Error duplicating file name: %s\nstrdup: ", name) ;
        PUSH_ERRNO ;  logClose (*log) ;  *log = NULL ;  POP_ERRNO ;
        return (errno) ;
    }
    (*log)->interval = interval ;
    (*log)->numWrites = 0 ;

/* Open the log file. */

    (*log)->file = fopen ((*log)->name, "a") ;
    if ((*log)->file == NULL) {
        LGE "(logOpen) Error opening: %s\nfopen: ", (*log)->name) ;
        PUSH_ERRNO ;  logClose (*log) ;  *log = NULL ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(logOpen) Opened: %s\n", (name == NULL) ? "<nil>" : name) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    logWrite ()

    Write a Message to a Log File.


Purpose:

    Function logWrite() formats and writes a message to a log file.


    Invocation:

        status = logWrite (log, format, ...) ;

    where:

        <log>		- I
            is the log file handle returned by logOpen().
        <format>	- I
        <...>		- I
            is a PRINTF(3)-style format string and list of arguments
            used to format the output text.
        <status>	- O
            returns the status of writing the text to the log file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  logWrite (

#    if PROTOTYPES
        LogFile  log,
        const  char  *format,
        ...)
#    else
        log, format, va_alist)

        LogFile  log ;
        char  *format ;
        va_dcl
#    endif

{    /* Local variables. */
    int  numItems ;
    va_list  ap ;



/* If the specified number of writes since the last reopen have been performed,
   then close and reopen the file. */

    if ((log != NULL) && (log->file != NULL) && (log->interval > 0) &&
        (log->numWrites >= log->interval)) {
        LGI "(logWrite) Reopening: %s\n", log->name) ;
        if (fclose (log->file)) {
            LGE "(logWrite) Error closing: %s\nfclose: ", log->name) ;
        }
        log->file = fopen (log->name, "a") ;
        if (log->file == NULL) {
            LGE "(logWrite) Error opening: %s\nfopen: ", log->name) ;
        }
        log->numWrites = 0 ;
    }

/* Format and output the text. */

#if HAVE_STDARG_H
    va_start (ap, format) ;
#else
    va_start (ap) ;
#endif
    if ((log == NULL) || (log->file == NULL)) {
        numItems = vfprintf (stdout, format, ap) ;  fflush (stdout) ;
    } else {
        numItems = vfprintf (log->file, format, ap) ;
    }
    va_end (ap) ;

    if (numItems < 0) {
        LGE "(logWrite) Error writing to %s.\nvfprintf: ", log->name) ;
        return (errno) ;
    }

    log->numWrites++ ;

    return (0) ;

}
