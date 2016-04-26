/* $Id: shm_util.c,v 1.14 2003/09/11 23:37:27 alex Exp $ */
/*******************************************************************************

File:

    shm_util.c

    Shared Memory Utilities.


Author:    Alex Measday


Purpose:

    The SHM utilities provide a high-level interface to the underlying
    operating system's shared memory facility.

    Creating and/or mapping to a shared memory segment is as simple as
    this:

        #include  "shm_util.h"			-- Shared memory utilities.
        SharedMemory  memory ;
        long  size = <numBytes> ;
        void  *address ;
        ...
        address = NULL ;
        shmCreate ("my_shared_memory", size, &address, &memory) ;
        ...
        ... access shared memory at address returned in ADDRESS ...
        ...

    The first process that calls shmCreate() for a given segment creates
    the shared memory segment.  Subsequent calls to shmCreate() by other
    processes map to the existing segment.

    If the operating system supports it, shared memory segments can be
    created at specific addresses.  This is accomplished by setting the
    address argument to the desired address before calling shmCreate():

        ...
        address = <desired_address> ;
        shmCreate ("my_shared_memory", size, &address, &memory) ;
        ...

    Presumably, the creator and users of that memory segment know what
    they are doing.

    The binary contents of a shared memory segment can be saved to
    a file and restored from a file with shmSave() and shmLoad(),
    respectively:

        shmSave (memory, "<file_name>") ;	-- Save contents.
        ...
        shmLoad (memory, "<file_name>") ;	-- Restore contents.

    A shared memory segment is unmapped by calling shmDestroy():

        shmDestroy (memory) ;

    The shared memory segment isn't deleted from the system until the last
    process mapped to it deletes it.


Notes (UNIX):

    The UNIX shared memory functions, SHMGET(2) et al, are used to create,
    attach to, and delete shared memory segments.  The name/IPC identifier
    mappings and reference counts are stored in the named object database
    (see NOB_UTIL.C).

    Processes should delete all shared memory segments before exiting;
    if a process exits prematurely, the named object database could be
    left in an inconsistent state.


Notes (VxWorks):

    Since all tasks run in a single address space, "shared" memory segments
    are simply allocated from the MALLOC(3) heap; under VxMP, the segments
    are allocated from global memory.  Segments can be located at arbitrary
    addresses by specifying an address in the shmCreate() call.  The
    name/address mappings and reference counts are stored in the named
    object database (see NOB_UTIL.C).

    Processes should delete all shared memory segments before exiting;
    if a process exits prematurely, the named object database could be
    left in an inconsistent state.


Procedures:

    shmAddress() - returns the address of a shared memory segment.
    shmCreate() - creates and/or maps to a shared memory segment.
    shmDestroy() - deletes a shared memory segment.
    shmId() - returns the IPC identifier for a shared memory segment.
    shmLoad() - loads the contents of a file into a shared memory segment.
    shmSave() - saves the contents of a shared memory segment to a file.
    shmSizeOf() - returns the size of a shared memory segment.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <sys/stat.h>			/* File status definitions. */
#if defined(VMS)
    /* ... */
#elif defined(VXWORKS)
#    ifdef VxMP
#        include  <smMemLib.h>		/* Shared memory definitions. */
#    endif
#else
#    include  <sys/types.h>		/* System type definitions. */
#    include  <sys/ipc.h>		/* Inter-process communication definitions. */
#    include  <sys/shm.h>		/* Shared memory definitions. */
#    ifdef SHMAT_NOTDECLARED
        extern  char  *shmat () ;	/* Needed under SunOS 4.1.3? */
#    endif
#endif
#include  "fnm_util.h"			/* Filename utilities. */
#include  "meo_util.h"			/* Memory operations. */
#include  "nob_util.h"			/* Named object definitions. */
#include  "shm_util.h"			/* Shared memory utility definitions. */


/*******************************************************************************
    Shared Memory Segment -
*******************************************************************************/

typedef  struct  _SharedMemory {
    NamedObject  object ;		/* Handle of segment's named object. */
    void  *address ;			/* Location of segment. */
    long  size ;			/* Size in bytes of segment. */
#ifndef VXWORKS
    int  ipc_id ;			/* System IPC ID for the segment. */
#endif
}  _SharedMemory ;


int  shm_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  shm_util_debug

/*!*****************************************************************************

Procedure:

    shmAddress ()

    Return the Address of a Shared Memory Segment.


Purpose:

    Function shmAddress() returns the address of a shared memory segment.


    Invocation:

        address = shmAddress (memory) ;

    where:

        <memory>	- I
            is the shared memory handle returned by shmCreate().
        <address>	- O
            returns the (VOID *) address of the shared memory segment.

*******************************************************************************/


void  *shmAddress (

#    if PROTOTYPES
        SharedMemory  memory)
#    else
        memory)

        SharedMemory  memory ;
#    endif

{

    return ((memory == NULL) ? NULL : memory->address) ;

}

/*!*****************************************************************************

Procedure:

    shmCreate ()

    Create and/or Map to a Shared Memory Segment.


Purpose:

    Function shmCreate() creates and/or maps to a shared memory segment.


    Invocation:

        void  *address = NULL ;			-- Or a desired address.
        ...
        status = shmCreate (name, size, &address, &memory) ;

    where:

        <name>		- I
            is the name of the shared memory segment.
        <size>		- I
            is the size of the shared memory segment.  If the size is zero,
            shmCreate() attempts to map to an existing segment; it is an
            error if the segment doesn't exist.  If the size is greater than
            zero, shmCreate() will create the segment if it doesn't already
            exist; it is an error if the size of an existing segment is less
            than the requested size.
        <address>	- I/O
            specifies/receives the address at which the shared memory segment
            is located.  This argument is the address of (VOID *) variable.
            If the argument is NULL or if the value of the variable is NULL,
            shmCreate() will allow the operating system to choose the location
            of the shared memory.  If the value of the variable is not NULL,
            shmCreate() attempts to locate the shared memory at the specified
            address, assuming the operating system supports this capability.
        <memory>	- O
            returns a handle for the shared memory segment that is to be
            used in calls to the other SHM_UTIL functions.
        <status>	- O
            returns the status of creating and/or mapping to the shared
            memory, zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  shmCreate (

#    if PROTOTYPES
        const  char  *name,
        long  size,
        void  **address,
        SharedMemory  *memory)
#    else
        name, size, address, memory)

        const  char  *name ;
        long  size ;
        void  **address ;
        SharedMemory  *memory ;
#    endif

{    /* Local variables. */
    NamedObject  object ;
    void  *segment ;




/*******************************************************************************
    Create a named object for the shared memory segment.  Under UNIX, the
    value of the object is the segment ID assigned by the operating system.
    Under VxWorks, the value of the object is the address of a SharedMemory
    object that is shared by all tasks that access the shared memory.
*******************************************************************************/

    if (!nobCreate (name, multiCPU, &object)) {		/* Brand new? */

/* A new shared memory segment is being created.  Signal an error if
   the caller wanted to map to an existing segment. */

        if (size <= 0) {
            nobAbort (object) ;
            SET_ERRNO (ENOENT) ;
            LGE "(shmCreate) Existing %s shared memory not found.\n", name) ;
            return (errno) ;
        }

#ifdef VXWORKS


/* Create a shared memory object that will be shared by all tasks accessing
   the shared memory. */

#    ifdef VxMP
        *memory = (SharedMemory) smMemMalloc (sizeof (_SharedMemory)) ;
        if (*memory == NULL) {
            LGE "(shmCreate) Error creating shared memory object for %s.\nsmMemMalloc: ",
                name) ;
#    else
        *memory = (SharedMemory) malloc (sizeof (_SharedMemory)) ;
        if (*memory == NULL) {
            LGE "(shmCreate) Error creating shared memory object for %s.\nmalloc: ",
                name) ;
#    endif
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }

        (*memory)->object = object ;
        (*memory)->size = size ;

/* (VxWorks) Create the actual shared memory segment. */

#    ifdef VxMP
        (*memory)->address = (void *) smMemMalloc (size) ;
        if ((*memory)->address == NULL) {
            LGE "(shmCreate) Error creating %d-byte %s shared memory.\nsmMemMalloc: ",
                size, name) ;
#    else
        (*memory)->address = (void *) malloc (size) ;
        if ((*memory)->address == NULL) {
            LGE "(shmCreate) Error creating %d-byte %s shared memory.\nmalloc: ",
                size, name) ;
#    endif
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }

        segment = (void *) *memory ;

#else

/* (UNIX) Create the actual shared memory segment. */

      { int  id ;

        id = shmget (IPC_PRIVATE, size, 0660) ;
        if (id < 0) {
            LGE "(shmCreate) Error creating %d-byte %s shared memory.\nshmget: ",
                size, name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
        segment = (void *) id ;
      }

#endif

/* Add the name/ID mapping for the shared memory segment to the named object
   database. */

        if (nobCommit (object, segment)) {
            LGE "(shmCreate) Error commiting named object for %s.\nnobCommit: ",
                name) ;
            return (errno) ;
        }

    }

/* If the named object already exists, its value is the ID of the existing
   shared memory segment. */

    else if (errno == EEXIST) {

        segment = nobValue (object) ;

    } else {

        LGE "(shmCreate) Error creating named object for %s.\nnobCreate: ",
            name) ;
        return (errno) ;

    }


/*******************************************************************************
    Create a SharedMemory object for the shared memory segment.  (Under
    VxWorks, the SharedMemory object was created or retrieved above.)
*******************************************************************************/

#ifdef VXWORKS

    *memory = (SharedMemory) segment ;

#else

  { struct  shmid_ds  shmem_info ;

    *memory = (SharedMemory) malloc (sizeof (_SharedMemory)) ;
    if (*memory == NULL) {
        LGE "(shmCreate) Error creating shared memory object for %s.\nmalloc: ",
            name) ;
        return (errno) ;
    }

    (*memory)->object = object ;
    (*memory)->ipc_id = (int) segment ;

/* Map to the share memory. */

    (*memory)->address = shmat ((int) segment,
                                (address == NULL) ? NULL : (char *) *address,
                                SHM_RND) ;
    if ((*memory)->address == (char *) -1) {
        LGE "(shmCreate) Error mapping to %s shared memory.\nshmat: ", name) ;
        return (errno) ;
    }

/* Retrieve the size of the shared memory. */

    if (shmctl ((int) segment, IPC_STAT, &shmem_info)) {
        LGE "(shmCreate) Error getting %s shared memory status.\nshmctl: ",
            name) ;
        return (errno) ;
    }
    (*memory)->size = shmem_info.shm_segsz ;
  }

#endif


    if (address != NULL)  *address = (*memory)->address ;


    LGI "(shmCreate)  Segment: %s  Address: %p  Size: %ld bytes\n",
        name, (*memory)->address, (*memory)->size) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    shmDestroy ()

    Delete a Shared Memory Segment.


Purpose:

    Function shmDestroy() detaches the current process from a shared
    memory segment and, if no more processes remain attached to the
    segment, deletes the segment.


    Invocation:

        status = shmDestroy (memory) ;

    where:

        <memory>	- I
            is the shared memory handle returned by shmCreate().
        <status>	- O
            returns the status of deleting the shared memory, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  shmDestroy (

#    if PROTOTYPES
        SharedMemory  memory)
#    else
        memory)

        SharedMemory  memory ;
#    endif

{

    if (memory == NULL)  return (0) ;

    LGI "(shmDestroy) Deleting %s shared memory (%d).\n",
        nobName (memory->object), nobCount (memory->object)) ;

/* Detach the shared memory segment from the process. */

#ifdef VXWORKS
    /* Not Necessary */
#else
    if (memory->address != NULL)  shmdt (memory->address) ;
#endif

/* Delete the shared memory segment's named object. */

    if (!nobDestroy (memory->object)) {		/* Last attached process? */

#if defined(VxMP)
        if (memory->address != NULL)  smMemFree (memory->address) ;
        smMemFree (memory) ;
#elif defined(VXWORKS)
        if (memory->address != NULL)  free (memory->address) ;
        free (memory) ;
#else
        if (shmctl (memory->ipc_id, IPC_RMID, NULL)) {
            LGE "(shmDestroy) Error deleting shared memory %d.\nshmctl: ",
                memory->ipc_id) ;
            return (errno) ;
        }
#endif

    } else if (errno != EWOULDBLOCK) {

        LGE "(shmDestroy) Error deleting named object.\nnobDestroy: ") ;
        return (errno) ;

    }

/* Delete the segment's SharedMemory object. */

#ifndef VXWORKS
    free (memory) ;
#endif

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    shmId ()

    Return the IPC Identifier for a Shared Memory Segment.


Purpose:

    Function shmId() returns the system IPC identifier for a shared memory
    segment.


    Invocation:

        identifier = shmId (memory) ;

    where:

        <memory>	- I
            is the shared memory handle returned by shmCreate().
        <identifier>	- O
            returns the system IPC identifier for the shared memory segment;
            -1 is returned if the operating system doesn't use IPC IDs.

*******************************************************************************/


int  shmId (

#    if PROTOTYPES
        SharedMemory  memory)
#    else
        memory)

        SharedMemory  memory ;
#    endif

{

#ifdef VXWORKS
    return (-1) ;
#else
    return ((memory == NULL) ? -1 : memory->ipc_id) ;
#endif

}

/*!*****************************************************************************

Procedure:

    shmLoad ()

    Load a Shared Memory Segment from a File.


Purpose:

    Function shmLoad() loads the binary contents of a shared memory
    segment from a disk file.  The contents must have been previously
    saved using shmSave().


    Invocation:

        status = shmLoad (memory, fileName) ;

    where:

        <memory>	- I
            is the shared memory handle returned by shmCreate().
        <fileName>	- I
            is the name of the file from which the shared memory contents
            will be loaded.  Environment variables may be embedded in the
            file name.
        <status>	- O
            returns the status of loading the segment from a file, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


int  shmLoad (

#    if PROTOTYPES
        SharedMemory  memory,
        const  char  *fileName)
#    else
        memory, fileName)

        SharedMemory  memory ;
        char  *fileName ;
#    endif

{    /* Local variables. */
    FILE  *file ;
    long  file_size ;
    struct  stat  info ;



/* Open the input file. */

    fileName = fnmBuild (FnmPath, fileName, NULL) ;
    file = fopen (fileName, "rb") ;
    if (file == NULL) {
        LGE "(shmLoad) Error opening %s to load %s shared memory.\nfopen: ",
            fileName, nobName (memory->object)) ;
        return (errno) ;
    }

/* Generate a warning message if the sizes of the disk file and the shared
   memory segment are not equal. */

    if (fstat (fileno (file), &info)) {
        LGE "(shmLoad) Error determining size of %s for %s shared memory.\nfstat: ",
            fileName, nobName (memory->object)) ;
        return (errno) ;
    }
    file_size = info.st_size ;
    if (file_size != memory->size) {
        LGE "(shmLoad) Unequal sizes - %s shared memory: %ld bytes  %s: %ld bytes\n",
            nobName (memory->object), memory->size, fileName, file_size) ;
    }

/* Read the (possibly truncated) contents of the file into shared memory. */

    if (file_size > memory->size)  file_size = memory->size ;
    if (fread (memory->address, file_size, 1, file) != 1) {
        LGE "(shmLoad) Error reading %ld-byte %s shared memory from %s.\nfread: ",
            file_size, nobName (memory->object), fileName) ;
        return (errno) ;
    }

/* Close the file. */

    fclose (file) ;

    LGI "(shmLoad) Loaded %s shared memory: %s\n",
        nobName (memory->object), fileName) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    shmSave ()

    Save a Shared Memory Segment to a File.


Purpose:

    Function shmSave() saves the binary contents of a shared memory
    segment to a disk file.  The contents can be reloaded at a later
    time using shmLoad().


    Invocation:

        status = shmSave (memory, fileName) ;

    where:

        <memory>	- I
            is the shared memory handle returned by shmCreate().
        <fileName>	- I
            is the name of the file to which the shared memory contents
            are saved.  Environment variables may be embedded in the
            file name.
        <status>	- O
            returns the status of saving the segment to a file, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


int  shmSave (

#    if PROTOTYPES
        SharedMemory  memory,
        const  char  *fileName)
#    else
        memory, fileName)

        SharedMemory  memory ;
        char  *fileName ;
#    endif

{

    return (meoSave (memory->address, memory->size, fileName, 0)) ;

}

/*!*****************************************************************************

Procedure:

    shmSizeOf ()

    Return the Size of a Shared Memory Segment.


Purpose:

    Function shmSizeOf() returns the size of a shared memory segment.


    Invocation:

        numBytes = shmSizeOf (memory) ;

    where:

        <memory>	- I
            is the shared memory handle returned by shmCreate().
        <numBytes>	- O
            returns the size of the shared memory segment; -1 is returned
            if an invalid handle was specified.

*******************************************************************************/


long  shmSizeOf (

#    if PROTOTYPES
        SharedMemory  memory)
#    else
        memory)

        SharedMemory  memory ;
#    endif

{

    return ((memory == NULL) ? -1 : memory->size) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the SHM_UTIL functions.

    Under UNIX:
        compile and link as follows:
            % cc -DTEST shm_util.c <libraries> -o shm_test
        and run with the following command line:
            % shm_test <shared_memory_name>

*******************************************************************************/

#ifdef VXWORKS

    void  shm_test (
        char  *commandLine)

#else

    main (argc, argv)
        int  argc ;
        char  *argv[] ;

#endif

{    /* Local variables. */
    SharedMemory  memory1, memory2 ;




#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("shm_util", commandLine, &argc, &argv) ;
#endif

    shm_util_debug = 1 ;

    if (argc < 2) {
        fprintf (stderr, "Usage:  shm_test  <shared_memory_name>\n") ;
        exit (EINVAL) ;
    }

    if (shmCreate (argv[1], 16384, NULL, &memory1)) {
        LGE "[SHM_TEST] Error creating %s shared memory.\nshmCreate: ",
            argv[1]) ;
        exit (errno) ;
    }

    if (shmCreate (argv[1], 0, NULL, &memory2)) {
        LGE "[SHM_TEST] Error creating existing %s shared memory.\nshmCreate: ",
                 argv[1]) ;
        exit (errno) ;
    }

    printf ("Shared memory (%d) located at %p.\n",
            shmId (memory1), shmAddress (memory1)) ;

    printf ("Shared memory (%d) located at %p.\n",
            shmId (memory2), shmAddress (memory2)) ;

    shmDestroy (memory1) ;
    shmDestroy (memory2) ;

}

#endif  /* TEST */
