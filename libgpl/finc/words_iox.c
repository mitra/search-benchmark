/* $Id: words_iox.c,v 1.2 2009/07/14 15:06:43 alex Exp alex $ */
/*******************************************************************************

File:

    words_iox.c

    I/O Event Dispatching Utilities.


Author:    Alex Measday


Purpose:

    The WORDS_IOX package defines words for monitoring and responding to
    network I/O events.

        <seconds> <user> <word> <dispatcher> IOX-AFTER
        <callback> IOX-CANCEL
        IOX-CREATE
        <callback> IOX-DISPATCHER
        <value> IOX-DEBUG
        <dispatcher> IOX-DESTROY
        <seconds> <user> <word> <dispatcher> IOX-EVERY
        <dispatcher> <timeout> IOX-MONITOR
        <socket> <mode> <user> <word> <dispatcher> IOX-ONIO
        <user> <word> <dispatcher> IOX-WHENIDLE


Public Procedures:

    buildWordsIOX() - registers the words with the FICL system.

Private Procedures:

    word_IOX_AFTER() - implements the IOX-AFTER word.
    word_IOX_CANCEL() - implements the IOX-CANCEL word.
    word_IOX_CREATE() - implements the IOX-CREATE word.
    word_IOX_DEBUG() - implements the IOX-DEBUG word.
    word_IOX_DESTROY() - implements the IOX-DESTROY word.
    word_IOX_DISPATCHER() - implements the IOX-DISPATCHER word.
    word_IOX_EVERY() - implements the IOX-EVERY word.
    word_IOX_MONITOR() - implements the IOX-MONITOR word.
    word_IOX_ONIO() - implements the IOX-ONIO word.
    word_IOX_WHENIDLE() - implements the IOX-WHENIDLE word.
    wordIOXCB() - handles I/O event callbacks.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    FoxCallback - binds a Ficl word to an IOX callback.
*******************************************************************************/

typedef  struct  FoxCallback {
    IoxCallback  callback ;		/* The registered IOX callback. */
    ficlVm  *vm ;			/* Ficl virtual machine. */
    ficlWord  *word ;			/* Ficl word to execute on callback. */
    void  *parameter ;			/* User data to pass to Ficl word. */
}  FoxCallback ;


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_IOX_AFTER P_((ficlVm *vm)) ;
static  void  word_IOX_CANCEL P_((ficlVm *vm)) ;
static  void  word_IOX_CREATE P_((ficlVm *vm)) ;
static  void  word_IOX_DEBUG P_((ficlVm *vm)) ;
static  void  word_IOX_DESTROY P_((ficlVm *vm)) ;
static  void  word_IOX_DISPATCHER P_((ficlVm *vm)) ;
static  void  word_IOX_EVERY P_((ficlVm *vm)) ;
static  void  word_IOX_MONITOR P_((ficlVm *vm)) ;
static  void  word_IOX_ONIO P_((ficlVm *vm)) ;
static  void  word_IOX_WHENIDLE P_((ficlVm *vm)) ;

static  errno_t  wordIOXCB (
#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    buildWordsIOX ()

    Enter the IOX Words into the Dictionary.


Purpose:

    Function buildWordsIOX() enters the IOX words into the system dictionary.


    Invocation:

        buildWordsIOX (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsIOX (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-AFTER", word_IOX_AFTER,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-CANCEL", word_IOX_CANCEL,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-CREATE", word_IOX_CREATE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-DEBUG", word_IOX_DEBUG,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-DESTROY", word_IOX_DESTROY,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-DISPATCHER", word_IOX_DISPATCHER,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-EVERY", word_IOX_EVERY,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-MONITOR", word_IOX_MONITOR,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-ONIO", word_IOX_ONIO,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "IOX-WHENIDLE", word_IOX_WHENIDLE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetConstant (ficlSystemGetDictionary (sys),
                               "IOX_READ", IoxRead) ;

    ficlDictionarySetConstant (ficlSystemGetDictionary (sys),
                               "IOX_WRITE", IoxWrite) ;

    ficlDictionarySetConstant (ficlSystemGetDictionary (sys),
                               "IOX_EXCEPT", IoxExcept) ;

    ficlDictionarySetConstant (ficlSystemGetDictionary (sys),
                               "IOX_IO", IoxIO) ;

    ficlDictionarySetConstant (ficlSystemGetDictionary (sys),
                               "IOX_FIRE", IoxFire) ;

    ficlDictionarySetConstant (ficlSystemGetDictionary (sys),
                               "IOX_IDLE", IoxIdle) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_AFTER ()

    Register a Single-Shot Timer.


Purpose:

    Function word_IOX_AFTER() implements the IOX-AFTER word, which registers
    a single-shot timer with an I/O event dispatcher.

        IOX-AFTER

            ( r c-addr xt dp -- cb )

        Register a single-shot timer of duration r seconds with I/O event
        dispatcher dp.  When the timer expires, user data c-addr will be
        pushed on the stack and execution token xt will be executed.  A
        handle, cb, is returned on the stack and can be used to cancel
        the callback with IOX-CANCEL.


    Invocation:

        word_IOX_AFTER (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_AFTER (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    double  interval ;
    ficlCell  cell ;
    ficlWord  *word ;
    FoxCallback  *fox ;
    IoxDispatcher  dispatcher ;
    void  *parameter ;



    FICL_STACK_CHECK (vm->dataStack, 3, 1) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Dispatcher. */
    dispatcher = (IoxDispatcher) cell.p ;
    cell = ficlVmPop (vm) ;			/* Execution token. */
    word = (ficlWord *) cell.p ;
    cell = ficlVmPop (vm) ;			/* User data. */
    parameter = (void *) cell.p ;
						/* Time interval. */
    interval = (double) ficlStackPopFloat (vm->floatStack) ;

/* Allocate a structure to hold the execution token and the user data;
   a pointer to this structure will be passed to wordIOXCB() when it
   is invoked for a callback. */

    fox = (FoxCallback *) malloc (sizeof (FoxCallback)) ;
    if (fox == NULL) {
        cell.p = NULL ;
        ficlVmPush (vm, cell) ;
        return ;
    }

    fox->callback = NULL ;
    fox->vm = vm ;
    fox->word = word ;
    fox->parameter = parameter ;

/* Register the timer with the dispatcher.  When the specified interval has
   elapsed, the dispatcher will call wordIOXCB(), which, in turn, will execute
   the word in the FoxCallback structure. */

    fox->callback = ioxAfter (dispatcher, wordIOXCB, fox, interval) ;
    if (fox->callback == NULL) {
        free (fox) ;
        fox = NULL ;
    }

/* Return the callback handle on the stack. */

    cell.p = fox ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_CANCEL ()

    Cancel a Callback.


Purpose:

    Function word_IOX_CANCEL() implements the IOX-CANCEL word, which
    cancels a previously registered callback.

        IOX-CANCEL

            ( cb -- ior )

        Cancel callback cb; the callback should no longer be referenced.
        The I/O result of cancelling the callback is returned on the stack.


    Invocation:

        word_IOX_CANCEL (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_CANCEL (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;
    FoxCallback  *fox ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Callback. */
    fox = (FoxCallback *) cell.p ;

/* Cancel the callback.  The dispatcher will invoke the callback with
   the IoxCancel reason and the callback will deallocate the FoxCallback
   structure. */

    ior = ioxCancel (fox->callback) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_CREATE ()

    Create an I/O Event Dispatcher.


Purpose:

    Function word_IOX_CREATE() implements the IOX-CREATE word, which
    creates an I/O event dispatcher.

        IOX-CREATE

            ( -- dp 0 | ior )

        Create an I/O event dispatcher.  The dispatcher is returned on
        the stack as dp, along with the I/O result ior.


    Invocation:

        word_IOX_CREATE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_CREATE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;
    IoxDispatcher  dispatcher ;



    FICL_STACK_CHECK (vm->dataStack, 0, 2) ;

/* Create the dispatcher. */

    ior = ioxCreate (&dispatcher) ;

/* Return the dispatcher and the I/O result on the stack. */

    if (ior == 0) {			/* Successfully created dispatcher? */
        cell.p = dispatcher ;
        ficlVmPush (vm, cell) ;
    }
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_DEBUG ()

    Enable/Disable I/O Event Dispatching Debug Output.


Purpose:

    Function word_IOX_DEBUG() implements the IOX-DEBUG word, which enables
    or disables I/O event dispatching debug.

        IOX-DEBUG

            ( n -- )

        Set the I/O event dispatching debug flag to n.  A value of 0
        disables debug; a non-zero value enables debug.  Debug is written
        to standard output.


    Invocation:

        word_IOX_DEBUG (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_DEBUG (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;



    FICL_STACK_CHECK (vm->dataStack, 1, 0) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Debug flag. */
    iox_util_debug = cell.i ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_DESTROY ()

    Destroy an I/O Event Dispatcher.


Purpose:

    Function word_IOX_DESTROY() implements the IOX-DESTROY word, which
    destroys an I/O event dispatcher.

        IOX-DESTROY

            ( dp -- ior )

        Destroy I/O event dispatcher dp; the dispatcher should no longer
        be referenced.


    Invocation:

        word_IOX_DESTROY (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_DESTROY (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    int  ior ;
    IoxDispatcher  dispatcher ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Dispatcher. */
    dispatcher = (IoxDispatcher) cell.p ;

/* Destroy the dispatcher. */

    ior = ioxDestroy (dispatcher) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_DISPATCHER ()

    Get a Callback's Dispatcher.


Purpose:

    Function word_IOX_DISPATCHER() implements the IOX-DISPATCHER word, which
    returns the dispatcher with which the callback is registered.

        IOX-DISPATCHER

            ( cb -- dp )

        Get callback cb's dispatcher dp.


    Invocation:

        word_IOX_DISPATCHER (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_DISPATCHER (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    FoxCallback  *fox ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Callback. */
    fox = (FoxCallback *) cell.p ;

/* Return the callback's dispatcher on the stack. */

    cell.p = ioxDispatcher (fox->callback) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_EVERY ()

    Register a Periodic Timer.


Purpose:

    Function word_IOX_EVERY() implements the IOX-EVERY word, which registers
    a periodic timer with an I/O event dispatcher.

        IOX-EVERY

            ( r c-addr xt dp -- cb )

        Register a periodic timer of interval r seconds with I/O event
        dispatcher dp.  Each time the timer fires, user data c-addr will
        be pushed on the stack and execution token xt will be executed.
        A handle, cb, is returned on the stack and can be used to cancel
        the callback with IOX-CANCEL.


    Invocation:

        word_IOX_EVERY (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_EVERY (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    double  interval ;
    ficlCell  cell ;
    ficlWord  *word ;
    FoxCallback  *fox ;
    IoxDispatcher  dispatcher ;
    void  *parameter ;



    FICL_STACK_CHECK (vm->dataStack, 3, 1) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Dispatcher. */
    dispatcher = (IoxDispatcher) cell.p ;
    cell = ficlVmPop (vm) ;			/* Execution token. */
    word = (ficlWord *) cell.p ;
    cell = ficlVmPop (vm) ;			/* User data. */
    parameter = (void *) cell.p ;
						/* Time interval. */
    interval = (double) ficlStackPopFloat (vm->floatStack) ;

/* Allocate a structure to hold the execution token and the user data;
   a pointer to this structure will be passed to wordIOXCB() when it
   is invoked for a callback. */

    fox = (FoxCallback *) malloc (sizeof (FoxCallback)) ;
    if (fox == NULL) {
        cell.p = NULL ;
        ficlVmPush (vm, cell) ;
        return ;
    }

    fox->callback = NULL ;
    fox->vm = vm ;
    fox->word = word ;
    fox->parameter = parameter ;

/* Register the timer with the dispatcher.  When the specified interval has
   elapsed, the dispatcher will call wordIOXCB(), which, in turn, will execute
   the word in the FoxCallback structure. */

    fox->callback = ioxEvery (dispatcher, wordIOXCB, fox, -1.0, interval) ;
    if (fox->callback == NULL) {
        free (fox) ;
        fox = NULL ;
    }

/* Return the callback handle on the stack. */

    cell.p = fox ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_MONITOR ()

    Monitor I/O Events.


Purpose:

    Function word_IOX_MONITOR() implements the IOX-MONITOR word, which
    passes control to a dispatcher to monitor I/O events, timers, and
    idle tasks:

        IOX-MONITOR

            ( dp r -- ior )

        Monitor and dispatch I/O events, timers, and idle tasks for r seconds
        using dispatcher dp.  If r is less than zero, the dispatcher will
        monitor events forever.  The I/O result of monitoring events is
        returned on the stack.


    Invocation:

        word_IOX_MONITOR (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_MONITOR (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    double  timeout ;
    ficlCell  cell ;
    int  ior ;
    IoxDispatcher  dispatcher ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;
    FICL_STACK_CHECK (vm->floatStack, 1, 0) ;

/* Get the argument from the stack. */
						/* Timeout. */
    timeout = (double) ficlStackPopFloat (vm->floatStack) ;
    cell = ficlVmPop (vm) ;			/* Dispatcher. */
    dispatcher = (IoxDispatcher) cell.p ;

/* Monitor events for the specified interval. */

    ior = ioxMonitor (dispatcher, timeout) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_ONIO ()

    Register an I/O Source.


Purpose:

    Function word_IOX_ONIO() implements the IOX-ONIO word, which registers
    an I/O source (e.g., a network connection) with an I/O event dispatcher.

        IOX-ONIO

            ( fd n c-addr xt dp -- cb )

        Register I/O file descriptor fd with I/O event dispatcher dp.  Mask n
        is the bit-wise OR of the types of I/O events to monitor: 0x1 for
        input-pending, 0x2 for output-ready, and 0x4 for OOB-input-pending.
        When an event of the monitored types is detected on the I/O source,
        user data c-addr will be pushed on the stack and execution token xt
        will be executed.  A handle, cb, is returned on the stack and can be
        used to cancel the callback with IOX-CANCEL.


    Invocation:

        word_IOX_ONIO (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_ONIO (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    ficlWord  *word ;
    FoxCallback  *fox ;
    IoFd  fd ;
    IoxDispatcher  dispatcher ;
    IoxReason  reason ;
    void  *parameter ;



    FICL_STACK_CHECK (vm->dataStack, 5, 1) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Dispatcher. */
    dispatcher = (IoxDispatcher) cell.p ;
    cell = ficlVmPop (vm) ;			/* Execution token. */
    word = (ficlWord *) cell.p ;
    cell = ficlVmPop (vm) ;			/* User data. */
    parameter = (void *) cell.p ;
    cell = ficlVmPop (vm) ;			/* Event types to monitor. */
    reason = (IoxReason) cell.i ;
    cell = ficlVmPop (vm) ;			/* File descriptor. */
    fd = (IoFd) cell.i ;

/* Allocate a structure to hold the execution token and the user data;
   a pointer to this structure will be passed to wordIOXCB() when it
   is invoked for a callback. */

    fox = (FoxCallback *) malloc (sizeof (FoxCallback)) ;
    if (fox == NULL) {
        cell.p = NULL ;
        ficlVmPush (vm, cell) ;
        return ;
    }

    fox->callback = NULL ;
    fox->vm = vm ;
    fox->word = word ;
    fox->parameter = parameter ;

/* Register the I/O source with the dispatcher.  When an I/O event of the
   specified type is detected on the source, the dispatcher will call
   wordIOXCB(), which, in turn, will execute the word in the FoxCallback
   structure. */

    fox->callback = ioxOnIO (dispatcher, wordIOXCB, fox, reason, fd) ;
    if (fox->callback == NULL) {
        free (fox) ;
        fox = NULL ;
    }

/* Return the callback handle on the stack. */

    cell.p = fox ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_IOX_WHENIDLE ()

    Register an Idle Task.


Purpose:

    Function word_IOX_WHENIDLE() implements the IOX-WHENIDLE word, which
    registers an idle task with an I/O event dispatcher.

        IOX-WHENIDLE

            ( c-addr xt dp -- cb )

        Register an idle task with I/O event dispatcher dp.  When the
        dispatcher is idle, user data c-addr will be pushed on the stack
        and execution token xt will be executed.  A handle, cb, is
        returned on the stack and can be used to cancel the callback
        with IOX-CANCEL.


    Invocation:

        word_IOX_WHENIDLE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_IOX_WHENIDLE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    ficlWord  *word ;
    FoxCallback  *fox ;
    IoxDispatcher  dispatcher ;
    void  *parameter ;



    FICL_STACK_CHECK (vm->dataStack, 3, 1) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Dispatcher. */
    dispatcher = (IoxDispatcher) cell.p ;
    cell = ficlVmPop (vm) ;			/* Execution token. */
    word = (ficlWord *) cell.p ;
    cell = ficlVmPop (vm) ;			/* User data. */
    parameter = (void *) cell.p ;

/* Allocate a structure to hold the execution token and the user data;
   a pointer to this structure will be passed to wordIOXCB() when it
   is invoked for a callback. */

    fox = (FoxCallback *) malloc (sizeof (FoxCallback)) ;
    if (fox == NULL) {
        cell.p = NULL ;
        ficlVmPush (vm, cell) ;
        return ;
    }

    fox->callback = NULL ;
    fox->vm = vm ;
    fox->word = word ;
    fox->parameter = parameter ;

/* Register the idle task with the dispatcher.  When the dispatcher is idle,
   it will call wordIOXCB(), which, in turn, will execute the word in the
   FoxCallback structure. */

    fox->callback = ioxWhenIdle (dispatcher, wordIOXCB, fox) ;
    if (fox->callback == NULL) {
        free (fox) ;
        fox = NULL ;
    }

/* Return the callback handle on the stack. */

    cell.p = fox ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    wordIOXCB ()

    Handle an I/O Event Dispatcher Callback.


Purpose:

    Function wordIOXCB() is a generic IOX handler function assigned to
    IOX callbacks when a to-be-monitored event is registered using an
    IOX word.  At the time the event was registered, a FoxCallback
    structure was created containing the execution token of a Ficl
    word to be executed when the callback is invoked and the user
    data to be passed to the word.

    When the callback is invoked by the I/O event dispatcher, wordIOXCB()
    pushes the FoxCallback address, the user data, and the callback reason
    on the stack, and then executes the Ficl word.


    Invocation:

        status = wordIOXCB (callback, reason, userData) ;

    where:

        <callback>	- I
            is the handle assigned to the callback by one of the IOX
            registration functions.
        <reason>	- I
            is the reason (e.g., IoxRead, IoxFire) the callback is being
            invoked.
        <userData>	- I
            is the address of the FoxCallback structure created when the
            callback was registered with the dispatcher.
        <status>	- O
            returns the status of handling the callback, zero if there
            were no errors and ERRNO otherwise.  The status value is
            ignored by the IOX dispatcher, but it may be useful if the
            application calls wordIOXCB() directly.

*******************************************************************************/


static  errno_t  wordIOXCB (

#    if PROTOTYPES
        IoxCallback  callback,
        IoxReason  reason,
        void  *userData)
#    else
        dispatcher, reason, userData)

        IoxCallback  callback ;
        IoxReason  reason ;
        void  *userData ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    FoxCallback  *fox = (FoxCallback *) userData ;



/* If the callback is being cancelled, then deallocate the FoxCallback
   structure. */

    if (reason == IoxCancel) {
        fox->callback = NULL ;
        fox->vm = NULL ;
        fox->word = NULL ;
        fox->parameter = NULL ;
        free (fox) ;
        return (0) ;
    }

/* Otherwise, execute the Ficl word bound to the callback, passing it the
   address of the FoxCallback structure, the user data, and the callback
   reason. */

    cell.p = fox ;
    ficlVmPush (fox->vm, cell) ;
    cell.p = fox->parameter ;
    ficlVmPush (fox->vm, cell) ;
    cell.i = reason ;
    ficlVmPush (fox->vm, cell) ;

    ficlVmExecuteXT (fox->vm, fox->word) ;

    return (0) ;

}
