/* $Id: sem_util.c,v 1.14 2009/09/09 17:13:10 alex Exp $ */
/*******************************************************************************

File:

    sem_util.c

    Semaphore Utilities.


Author:    Alex Measday


Purpose:

    The SEM utilities provide a high-level interface to the underlying
    operating system's semaphore facility.

    Creating a semaphore (or accessing an existing semaphore) is as
    simple as this:

        #include  "sem_util.h"
        Semaphore  semaphore ;
        ...
        sem_create ("my_semaphore", N, &semaphore) ;
        ...

    where N is initial number of resources being guarded by the semaphore.
    For example, a mutex semaphore guarding a critical region would have
    an initial value of 1.  A semaphore guarding a pool of 5 buffers would
    have an initial value of 5.

    A process gains access to a guarded resource by "taking" its semaphore;
    the process is suspended until access to the resource is granted.  When
    the process is finished with the resource, it "gives" the resource back
    to the semaphore, thereby allowing other processes to access the resource:

        if (!sem_take (semaphore, -1.0)) {	-- Wait for access.
            ... access guarded resource ...
            sem_give (semaphore) ;		-- Relinquish access.
        } else {
            ... error ...
        }

    A process can limit the amount of time it will wait for access to a
    resource by specifying a timeout in the SEM_TAKE() call:

        if (!sem_take (semaphore, 5.0)) {	-- Wait 5 seconds for access.
            ... access guarded resource ...
            sem_give (semaphore) ;		-- Relinquish access.
        } else if (errno == EWOULDBLOCK) {
            ... timeout ...
        } else {
            ... error ...
        }

    When a semaphore is no longer needed by a process, it should be deleted:

        sem_delete (semaphore) ;

    The semaphore isn't actually deleted from the system until the last
    process using it deletes it.


Notes (UNIX):

    The UNIX semaphore functions, SEMGET(2) et al, are used to create and
    access semaphores.  The name/IPC identifier mappings and reference
    counts are stored in the named object database (see NOB_UTIL.C).

    Timeouts in the SEM_TAKE() call are not supported under UNIX.  I used
    to have a semaphore utility that polled its semaphore once a second,
    but, without getting in line, a polling process could conceivably
    never get the semaphore.

    Processes should delete all semaphores before exiting; if a process
    exits prematurely, the named object database could be left in an
    inconsistent state.


Notes (VxWorks):

    The SEM semaphores are counting semaphores (see "semCLib(1)"); under
    VxMP, the semaphore are visible to other CPUs (see "semSmLib(1)").
    The name/identifier mappings and reference counts are stored in the
    named object database (see NOB_UTIL.C).

    Timeouts in the SEM_TAKE() call are supported.  Note that the timeout
    error code returned by the VxWorks system call is converted into an
    EWOULDBLOCK error code.

    Processes should delete all semaphores before exiting; if a process
    exits prematurely, the named object database could be left in an
    inconsistent state.


Procedures:

    SEM_CREATE - creates a semaphore.
    SEM_DELETE - deletes a semaphore.
    SEM_GIVE - releases a semaphore.
    SEM_ID - returns the IPC identifier for a semaphore.
    SEM_TAKE - waits for and holds a semaphore.
    SEM_VALUE - returns the value of a semaphore.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#if defined(VMS)
    /* ... */
#elif defined(VXWORKS)
#    include  <semLib.h>		/* Semaphore definitions. */
#    ifdef VxMP
#        include  <semSmLib.h>		/* Shared semaphore definitions. */
#    endif
#    include  <sysLib.h>		/* System library definitions. */
#else
#    include  <sys/types.h>		/* System type definitions. */
#    include  <sys/ipc.h>		/* Inter-process communication definitions. */
#    include  <sys/sem.h>		/* Semaphore definitions. */
#endif
#include  "nob_util.h"			/* Named object definitions. */
#include  "sem_util.h"			/* Semaphore utility definitions. */


/*******************************************************************************
    Semaphore -
*******************************************************************************/

typedef  struct  _Semaphore {
    NamedObject  object ;		/* Handle of semaphore's named object. */
#ifdef VXWORKS
    SEM_ID  id ;			/* System ID for the semaphore. */
#else
    int  id ;				/* System IPC ID for the semaphore. */
#endif
}  _Semaphore ;


int  sem_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  sem_util_debug

/*!*****************************************************************************

Procedure:

    sem_create ()

    Create a Semaphore.


Purpose:

    Function SEM_CREATE creates a new semaphore or, if the semaphore
    already exists, establishes access to the semaphore.


    Invocation:

        status = sem_create (name, initial_value, &semaphore) ;

    where:

        <name>		- I
            is the name of the semaphore.
        <initial_value>	- I
            specifies the initial count (e.g., the number of resources
            being guarded) of a new semaphore.  If the semaphore already
            exists, this argument is ignored.
        <semaphore>	- O
            returns a handle for the semaphore that is to be used in
            calls to the other SEM_UTIL functions.
        <status>	- O
            returns the status of creating or establishing access to the
            semaphore: zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  sem_create (

#    if PROTOTYPES
        const  char  *name,
        int  initial_value,
        Semaphore  *semaphore)
#    else
        name, initial_value, semaphore)

        const  char  *name ;
        int  initial_value ;
        Semaphore  *semaphore ;
#    endif

{    /* Local variables. */
    NamedObject  object ;
#ifdef VXWORKS
    SEM_ID  id ;
#else
    int  id ;
    struct  sembuf  V_op ;
#endif





/*******************************************************************************
    Create a named object for the semaphore.
*******************************************************************************/

    if (!nobCreate (name, multiCPU, &object)) {		/* Brand new? */

/* Create a new semaphore and set its initial value. */

#if defined(VxMP)
        id = semCSmCreate (SEM_Q_FIFO, initial_value) ;
        if (id == NULL) {
            LGE "(sem_create) Error creating %s semaphore.\nsemCSmCreate: ",
                name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
#elif defined(VXWORKS)
        id = semCCreate (SEM_Q_FIFO, initial_value) ;
        if (id == NULL) {
            LGE "(sem_create) Error creating %s semaphore.\nsemCCreate: ",
                name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
#else
        id = semget (IPC_PRIVATE, 1, (IPC_CREAT | 0660)) ;
        if (id < 0) {
            LGE "(sem_create) Error creating %s semaphore.\nsemget: ", name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
        V_op.sem_num = 0 ;  V_op.sem_op = initial_value ;  V_op.sem_flg = 0 ;
        if (semop (id, &V_op, 1)) {
            LGE "(sem_create) Error initializing %s semaphore.\nsemop: ",
                name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
#endif

/* Add the name/ID mapping for the semaphore to the named object database. */

        if (nobCommit (object, (void *) id)) {
            LGE "(sem_create) Error commiting named object for %s.\nnobCommit: ",
                name) ;
            return (errno) ;
        }

    }

/* If the named object already exists, its value is the ID of the existing
   semaphore. */

    else if (errno == EEXIST) {

#ifdef VXWORKS
        id = (SEM_ID) nobValue (object) ;
#else
        id = (int) nobValue (object) ;
#endif

    } else {

        LGE "(sem_create) Error creating named object for %s.\nnobCreate: ",
            name) ;
        return (errno) ;

    }


/*******************************************************************************
    Create a SEM object for the semaphore.
*******************************************************************************/

    *semaphore = (Semaphore) malloc (sizeof (_Semaphore)) ;
    if (*semaphore == NULL) {
        LGE "(sem_create) Error creating semaphore object for %s.\nmalloc: ",
            name) ;
        return (errno) ;
    }

    (*semaphore)->object = object ;
    (*semaphore)->id = id ;


#ifdef VXWORKS
    LGI "(sem_create)  Semaphore: %s  ID: %p\n", name, id) ;
#else
    LGI "(sem_create)  Semaphore: %s  ID: %d\n", name, id) ;
#endif

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sem_delete ()

    Delete a Semaphore.


Purpose:

    Function SEM_DELETE terminates a process's use of a semaphore and, if
    no more processes are using the semaphore, deletes the semaphore from
    the system.


    Invocation:

        status = sem_delete (semaphore) ;

    where:

        <semaphore>	- I
            is the semaphore handle returned by SEM_CREATE().
        <status>	- O
            returns the status of deleting the semaphore: zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


int  sem_delete (

#    if PROTOTYPES
        Semaphore  semaphore)
#    else
        semaphore)

        Semaphore  semaphore ;
#    endif

{

    if (semaphore == NULL)  return (0) ;

    LGI "(sem_delete) Deleting %s semaphore (%d).\n",
        nobName (semaphore->object), nobCount (semaphore->object)) ;

/* Delete the semaphore's named object. */

    if (!nobDestroy (semaphore->object)) {	/* Last process using it? */

#if defined(VXWORKS)
        if (semDelete (semaphore->id) == ERROR) {
            LGE "(sem_delete) Error deleting semaphore %p.\nsemDelete: ",
                semaphore->id) ;
#else
        if (semctl (semaphore->id, 0, IPC_RMID)) {
            LGE "(sem_delete) Error deleting semaphore %d.\nsemctl: ",
                semaphore->id) ;
#endif
            return (errno) ;
        }

    } else if (errno != EWOULDBLOCK) {

        LGE "(sem_delete) Error deleting named object.\nnobDestroy: ") ;
        return (errno) ;

    }

/* Delete the semaphore's SEM object. */

    free (semaphore) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sem_give ()

    Release a Semaphore.


Purpose:

    Function SEM_GIVE increments a semaphore's value (Dijkstra's V operation)
    by 1.  The next task waiting on the semaphore, via a SEM_TAKE() call, is
    enabled to resume execution.


    Invocation:

        status = sem_give (semaphore) ;

    where:

        <semaphore>	- I
            is the semaphore handle returned by SEM_CREATE().
        <status>	- O
            returns the status of signalling the semaphore: zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/


int  sem_give (

#    if PROTOTYPES
        Semaphore  semaphore)
#    else
        semaphore)

        Semaphore  semaphore ;
#    endif

{    /* Local variables. */
#ifndef VXWORKS
    struct  sembuf  V_op ;
#endif



    if (semaphore == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(sem_give) NULL semaphore handle: ") ;
        return (errno) ;
    }

    LGI "(sem_give) Releasing the %s semaphore.\n",
        nobName (semaphore->object)) ;

/* Increment the semaphore's value by 1. */

#ifdef VXWORKS
    if (semGive (semaphore->id) == ERROR) {
        LGE "(sem_give) Error signalling %s semaphore.\nsemGive: ",
            nobName (semaphore->object)) ;
#else
    V_op.sem_num = 0 ;  V_op.sem_op = 1 ;  V_op.sem_flg = 0 ;
    if (semop (semaphore->id, &V_op, 1)) {
        LGE "(sem_give) Error signalling %s semaphore.\nsemop: ",
            nobName (semaphore->object)) ;
#endif
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sem_id ()

    Return the IPC Identifier for a Semaphore.


Purpose:

    Function SEM_ID returns the system IPC identifier for a semaphore.


    Invocation:

        identifier = sem_id (semaphore) ;

    where:

        <semaphore>	- I
            is the semaphore handle returned by SEM_CREATE().
        <identifier>	- O
            returns the system IPC identifier for the semaphore; -1 is
            returned if the operating system doesn't use IPC IDs.

*******************************************************************************/


int  sem_id (

#    if PROTOTYPES
        Semaphore  semaphore)
#    else
        semaphore)

        Semaphore  semaphore ;
#    endif

{
#ifdef VXWORKS
    return ((semaphore == NULL) ? -1 : (int) semaphore->id) ;
#else
    return ((semaphore == NULL) ? -1 : semaphore->id) ;
#endif
}

/*!*****************************************************************************

Procedure:

    sem_take ()

    Wait For and Hold a Semaphore.


Purpose:

    Function SEM_TAKE tries to decrement a semaphore's value (Dijkstra's
    P operation) by 1.  If the value can't be decremented, the process
    is suspended until the value can be decremented; i.e., until another
    process does a SEM_GIVE() on the semaphore.


    Invocation:

        status = sem_take (semaphore, timeout) ;

    where:

        <semaphore>	- I
            is the semaphore handle returned by SEM_CREATE().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the possession of the semaphore.
            A fractional time can be specified; e.g., 2.5 seconds.
            A negative timeout (e.g., -1.0) causes an infinite wait;
            a zero timeout (0.0) causes an immediate return if the
            semaphore is held by another process.
        <status>	- O
            returns the status of waiting for the semaphore: zero if no
            errors occurred, EWOULDBLOCK if the timeout interval expires
            without the semaphore being obtained, and ERRNO otherwise.

*******************************************************************************/


int  sem_take (

#    if PROTOTYPES
        Semaphore  semaphore,
        double  timeout)
#    else
        semaphore, timeout)

        Semaphore  semaphore ;
        double  timeout ;
#    endif

{    /* Local variables. */
#ifdef VXWORKS
    int  ticks_to_wait ;
#else
    struct  sembuf  P_op ;
#endif



    if (semaphore == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(sem_take) NULL semaphore handle: ") ;
        return (errno) ;
    }

    LGI "(sem_take) Waiting on the %s semaphore.\n",
        nobName (semaphore->object)) ;

/* Wait for possession of the semaphore and decrement its value by 1. */

#ifdef VXWORKS
    if (timeout < 0.0)
        ticks_to_wait = WAIT_FOREVER ;
    else if (timeout == 0.0)
        ticks_to_wait = NO_WAIT ;
    else
        ticks_to_wait = timeout * sysClkRateGet () ;

    if (semTake (semaphore->id, ticks_to_wait) == ERROR) {
        if (errno == S_objLib_OBJ_TIMEOUT)  SET_ERRNO (EWOULDBLOCK) ;
        LGE "(sem_take) Error waiting on %s semaphore.\nsemTake: ",
            nobName (semaphore->object)) ;
#else
    P_op.sem_num = 0 ;  P_op.sem_op = -1 ;  P_op.sem_flg = 0 ;
    if (semop (semaphore->id, &P_op, 1)) {
        LGE "(sem_take) Error waiting on %s semaphore.\nsemop: ",
            nobName (semaphore->object)) ;
#endif
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    sem_value ()

    Return the Current Value of a Semaphore.


Purpose:

    Function SEM_VALUE returns the current value of a semaphore.  The
    availability of this value is operating system-dependent.


    Invocation:

        value = sem_value (semaphore) ;

    where:

        <semaphore>	- I
            is the semaphore handle returned by SEM_CREATE().
        <value>		- O
            returns the current value of the semaphore.  -1 is returned
            in the event of an error or if the operating system does not
            allow you examine the value of the semaphore.

*******************************************************************************/


int  sem_value (

#    if PROTOTYPES
        Semaphore  semaphore)
#    else
        semaphore)

        Semaphore  semaphore ;
#    endif

{
#ifdef VXWORKS
    return (-1) ;
#else
    return ((semaphore == NULL) ? -1 : semctl (semaphore->id, 0, GETVAL)) ;
#endif
}

#ifdef  TEST

/*******************************************************************************

    Program to test the SEM_UTIL functions.

    Under UNIX:
        compile and link as follows:
            % cc -DTEST sem_util.c <libraries> -o sem_test
        and run with the following command line:
            % sem_test <semaphore_name>

*******************************************************************************/

#ifdef VXWORKS

    void  sem_test (
        char  *command_line)

#else

    main (argc, argv)
        int  argc ;
        char  *argv[] ;

#endif

{    /* Local variables. */
    int  i ;
    Semaphore  semaphore1, semaphore2 ;



#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("sem_util", command_line, &argc, &argv) ;
#endif

    nob_util_debug = 1 ;
    sem_util_debug = 1 ;

    if (argc < 2) {
        fprintf (stderr, "Usage:  sem_test  <semaphore_name>\n") ;
        exit (EINVAL) ;
    }

    if (sem_create (argv[1], 0, &semaphore1)) {
        LGE "[SEM_TEST] Error creating %s semaphore.\nsem_create: ", argv[1]) ;
        exit (errno) ;
    }

    if (sem_create (argv[1], 0, &semaphore2)) {
        LGE "[SEM_TEST] Error creating existing %s semaphore.\nsem_create: ",
            argv[1]) ;
        exit (errno) ;
    }

    printf ("Semaphore (%d).\n", sem_id (semaphore1)) ;
    printf ("Semaphore (%d).\n", sem_id (semaphore2)) ;

    if (argc > 2)
        sem_give (semaphore1) ;
    else {
        sem_take (semaphore1, 30.0) ;
        for (i = 0 ;  i < 5 ;  i++) {
            printf ("Value of semaphore is %d\n", sem_value (semaphore1)) ;
            sem_give (semaphore1) ;
        }
        printf ("Value of semaphore is %d\n", sem_value (semaphore1)) ;
    }

    sem_delete (semaphore1) ;
    sem_delete (semaphore2) ;

}

#endif  /* TEST */
