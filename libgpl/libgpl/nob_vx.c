/* $Id: nob_vx.c,v 1.10 2004/05/18 16:22:09 alex Exp $ */
/*******************************************************************************

File:

    nob_vx.c

    Named Object Utilities (VxWorks Version).


Author:    Alex Measday


Purpose:

    The Named Object (NOB) utilities provide a general means of assigning
    names to arbitrary objects so that other tasks (possibly on other CPUs)
    can access the objects by name.

    The NOB utilities are intended for, but not limited to, use in library
    functions that both create new objects and access existing objects.
    An application can call such a library function without caring if the
    target object exists or not - the object will be created automatically
    if need be.  For example, message queues are known by their ID under
    VxWorks.  A library function that creates new named message queues or
    accesses existing ones would use the NOB utilities as follows:

        #include  "nob_util.h"			-- Named object definitions.
        MSG_Q_ID  queue ;
        NamedObject  qobj ;

        ...

        if (!nobCreate ("MY_MSGQ", multiCPU, &qobj)) {
            queue = msgQCreate (10, 100, MSG_Q_FIFO) ;	-- Brand new?
            nobCommit (qobj, (void *) queue) ;
        } else if (errno == EEXIST) {			-- Already exists?
            queue = (MSG_Q_ID) nobValue (qobj) ;
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

        qobj = nobExists ("MY_MSGQ", multiCPU) ;
        if (qobj == NULL) {			-- Doesn't exist or error?
            ... error ...
        } else {				-- Exists?
            queue = (MSG_Q_ID) nobValue (qobj) ;
        }

    Processes can register a reference to an object through an existing
    object handle by calling nobReference():

        static  namedObject  qobj = NULL ;	-- Save from call to call.
        ...
        if (qobj == NULL)			-- First lookup?
            qobj = nobExists ("COMMON_MSGQ", multiCPU) ;
        else					-- Subsequent reference.
            nobReference (qobj) ;
        queue = (MSG_Q_ID) nobValue (qobj) ;

    Deleting the named message queue is done as follows:

        if (!nobDestroy (qobj)) {		-- Last user of queue?
            msgQDelete (queue) ;
        } else if (errno != EWOULDBLOCK) {	-- Error?
            ... error ...
        }

    Note that the last task using the message queue is the one that actually
    deletes the queue.


Implementation Notes (VxWorks):

    Under VxWorks, the "named object database" is implemented using the
    system symbol table and, when VxMP is available, the shared memory
    database.  The single-/multi-CPU scope argument to nobCreate()
    provides for a two-level database.

    A semaphore is used to prevent simultaneous updates to the "named
    object database".  This semaphore is itself accessed by name by
    different tasks.  For local symbols (or on a non-VxMP system), the
    semaphore's name is stored in the system symbol table, which allows
    multiple instances of a symbol.  It is possible that N tasks could
    simultaneously find the semaphore absent from the symbol table,
    create N new semaphores, and add N semaphores to the symbol table.
    To prevent this from happening, the tasks are allowed to create the
    semaphore, but then an internal function, nobExamine(), scans the
    symbol table for the earliest-added NOB semaphore; that semaphore
    becomes *the* NOB semaphore; all others are deleted by the tasks
    who created them.  (Scanning the symbol table might seem slow, but
    it only happens when the semaphore is not found in the table; once
    the semaphore is created, no scanning is necessary.)

    A separate semaphore is used for global symbols on a VxMP system.
    This semaphore's name is entered in the VxMP shared memory database,
    which doesn't allow multiple instances of a given symbol.  Under
    VxMP, the objects are stored in VxMP shared memory; the NOB functions
    make the appropriate global-to-local address conversions.  However,
    if an object's value requires conversions, the creator and users of
    the object are responsible for performing the necessary conversions.

    Tasks that terminate prematurely or that don't delete their objects can
    leave the "named object database" in an indeterminate or unaccessible
    state.


Public Procedures:

    nobCommit() - finalizes the creation of a named object.
    nobCount() - returns the number of tasks referencing a named object.
    nobCreate() - creates a named object.
    nobDestroy() - deletes a named object.
    nobExists() - looks up a named object.
    nobName() - returns a named object's name.
    nobReference() - registers a reference to a named object through
        an existing handle.
    nobValue() - returns a named object's value.

Private Procedures:

    nobSemaphore() - returns the ID of the NOB semaphore.
    nobExamine() - examines each entry in the system symbol table.

*******************************************************************************/


#include  <vxWorks.h>			/* VxWorks definitions. */
#include  <errno.h>			/* System error definitions. */
#include  <limits.h>			/* Maximum/minimum value definitions. */
#include  <semLib.h>			/* Semaphore definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <sysLib.h>			/* System library definitions. */
#include  <taskLib.h>			/* Task library definitions. */
#define  getpid  taskIdSelf
#define  sleep(seconds)  taskDelay ((seconds) * sysClkRateGet())
#ifdef VxMP
#    include  <smMemLib.h>		/* Shared memory definitions. */
#    include  <smNameLib.h>		/* Shared name database definitions. */
#    include  <smObjLib.h>		/* Shared memory object definitions. */
#    include  <semSmLib.h>		/* Shared semaphore definitions. */
#else
#    include  <symLib.h>		/* Symbol table definitions. */
#    include  <sysSymTbl.h>		/* System symbol table definitions. */
#endif
#include  "nob_util.h"			/* Named object definitions. */


/*******************************************************************************
    Named Object - contains an object's name, its value, and a reference count.
*******************************************************************************/

typedef  struct  _NamedObject {
    char  *name ;			/* Object's name. */
    void  *value ;			/* Object's value. */
    NamedObjectScope  scope ;		/* Single- or multi-CPU scope (VxMP only). */
    int  references ;			/* Number of references to object. */
    SEM_ID  mutex ;			/* ID of the NOB semaphore. */
}  _NamedObject ;


/*******************************************************************************
    Name and ID - is an internal structure used by private function
        nobSemaphore() to pass information to nobExamine().
*******************************************************************************/

#define  NOB_SEMAPHORE  "NOB_SEMAPHORE"

typedef  struct  NameID {
    char  *name ;			/* Semaphore name. */
    SEM_ID  ID ;			/* Semaphore ID. */
}  NameID ;


int  nob_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  nob_util_debug


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  SEM_ID  nobSemaphore P_((NamedObjectScope scope)) ;

static  BOOL  nobExamine P_((char *name,
                             int value,
                             SYM_TYPE type,
                             int argument,
                             UINT16 group)) ;

/*******************************************************************************

Procedure:

    nobAbort ()

    Abort the Creation of a Named Object.


Purpose:

    Function nobAbort() completes the creation of a new object.
    The partially created object is deleted and exclusive access
    to the "named object database" is released.  Like nobCommit(),
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
    SEM_ID  mutex ;




    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobAbort/%X) NULL object handle: ", getpid ()) ;
        return (errno) ;
    }

    LGI "(nobAbort/%X) Aborting creation of \"%s\".\n",
        getpid (), nobName (object)) ;

/* Delete the object. */

    object->references = 0 ;
    mutex = object->mutex ;
    object->mutex = NULL ;		/* Tell nobDestroy() not to lock. */

    status = nobDestroy (object) ;
    if (status) {
        LGE "(nobAbort/%X) Error deleting object.\nnobDestroy: ", getpid ()) ;
    }

/* Release exclusive access to the "named object database". */

    semGive (mutex) ;

    SET_ERRNO (status) ;

    return (errno) ;

}

/*******************************************************************************

Procedure:

    nobCommit ()

    Complete the Creation of a Named Object.


Purpose:

    Function nobCommit() completes the creation of a new object and
    makes it available to other tasks.  This involves storing the
    caller-specified value in the object, initializing the object's
    reference count to one, and releasing exclusive access to the
    "named object database".  nobCommit() must be called after
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

{

    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobCommit/%X) NULL object handle: ", getpid ()) ;
        return (errno) ;
    }

/* Store the value in the object and initialize the reference count to 1. */

    object->value = value ;
    object->references = 1 ;

/* Release exclusive access to the "named object database". */

    semGive (object->mutex) ;

    LGI "(nobCommit/%X) Completed creation of \"%s\".\n",
        getpid (), nobName (object)) ;

    return (0) ;

}

/*******************************************************************************

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

{
    return ((object == NULL) ? -1 : object->references) ;
}

/*******************************************************************************

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
            specifies the scope of the object when this function is built
            under VxMP:
                singleCPU - The object and its name are stored and
                    are known on the local CPU only.
                multiCPU - The object is stored in shared memory and
                    its name is stored in the shared memory database,
                    so that tasks on any CPU in the system can access
                    the object.
            "singleCPU" and "multiCPU" are enumerated values defined in
            the "nob_util.h" header file.  This argument is ignored if the
            function is not built under VxMP
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
    char  object_name[PATH_MAX] ;	/* Protected by NOB semaphore. */
    int  status, type ;
    SEM_ID  mutex ;
    void  *value ;





/*******************************************************************************
    Wait on the NOB semaphore for exclusive access to the "named object
    database" (a virtual entity).
*******************************************************************************/

    mutex = nobSemaphore (scope) ;
    if (mutex == NULL) {
        LGE "(nobCreate/%X) Error getting the NOB semaphore ID.\n", getpid ()) ;
        return (errno) ;
    }

    if (semTake (mutex, WAIT_FOREVER) == ERROR) {
        LGE "(nobCreate/%X) Error waiting on the NOB semaphore.\nsemTake: ",
            getpid ()) ;
        return (errno) ;
    }


/*******************************************************************************
    Lookup the object's name in the "named object database"; i.e., the system
    symbol table (or the shared memory database under VxMP).  If the name is
    found (i.e., the object exists), then return the object's handle to the
    caller.
*******************************************************************************/

    sprintf (object_name, "NOB_%s", name) ;
    *object = NULL ;

#ifdef VxMP
    if (scope == multiCPU) {
        if (smNameFind (object_name, &value, &type, NO_WAIT) == OK)
            *object = smObjGlobalToLocal (value) ;
    } else
#endif
    if (symFindByName (sysSymTbl, object_name,
                       (char **) &value, (SYM_TYPE *) &type) == OK) {
        *object = value ;
    }

    if (*object != NULL) {
        (*object)->references++ ;
        semGive (mutex) ;
        SET_ERRNO (EEXIST) ;
        return (errno) ;
    }


/*******************************************************************************
    The object doesn't exist yet - create it.
*******************************************************************************/

#ifdef VxMP

    if (scope == multiCPU) {		/* Allocate storage from shared memory. */

        *object = smMemMalloc (sizeof (_NamedObject)) ;
        if (*object == NULL) {
            LGE "(nobCreate/%X) Error allocating shared object structure for %s object.\nsmMemMalloc: ",
                getpid (), name) ;
            PUSH_ERRNO ;  semGive (mutex) ;  POP_ERRNO ;
            return (errno) ;
        }

        (*object)->name = smMemMalloc (strlen (name) + 1) ;
        if ((*object)->name == NULL) {
            LGE "(nobCreate/%X) Error duplicating name of %s object in shared memory.\nsmMemMalloc: ",
                getpid (), name) ;
            PUSH_ERRNO ;  semGive (mutex) ;  POP_ERRNO ;
            return (errno) ;
        }
        strcpy ((*object)->name, name) ;
        (*object)->name = smObjLocalToGlobal ((*object)->name) ;

    } else

#endif

    {					/* Allocate storage from local heap. */

        *object = malloc (sizeof (_NamedObject)) ;
        if (*object == NULL) {
            LGE "(nobCreate/%X) Error allocating object structure for %s object.\nmalloc: ",
                getpid (), name) ;
            PUSH_ERRNO ;  semGive (mutex) ;  POP_ERRNO ;
            return (errno) ;
        }

        (*object)->name = malloc (strlen (name) + 1) ;
        if ((*object)->name == NULL) {
            LGE "(nobCreate/%X) Error duplicating name of %s object.\nmalloc: ",
                getpid (), name) ;
            PUSH_ERRNO ;  semGive (mutex) ;  POP_ERRNO ;
            return (errno) ;
        }
        strcpy ((*object)->name, name) ;

    }

    (*object)->value = NULL ;
    (*object)->scope = scope ;
    (*object)->references = 0 ;
    (*object)->mutex = mutex ;


/*******************************************************************************
    Add the new name-object mapping to the "named object database".
*******************************************************************************/

#ifdef VxMP
    if (scope == multiCPU) {		/* Add to shared memory database. */
        value = smObjLocalToGlobal ((void *) (*object)) ;
        if (smNameAdd (object_name, value, T_SM_BLOCK) == ERROR)) {
            LGE "(nobCreate/%X) Error adding %s to shared name database.\nsmNameAdd: ",
                getpid (), object_name) ;
            PUSH_ERRNO ;  nobAbort (*object) ;  POP_ERRNO ;
            return (errno) ;
        }
    } else
#endif
					/* Add to local symbol table. */
    if (symAdd (sysSymTbl, object_name, (char *) *object, 0, 0) == ERROR) {
        LGE "(nobCreate/%X) Error adding %s to system symbol table.\nsymAdd: ",
            getpid (), object_name) ;
        PUSH_ERRNO ;  nobAbort (*object) ;  POP_ERRNO ;
        return (errno) ;
    }


/*******************************************************************************
    Done!  The caller is responsible for finalizing creation of the object
    and releasing exlusive access to the "named object database" by calling
    nobCommit() or nobAbort().
*******************************************************************************/

    LGI "(nobCreate/%X) Created \"%s\".\n", getpid (), name) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    nobDestroy ()

    Delete a Named Object.


Purpose:

    Function nobDestroy() deletes a named object.  The number of references
    to the object is decremented and, if that number drops to zero, the
    object is deleted and its name removed from the "named object database".
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
    int  status ;
    SEM_ID  mutex ;
    static  char  object_name[PATH_MAX] ;	/* Protected by NOB semaphore. */





    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobDestroy/%X) NULL object handle: ", getpid ()) ;
        return (errno) ;
    }

    LGI "(nobDestroy/%X) Deleting \"%s\" (%d).\n",
        getpid (), nobName (object), object->references) ;


/* Wait on the NOB semaphore for exclusive access to the "named object
   database". */

    mutex = object->mutex ;
    if ((mutex != NULL) && (semTake (mutex, WAIT_FOREVER) == ERROR)) {
        LGE "(nobDestroy/%X) Error waiting on the NOB semaphore.\nsemTake: ",
            getpid ()) ;
        return (errno) ;
    }


/* Decrement the object's reference count.  If references to the object
   remain, release exclusive access to the "named object database" and
   return to the caller. */

    object->references-- ;
    if (object->references > 0) {
        if (mutex != NULL)  semGive (mutex) ;
        errno = EWOULDBLOCK ;
        return (errno) ;
    }


/* This was the last remaining reference to the object.  Remove the object's
   name from the "named object database"; i.e., the system symbol table (or
   the shared memory database under VxMP). */

    sprintf (object_name, "NOB_%s", nobName (object)) ;
#ifdef VxMP
    if (object->scope == multiCPU) {		/* From shared memory database. */
        if (smNameRemove (object_name)) {
            LGE "(nobDestroy/%X) Error deleting %s from the shared memory database.\nsmNameRemove: ",
                getpid (), object_name) ;
            PUSH_ERRNO ;  if (mutex != NULL)  semGive (mutex) ;  POP_ERRNO ;
            return (errno) ;
        }
    } else
#endif						/* From local symbol table. */
    if (symRemove (sysSymTbl, object_name, 0) == ERROR) {
        LGE "(nobDestroy/%X) Error deleting %s from the system symbol table.\nsymRemove: ",
            getpid (), object_name) ;
        PUSH_ERRNO ;  if (mutex != NULL)  semGive (mutex) ;  POP_ERRNO ;
        return (errno) ;
    }


/* Deallocate the object. */

#ifdef VxMP
    if (object->scope == multiCPU) {		/* From shared memory. */
        if (object->name != NULL)
            smMemFree (smObjGlobalToLocal (object->name)) ;
        smMemFree (object) ;
    } else
#endif
    {						/* From the local heap. */
        if (object->name != NULL)  free (object->name) ;
        free (object) ;
    }


/* Release exclusive access to the "named object database". */

    if (mutex != NULL)  semGive (mutex) ;


    return (0) ;

}

/*******************************************************************************

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
            specifies the scope of the object when this function is built
            under VxMP:
                singleCPU - The object and its name are stored and
                    are known on the local CPU only.
                multiCPU - The object is stored in shared memory and
                    its name is stored in the shared memory database.
            "singleCPU" and "multiCPU" are enumerated values defined in
            the "nob_util.h" header file.  This argument is ignored if the
            function is not built under VxMP
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
    char  object_name[PATH_MAX] ;
    int  type ;
    NamedObject  object ;
    SEM_ID  mutex ;
    void  *value ;




/* Wait on the NOB semaphore for exclusive access to the "named object
   database" (a virtual entity). */

    mutex = nobSemaphore (scope) ;
    if (mutex == NULL) {
        LGE "(nobExists/%X) Error getting the NOB semaphore ID: ", getpid ()) ;
        return (NULL) ;
    }

    if (semTake (mutex, WAIT_FOREVER) == ERROR) {
        LGE "(nobExists/%X) Error waiting on the NOB semaphore.\nsemTake: ",
            getpid ()) ;
        return (NULL) ;
    }


/* Lookup the object's name in the "named object database"; i.e., the system
   symbol table (or the shared memory database under VxMP).  If the name is
   found (i.e., the object exists), then return the object's handle to the
   caller. */

    sprintf (object_name, "NOB_%s", name) ;
    object = NULL ;

#ifdef VxMP
    if (scope == multiCPU) {
        if (smNameFind (object_name, &value, &type, NO_WAIT) == OK)
            object = smObjGlobalToLocal (value) ;
    } else
#endif
    if (symFindByName (sysSymTbl, object_name,
                       (char **) &value, (SYM_TYPE *) &type) == OK) {
        object = value ;
    }

    if (object != NULL)  object->references++ ;	/* Increment reference count. */

    semGive (mutex) ;				/* Release the database lock. */

    if (object == NULL)  errno = ENOENT ;	/* Not found? */

    return (object) ;

}

/*******************************************************************************

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

    if (object == NULL)  return ("<nil>") ;

#ifdef VxMP
    if (object->scope == multiCPU)
        return (smObjGlobalToLocal (object->name)) ;
#endif

    return (object->name) ;

}

/*******************************************************************************

Procedure:

    nobReference ()

    Reference a Named Object through an Existing Handle.


Purpose:

    Function nobReference() registers a reference to a named object through
    an existing handle.  Under UNIX, nobReference() avoids the overhead of
    creating a new handle for each reference to an object.  Under VxWorks,
    however, the same handle is used for all references to an object and
    nobReference() is implemented by simply calling nobExists().
    nobDestroy() must still be called for each reference to the object.


    Invocation:

        status = nobReference (object) ;

    where:

        <object>	- I
            is the object handle returned by nobCreate() or nobExists().
        <status>	- O
            returns the status of registering a reference to the object,
            zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  nobReference (

#    if PROTOTYPES
        NamedObject  object)
#    else
        object)

        NamedObject  object ;
#    endif

{

    if (object == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nobReference) NULL object handle: ") ;
        return (errno) ;
    }

/* Since the handle IS the object under our VxWorks implementation,
   we can use nobExists() to increment the object's reference count. */

    if (nobExists (object->name, object->scope) == NULL) {
        LGE "(nobReference/%X) Error referencing %s.\nnobExists: ",
            getpid (), object->name) ;
        return (errno) ;
    }

    LGI "(nobReference/%X) Referencing \"%s\".\n",
        getpid (), nobName (object)) ;

    return (0) ;

}

/*******************************************************************************

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

/*******************************************************************************

Procedure:

    nobSemaphore ()

    Get the ID of the NOB Semaphore.


Purpose:

    Private function nobSemaphore() returns the ID of the "global" named
    object semaphore.  The NOB semaphore is a single semaphore used during:

        Object creation - to prevent multiple tasks from simultaneously
            trying to create the same object.

        Object lookup - to prevent tasks from accessing an existing object
            before the creator has finished creating the object.

        Object deletion - to control access to the object's reference count.


    Invocation:

        semaphore = nobSemaphore (scope) ;

    where:

        <scope>		- I
            specifies the scope of the semaphore when this function is
            built under VxMP:
                singleCPU - The semaphore is known on the local CPU only.
                multiCPU - The semaphore's ID is stored in the system-wide
                    shared memory database, so that tasks on other CPUs can
                    access the semaphore.
            "singleCPU" and "multiCPU" are enumerated values defined in
            the "nob_util.h" header file.  This argument is ignored if the
            function is not built under VxMP
        <semaphore>	- O
            returns the system ID of the NOB semaphore.  NULL is returned
            in the event of an error.

*******************************************************************************/


static  SEM_ID  nobSemaphore (

#    if PROTOTYPES
        NamedObjectScope  scope)
#    else
        scope)

        NamedObjectScope  scope ;
#    endif

{    /* Local variables. */
    NameID  name_and_ID ;
    SEM_ID  mutex ;
#ifdef VxMP
    int  type ;
    void  *value ;
#endif




/*******************************************************************************
    Lookup the semaphore's name and ID in the system symbol table (or the
    shared memory database under VxMP).  If the name is found (i.e., the
    semaphore exists), then simply return the ID to the caller.
*******************************************************************************/

#ifdef VxMP
    if (scope == multiCPU) {		/* Lookup in shared memory database. */
        if (smNameFind ("NOB_SEMAPHORE", &value, &type, NO_WAIT) == OK)
            return ((SEM_ID) value) ;
    } else
#endif
    {					/* Lookup in local symbol table. */
        name_and_ID.name = "NOB_SEMAPHORE" ;
        name_and_ID.ID = NULL ;
        symEach (sysSymTbl, (FUNCPTR) nobExamine, (int) &name_and_ID) ;
        if (name_and_ID.ID != NULL)
            return (name_and_ID.ID) ;
    }


/*******************************************************************************
    The semaphore's name was not found in the symbol table.  Create a new
    semaphore.
*******************************************************************************/

#ifdef VxMP
    if (scope == multiCPU) {		/* Create a shared semaphore. */
        mutex = semBSmCreate (SEM_Q_FIFO, SEM_EMPTY) ;
        if (mutex == NULL) {
            LGE "(nobSemaphore/%X) Error creating shared mutex semaphore.\nsemBSmCreate: ",
                getpid ()) ;
            return (NULL) ;
        }
    } else
#endif
    {					/* Create a local semaphore. */
        mutex = semMCreate (SEM_Q_FIFO | SEM_DELETE_SAFE) ;
        if (mutex == NULL) {
            LGE "(nobSemaphore/%X) Error creating mutex semaphore.\nsemMCreate: ",
                getpid ()) ;
            return (NULL) ;
        }
    }


/*******************************************************************************
    Under VxMP, add the semaphore name-ID mapping to the shared memory
    database.  If someone else already added such a mapping, then delete
    our semaphore and call nobSemaphore() recursively to lookup the ID
    of the existing semaphore.
*******************************************************************************/

#ifdef VxMP

    if (scope == multiCPU) {

        if (smNameAdd (NOB_SEMAPHORE, (void *) mutex, T_SM_SEM_B) == ERROR)) {
            if (errno == S_smNameLib_NAME_ALREADY_EXIST) {
                semDelete (mutex) ;
                return (nobSemaphore (object)) ;
            }
            LGE "(nobSemaphore/%X) Error adding %s to shared name database.\nsmNameAdd: ",
                getpid (), NOB_SEMAPHORE) ;
            semDelete (mutex) ;
            return (NULL) ;
        }

        return (mutex) ;

    }

#endif


/*******************************************************************************
    On a single-CPU system, add the semaphore-name ID mapping to the system
    symbol table and then lookup the mapping again.  If ours is not the
    earliest-entered mapping, then delete our semaphore and return the ID
    from the earliest mapping.
*******************************************************************************/

    if (symAdd (sysSymTbl, NOB_SEMAPHORE, (char *) mutex, 0, 0) == ERROR) {
        LGE "(nobSemaphore/%X) Error adding %s to system symbol table.\nsymAdd: ",
            getpid (), NOB_SEMAPHORE) ;
        semDelete (mutex) ;
        return (NULL) ;
    }

    symEach (sysSymTbl, (FUNCPTR) nobExamine, (int) &name_and_ID) ;
    if (name_and_ID.ID != mutex) {		/* We weren't the first? */
        symRemove (sysSymTbl, NOB_SEMAPHORE, 0) ;
        semDelete (mutex) ;
    }


    return (name_and_ID.ID) ;

}

/*******************************************************************************

Procedure:

    nobExamine ()

    Examine Each Entry in the Symbol Table.


Purpose:

    Private function nobExamine() examines each entry in the system symbol
    table and "returns" the earliest-added entry for the NOB semaphore.
    nobSemaphore() calls "symEach(2)" to scan the symbol table; "symEach(2)",
    in turn, calls nobExamine() for each entry in the table.  For each
    instance of the NOB semaphore encountered, nobExamine() stores the
    corresponding ID in the argument block passed to it.  Since the last
    ID stored corresponds to the earliest-added entry, nobSemaphore()
    will be "returned" the ID of the earliest semaphore.

      NOTE: nobExamine() assumes that it will encounter the instances
            of the NOB semaphore in reverse chronological order.  The
            order in which "symEach(2)" scans the symbols is not documented.


    Invocation:

        continue = nobExamine (name, value, type, argument, group) ;

    where:

        <name>		- I
            is the name of the symbol being examined.
        <value>		- I
            is the symbols's value.
        <type>		- I
            is the symbol's type.
        <argument>	- I
            is the address of the name/ID block passed by nobSemaphore().
        <group>		- I
            is the symbol's group number.
        <continue>	- O
            returns TRUE if "symEach(2)" should continue scanning the
            symbol table and FALSE if it shouldn't.

*******************************************************************************/


static  BOOL  nobExamine (

#    if PROTOTYPES
        char  *name,
        int  value,
        SYM_TYPE  type,
        int  argument,
        UINT16  group)
#    else
        name, value, type, argument, group)

        char  *name ;
        int  value ;
        SYM_TYPE  type ;
        int  argument ;
        UINT16  group ;
#    endif

{    /* Local variables. */
    NameID  *name_and_ID = (NameID *) argument ;




/* If the symbol name matches that of the NOB semaphore, then "return"
   the symbol's value (i.e., the semaphore ID) to nobSemaphore().
   Note that the last match found is the one actually "returned". */

    if ((name[0] == name_and_ID->name[0]) &&
        (strcmp (name, name_and_ID->name) == 0)) {
        name_and_ID->ID = (SEM_ID) value ;
    }

    return (TRUE) ;

}

#ifdef TEST

/*******************************************************************************

    Program to test the NOB_UTIL functions.

    Compile and link as follows:
        % cc -DTEST nob_util.c <libraries> -o nob_test
    and load and run with the following commands:
        -> ld <nob_test.vx.o
        -> sp nob_test, "<object_name> <value> <delay>"

*******************************************************************************/


int  nob_test (char *command_line)

{    /* Local variables. */
    char  **argv ;
    int  argc ;
    NamedObject  object1, object2, object3, object4 ;




		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("nob_util", command_line, &argc, &argv) ;

    vperror_print = 1 ;
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
    nobReference (object1) ;

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
    printf ("(1) Delete status = %d\n", nobDestroy (object1)) ;

    return (0) ;

}

#endif
