/* $Id: msq_util.c,v 1.14 2004/02/16 20:53:04 alex Exp $ */
/*******************************************************************************

File:

    msq_util.c

    Message Queue Utilities.


Author:    Alex Measday


Purpose:

    The MSQ utilities provide a high-level interface to the underlying
    operating system's message queue facility.

    Creating a message queue (or access an existing message queue) is as simple
    as this:

        #include  "msq_util.h"
        MessageQueue  queue ;
        ...
        msqOpen ("my_queue", &queue) ;
        ...

    Reading messages from and writing messages to a message queue are
    equally simple, as shown in this loopback example:

        char  buffer[MAX_SIZE] ;
        int  length ;
        ...
        sprintf (buffer, "Hello!") ;		-- Send a message.
        msqWrite (queue, -1.0, buffer, -1) ;
        ...					-- Read the message.
        msqRead (queue, -1.0, sizeof buffer, buffer, &length) ;
        ...

    Timeouts can be specified so that msqRead() or msqWrite() will
    return if the timeout interval expires without the intended
    operation completing.  In the examples above, a timeout of -1.0
    causes the calling process to wait as long as is necessary to
    read from or write to the queue.

    When a message queue is no longer needed by a process, it should be
    deleted:

        msqClose (queue) ;

    The message queue isn't actually deleted from the system until the
    last process using it deletes it.


Notes (UNIX):

    The UNIX message queue functions, MSGGET(2) et al, are used to create
    and access message queues.  The name/IPC identifier mappings and
    reference counts are stored in the named object database (see NOB_UTIL.C).

    Timeouts in msqRead() and msqWrite() are implemented by polling the
    queue every second to see if it is ready for reading or writing.
    Consequently, timeout intervals have a one-second resolution under
    UNIX; fractions of seconds are essentially truncated.

    Processes should delete all message queues before exiting; if a
    process exits prematurely, the named object database could be left
    in an inconsistent state.


Notes (VxWorks):

    Under VxMP, the message queues are accessible on other CPUs (see
    "msgQSmLib(1)").  The name/identifier mappings and reference counts
    are stored in the named object database (see NOB_UTIL.C).

    Timeouts in the msqRead() call are supported.  Note that the timeout
    error code returned by the VxWorks system call is converted into an
    EWOULDBLOCK error code.

    Processes should delete all message queues before exiting; if a
    process exits prematurely, the named object database could be left
    in an inconsistent state.


Public Procedures:

    msqClose() - deletes a message queue.
    msqId() - returns the IPC identifier for a message queue.
    msqOpen() - creates a message queue.
    msqPoll() - returns the number of messages waiting to be read from a queue.
    msqRead() - reads the next message from a message queue.
    msqWrite() - writes a message to a message queue.

Public Variables:

    MSQ_MAX_MESSAGES - is the maximum number of messages in a queue.
    MSQ_MAX_LENGTH - is the maximum length of a single message.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#if defined(VMS)
    /* ... */
#elif defined(VXWORKS)
#    include  <msgQLib.h>		/* Message queue definitions. */
#    ifdef VxMP
#        include  <msgQSmLib.h>		/* Shared message queue definitions. */
#    endif
#    include  <sysLib.h>		/* System library definitions. */
#else
#    include  <unistd.h>		/* UNIX I/O definitions. */
#    include  <sys/types.h>		/* System type definitions. */
#    include  <sys/ipc.h>		/* Inter-process communication definitions. */
#    include  <sys/msg.h>		/* Message queue definitions. */
#endif
#if defined(HAVE_MEMCPY) && !HAVE_MEMCPY
#    define  memmove(dest,src,length)  bcopy(src,dest,length)
#endif
#include  "nob_util.h"			/* Named object definitions. */
#include  "msq_util.h"			/* Message queue utility definitions. */


/*******************************************************************************
    Message Queue -
*******************************************************************************/

typedef  struct  _MessageQueue {
    NamedObject  object ;		/* Handle of queue's named object. */
#ifdef VXWORKS
    MSG_Q_ID  id ;			/* System ID for the message queue. */
#else
    int  id ;				/* System IPC ID for the queue. */
#endif
}  _MessageQueue ;


int  msq_max_messages = 16 ;		/* Message queue sizing parameters. */
int  msq_max_length = 256 ;
int  msq_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  msq_util_debug

/*******************************************************************************

Procedure:

    msqOpen ()

    Create a Message Queue.


Purpose:

    Function msqOpen() creates a new message queue or, if the queue
    already exists, establishes access to the message queue.

    The size of the queue can be varied by setting global variables,
    MSQ_MAX_MESSAGES (the maximum number of messages allowed in the
    queue) and MSQ_MAX_LENGTH (the maximum length of a message) prior
    to calling msqOpen().  These parameters are automatically reset
    back to their defaults (16 and 256, respectively) upon return from
    msqOpen().


    Invocation:

        status = msqOpen (name, &queue) ;

    where:

        <name>		- I
            is the name of the message queue.
        <queue>		- O
            returns a handle for the message queue that is to be used
            in calls to the other MSQ_UTIL functions.
        <status>	- O
            returns the status of creating or establishing access to the
            message queue: zero if there were no errors and ERRNO otherwise.

    Public Variables:

        msq_max_messages - is the maximum number of messages allowed in the
            queue being created; the default is 16.  (VxWorks only; ignored
            under UNIX)

        msq_max_length - is the maximum length in bytes of a single message
            in the queue being created; the default is 256.  (VxWorks only;
            ignored under UNIX)

*******************************************************************************/


int  msqOpen (

#    if PROTOTYPES
        const  char  *name,
        MessageQueue  *queue)
#    else
        name, queue)

        const  char  *name ;
        MessageQueue  *queue ;
#    endif

{    /* Local variables. */
    NamedObject  object ;
#ifdef VXWORKS
    int  maxLength, maxMessages ;
    MSG_Q_ID  id ;
#else
    int  id ;
#endif




/* Under VxWorks, choose between the user-specified and default queue
   sizing parameters.  Then, reset the parameters to the defaults for
   subsequent calls. */

#ifdef VXWORKS
    maxMessages = (msq_max_messages > 0) ? msq_max_messages : 16 ;
    msq_max_messages = -1 ;
    maxLength = (msq_max_length > 0) ? msq_max_length : 256 ;
    msq_max_length = -1 ;
#endif


/*******************************************************************************
    Create a named object for the message queue.
*******************************************************************************/

    if (!nobCreate (name, multiCPU, &object)) {		/* Brand new? */

/* Create a new message queue. */

#if defined(VxMP)
        id = msgQSmCreate (maxMessages, maxLength, MSG_Q_FIFO) ;
        if (id == NULL) {
            LGE "(msqOpen) Error creating %s message queue.\nmsgQSmCreate: ",
                name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
#elif defined(VXWORKS)
        id = msgQCreate (maxMessages, maxLength, MSG_Q_FIFO) ;
        if (id == NULL) {
            LGE "(msqOpen) Error creating %s message queue.\nmsgQCreate: ",
                name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
#else
        id = msgget (IPC_PRIVATE, (IPC_CREAT | 0620)) ;
        if (id < 0) {
            LGE "(msqOpen) Error creating %s message queue.\nmsgget: ", name) ;
            PUSH_ERRNO ;  nobAbort (object) ;  POP_ERRNO ;
            return (errno) ;
        }
#endif

/* Add the name/ID mapping for the message queue to the named object database. */

        if (nobCommit (object, (void *) id)) {
            LGE "(msqOpen) Error commiting named object for %s.\nnobCommit: ",
                name) ;
            return (errno) ;
        }

    }

/* If the named object already exists, its value is the ID of the existing
   message queue. */

    else if (errno == EEXIST) {

#ifdef VXWORKS
        id = (MSG_Q_ID) nobValue (object) ;
#else
        id = (int) nobValue (object) ;
#endif

    } else {

        LGE "(msqOpen) Error creating named object for %s.\nnobCreate: ",
            name) ;
        return (errno) ;

    }


/*******************************************************************************
    Create a MSQ object for the message queue.
*******************************************************************************/

    *queue = (MessageQueue) malloc (sizeof (_MessageQueue)) ;
    if (*queue == NULL) {
        LGE "(msqOpen) Error creating message queue object for %s.\nmalloc: ",
            name) ;
        return (errno) ;
    }

    (*queue)->object = object ;
    (*queue)->id = id ;


#ifdef VXWORKS
    LGI "(msqOpen)  Message Queue: %s  ID: %p\n", name, id) ;
#else
    LGI "(msqOpen)  Message Queue: %s  ID: %d\n", name, id) ;
#endif

    return (0) ;

}

/*******************************************************************************

Procedure:

    msqClose ()

    Delete a Message Queue.


Purpose:

    Function msqClose() terminates a process's access to a message queue
    and, if no more processes are using the queue, deletes the message
    queue from the system.


    Invocation:

        status = msqClose (queue) ;

    where:

        <queue>		- I
            is the message queue handle returned by msqOpen().
        <status>	- O
            returns the status of deleting the message queue:
            zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  msqClose (

#    if PROTOTYPES
        MessageQueue  queue)
#    else
        queue)

        MessageQueue  queue ;
#    endif

{

    if (queue == NULL)  return (0) ;

    LGI "(msqClose) Deleting %s message queue (%d).\n",
        nobName (queue->object), nobCount (queue->object)) ;

/* Delete the message queue's named object. */

    if (!nobDestroy (queue->object)) {		/* Last process using it? */

#if defined(VXWORKS)
        if (msgQDelete (queue->id) == ERROR) {
            LGE "(msqClose) Error deleting message queue %p.\nmsgQDelete: ",
                queue->id) ;
#else
        if (msgctl (queue->id, IPC_RMID, NULL)) {
            LGE "(msqClose) Error deleting message queue %d.\nmsgctl: ",
                queue->id) ;
#endif
            return (errno) ;
        }

    } else if (errno != EWOULDBLOCK) {

        LGE "(msqClose) Error deleting named object.\nnobDestroy: ") ;
        return (errno) ;

    }

/* Delete the message queue's MSQ object. */

    free (queue) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    msqId ()

    Return the IPC Identifier for a Message Queue.


Purpose:

    Function msqId() returns the system IPC identifier for a message queue.


    Invocation:

        identifier = msqId (queue) ;

    where:

        <queue>		- I
            is the message queue handle returned by msqOpen().
        <identifier>	- O
            returns the system IPC identifier for the message queue;
            -1 is returned if the operating system doesn't use IPC IDs.

*******************************************************************************/


int  msqId (

#    if PROTOTYPES
        MessageQueue  queue)
#    else
        queue)

        MessageQueue  queue ;
#    endif

{
#ifdef VXWORKS
    return ((queue == NULL) ? -1 : (int) queue->id) ;
#else
    return ((queue == NULL) ? -1 : queue->id) ;
#endif
}

/*******************************************************************************

Procedure:

    msqPoll ()

    Return the Number of Pending Messages in a Message Queue.


Purpose:

    Function msqPoll() returns the number of messages waiting to be read
    from a message queue.


    Invocation:

        num_messages = msqPoll (queue) ;

    where:

        <queue>		- I
            is the message queue handle returned by msqOpen().
        <num_messages>	- O
            returns the number of messages waiting to be read from the
            message queue; -1 is returned in the event of an error.

*******************************************************************************/


int  msqPoll (

#    if PROTOTYPES
        MessageQueue  queue)
#    else
        queue)

        MessageQueue  queue ;
#    endif

{

    if (queue == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(msqPoll) NULL message queue handle: ") ;
        return (-1) ;
    }

#if defined(VXWORKS)
  { int  num_messages ;

    num_messages = msgQNumMsgs (queue->id) ;
    if (num_messages == ERROR) {
        LGE "(msqPoll) Error polling %s.\nmsgQNumMsgs: ",
            nobName (queue->object)) ;
        return (-1) ;
    }
    return (num_messages) ;
  }
#else
  { struct  msqid_ds  info ;

    if (msgctl (queue->id, IPC_STAT, &info)) {
        LGE "(msqPoll) Error polling %s.\nmsgctl: ", nobName (queue->object)) ;
        return (-1) ;
    }
    return (info.msg_qnum) ;
  }
#endif

}

/*******************************************************************************

Procedure:

    msqRead ()

    Read the Next Message from a Message Queue.


Purpose:

    Function msqRead() reads the next message from a message queue.


    Invocation:

        status = msqRead (queue, timeout, maxLength, &message, &length) ;

    where:

        <queue>		- I
            is the message queue handle returned by msqOpen().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the next message to be received
            in the queue.  A fractional time can be specified; e.g.,
            2.5 seconds.  A negative timeout (e.g., -1.0) causes an
            infinite wait; a zero timeout (0.0) returns immediately,
            reading a message only if one is already available in the
            queue.
        <maxLength>	- I
            specifies the size of the caller's message buffer.
        <message>	- O
            is the address of a buffer into which is copied the next
            message read from the queue.  The message is truncated if
            the buffer is smaller than the incoming message.
        <length>	- O
            returns the length of the input message; if the message was
            truncated, this argument returns the truncated length.
        <status>	- O
            returns the status of reading from the message queue:
            zero if no errors occurred, EWOULDBLOCK if the timeout
            interval expired with no message being read, and ERRNO
            otherwise.

*******************************************************************************/


int  msqRead (

#    if PROTOTYPES
        MessageQueue  queue,
        double  timeout,
        int  maxLength,
        char  *message,
        int  *length)
#    else
        queue, timeout, maxLength, message, length)

        MessageQueue  queue ;
        double  timeout ;
        int  maxLength ;
        char  *message ;
        int  *length ;
#    endif

{    /* Local variables. */
#ifdef VXWORKS
    int  ticksToWait ;
#else
    int  timeRemaining ;
#endif




    if (queue == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(msqRead) NULL message queue handle: ") ;
        return (errno) ;
    }

#ifdef VXWORKS

/*******************************************************************************
    Read the next message from the message queue.  (VxWorks)
*******************************************************************************/

    if (timeout < 0.0)
        ticksToWait = WAIT_FOREVER ;
    else if (timeout == 0.0)
        ticksToWait = NO_WAIT ;
    else
        ticksToWait = (int) (timeout * (double) sysClkRateGet ()  +  0.5) ;

    *length = msgQReceive (queue->id, message, maxLength, ticksToWait) ;
    if (*length == ERROR) {
        LGE "(msqRead) Error reading message from %s.\nmsgQReceive: ",
            nobName (queue->object)) ;
        if ((errno == S_objLib_OBJ_UNAVAILABLE) ||
            (errno == S_objLib_OBJ_TIMEOUT))  SET_ERRNO (EWOULDBLOCK) ;
        return (errno) ;
    }

#else

/*******************************************************************************
    Read the next message from the message queue.  (UNIX)
*******************************************************************************/

/* Poll the message queue until a message can be read or we timeout.
   Note that the message queue facility inserts a message type code
   (of type "long") in the incoming message; this field is removed
   before the message is returned to the caller. */

    timeRemaining = timeout ;
    for ( ; ; ) {
        *length = msgrcv (queue->id, message, maxLength,
                          0, MSG_NOERROR | IPC_NOWAIT) ;
        if (*length >= 0)  break ;		/* Successful read? */
        if (errno != ENOMSG)  break ;		/* Error? */
        SET_ERRNO (EWOULDBLOCK) ;		/* Timeout? */
        if ((timeout >= 0.0) && (--timeRemaining <= 0))  break ;
        sleep (1) ;
    }

    if (*length < 0) {
        LGE "(msqRead) Error reading message from %s.\nmsgrcv: ",
            nobName (queue->object)) ;
        return (-1) ;
    }

/* Move the message text down a few bytes to remove the message type code
   inserted by the message queue facility. */

    memmove (message, message + sizeof (long), *length) ;

#endif

					/* Append null terminator to message. */
    if (*length < maxLength)  message[*length] = '\0' ;

    LGI "(msqRead) Read %d-byte message from %s.\n",
        *length, nobName (queue->object)) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    msqWrite ()

    Write a Message to a Message Queue.


Purpose:

    Function msqWrite() writes a message to a message queue.


    Invocation:

        status = msqWrite (queue, timeout, length, message) ;

    where:

        <queue>		- I
            is the message queue handle returned by msqOpen().
        <timeout>	- I
            specifies the maximum amount of time (in seconds) that the
            caller wishes to wait for the message to be written to the
            queue when the queue is full.  A fractional time can be
            specified; e.g., 2.5 seconds.  A negative timeout (e.g.,
            -1.0) causes an infinite wait; a zero timeout (0.0) returns
            immediately if the queue is full.
        <length>	- I
            is the length of the message being output.  If this argument
            is negative (e.g., -1), the message is assumed to be a null-
            terminated string whose length is computed automatically.
        <message>	- I
            is the message to be written to the queue.
        <status>	- O
            returns the status of writing to the message queue: zero if
            no errors occurred, EWOULDBLOCK if the timeout interval expired
            without the message being written, and ERRNO otherwise.

*******************************************************************************/


int  msqWrite (

#    if PROTOTYPES
        MessageQueue  queue,
        double  timeout,
        int  length,
        const  char  *message)
#    else
        queue, timeout, length, message)

        MessageQueue  queue ;
        double  timeout ;
        int  length ;
        char  *message ;
#    endif

{    /* Local variables. */
#ifdef VXWORKS
    int  ticksToWait ;
#else
    char  *typedMessage ;
    int  timeRemaining, typedLength ;
#endif




    if (queue == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(msqWrite) NULL message queue handle: ") ;
        return (errno) ;
    }

    if ((length < 0) && (message != NULL))  length = strlen (message) ;

#ifdef VXWORKS

/*******************************************************************************
    Write the message to the message queue.  (VxWorks)
*******************************************************************************/

    if (timeout < 0.0)
        ticksToWait = WAIT_FOREVER ;
    else if (timeout == 0.0)
        ticksToWait = NO_WAIT ;
    else
        ticksToWait = (int) (timeout * (double) sysClkRateGet ()  +  0.5) ;

    status = msgQSend (queue->id, (char *) message, length,
                       ticksToWait, MSG_PRI_NORMAL) ;
    if (status == ERROR) {
        LGE "(msqWrite) Error sending %d-byte message to %s.\nmsgQSend: ",
            length, nobName (queue->object)) ;
        if ((errno == S_objLib_OBJ_UNAVAILABLE) ||
            (errno == S_objLib_OBJ_TIMEOUT))  SET_ERRNO (EWOULDBLOCK) ;
        return (errno) ;
    }

#else

/*******************************************************************************
    Write the message to the message queue.  (UNIX)
*******************************************************************************/

/* The message queue facility expects a type code at the beginning of the
   message, so insert one. */

    typedLength = sizeof (long) + length ;
    typedMessage = (char *) malloc (typedLength) ;
    if (typedMessage == NULL) {
        LGE "(msqWrite) Error constructing %d-byte output message for %s.\nmalloc: ",
            typedLength, nobName (queue->object)) ;
        return (errno) ;
    }
    *((long *) typedMessage) = 1 ;		/* Message type. */
    memcpy (&typedMessage[sizeof (long)],	/* Message text. */
            message, length) ;

/* Repeatedly attempt to send the message until the message is written or
   we timeout. */

    timeRemaining = timeout ;
    for ( ; ; ) {
        if (!msgsnd (queue->id, typedMessage, length, IPC_NOWAIT)) {
            SET_ERRNO (0) ;  break ;		/* Sucessful write. */
        }
        if (errno != EAGAIN)  break ;		/* Error? */
        SET_ERRNO (EWOULDBLOCK) ;		/* Timeout? */
        if ((timeout >= 0.0) && (--timeRemaining <= 0))  break ;
        sleep (1) ;
    }

    PUSH_ERRNO ;  free (typedMessage) ;  POP_ERRNO ;

    if (errno) {
        LGE "(msqWrite) Error sending %d-byte message to %s.\nmsgsnd: ",
            length, nobName (queue->object)) ;
        return (-1) ;
    }

#endif


    if (msq_util_debug)  LGI "(msqWrite) Wrote %d-byte message to %s.\n",
                                 length, nobName (queue->object)) ;

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the MSQ_UTIL functions.

    Under UNIX:
        compile and link as follows:
            % cc -DTEST msq_util.c <libraries> -o msq_test
        and run with the following command line:
            % msq_test <message_queue_name>

*******************************************************************************/

#ifdef VXWORKS

    void  msq_test (
        char  *command_line)

#else

    main (argc, argv)
        int  argc ;
        char  *argv[] ;

#endif

{    /* Local variables. */
    char  buffer[128] ;
    int  i, length ;
    MessageQueue  queue1, queue2 ;




#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("msq_util", command_line, &argc, &argv) ;
#endif

    nob_util_debug = 1 ;
    msq_util_debug = 1 ;

    if (argc < 2) {
        fprintf (stderr, "Usage:  msq_test  <message_queue_name>\n") ;
        exit (EINVAL) ;
    }

    if (msqOpen (argv[1], &queue1)) {
        LGE "[MSQ_TEST] Error creating %s message queue.\nmsqOpen: ", argv[1]) ;
        exit (errno) ;
    }

    if (msqOpen (argv[1], &queue2)) {
        LGE "[MSQ_TEST] Error creating existing %s message queue.\nmsqOpen: ",
            argv[1]) ;
        exit (errno) ;
    }

    printf ("Message queue (%d).\n", msqId (queue1)) ;
    printf ("Message queue (%d).\n", msqId (queue2)) ;

    if (argc > 2) {
        for (i = 0 ;  i < atoi (argv[2]) ;  i++) {
            sprintf (buffer, "Message #%d", i) ;
            if (msqWrite (queue1, 30.0, buffer, -1))  break ;
        }
    } else {
        for ( ; ; ) {
            if (msqRead (queue2, 30.0, sizeof buffer, buffer, &length))
                break ;
            printf ("Pending: %d  Received: \"%.*s\"\n",
                    msqPoll (queue2), length, buffer) ;
        }
    }

    msqClose (queue1) ;
    msqClose (queue2) ;

}

#endif  /* TEST */
