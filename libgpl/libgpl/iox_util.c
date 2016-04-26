/* $Id: iox_util.c,v 1.2 2011/07/18 17:40:24 alex Exp $ */
/*******************************************************************************

File:

    iox_util.c

    I/O Event Dispatcher


Author:    Alex Measday


Purpose:

    The functions in this file implement an I/O event dispatcher.  Applications
    that make use of the IOX dispatcher are generally structured as follows:

          Perform any application-specific initialization activities.
          Register event handlers with the IOX event dispatcher.
          DO forever
              Wait for the next event.
              CALL the handler function bound to the event.
          ENDDO

    The event processing loop is encapsulated in an IOX function, ioxMonitor().
    Other IOX functions are available to:

      - Register an I/O source with the dispatcher.  When an I/O condition
        (input-pending, output-ready, OOB-input-pending) is detected on the
        source, the dispatcher automatically invokes an application-defined
        handler function to respond to the event (e.g., to read input from
        a network connection).

      - Register a single-shot or periodic timer with the dispatcher.  When
        the specified time interval has elapsed, the dispatcher automatically
        invokes an application-defined handler function to react to the timeout.

      - Register an idle task (an application-defined function) with the
        dispatcher.  When no I/O sources are active and no timers are ready
        to fire, the dispatcher will execute the next idle task on its queue.
        Idle tasks are intended to perform "background" work.

    I/O sources are registered with the NIX dispatcher by ioxOnIO(), timers
    by ioxAfter() and ioxEvery(), and idle tasks by ioxWhenIdle().  The
    monitoring of I/O sources, timers, and idle tasks is initiated by calling
    ioxMonitor().  Control can remain in ioxMonitor() for the lifetime of the
    application; the dispatcher loops forever, waiting for I/O and timer events
    and invoking the application-specified callback functions bound to the
    events.

    Alternatively, a time interval can be specified to limit how long
    ioxMonitor() monitors events.  The latter capability allows a form
    of polling and is useful when there are multiple dispatchers in an
    application (e.g., if the primary event dispatcher is a non-IOX dispatcher).


Notes:

    The IOX package was derived, 12 years later, from my NIX (Not Including X)
    package.  (And was influenced by my experience with my C++ Dispatcher
    class.)  The NIX functions were patterned after the corresponding X
    Toolkit functions.  Mimicing the X Toolkit names and function signatures
    was intended to make it easier to reuse X- and non-X-based event-related
    code (if only through "cut, paste, search, and replace").  Unfortunately,
    opportunities to take advantage of such reuse did not present themselves
    subsequently and, in the mean time, I was saddled with a klunky API.
    The IOX package offers a simpler, more concise API - I hope!

    The IOX monitoring function is implemented using a UNIX SELECT(2) call
    and it supports read, write, and exceptional I/O events.  The ancestral
    NIX package has been tested on various operating systems (Windows, Linux,
    UNIX, VMS, and VxWorks).  The IOX package is largely the same code and I
    will test it on the various platforms as time permits.  If you get the
    header files straight, the IOX package should work under other operating
    systems that support SELECT(2).

    The Windows WINSOCK and VMS UCX implementations of SELECT(2) only support
    socket I/O and not arbitrary device I/O as in UNIX.  In particular, you
    can't monitor standard input as an I/O source; I usually use IOX timers
    to periodically poll stdin for input.


Public Procedures (for dispatchers):

    ioxAfter() - registers a single-shot timer with the dispatcher.
    ioxCreate() - creates an I/O event dispatcher.
    ioxDestroy() - destroys an I/O event dispatcher.
    ioxEvery() - registers a periodic timer with the dispatcher.
    ioxMonitor() - monitors and responds to I/O events.
    ioxOnIO() - registers an I/O source with the dispatcher.
    ioxWhenIdle() - registers an idle task with the dispatcher.

Public Procedures (for callbacks):

    ioxCancel() - cancels a registered callback.
    ioxDepth() - gets the callback nesting of a callback's dispatcher.
    ioxDispatcher() - gets a callback's dispatcher.
    ioxExpiration() - gets a timer callback's expiration time.
    ioxFd() - gets an I/O callback's I/O source.
    ioxInterval() - gets a timer callback's time interval.
    ioxOnCancel() - sets a callback's invoke-on-cancel flag.

Private Procedures (for callbacks):

    ioxAdd() - adds a callback to its dispatcher's callback lists.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#if defined(VMS)
#    include  <libdtdef.h>		/* VMS date/time definitions. */
#    include  <socket.h>		/* Socket-related definitions. */
#    include  <stsdef.h>		/* VMS status code structure. */
#    include  "fd.h"			/* File descriptor set definitions. */
#elif defined(VXWORKS)
#    include  <selectLib.h>		/* SELECT(2) definitions. */
#endif
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "iox_util.h"			/* I/O event dispatcher definitions. */


/*******************************************************************************
    Callback - maps an event type to a handler function.  The "reason" mask
        specifies which event types are handled by the callback.  A callback
        registered via ioxOnIO() monitors a file descriptor for IoxRead (input
        pending), IoxWrite (output ready), and/or IoxExcept (OOB input pending)
        conditions.  Timer callbacks registered via ioxAfter() or ioxEvery()
        have a reason of IoxFire.  The expiration time is computed by adding
        the interval to the time of callback registration; the periodic flag
        indicates whether the callback is for a single-shot timer or a periodic
        timer.  Idle callbacks registered via ioxWhenIdle() have a reason of
        IoxIdle.
*******************************************************************************/

typedef  struct  _IoxCallback {
    IoxDispatcher  dispatcher ;		/* With whom callback is registered. */
    IoxReason  reason ;			/* Mask of event types handled by callback. */
    IoxHandler  handler ;		/* User's handler function. */
    void  *userData ;			/* Data passed to handler function. */
    bool  onCancel ;			/* Invoke callback on cancel? */
    IoFd  source ;			/* File descriptor (OnIO). */
    double  interval ;			/* Time interval in seconds (After, Every). */
    bool  periodic ;			/* Periodic timer (Every)? */
    struct  timeval  expiration ;	/* Absolute time of expiration (After, Every). */
    struct  _IoxCallback  *next ;
}  _IoxCallback ;


/*******************************************************************************
    Dispatcher - monitors the events for which callbacks have been registered.
*******************************************************************************/

typedef  struct  _IoxDispatcher {
    int  depth ;			/* Callback nesting. */
    _IoxCallback  *ioList ;		/* List of registered I/O sources. */
    _IoxCallback  *timerList ;		/* List of registered timers. */
    _IoxCallback  *idleQueue ;		/* Queue of registered idle callbacks. */
}  _IoxDispatcher ;


int  iox_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  iox_util_debug


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  void  ioxAdd (
#    if PROTOTYPES
        IoxCallback  callback
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    ioxAfter ()

    Register a Single-Shot Timer Callback.


Purpose:

    Function ioxAfter() registers a single-shot timer with the dispatcher.
    When the specified time interval expires, the caller's handler function
    is invoked with the IoxFire reason.  At a minimum, the specified time
    interval will elapse before the handler function is called; there is
    no guarantee on how soon the handler function will be called after the
    timer fires.  The dispatcher maintains the timers in a list sorted by
    expiration time.


    Invocation:

        callback = ioxAfter (dispatcher, handlerF, userData, interval) ;

    where:

        <dispatcher>	- I
            is the dispatcher handle returned by ioxCreate().
        <handlerF>	- I
            is the function that is to be called when the timeout interval
            expires.  The handler function should be declared as follows:
                int  handler_function (IoxCallback callback,
                                       IoxReason reason,
                                       void *userData) ;
            where "callback" is the callback handle returned by ioxAfter(),
            "reason" is IoxFire, and "userData" is the argument that was
            passed into ioxAfter().  The return value of the handler
            function is ignored by the dispatcher.
        <userData>	- I
            is a caller-supplied (VOID *) value that will be passed to the
            handler function when it is invoked.
        <interval>	- I
            specifies the timeout interval in seconds.  (This is a real
            number, so fractions of a second can be specified.)
        <callback>	- O
            returns a handle for the registered callback.  This handle is used
            in calls to the other (callback-related) IOX functions.  NULL is
            returned in the event of an error.

*******************************************************************************/


IoxCallback  ioxAfter (

#    if PROTOTYPES
        IoxDispatcher  dispatcher,
        IoxHandler  handlerF,
        void  *userData,
        double  interval)
#    else
        dispatcher, handlerF, userData, interval)

        IoxDispatcher  dispatcher ;
        IoxHandler  handlerF ;
        void  *userData ;
        double  interval ;
#    endif

{    /* Local variables. */
    IoxCallback  cb ;



    if (dispatcher == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxAfter) NULL dispatcher handle.\n") ;
        return (NULL) ;
    }

/* Allocate a callback structure for the timer. */

    cb = (IoxCallback) malloc (sizeof (_IoxCallback)) ;
    if (cb == NULL) {
        LGE "(ioxAfter) Error allocating callback structure.\nmalloc: ") ;
        return (NULL) ;
    }

    cb->dispatcher = dispatcher ;
    cb->reason = IoxFire ;
    cb->handler = handlerF ;
    cb->userData = userData ;
    cb->onCancel = false ;
    cb->source = INVALID_SOCKET ;
    cb->interval = interval ;
    cb->periodic = false ;
    cb->expiration = tvAdd (tvTOD (), tvCreateF (interval)) ;

/* Add the timer to the list of registered timers.  The list is sorted by
   expiration time. */

    ioxAdd (cb) ;

    LGI "(ioxAfter) Callback %p, handler %p, data %p, interval %g.\n",
        (void *) cb, (void *) handlerF, userData, interval) ;

    return (cb) ;

}

/*!*****************************************************************************

Procedure:

    ioxCreate ()

    Create an I/O Event Dispatcher.


Purpose:

    Function ioxCreate() creates a new I/O event dispatcher.


    Invocation:

        status = ioxCreate (&dispatcher) ;

    where:

        <dispatcher>	- O
            returns a handle for the new dispatcher.  This handle is used in
            calls to the other (dispatcher-related) IOX functions.
        <status>	- O
            returns the status of creating the dispatcher, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  ioxCreate (

#    if PROTOTYPES
        IoxDispatcher  *dispatcher)
#    else
        dispatcher)

        IoxDispatcher  *dispatcher ;
#    endif

{

    *dispatcher = (IoxDispatcher) malloc (sizeof (_IoxDispatcher)) ;
    if (*dispatcher == NULL) {
        LGE "(ioxCreate) Error allocating dispatcher structure.\nmalloc: ") ;
        return (errno) ;
    }

    (*dispatcher)->depth = 0 ;
    (*dispatcher)->ioList = NULL ;
    (*dispatcher)->timerList = NULL ;
    (*dispatcher)->idleQueue = NULL ;

    LGI "(ioxCreate) Created dispatcher %p.\n", (void *) *dispatcher) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    ioxDestroy ()

    Destroy an I/O Event Dispatcher.


Purpose:

    Function ioxDestroy() destroys an I/O event dispatcher.


    Invocation:

        status = ioxDestroy (dispatcher) ;

    where:

        <dispatcher>	- I
            is the dispatcher handle returned by ioxCreate().
        <status>	- O
            returns the status of destroying the dispatcher,
            zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  ioxDestroy (

#    if PROTOTYPES
        IoxDispatcher  dispatcher)
#    else
        dispatcher)

        IoxDispatcher  dispatcher ;
#    endif

{

    LGI "(ioxDestroy) Destroying dispatcher %p.\n", (void *) dispatcher) ;

    if (dispatcher == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxDestroy) NULL dispatcher handle.\n") ;
        return (errno) ;
    }

/* Remove the registered I/O sources. */

    while (dispatcher->ioList != NULL)
        ioxCancel (dispatcher->ioList) ;

/* Remove the registered timers. */

    while (dispatcher->timerList != NULL)
        ioxCancel (dispatcher->timerList) ;

/* Remove the registered idle tasks. */

    while (dispatcher->idleQueue != NULL)
        ioxCancel (dispatcher->idleQueue) ;

/* Finally, delete the dispatcher itself. */

    if (dispatcher->depth <= 0)  free (dispatcher) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    ioxEvery ()

    Register a Periodic Timer Callback.


Purpose:

    Function ioxEvery() registers a periodic timer with the dispatcher.
    When the specified time interval expires, the timer is "re-registered"
    for the next firing and the caller's handler function is invoked with
    the IoxFire reason.  At a minimum, the specified time interval will
    elapse before the handler function is called; there is no guarantee
    on how soon the handler function will be called after the timer fires.
    Since the timer is re-registered before the handler function is called,
    the firings should occur (more or less) on the original schedule, rather
    than being characterized by a creeping delay.

    This function is implemented by simply calling ioxAfter() to create a
    one-shot timer and then setting the timer's "periodic" flag to true.
    NOTE that the application must explicitly call ioxCancel() in order
    to stop the timer.


    Invocation:

        callback = ioxEvery (dispatcher, handlerF, userData, delay, interval) ;

    where:

        <dispatcher>	- I
            is the dispatcher handle returned by ioxCreate().
        <handlerF>	- I
            is the function that is to be called when the timeout interval
            expires.  The handler function should be declared as follows:
                int  handler_function (IoxCallback callback,
                                       IoxReason reason,
                                       void *userData) ;
            where "callback" is the callback handle returned by ioxEvery(),
            "reason" is IoxFire, and "userData" is the argument that was
            passed into ioxEvery().  The return value of the handler
            function is ignored by the dispatcher.
        <userData>	- I
            is a caller-supplied (VOID *) value that will be passed to the
            handler function when it is invoked.
        <delay>		- I
            specifies a delay in seconds before the callback is first invoked.
            (This is a real number, so fractions of a second can be specified.)
            Thereafter, the timer will fire periodically at the interval
            specified below.  A negative delay (e.g., -1.0) means the first
            invocation of the callback will happen after the normal interval
            has elapsed.  A zero delay (0.0) causes an immediate invocation
            of the callback, after which the timer willl fire at the specified
            intervals.
        <interval>	- I
            specifies the timeout interval in seconds.  (This is a real number,
            so fractions of a second can be specified.)
        <callback>	- O
            returns a handle for the registered callback.  This handle is used
            in calls to the other (callback-related) IOX functions.  NULL is
            returned in the event of an error.

*******************************************************************************/


IoxCallback  ioxEvery (

#    if PROTOTYPES
        IoxDispatcher  dispatcher,
        IoxHandler  handlerF,
        void  *userData,
        double  delay,
        double  interval)
#    else
        dispatcher, handlerF, userData, delay, interval)

        IoxDispatcher  dispatcher ;
        IoxHandler  handlerF ;
        void  *userData ;
        double  delay ;
        double  interval ;
#    endif

{    /* Local variables. */
    IoxCallback  cb ;



/* Register a single-shot timer with the specified delay. */

    cb = ioxAfter (dispatcher, handlerF, userData,
                   (delay < 0.0) ? interval : delay) ;
    if (cb == NULL) {
        LGE "(ioxEvery) Error creating initial single-shot timer.\nioxAfter: ") ;
        return (NULL) ;
    }

/* Convert the single-shot timer into a periodic timer by setting its
   periodic flag to true! */

    cb->interval = interval ;
    cb->periodic = true ;

    LGI "(ioxEvery) Callback %p, handler %p, data %p, delay %g, interval %g.\n",
        (void *) cb, (void *) handlerF, userData, delay, interval) ;

    return (cb) ;

}

/*!*****************************************************************************

Procedure:

    ioxMonitor ()

    Monitor I/O Events, Timers, and Idle Task.


Purpose:

    Function ioxMonitor() monitors registered I/O sources, timers, and idle
    tasks.  When an I/O condition is detected on an I/O source, a timer
    expires, or the dispatcher is idle, the dispatcher invokes the handler
    function bound to the callback for the given event.

    Before calling ioxMonitor(), an application must first register callbacks
    for the events to be monitored: ioxOnIO() for I/O sources, ioxAfter() and
    ioxEvery() for timers, and ioxWhenIdle() for idle task.  During the
    application's lifetime, new callbacks can be registered using the above
    functions and existing callbacks can be "unregistered" using ioxCancel().


    Invocation:

        status = ioxMonitor (dispatcher, interval) ;

    where:

        <dispatcher>	- I
            is the dispatcher handle returned by ioxCreate().
        <interval>	- I
            specifies the time interval in seconds after which ioxMonitor()
            should return to the caller.  At a minimum, that many seconds will
            pass before ioxMonitor() returns, but there is no guarantee how
            soon after the time interval elapses that ioxMonitor() will return.
            A negative time keeps ioxMonitor() from ever returning - at least
            until there are no more I/O sources or timers to monitor and no
            idle tasks to execute.
            (The time in seconds is a real number, so fractions of a second
            can be specified.)
        <status>	- O
            returns the status (ERRNO) of monitoring the registered events.
            An error status most likely indicates an invalid I/O source
            (e.g., a now-closed file descriptor) remaining registered for
            monitoring.

*******************************************************************************/


errno_t  ioxMonitor (

#    if PROTOTYPES
        IoxDispatcher  dispatcher,
        double  interval)
#    else
        dispatcher, interval)

        IoxDispatcher  dispatcher ;
        double  timeout ;
#    endif

{    /* Local variables. */
    bool  isIdle ;
    fd_set  exceptMask, exceptMaskSave ;
    fd_set  readMask, readMaskSave ;
    fd_set  writeMask, writeMaskSave ;
#ifdef VMS
    float  f_timeout ;
#endif
    int  numActive ;
    IoxCallback  cb ;
    struct  timeval  timeout ;




    if (dispatcher == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxMonitor) NULL dispatcher handle.\n") ;
        return (errno) ;
    }


/*******************************************************************************
    Loop forever, "listening" for and responding to I/O events and timeouts.
    When a monitored I/O event is detected, invoke the callback function
    bound by ioxOnIO() to the source of the event.  When a timeout interval
    expires, invoke the callback function bound by ioxAfter() or ioxEvery()
    to the timer.  When no I/O source is active and no timers have expired,
    then invoke the next registered idle task.
*******************************************************************************/


    for ( ; ; ) {


/* Construct the SELECT(2) masks for the I/O sources being monitored. */

        FD_ZERO (&readMaskSave) ;
        FD_ZERO (&writeMaskSave) ;
        FD_ZERO (&exceptMaskSave) ;
        numActive = 0 ;

        for (cb = dispatcher->ioList ;  cb != NULL ;  cb = cb->next) {
            if (cb->reason & IoxRead) {
                FD_SET (cb->source, &readMaskSave) ;  numActive++ ;
            }
            if (cb->reason & IoxWrite) {
                FD_SET (cb->source, &writeMaskSave) ;  numActive++ ;
            }
            if (cb->reason & IoxExcept) {
                FD_SET (cb->source, &exceptMaskSave) ;  numActive++ ;
            }
        }

        if ((numActive == 0) && (dispatcher->timerList == NULL) &&
            (dispatcher->idleQueue == NULL)) {
            SET_ERRNO (EINVAL) ;
            LGE "(ioxMonitor) No I/O sources or timeouts to monitor.\n") ;
            return (errno) ;
        }


/* Wait for an I/O event to occur or for the timeout interval to expire. */

        for ( ; ; ) {
            readMask = readMaskSave ;
            writeMask = writeMaskSave ;
            exceptMask = exceptMaskSave ;
            LGI "(ioxMonitor) 0x%08lX 0x%08lX 0x%08lX\n",
                *((long *) &readMask),
                *((long *) &writeMask),
                *((long *) &exceptMask)) ;
            if (dispatcher->idleQueue != NULL) 	{	/* Idle tasks to run? */
                if (numActive > 0) {
                    timeout.tv_sec = timeout.tv_usec = 0 ;
                    numActive = select (FD_SETSIZE, &readMask, &writeMask,
                                        &exceptMask, &timeout) ;
                }
            } else if (dispatcher->timerList == NULL) {	/* Wait forever. */
                numActive = select (FD_SETSIZE, &readMask, &writeMask,
                                    &exceptMask, NULL) ;
            } else {		/* Wait for I/O or until timeout expires. */
                timeout = tvSubtract ((dispatcher->timerList)->expiration,
                                      tvTOD ()) ;
            LGI "(ioxMonitor) timeout = %ld %ld\n",
                (long) timeout.tv_sec, (long) timeout.tv_usec) ;
#ifdef VMS
                if (numActive > 0) {
#endif
                numActive = select (FD_SETSIZE, &readMask, &writeMask,
                                    &exceptMask, &timeout) ;
#ifdef VMS
                } else {
			/* VMS doesn't allow SELECT(2)ing when no bits are set
			   in the masks, so LIB$WAIT() is used for timeouts. */
                    f_timeout = (float) timeout.tv_sec +
                                (timeout.tv_usec / 1000000.0) ;
                    LIB$WAIT (&f_timeout) ;
                    numActive = 0 ;
                }
#endif
            }
            if (numActive >= 0)  break ;
            if (errno == EINTR)  continue ;	/* Retry on signal interrupt. */
            fflush (stdout) ;
            LGE "(ioxMonitor) Error monitoring I/O sources.\nselect: ") ;
            return (errno) ;
        }


        dispatcher->depth++ ;		/* If a callback calls ioxDestroy(),
					   don't free(3) the dispatcher yet. */

/* Scan the SELECT(2) bit masks.  For each I/O condition detected, invoke
   the callback function bound to that condition and its source.  In case
   a callback modifies the list of monitored I/O events (e.g., unregistering
   a related connection), the callback's source is cleared in the SELECT(2)
   bit masks and the scan begins all over again.  Note that, if a single
   callback is bound to an ORed mask of conditions and two or more of the
   conditions are simultaneously detected (e.g., input-available and
   output-ready), the callback is only invoked once; the callback is
   responsible, in this case, for checking for both conditions. */

        isIdle = true ;

        cb = dispatcher->ioList ;
        while (cb != NULL) {
            IoxReason  conditions = 0 ;
            if ((cb->reason & IoxRead) && FD_ISSET (cb->source, &readMask))
                conditions |= IoxRead ;
            if ((cb->reason & IoxWrite) && FD_ISSET (cb->source, &writeMask))
                conditions |= IoxWrite ;
            if ((cb->reason & IoxExcept) && FD_ISSET (cb->source, &exceptMask))
                conditions |= IoxExcept ;
            if (conditions & IoxIO) {		/* I/O condition detected? */
                FD_CLR (cb->source, &readMask) ;
                FD_CLR (cb->source, &writeMask) ;
                FD_CLR (cb->source, &exceptMask) ;
                cb->handler (cb, conditions, cb->userData) ;
                isIdle = false ;
                cb = dispatcher->ioList ;	/* Re-scan list. */
            } else {
                cb = cb->next ;			/* Next item in list. */
            }
        }


/* If a timer has fired, invoke the callback function bound to the timer.
   Since the timer list is sorted by expiration time, only the first entry
   in the timer list needs to be examined. */

        cb = dispatcher->timerList ;
        if ((cb != NULL) && (tvCompare (tvTOD (), cb->expiration) >= 0)) {
            bool  periodic = cb->periodic ;
            if (periodic) {		/* Reschedule periodic timers. */
                cb->expiration = tvAdd (cb->expiration,
                                        tvCreateF (cb->interval)) ;
                dispatcher->timerList = cb->next ;
                ioxAdd (cb) ;
            }				/* Invoke the handler function. */
            cb->handler (cb, IoxFire, cb->userData) ;
            if (!periodic)		/* Cancel single-shot timers. */
                ioxCancel (cb) ;
            isIdle = false ;
            dispatcher->depth-- ;
            continue ;			/* In case the callback modified
					   the list of monitored events. */
        }


/* If no I/O sources were active and no timers fired, then execute the next
   idle task. */

        if (isIdle && (dispatcher->idleQueue != NULL)) {
            cb = dispatcher->idleQueue ;
            dispatcher->idleQueue = cb->next ;
            ioxAdd (cb) ;
            cb->handler (cb, IoxIdle, cb->userData) ;
        }


        dispatcher->depth-- ;		/* Now ioxDestroy() can free(3) the
					   dispatcher. */

    }     /* Loop forever */


#ifdef WHEN_INTERVAL_LIMIT_IMPLEMENTED
    return (0) ;
#endif

}

/*!*****************************************************************************

Procedure:

    ioxOnIO ()

    Register an I/O Event Callback.


Purpose:

    Function ioxOnIO() registers an I/O source (e.g., a network socket) with
    the dispatcher.  When one of the specified I/O conditions (input-pending,
    output-ready, OOB-available) is detected on the source, the caller's handler
    function is invoked with the appropriate reason (IoxRead, IoxWrite, and
    IoxExcept, respectively).  The application is responsible for cancelling
    the callback when the I/O source is closed.


    Invocation:

        callback = ioxOnIO (dispatcher, handlerF, userData, reason, source) ;

    where:

        <dispatcher>	- I
            is the dispatcher handle returned by ioxCreate().
        <handlerF>	- I
            is the function that is to be called when one of the specified
            I/O conditions is detected on the source.  The handler function
            should be declared as follows:
                int  handler_function (IoxCallback callback,
                                       IoxReason reason,
                                       void *userData) ;
            where "callback" is the callback handle returned by ioxOnIO();
            "reason" is IoxRead, IoxWrite, or IoxExcept; and "userData" is
            the argument that was passed into ioxOnIO().  The return value
            of the handler function is ignored by the dispatcher.
        <userData>	- I
            is a caller-supplied (VOID *) value that will be passed to the
            handler function when it is invoked.
        <reason>	- I
            specifies the I/O conditions for which the source is to be
            monitored.  This argument is the bit-wise ORing of one or
            more of the following values: IoxRead (input-pending),
            IoxWrite (output-ready), and IoxExcept (OOB-input-pending).
        <source>	- I
            is the UNIX file descriptor to be monitored for the specified
            I/O condition(s).
        <callback>	- O
            returns a handle for the registered callback.  This handle is used
            in calls to the other (callback-related) IOX functions.  NULL is
            returned in the event of an error.

*******************************************************************************/


IoxCallback  ioxOnIO (

#    if PROTOTYPES
        IoxDispatcher  dispatcher,
        IoxHandler  handlerF,
        void  *userData,
        IoxReason  reason,
        IoFd  source)
#    else
        dispatcher, handlerF, userData, reason, source)

        IoxDispatcher  dispatcher ;
        IoxHandler  handlerF ;
        void  *userData ;
        IoxReason  reason ;
        IoFd  source ;
#    endif

{    /* Local variables. */
    IoxCallback  cb ;



    if (dispatcher == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxOnIO) NULL dispatcher handle.\n") ;
        return (NULL) ;
    }

/* Allocate a callback structure for the I/O source. */

    cb = (IoxCallback) malloc (sizeof (_IoxCallback)) ;
    if (cb == NULL) {
        LGE "(ioxOnIO) Error allocating callback structure.\nmalloc: ") ;
        return (NULL) ;
    }

    cb->dispatcher = dispatcher ;
    cb->reason = reason ;
    cb->handler = handlerF ;
    cb->userData = userData ;
    cb->onCancel = false ;
    cb->source = source ;
    cb->interval = 0.0 ;
    cb->periodic = false ;

/* Insert the I/O callback in the unsorted list of registered I/O callbacks. */

    ioxAdd (cb) ;

    LGI "(ioxOnIO) Callback %p, handler %p, data %p, reason %d, source %ld.\n",
        (void *) cb, (void *) handlerF, userData, reason, (long) source) ;

    return (cb) ;

}

/*!*****************************************************************************

Procedure:

    ioxWhenIdle ()

    Register an Idle Callback.


Purpose:

    Function ioxWhenIdle() registers an idle task to be executed when
    no I/O events or timers are awaiting attention from the dispatcher.
    Idle tasks are kept in a FIFO queue; the tasks effectively execute in
    "background" mode.  Idle tasks are responsible for returning control
    to the dispatcher in a timely fashion.  The dispatcher automatically
    requeues the idle task for its next invocation.  When an idle task
    is no longer needed, the application must explicitly cancel it.


    Invocation:

        callback = ioxWhenIdle (dispatcher, handlerF, userData) ;

    where:

        <dispatcher>	- I
            is the dispatcher handle returned by ioxCreate().
        <handlerF>	- I
            is the function ("idle task") that is to be called when the
            dispatcher has nothing else to do.  The handler function
            should be declared as follows:
                int  handler_function (IoxCallback callback,
                                       IoxReason reason,
                                       void *userData) ;
            where "callback" is the callback handle returned by ioxWhenIdle(),
            "reason" is IoxIdle, and "userData" is the argument that was
            passed into ioxWhenIdle().  The return value of the handler
            function is ignored by the dispatcher.
        <userData>	- I
            is a (VOID *) value to be passed to the handler function.
        <callback>	- O
            returns a handle for the registered callback.  This handle is used
            in calls to the other (callback-related) IOX functions.  NULL is
            returned in the event of an error.

*******************************************************************************/


IoxCallback  ioxWhenIdle (

#    if PROTOTYPES
        IoxDispatcher  dispatcher,
        IoxHandler  handlerF,
        void  *userData)
#    else
        dispatcher, handlerF, userData)

        IoxDispatcher  dispatcher ;
        IoxHandler  handlerF ;
        void  *userData ;
#    endif

{    /* Local variables. */
    IoxCallback  cb ;



    if (dispatcher == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxWhenIdle) NULL dispatcher handle.\n") ;
        return (NULL) ;
    }

/* Allocate a callback structure for the idle task. */

    cb = (IoxCallback) malloc (sizeof (_IoxCallback)) ;
    if (cb == NULL) {
        LGE "(ioxWhenIdle) Error allocating callback structure.\nmalloc: ") ;
        return (NULL) ;
    }

    cb->dispatcher = dispatcher ;
    cb->reason = IoxIdle ;
    cb->handler = handlerF ;
    cb->userData = userData ;
    cb->onCancel = false ;
    cb->source = INVALID_SOCKET ;
    cb->interval = 0.0 ;
    cb->periodic = false ;

/* Add the callback to the queue of registered idle callbacks. */

    ioxAdd (cb) ;

    LGI "(ioxWhenIdle) Callback %p, handler %p, data %p.\n",
        (void *) cb, (void *) handlerF, userData) ;

    return (cb) ;

}

/*!*****************************************************************************

Procedure:

    ioxCancel ()

    Cancel a Registered Callback.


Purpose:

    Function ioxCancel() cancels a previously registered callback.


    Invocation:

        status = ioxCancel (callback) ;

    where:

        <callback>	- I
            is the callback handle returned by one of the IOX registration
            functions.
        <status>	- O
            returns the status of unregistering the callback, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  ioxCancel (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{    /* Local variables. */
    IoxCallback  cb, prev ;
    IoxDispatcher  dispatcher ;




    LGI "(ioxCancel) Cancelling callback %p.\n", (void *) callback) ;

    if ((callback == NULL) || (callback->dispatcher == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxCancel) NULL callback handle or dispatcher.\n") ;
        return (errno) ;
    }

    dispatcher = callback->dispatcher ;

/* If the callback is an I/O callback, remove it from the dispatcher's list
   of I/O callbacks. */

    if (callback->reason & IoxIO) {

        for (prev = NULL, cb = dispatcher->ioList ;
             cb != NULL ;  cb = cb->next) {
            if (cb == callback)  break ;
            prev = cb ;
        }

        if (cb == NULL) {
            SET_ERRNO (EINVAL) ;
            LGE "(ioxCancel) I/O callback %p not found.\n", callback) ;
            return (errno) ;
        }

        if (prev == NULL)
            dispatcher->ioList = cb->next ;
        else
            prev->next = cb->next ;

    }

/* If the callback is a timer callback, remove it from the dispatcher's list
   of timer callbacks. */

    else if (callback->reason & IoxFire) {

        for (prev = NULL, cb = dispatcher->timerList ;
             cb != NULL ;  cb = cb->next) {
            if (cb == callback)  break ;
            prev = cb ;
        }

        if (cb == NULL) {
            SET_ERRNO (EINVAL) ;
            LGE "(ioxCancel) Timer callback %p not found.\n", callback) ;
            return (errno) ;
        }

        if (prev == NULL)
            dispatcher->timerList = cb->next ;
        else
            prev->next = cb->next ;

    }

/* If the callback is an idle callback, remove it from the dispatcher's queue
   of idle callbacks. */

    else if (callback->reason & IoxIdle) {

        for (prev = NULL, cb = dispatcher->idleQueue ;
             cb != NULL ;  cb = cb->next) {
            if (cb == callback)  break ;
            prev = cb ;
        }

        if (cb == NULL) {
            SET_ERRNO (EINVAL) ;
            LGE "(ioxCancel) Idle callback %p not found.\n", callback) ;
            return (errno) ;
        }

        if (prev == NULL)
            dispatcher->idleQueue = cb->next ;
        else
            prev->next = cb->next ;

    } else {

        SET_ERRNO (EINVAL) ;
        LGE "(ioxCancel) Unmonitored callback %p, reason(s) 0x%08X.\n",
            callback, callback->reason) ;
        return (errno) ;

    }

/* If the handler function is flagged to be invoked when the callback is
   cancelled, then call the handler function with the IoxCancel reason. */

    if (callback->onCancel && (callback->handler != NULL)) {
        callback->handler (callback, IoxCancel, callback->userData) ;
    }

/* Free the callback structure. */

    free (callback) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    ioxDepth ()

    Get the Callback Invocation Depth.


Purpose:

    Function ioxDepth() returns the callback invocation depth of a
    callback's dispatcher.


    Invocation:

        depth = ioxDepth (callback) ;

    where:

        <callback>	- I
            is the callback handle returned by one of the IOX registration
            functions.
        <depth>		- O
            returns the callback invocation depth of the callback's dispatcher.

*******************************************************************************/


int  ioxDepth (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{

    if ((callback == NULL) || (callback->dispatcher == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxDepth) NULL callback handle or dispatcher.\n") ;
        return (0) ;
    }

    return ((callback->dispatcher)->depth) ;

}

/*!*****************************************************************************

Procedure:

    ioxDispatcher ()

    Get a Callback's Dispatcher.


Purpose:

    Function ioxDispatcher() returns a callback's dispatcher.


    Invocation:

        dispatcher = ioxDispatcher (callback) ;

    where:

        <callback>	- I
            is the callback handle returned by one of the IOX registration
            functions.
        <dispatcher>	- O
            returns the handle of the dispatcher with which the callback is
            registered; NULL is returned in the event of an error.

*******************************************************************************/


IoxDispatcher  ioxDispatcher (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{

    if ((callback == NULL) || (callback->dispatcher == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxDispatcher) NULL callback handle or dispatcher.\n") ;
        return (NULL) ;
    }

    return (callback->dispatcher) ;

}

/*!*****************************************************************************

Procedure:

    ioxExpiration ()

    Get a Timer Callback's Expiration Time.


Purpose:

    Function ioxExpiration() returns a timer callback's expiration time.


    Invocation:

        expiration = ioxExpiration (callback) ;

    where:

        <callback>	- I
            is the callback handle returned by ioxAfter() or ioxEvery().
        <expiration>	- O
            returns the expiration time of the timer.

*******************************************************************************/


struct  timeval  ioxExpiration (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{

    if ((callback == NULL) || !(callback->reason & IoxFire)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxInterval) NULL callback handle or non-timer callback.\n") ;
        return (tvCreate (0, 0)) ;
    }

    return (callback->expiration) ;

}

/*!*****************************************************************************

Procedure:

    ioxFd ()

    Get an I/O Event Callback's File Descriptor.


Purpose:

    Function ioxFd() returns the file descriptor being monitored for an
    I/O event callback.


    Invocation:

        fd = ioxFd (callback) ;

    where:

        <callback>	- I
            is the callback handle returned by ioxOnIO().
        <fd>		- O
            returns the file descriptor being monitored for I/O events.

*******************************************************************************/


IoFd  ioxFd (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{

    if ((callback == NULL) || !(callback->reason & IoxIO)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxFd) NULL callback handle or non-I/O callback.\n") ;
        return (INVALID_SOCKET) ;
    }

    return (callback->source) ;

}

/*!*****************************************************************************

Procedure:

    ioxInterval ()

    Get a Timer Callback's Interval.


Purpose:

    Function ioxInterval() returns a timer callback's interval.


    Invocation:

        interval = ioxInterval (callback) ;

    where:

        <callback>	- I
            is the callback handle returned by ioxAfter() or ioxEvery().
        <interval>	- O
            returns the time interval in seconds of the timer.

*******************************************************************************/


double  ioxInterval (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{

    if ((callback == NULL) || !(callback->reason & IoxFire)) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxInterval) NULL callback handle or non-timer callback.\n") ;
        return (0.0) ;
    }

    return (callback->interval) ;

}

/*!*****************************************************************************

Procedure:

    ioxOnCancel ()

    Set Callback's Invoke-on-Cancel Flag.


Purpose:

    Function ioxOnCancel() is used to control whether or not a callback's
    handler function is to be invoked (with the IoxCancel reason) when the
    callback is cancelled.  By default, the handler function is NOT invoked
    when the callback is cancelled.  The application must explicitly call
    ioxOnCancel() if it wants the invoke-on-cancel behavior for a callback.


    Invocation:

        status = ioxOnCancel (callback, onCancel) ;

    where:

        <callback>	- I
            is the callback handle returned by one of the IOX registration
            functions.
        <onCancel>	- I
            specifies whether or the callback's handler function should be
            invoked when the callback is cancelled.
        <status>	- O
            returns the status of setting the flag, zero if there were no
            errors and ERRNO otherwise.

*******************************************************************************/


errno_t  ioxOnCancel (

#    if PROTOTYPES
        IoxCallback  callback,
        bool  onCancel)
#    else
        callback, onCancel)

        IoxCallback  callback ;
        bool  onCancel ;
#    endif

{

    LGI "(ioxOnCancel) Callback %p, %s.\n",
        (void *) callback,
        onCancel ? "INVOKE" : "DON'T INVOKE") ;

    if (callback == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(ioxOnCancel) NULL callback handle.\n") ;
        return (errno) ;
    }

    callback->onCancel = onCancel ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    ioxAdd ()

    Add a Callback to its Dispatcher's Callback Lists.


Purpose:

    Function ioxAdd() adds a callback to the appropriate list of a dispatcher's
    callbacks (i.e., an I/O callback is added to the I/O list, a timer is added
    to the timer list, and an idle task is added to the idle list).


    Invocation:

        ioxAdd (callback) ;

    where:

        <callback>	- I
            is the handle for a callback; see the IOX registration functions.

*******************************************************************************/


static  void  ioxAdd (

#    if PROTOTYPES
        IoxCallback  callback)
#    else
        callback)

        IoxCallback  callback ;
#    endif

{    /* Local variables. */
    IoxCallback  next, prev, rear ;
    IoxDispatcher  dispatcher ;



    dispatcher = callback->dispatcher ;

/* If the callback is an I/O callback, then insert the I/O callback
   at the front of the unsorted list of registered I/O callbacks. */

    if (callback->reason & IoxIO) {

        callback->next = dispatcher->ioList ;
        dispatcher->ioList = callback ;

    }

/* If the callback is a timer callback, then add the timer to the list of
   registered timers.  The list is sorted by expiration time. */

    else if (callback->reason & IoxFire) {

        prev = NULL ;  next = dispatcher->timerList ;
        while (next != NULL) {
            if (tvCompare (callback->expiration, next->expiration) < 0)  break ;
            prev = next ;  next = next->next ;
        }

        if (prev == NULL) {			/* Beginning of list? */
            callback->next = dispatcher->timerList ;
            dispatcher->timerList = callback ;
        } else {				/* Middle or end of list. */
            callback->next = next ;
            prev->next = callback ;
        }

    }

/* If the callback is an idle callback, then add the callback at the end of
   the queue of registered idle callbacks. */

    else if (callback->reason & IoxIdle) {

        callback->next = NULL ;

        for (rear = dispatcher->idleQueue ;  rear != NULL ;  rear = rear->next)
            if (rear->next == NULL)  break ;

        if (rear == NULL)		/* Add to empty queue? */
            dispatcher->idleQueue = callback ;
        else				/* Append to non-empty queue. */
            rear->next = callback ;

    }

    return ;

}
