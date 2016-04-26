/* $Id: bmw_util.c,v 1.10 2003/03/26 02:17:11 alex Exp $ */
/*******************************************************************************

File:

    bmw_util.c


Author:    Alex Measday


Purpose:

    The BMW utilities provide a simple means of measuring the performance
    of an arbitrary task, where the measure is the number of <something>
    performed per second.  For example, the following program measures
    the performance of standard output in number of lines per second:

        #include  <stdio.h>			-- Standard I/O definitions.
        #include  "bmw_util.h"			-- Benchmarking definitions.

        int  main (int argc, char *argv[])
        {
            BmwClock  clock ;
            int  i, numLines ;

            numLines = atoi (argv[1]) ;

            bmwStart (&clock) ;
            for (i = 0 ;  i < numLines ;  i++)
                printf ("Hello, World!\n") ;
            bmwStop (&clock) ;

            printf ("%g lines per second.\n",
                    bmwRate (&clock, (long) numLines)) ;
        }


Notes:

    These functions are reentrant under VxWorks.


Procedures:

    bmwElapsed() - returns the elapsed time for a timer.
    bmwRate() - computes the rate (# of X per second) for a timer.
    bmwStart() - starts a timer.
    bmwStop() - stops a timer.

*******************************************************************************/


#ifdef CPU_TIME
#    include  <sys/resource.h>		/* Process resource definitions. */
#    define  tvTOD  bmwCPUTime
#endif
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "bmw_util.h"			/* Benchmarking functions. */


/*******************************************************************************
    Private Functions.
*******************************************************************************/

#ifdef CPU_TIME
static  struct  timeval  bmwCPUTime (
#    if PROTOTYPES
        void
#    endif
    ) ;
#endif

#ifdef CPU_TIME

/*******************************************************************************

Procedure:

    bmwCPUTime ()


Purpose:

    Function bmwCPUTime() returns the cumulative amount of CPU time (in
    user and system space) used by the process.


    Invocation:

        cpuTime = bmwCPUTime () ;

    where

        <cpuTime>	- O
            returns the CPU time used by the process.

*******************************************************************************/


static  struct  timeval  bmwCPUTime (

#    if PROTOTYPES
        void)
#    else
        )
#    endif

{    /* Local variables. */
    struct  rusage  usage ;



/* Get the current usage statistics. */

    getrusage (RUSAGE_SELF, &usage) ;

/* Return the sum of the user CPU time and the system CPU time. */

/*    return (tvAdd (usage.ru_utime, usage.ru_stime)) ; */
    return (usage.ru_utime) ;

}

#endif

/*******************************************************************************

Procedure:

    bmwElapsed ()


Purpose:

    Function bmwElapsed() returns a benchmarking timer's elapsed time.
    If the timer has been stopped by a call to bmwStop(), the time
    returned by bmwElapsed() is the number of seconds between the
    start time and the stop time.  If the timer hasn't been stopped,
    the time returned is the difference between the start time and
    the current time of day.


    Invocation:

        elapsedTime = bmwElapsed (&timer) ;

    where

        <timer>		- I
            is the address of the timer used for this benchmark.
        <elapsedTime>	- O
            returns the floating-point number of seconds of elapsed time.

*******************************************************************************/


double  bmwElapsed (

#    if PROTOTYPES
        BmwClock  *timer)
#    else
        timer)

        BmwClock  *timer ;
#    endif

{    /* Local variables. */
    struct  timeval  elapsedTime ;



/* If the timer hasn't been stopped yet, then compute the current elapsed
   time.  If the timer has been stopped, then compute the time elapsed
   between the start time and stop time. */

    if ((timer->stopTime.tv_sec == 0) &&
        (timer->stopTime.tv_usec == 0))
        elapsedTime = tvSubtract (tvTOD (), timer->startTime) ;
    else
        elapsedTime = tvSubtract (timer->stopTime, timer->startTime) ;

    return (tvFloat (elapsedTime)) ;

}

/*******************************************************************************

Procedure:

    bmwRate ()


Purpose:

    Function bmwRate() returns a benchmarking timer's elapsed time.
    If the timer has been stopped by a call to bmwStop(), the time
    returned by bmwElapsed() is the number of seconds between the
    start time and the stop time.  If the timer hasn't been stopped,
    the time returned is the difference between the start time and
    the current time of day.


    Invocation:

        items_per_second = bmwRate (&timer, numItems) ;

    where

        <timer>			- I
            is the address of the timer used for this benchmark.
        <numItems>		- I
            is the count of that which is being measured.
        <items_per_second>	- O
            returns the number of items divided by the elapsed time.

*******************************************************************************/


double  bmwRate (

#    if PROTOTYPES
        BmwClock  *timer,
        long  numItems)
#    else
        timer, numItems)

        BmwClock  *timer ;
        long  numItems ;
#    endif

{
    return ((double) numItems / bmwElapsed (timer)) ;
}

/*******************************************************************************

Procedure:

    bmwStart ()


Purpose:

    Function bmwStart() initializes and starts a benchmarking timer.


    Invocation:

        bmwStart (&timer) ;

    where

        <timer>		- O
            is the address of a timer to be used for this benchmark.

*******************************************************************************/


void  bmwStart (

#    if PROTOTYPES
        BmwClock  *timer)
#    else
        timer)

        BmwClock  *timer ;
#    endif

{
    timer->startTime = tvTOD () ;		/* Current time. */
    timer->stopTime.tv_sec = 0 ;		/* Reset stop time. */
    timer->stopTime.tv_usec = 0 ;
}

/*******************************************************************************

Procedure:

    bmwStop ()


Purpose:

    Function bmwStop() stops a benchmarking timer.


    Invocation:

        bmwStop (&timer) ;

    where

        <timer>		- O
            is the address of the timer used for this benchmark.

*******************************************************************************/


void  bmwStop (

#    if PROTOTYPES
        BmwClock  *timer)
#    else
        timer)

        BmwClock  *timer ;
#    endif

{
    timer->stopTime = tvTOD () ;		/* Current time. */
}
