/* $Id: dae_util.c,v 1.8 2004/04/23 21:47:28 alex Exp $ */
/*******************************************************************************

File:

    dae_util.c

    Daemon Utilities.


Author:    Alex Measday


Purpose:

    The DAE_UTIL functions are used to write daemons.


Public Procedures:

    daeMonize() - makes a process a daemon.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#ifndef _NFILE
#    define  _NFILE  3
#endif
#include  <stdlib.h>			/* Standard C Library definitions. */
#if HAVE_WORKING_FORK
#    include  <signal.h>		/* System signal definitions. */
#    ifndef SIGCHLD
#        define  SIGCHLD  SIGCLD
#    endif
#    include  <unistd.h>		/* UNIX-specific definitions. */
#    include  <sys/stat.h>		/* File status definitions. */
#endif
#include  "dae_util.h"			/* Daemon utilities. */

/*******************************************************************************

Procedure:

    daeMonize ()

    Make a Process a Daemon.


Purpose:

    Function daeMonize() makes the current process a daemon.


    Invocation:

        status = daeMonize (numFDs) ;

    where

        <numFDs>	- I
            is the number of file descriptors, beginning with 0, to close.
        <status>	- O
            returns the status of making this process a daemon, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  daeMonize (

#    if PROTOTYPES
        int  numFDs)
#    else
        numFDs)

        int  numFDs ;
#    endif

{    /* Local variables. */
#if !HAVE_WORKING_FORK
    SET_ERRNO (EINVAL) ;
    LGE "(daeMonize) Operating system does not support daemons.\n") ;
    return (errno) ;
#else
    pid_t  childPID ;




/* Don't fall to pieces if the daemon attempts to read from or write to
   its controlling terminal. */

#ifdef SIGTTIN
    signal (SIGTTIN, SIG_IGN) ;
#endif
#ifdef SIGTTOU
    signal (SIGTTOU, SIG_IGN) ;
#endif
#ifdef SIGTSTP
    signal (SIGTSTP, SIG_IGN) ;
#endif

/* Put the daemon in the background by forking a child process and exiting
   the parent (i.e., the current process). */

    childPID = fork () ;
    if (childPID < 0) {
        LGE "(daeMonize) Error forking background process.\nfork: ") ;
        return (errno) ;
    }
    if (childPID > 0)  exit (0) ;	/* Terminate the parent process. */

/* Disassociate the daemon from its controlling terminal. */

    if (setsid () == -1) {
        LGE "(daeMonize) Error disassociating from the terminal.\nsetsid: ") ;
        return (errno) ;
    }

/* Close any open files. */

    if ((numFDs < 0) || (numFDs > _NFILE))  numFDs = _NFILE ;
    while (numFDs > 0)
        close (--numFDs) ;

/* Change the current directory to root to avoid preventing a mounted file
   system from being unmounted. */

    if (chdir ("/")) {
        LGE "(daeMonize) Error changing directory to the root file system.\nchdir: ") ;
        return (errno) ;
    }

/* Clear the inherited access mask for new files. */

    umask (0) ;

/* Ignore exited children (zombies). */

    signal (SIGCHLD, SIG_IGN) ;

    return (0) ;

#endif
}

#ifdef  TEST

/*******************************************************************************

    Program to test daeMonize().  Compile as follows:

        % cc -g -DTEST daeMonize.c -I<... includes ...>

    Invocation:

        % a.out

*******************************************************************************/

int  main (argc, argv)
    int  argc ;
    char  *argv[] ;
{

    aperror_print = 1 ;

    printf ("Parent PID: %lu\n", getpid ()) ;
    daeMonize (-1) ;
    printf ("Daemon PID: %lu\n", getpid ()) ;
    for ( ; ; ) {
        sleep (5) ;
    }

}
#endif  /* TEST */
