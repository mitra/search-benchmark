/* $Id: nob_util.c,v 1.19 2011/07/18 17:47:24 alex Exp $ */
/*******************************************************************************

File:

    nob_util.c

    Named Object Utilities (UNIX Version).


Author:    Alex Measday


Purpose:

    The Named Object (NOB) utilities provide a general means of assigning
    names to arbitrary objects so that other tasks can access the objects
    by name.

    The NOB utilities are intended for, but not limited to, use in library
    functions that both create new objects and access existing objects.
    An application can call such a library function without caring if the
    target object exists or not - the object will be created automatically
    if need be.  For example, message queues are known by their ID under
    UNIX.  A library function that creates new named message queues or
    accesses existing ones would use the NOB utilities as follows:

        #include  "nob_util.h"			-- Named object definitions.
        int  queue ;
        NamedObject  qobj ;

        ...

        if (!nobCreate ("MY_MSGQ", singleCPU, &qobj)) {
            queue = msgget (IPC_PRIVATE,		-- Brand new?
                            (IPC_CREAT | 0620)) ;
            nobCommit (qobj, (void *) queue) ;
        } else if (errno == EEXIST) {			-- Already exists?
            queue = (int) nobValue (qobj) ;
        } else {					-- Error?
            ... error ...
        }

    If the named queue already exists, the queue ID is retrieved from the
    object by calling nobValue().  If the queue doesn't exist, the queue is
    created and its ID is then stored in the object by a call to nobCommit().
    The creation of a new object can be aborted in the event of an error by
    calling nobAbort() instead of nobCommit().

    Processes that know an object exists or that depend upon the object
    existing can call nobExists() to lookup an object's value:

        qobj = nobExists ("MY_MSGQ", singleCPU) ;
        if (qobj == NULL) {			-- Doesn't exist or error?
            ... error ...
        } else {				-- Exists?
            queue = (int) nobValue (qobj) ;
        }

    Deleting the named message queue is done as follows:

        if (!nobDestroy (qobj)) {		-- Last user of queue?
            msgctl (queue, IPC_RMID, NULL) ;
        } else if (errno != EWOULDBLOCK) {	-- Error?
            ... error ...
        }

    Note that the last task using the message queue is the one that actually
    deletes the queue.


Implementation Notes (UNIX):

    Under UNIX, the named object database is implemented using the NDBM(3)
    database facility.  The base NDBM(3) pathname for the named object
    database files defaults to "/tmp/nob_database"; the user can specify
    a different pathname in environment variable, "NOB_DATABASE".

    If the directory in which the database files are stored is NFS mounted
    on multiple machines, the database is visible on each of those machines.
    The multi-system LOCKF(3) facility - LOCKF(2) depending on your OS - is
    used to prevent simultaneous updates to the database.

    Since objects such as message queues and semaphores are only accessible
    from the CPU on which they are created, be wary of storing their "values"
    (IPC IDs) in an NFS-mounted, multi-CPU NOB database.

    NDBM(3) caches retrieved records and updates in memory on a per-process
    basis.  To make sure every process sees the same database, the cached
    images must be synchronized with the disk image before each database
    fetch and after each database store.  The synchronization is performed
    by closing and reopening the database - a brute force approach, but no
    other solution has presented itself.

    To prevent alignment errors (which occurred in my first version), the
    unaligned records returned by DBM_FETCH() are copied into local, aligned
    record storage.


Public Procedures:

    nobCommit() - finalizes the creation of a named object.
    nobCount() - returns the number of tasks referencing a named object.
    nobCreate() - creates a named object.
    nobDestroy() - deletes a named object.
    nobExists() - looks up a named object.
    nobName() - returns a named object's name.
    nobValue() - returns a named object's value.

Private Procedures:

    nobInitialize() - initializes the NOB package.
    nobSynchronize() - synchronizes the cached database with the disk database.
    nobTerminate() - terminates the NOB package.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <errno.h>			/* System error definitions. */
#include  <fcntl.h>			/* File control definitions. */
#include  <limits.h>			/* Maximum/minimum value definitions. */
#ifndef PATH_MAX
#    ifdef _WIN32
#        include  <windows.h>           /* Windows definitions. */
#        define  PATH_MAX  MAX_PATH
#    else
#        include  <sys/param.h>         /* System parameters. */
#        define  PATH_MAX  MAXPATHLEN
#    endif
#endif
#include  <ndbm.h>			/* NDBM(3) database definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <unistd.h>			/* LOCKF(3) definitions. */
#if defined(HAVE_FLOCK) && HAVE_FLOCK
#    include  <sys/file.h>		/* FLOCK(2) definitions. */
#    define  LOCK(fd)  flock (fd, LOCK_EX)
#    define  UNLOCK(fd)  flock (fd, LOCK_UN)
#else
#    include  <unistd.h>		/* LOCKF(3) definitions. */
#    define  LOCK(fd)  lockf (fd, F_LOCK, 0)
#    define  UNLOCK(fd)  lockf (fd, F_ULOCK, 0)
#endif
#if NO_ATEXIT && !__STDC__ && defined(sun)
#   define  atexit(f)  on_exit (f, (char *) NULL)
#endif
#include  "fnm_util.h"			/* Filename utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "nob_util.h"			/* Named object definitions. */


/*******************************************************************************
    Named Object - contains an object's name, its value, and, in the database,
        a reference count.
*******************************************************************************/

typedef  struct  _NamedObject {
    char  *name ;			/* Object's name. */
    void  *value ;			/* Object's value. */
    int  mutex ;			/* File descriptor for NOB lock. */
}  _NamedObject ;

					/* NDBM(3) record. */
typedef  struct  NamedObjectRecord {
    void  *value ;			/* Object's value. */
    int  references ;			/* Number of references to object. */
}  NamedObjectRecord ;


static  DBM  *nob_database = NULL ;	/* NDBM named object database. */
static  int  nob_mutex = -1 ;		/* File descriptor for NOB lock. */

int  nob_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  nob_util_debug


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  int  nobInitialize (
#    if PROTOTYPES
        NamedObjectScope scope,
        bool createLock
#    endif
    ) ;

static  int  nobSynchronize (
#    if PROTOTYPES && !defined(__cplusplus)
        void
#    endif
    ) ;

static  void  nobTerminate (
#    if PROTOTYPES && !defined(__cplusplus)
        void
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    nobAbort ()

    Abort the Creation of a Named Object.


Purpose:

    Function nobAbort() completes the creation of a new object.
    The partially created object is deleted and exclusive access
    to the named object database is released.  Like nobCommit(),
    nobAbort() is called after nobCreate() indicates than an
    object is new:

        if (!nobCreate (name, singleCPU, &object)) {
            ... perform object-specific initialization ...
            if (... error ...)
                nobAbort (object) ;
            else
                nobCommit (object, value) ;
        }


    Invocation:

        status = nobAbort (object) ;

    where:

        <object>	- I
            is the object handle returned by nobCreate().
        <status>	- O
            returns the status of aborting the creation of the object,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  nobAbort (

#    if PROTOTYPES
        NamedObject  object)
#    else
        object)

        NamedObject  object ;
#    endif

{    /* Local variables. */
    int  status ;



    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobAbort) NULL object handle: ") ;
        return (errno) ;
    }

    LGI "(nobAbort) Aborting creation of \"%s\".\n", nobName (object)) ;

/* Delete the object. */

    object->mutex = -1 ;		/* Tell nobDestroy() not to lock. */

    status = nobDestroy (object) ;
    if (status) {
        LGE "(nobAbort) Error deleting object.\nnobDestroy: ") ;
    }

/* Release exclusive access to the named object database. */

    UNLOCK (nob_mutex) ;

    SET_ERRNO (status) ;

    return (status) ;

}

/*!*****************************************************************************

Procedure:

    nobCommit ()

    Complete the Creation of a Named Object.


Purpose:

    Function nobCommit() completes the creation of a new object and
    makes it available to other tasks.  This involves storing the
    caller-specified value in the object, initializing the object's
    reference count to one, and releasing exclusive access to the
    named object database.  nobCommit() must be called after
    nobCreate() indicates that an object is new:

        if (!nobCreate (name, singleCPU, &object)) {
            ... perform object-specific initialization ...
            nobCommit (object, value) ;
        }

    This two-step process allows the creator of an object to perform
    any object-specific initialization after the object is created,
    but before it is made known to the rest of the world.


    Invocation:

        status = nobCommit (object, value) ;

    where:

        <object>	- I
            is the object handle returned by nobCreate().
        <value>		- I
            specifies the value of the object, cast as a (VOID *) pointer.
        <status>	- O
            returns the status of completing the creation of the object,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  nobCommit (

#    if PROTOTYPES
        NamedObject  object,
        void  *value)
#    else
        object, value)

        NamedObject  object ;
        void  *value ;
#    endif

{    /* Local variables. */
    datum  content, key ;
    NamedObjectRecord  record ;



    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobCommit) NULL object handle: ") ;
        return (errno) ;
    }

/* Store the object's value and reference count (initialized to 1) in the
   named object database. */

    object->value = value ;
    record.value = object->value ;
    record.references = 1 ;

    key.dptr = object->name ;
    key.dsize = strlen (object->name) + 1 ;
    content.dptr = (char *) &record ;
    content.dsize = sizeof record ;

    if (dbm_store (nob_database, key, content, DBM_REPLACE)) {
        LGE "(nobCommit) Error updating the value of %s.\ndbm_store: ",
            object->name) ;
        PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        return (errno) ;
    }

    if (nobSynchronize ()) {
        LGE "(nobCommit) Error synchronizing the named object database.\nnobSynchronize: ") ;
        PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Release exclusive access to the named object database. */

    UNLOCK (nob_mutex) ;

    LGI "(nobCommit) Completed creation of \"%s\".\n", nobName (object)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nobCount ()

    Get the Number of Tasks Using a Named Object.


Purpose:

    Function nobCount() returns a count of the number of references to a
    named object.  Every call to nobCreate() for this object's name, by
    this or any other task, increments the object's reference count; each
    nobDestroy() of this object decrements the reference count.


    Invocation:

        num_references = nobCount (object) ;

    where:

        <object>		- I
            is the object handle returned by nobCreate() or nobExists().
        <num_references>	- O
            returns the current number of references to this object.

*******************************************************************************/


int  nobCount (

#    if PROTOTYPES
        NamedObject  object)
#    else
        object)

        NamedObject  object ;
#    endif

{    /* Local variables. */
    datum  key, value ;
    NamedObjectRecord  record ;



    if (object == NULL)  return (-1) ;

/* Fetch the object from the database. */

    if (nobSynchronize ()) {
        LGE "(nobCount) Error synchronizing the named object database.\nnobSynchronize: ") ;
        return (-1) ;
    }

    key.dptr = object->name ;
    key.dsize = strlen (object->name) + 1 ;
    value = dbm_fetch (nob_database, key) ;
    if (value.dptr == NULL) {
        LGE "(nobCount) Error retrieving %s from the named object database.\ndbm_fetch: ",
            object->name) ;
        return (-1) ;
    }

/* Return the reference count to the caller.  (The database record is copied
   into a local structure to prevent alignment errors.) */

    memcpy (&record, value.dptr, sizeof record) ;

    return (record.references) ;

}

/*!*****************************************************************************

Procedure:

    nobCreate ()

    Create a Named Object.


Purpose:

    Function nobCreate() creates a named object if it doesn't not already
    exist.  A handle for the new or existing object is returned to the
    caller.

      NOTE: The status value returned by nobCreate() indicates the
            age of the object: 0 if the object is new and EEXIST if
            the object already existed.  If the object is new, the
            caller MUST call nobAbort() or nobCommit() in a timely
            fashion in order to finalize the creation of the object.


    Invocation:

        status = nobCreate (name, scope, &object) ;

    where:

        <name>		- I
            is the name of the object being created.
        <scope>		- I
            specifies the scope of the object: "singleCPU" or "multiCPU"
            (enumerated values defined in the "nob_util.h" header file).
            This argument is ignored; the named object database is visible
            to any machine that NFS mounts the directory containing the
            database files (see the package prolog).
        <object>	- O
            returns a handle for the object.  This handle is used in calls
            to the other NOB functions.
        <status>	- O
            returns the status of creating the object, 0 if a new object
            was successfully created, EEXIST if the object already exists,
            and ERRNO otherwise.

*******************************************************************************/


int  nobCreate (

#    if PROTOTYPES
        const  char  *name,
        NamedObjectScope  scope,
        NamedObject  *object)
#    else
        name, scope, object)

        char  *name ;
        NamedObjectScope  scope ;
        NamedObject  *object ;
#    endif

{    /* Local variables. */
    datum  key, value ;
    NamedObjectRecord  record ;




/*******************************************************************************
    If this is the first access to the named object database, then initialize
    the NOB package.
*******************************************************************************/

    if ((nob_database == NULL) && nobInitialize (scope, true)) {
        LGE "(nobCreate) Error initializing the named object package for %s.\nnobInitialize: ",
            name) ;
        return (errno) ;
    }


/*******************************************************************************
    Construct a local copy of the object.
*******************************************************************************/

    *object = (NamedObject) malloc (sizeof (_NamedObject)) ;
    if (*object == NULL) {
        LGE "(nobCreate) Error allocating object structure for %s object.\nmalloc: ",
            name) ;
        return (errno) ;
    }

    (*object)->name = strdup (name) ;
    if ((*object)->name == NULL) {
        LGE "(nobCreate) Error duplicating name of %s object.\nstrdup: ",
            name) ;
        return (errno) ;
    }

    (*object)->value = NULL ;
    (*object)->mutex = nob_mutex ;


/*******************************************************************************
    Wait for exclusive access to the named object database.
*******************************************************************************/

    if (LOCK (nob_mutex)) {
        LGE "(nobCreate) Error locking the named object database for %s.\nlockf: ",
            name) ;
        return (errno) ;
    }

/*******************************************************************************
    Lookup the object's name in the named object database.  If the name is
    found (i.e., the object exists), then store the object's value in the
    local copy of the object, increment and update the reference count in
    the database, and return the object's handle to the caller.
*******************************************************************************/

    if (nobSynchronize ()) {
        LGE "(nobCreate) Error synchronizing the named object database.\nnobSynchronize: ") ;
        PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        return (errno) ;
    }

    key.dptr = (char *) name ;
    key.dsize = strlen (name) + 1 ;

    value = dbm_fetch (nob_database, key) ;

    if (value.dptr != NULL) {
						/* To prevent alignment errors. */
        memcpy (&record, value.dptr, sizeof record) ;
        (*object)->value = record.value ;
						/* Increment reference count. */
        record.references++ ;
						/* Update count in database. */
        value.dptr = (char *) &record ;
        value.dsize = sizeof record ;
        if (dbm_store (nob_database, key, value, DBM_REPLACE)) {
            LGE "(nobCreate) Error updating the reference count of %s.\ndbm_store: ",
                name) ;
            PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
            return (errno) ;
        }

        if (nobSynchronize ()) {
            LGE "(nobCreate) Error synchronizing the named object database.\nnobSynchronize: ") ;
            PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
            return (errno) ;
        }

        UNLOCK (nob_mutex) ;			/* Release lock on database. */

        SET_ERRNO (EEXIST) ;

        return (errno) ;

    }


/*******************************************************************************
    The object doesn't exist yet - "create" it by adding the initial
    name-object mapping to the named object database.
*******************************************************************************/

    record.value = NULL ;
    record.references = 0 ;
    value.dptr = (char *) &record ;
    value.dsize = sizeof record ;

    if (dbm_store (nob_database, key, value, DBM_INSERT)) {
        LGE "(nobCreate) Error adding %s to the named object database.\ndbm_store: ",
            name) ;
        PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        return (errno) ;
    }


/*******************************************************************************
    Done!  The caller is responsible for finalizing creation of the object
    and releasing exlusive access to the "named object database" by calling
    nobCommit() or nobAbort().
*******************************************************************************/

    LGI "(nobCreate) Created \"%s\".\n", name) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nobDestroy ()

    Delete a Named Object.


Purpose:

    Function nobDestroy() deletes a named object.  The number of references
    to the object is decremented and, if that number drops to zero, the
    object is deleted and its name removed from the named object database.
    The calling program can detect when the object is finally deleted by
    checking the status code returned by nobDestroy():

        if (!nobDestroy (object)) {		-- Last reference to object?
            ... object-specific delete processing ..
        } else if (errno != EWOULDBLOCK) {	-- Error?
            ... error ...
        }


    Invocation:

        status = nobDestroy (object) ;

    where:

        <object>	- I
            is the object handle returned by nobCreate() or nobExists().
        <status>	- O
            returns the status of deleting the object: EWOULDBLOCK if
            there are still outstanding references to the object, zero
            if there are no more references to the object and the object
            was successfully deleted, and ERRNO if an error occurred.

*******************************************************************************/


int  nobDestroy (

#    if PROTOTYPES
        NamedObject  object)
#    else
        object)

        NamedObject  object ;
#    endif

{    /* Local variables. */
    datum  key, value ;
    int  mutex ;
    NamedObjectRecord  record ;




    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobDestroy) NULL object handle: ") ;
        return (errno) ;
    }

    LGI "(nobDestroy) Deleting \"%s\" (%d).\n",
        nobName (object), nobCount (object)) ;


/* Wait for exclusive access to the named object database. */

    mutex = object->mutex ;
    if ((mutex >= 0) && LOCK (nob_mutex)) {
        LGE "(nobDestroy) Error locking the named object database.\nlockf: ") ;
        return (errno) ;
    }


/* Decrement the object's reference count in the named object database.
   If references to the object remain, release exclusive access to the
   database and return to the caller. */

    if (nobSynchronize ()) {
        LGE "(nobDestroy) Error synchronizing the named object database.\nnobSynchronize: ") ;
        if (mutex >= 0) {
            PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        }
        return (errno) ;
    }

    key.dptr = object->name ;
    key.dsize = strlen (object->name) + 1 ;

    value = dbm_fetch (nob_database, key) ;
    if (value.dptr == NULL) {
        LGE "(nobDestroy) Error retrieving %s from the named object database.\ndbm_fetch: ",
            object->name) ;
        if (mutex >= 0) {
            PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        }
        return (errno) ;
    }
						/* To prevent alignment errors. */
    memcpy (&record, value.dptr, sizeof record) ;
    record.references-- ;

    value.dptr = (char *) &record ;
    value.dsize = sizeof record ;

    if (record.references > 0) {

        if (dbm_store (nob_database, key, value, DBM_REPLACE)) {
            LGE "(nobCreate) Error updating %s's reference count in the named object database.\ndbm_store: ",
                object->name) ;
            if (mutex >= 0) {
                PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
            }
            return (errno) ;
        }

        if (nobSynchronize ()) {
            LGE "(nobDestroy) Error synchronizing the named object database.\nnobSynchronize: ") ;
            if (mutex >= 0) {
                PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
            }
            return (errno) ;
        }

        UNLOCK (nob_mutex) ;

        SET_ERRNO (EWOULDBLOCK) ;

        return (errno) ;

    }


/* This was the last remaining reference to the object.  Remove the object's
   name from the named object database. */

    if (dbm_delete (nob_database, key)) {
        LGE "(nobDestroy) Error deleting %s from the named object database.\ndbm_delete: ",
            object->name) ;
        if (mutex >= 0) {
            PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        }
        return (errno) ;
    }

    if (nobSynchronize ()) {
        LGE "(nobDestroy) Error synchronizing the named object database.\nnobSynchronize: ") ;
        if (mutex >= 0) {
            PUSH_ERRNO ;  UNLOCK (nob_mutex) ;  POP_ERRNO ;
        }
        return (errno) ;
    }


/* Deallocate the local copy of the object. */

    if (object->name != NULL)  free (object->name) ;
    free (object) ;


/* Release exclusive access to the "named object database". */

    UNLOCK (nob_mutex) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nobExists ()

    Lookup an Existing Named Object.


Purpose:

    Function nobExists() looks up and returns a handle for an existing object.


    Invocation:

        object = nobExists (name, scope) ;

    where:

        <name>		- I
            is the name of the object being looked up.
        <scope>		- I
            specifies the scope of the object: "singleCPU" or "multiCPU"
            (enumerated values defined in the "nob_util.h" header file).
            This argument is ignored; the named object database is visible
            to any machine that NFS mounts the directory containing the
            database files (see the package prolog).
        <object>	- O
            returns a handle for the object.  This handle is used in calls
            to the other NOB functions.  NULL is returned if the object
            doesn't exist or if an error occurs; in the latter case,
            ERRNO is set to indicate the type of error.

*******************************************************************************/


NamedObject  nobExists (

#    if PROTOTYPES
        const  char  *name,
        NamedObjectScope  scope)
#    else
        name, scope)

        char  *name ;
        NamedObjectScope  scope ;
#    endif

{    /* Local variables. */
    _NamedObject  dummy, *object ;



/* If this is the first access to the named object database, then
   initialize the NOB package. */

    if ((nob_database == NULL) && nobInitialize (scope, true)) {
        LGE "(nobExists) Error initializing the named object package for %s.\nnobInitialize: ",
            name) ;
        return (NULL) ;
    }

/* Check to see if the object exists in the named object database.
   (nobCount() attempts to fetch it from the database.) */

    dummy.name = (char *) name ;
    if (nobCount (&dummy) < 0) {		/* Quick check of existence. */
        SET_ERRNO (ENOENT) ;
        return (NULL) ;
    }

/* The object does exist - call nobCreate() to perform all the bookkeeping
   of creating an existing object. */

    if (nobCreate (name, scope, &object) && (errno != EEXIST)) {
        LGE "(nobExists) Error accessing an existing %s object.\nnobCreate: ",
            name) ;
        return (NULL) ;
    }

    return (object) ;

}

/*!*****************************************************************************

Procedure:

    nobName ()

    Get the Name of a Named Object.


Purpose:

    Function nobName() returns the name of a named object.


    Invocation:

        name = nobName (object) ;

    where:

        <object>	- I
            is the object handle returned by nobCreate() or nobExists().
        <name>		- O
            returns the name of the object.  The name is stored in memory
            belonging to the object and it should not be modified or freed
            by the caller.

*******************************************************************************/


const  char  *nobName (

#    if PROTOTYPES
        NamedObject  object)
#    else
        object)

        NamedObject  object ;
#    endif

{
    return ((object == NULL) ? "<nil>" : object->name) ;
}

/*!*****************************************************************************

Procedure:

    nobValue ()

    Get the Value of a Named Object.


Purpose:

    Function nobValue() returns an object's value, which was set when the
    object was nobCommit()ed.


    Invocation:

        value = nobValue (object) ;

    where:

        <object>	- I
            is the object handle returned by nobCreate() or nobExists().
        <value>		- O
            returns the object's value cast as a (VOID *) pointer.

*******************************************************************************/


void  *nobValue (

#    if PROTOTYPES
        NamedObject  object)
#    else
        object)

        NamedObject  object ;
#    endif

{
    return ((object == NULL) ? NULL : object->value) ;
}

/*!*****************************************************************************

Procedure:

    nobInitialize ()

    Initialize the Named Object Package.


Purpose:

    Private function nobInitialize() initializes the named object package.
    This includes:

          - Creating/opening the NDBM(3) database in which are stored the
            name/value mappings.
          - Opening the file used to lock the database.

    The lock file is used during:

        Object creation - to prevent multiple tasks from simultaneously
            trying to create the same object.

        Object lookup - to prevent tasks from accessing an existing object
            before the creator has finished creating the object.

        Object deletion - to control access to the object's reference count.


    Invocation:

        status = nobInitialize (scope, createLock) ;

    where:

        <scope>		- I
            specifies the scope of the named object database: "singleCPU"
            or "multiCPU" (enumerated values defined in the "nob_util.h"
            header file).  This argument is ignored; the named object database
            is visible to any machine that NFS mounts the directory containing
            the database files (see the package prolog).
        <createLock>	- I
            if true, specifies that a separate channel to the database should
            be opened as a LOCKF(3) lock.
        <status>	- O
            returns the status of initializing the NOB package, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


static  int  nobInitialize (

#    if PROTOTYPES
        NamedObjectScope  scope,
        bool  createLock)
#    else
        scope, createLock)

        NamedObjectScope  scope ;
        bool  createLock ;
#    endif

{    /* Local variables. */
    const  char  *pathname ;




/* Create/open the named object database using NDBM(3). */

    if (getenv ("NOB_DATABASE") == NULL)
        pathname = fnmBuild (FnmPath, "/tmp/nob_database", NULL) ;
    else
        pathname = fnmBuild (FnmPath, "$NOB_DATABASE", NULL) ;

    if (createLock)
        LGI "(nobInitialize) Named object database: %s\n", pathname) ;

    nob_database = dbm_open (pathname, (O_CREAT | O_RDWR), 0644) ;
    if (nob_database == NULL) {
        LGE "(nobInitialize) Error opening %s named object database.\ndbm_open: ",
            pathname) ;
        return (errno) ;
    }

    if (!createLock)  return (0) ;

/* Set up an exit handler to ensure that the database is closed on
   process exit. */

    atexit (nobTerminate) ;

/* Open one of the NDBM(3) files for use as a LOCKF(3) file. */

    pathname = fnmBuild (FnmPath, ".pag", pathname, NULL) ;
    nob_mutex = open (pathname, O_RDWR, 0) ;
    if (nob_mutex < 0) {
        LGE "(nobInitialize) Error opening %s lock file.\nopen: ", pathname) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nobSynchronize ()

    Synchronize the Named Object Database.


Purpose:

    Private function nobSynchronize() synchronizes the cached database with
    the disk database, so that updates made by this process are visible to
    other processes and vice-versa.  The synchronization is performed by
    simply closing and reopening the NDBM(3) database.


    Invocation:

        nobSynchronize () ;

*******************************************************************************/


static  int  nobSynchronize (

#    if PROTOTYPES && !defined(__cplusplus)
        void)
#    else
        )
#    endif

{
    dbm_close (nob_database) ;			/* Close the database. */
    nob_database = NULL ;
    return (nobInitialize (multiCPU, false)) ;	/* Reopen the database. */
}

/*!*****************************************************************************

Procedure:

    nobTerminate ()

    Terminate the Named Object Package.


Purpose:

    Private function nobTerminate() terminates the named object package.
    In particular, it closes the NDBM(3) database.


    Invocation:

        nobTerminate () ;

*******************************************************************************/


static  void  nobTerminate (

#    if PROTOTYPES && !defined(__cplusplus)
        void)
#    else
        )
#    endif

{
    if (LOCK (nob_mutex))  perror ("(nobTerminate) lockf") ;
    dbm_close (nob_database) ;
    if (UNLOCK (nob_mutex))  perror ("(nobTerminate) ulockf") ;
    close (nob_mutex) ;
}

#ifdef TEST

/*******************************************************************************

    Program to test the NOB_UTIL functions.

    Compile and link as follows:
        % cc -DTEST nob_util.c <libraries> -o nob_test
    and run with the following command:
        % nob_test <object_name> <value> <delay>

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
    NamedObject  object1, object2, object3, object4 ;




    nob_util_debug = 1 ;

    if (!nobCreate (argv[1], singleCPU, &object1)) {
        if (nobCommit (object1, (void *) atoi (argv[2]))) {
            LGE "[NOB_TEST] Error commiting %s object.\nnobCommit: ", argv[1]) ;
            exit (errno) ;
        }
    } else if (errno != EEXIST) {
        LGE "[NOB_TEST] Error creating %s object.\nnobCreate: ", argv[1]) ;
        exit (errno) ;
    }

    nobCreate (argv[1], singleCPU, &object2) ;
    nobCreate (argv[1], singleCPU, &object3) ;
    object4 = nobExists (argv[1], singleCPU) ;

    printf ("%s's (%p) reference count = %d\n",
            nobName (object1), object1, nobCount (object1)) ;

    printf ("%s's (%p) reference count = %d\n",
            nobName (object2), object2, nobCount (object2)) ;

    printf ("%s's (%p) reference count = %d\n",
            nobName (object3), object3, nobCount (object3)) ;

    printf ("%s's (%p) reference count = %d\n",
            nobName (object4), object4, nobCount (object4)) ;

    sleep (atoi (argv[3])) ;

    printf ("(4) Delete status = %d\n", nobDestroy (object4)) ;
    printf ("(3) Delete status = %d\n", nobDestroy (object3)) ;
    printf ("(2) Delete status = %d\n", nobDestroy (object2)) ;
    printf ("(1) Delete status = %d\n", nobDestroy (object1)) ;

}

#endif
