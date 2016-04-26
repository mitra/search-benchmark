/* $Id: words_tv.c,v 1.1 2009/10/07 08:43:52 alex Exp alex $ */
/*******************************************************************************

File:

    words_tv.c

    Timeval Manipulation.


Author:    Alex Measday


Purpose:

    The WORDS_TV package defines words for manipulating UNIX timeval structures:

        <time1> <time2> TV-ADD
        <time1> <time2> TV-COMPARE
        <time> <inLocal?> "<format>" TV-SHOW
        <time1> <time2> TV-SUBTRACT
        TV-TOD

    A UNIX timeval structure consists of two signed integer values represeting
    the number of seconds and microseconds since the start of January 1, 1970.
    The timeval fields are placed on the stack with the most significant
    seconds field first.  For example, TV-TOD returns the time-of-day on
    the stack as two signed integers:

        TV-TOD		( -- seconds microseconds )

    NOTE that these can't be treated as double-cell numbers, since a value
    of zero seconds, -123,000 microseconds has no indication in the seconds
    field that the time value is negative.  Hence, I put the most significant
    seconds field first on the stack to prevent any confusion.


Public Procedures:

    buildWordsTV() - registers the words with the FICL system.

Private Procedures:

    word_TV_ADD() - implements the TV-ADD word.
    word_TV_COMPARE() - implements the TV-COMPARE word.
    word_TV_SHOW() - implements the TV-SHOW word.
    word_TV_SUBTRACT() - implements the TV-SUBTRACT word.
    word_TV_TOT() - implements the TV-TOD word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_TV_ADD P_((ficlVm *vm)) ;
static  void  word_TV_COMPARE P_((ficlVm *vm)) ;
static  void  word_TV_SHOW P_((ficlVm *vm)) ;
static  void  word_TV_SUBTRACT P_((ficlVm *vm)) ;
static  void  word_TV_TOD P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsTV ()

    Enter the TV Words into the Dictionary.


Purpose:

    Function buildWordsTV() enters the TV words into the system dictionary.


    Invocation:

        buildWordsTV (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsTV (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TV-ADD", word_TV_ADD,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TV-COMPARE", word_TV_COMPARE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TV-SHOW", word_TV_SHOW,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TV-SUBTRACT", word_TV_SUBTRACT,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "TV-TOD", word_TV_TOD,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TV_ADD ()

    Add Two Time Values.


Purpose:

    Function word_TV_ADD() implements the TV-ADD word, which adds two UNIX
    timevals:

        TV-ADD

            ( time1 time2 -- time3 )

        Add two UNIX timevals, time1 plus time2, and return the sum, time3.


    Invocation:

        word_TV_ADD (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TV_ADD (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    struct  timeval  time1, time2, time3 ;



    FICL_STACK_CHECK (vm->dataStack, 4, 2) ;

/* Get the operands from the stack. */

    cell = ficlVmPop (vm) ;			/* Time2 (microseconds). */
    time2.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time2 (seconds). */
    time2.tv_sec = (time_t) cell.i ;

    cell = ficlVmPop (vm) ;			/* Time1 (microseconds). */
    time1.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time1 (seconds). */
    time1.tv_sec = (time_t) cell.i ;

/* Add the two times and return the result on the stack. */

    time3 = tvAdd (time1, time2) ;

    cell.i = (ficlInteger) time3.tv_sec ;	/* Time3 (seconds). */
    ficlVmPush (vm, cell) ;
    cell.i = (ficlInteger) time3.tv_usec ;	/* Time3 (microseconds). */
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TV_COMPARE ()

    Compare Two Time Values.


Purpose:

    Function word_TV_COMPARE() implements the TV-COMPARE word, which compares
    two UNIX timevals:

        TV-COMPARE

            ( time1 time2 -- n )

        Compare two UNIX timevals, time1 and time2, and return a signed integer
        indicating the result of the comparison: -1 if time1 is less than time2,
        zero if time1 equals time2, and +1 if time1 is greater than time2.


    Invocation:

        word_TV_COMPARE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TV_COMPARE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    struct  timeval  time1, time2 ;



    FICL_STACK_CHECK (vm->dataStack, 4, 1) ;

/* Get the operands from the stack. */

    cell = ficlVmPop (vm) ;			/* Time2 (microseconds). */
    time2.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time2 (seconds). */
    time2.tv_sec = (time_t) cell.i ;

    cell = ficlVmPop (vm) ;			/* Time1 (microseconds). */
    time1.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time1 (seconds). */
    time1.tv_sec = (time_t) cell.i ;

/* Compare the two times and return the result on the stack. */

    cell.i = tvCompare (time1, time2) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TV_SHOW ()

    Get ASCII Representation of a Binary Time Value.


Purpose:

    Function word_TV_SHOW() implements the TV-SHOW word, which returns the
    ASCII representation of a UNIX timeval:

        TV-SHOW

            ( time inLocal? c-addr1 u1 -- c-addr2 u2 | 0 )

        Format a binary time value, time, in ASCII using the strftime(3)
        format specified by c-addr1/u1.  If inLocal? is true, the local
        time is used; otherwise the GMT is used.  The formatted time is
        returned on the stack as c-addr2/u2, a string whose storage is
        private to the TV-SHOW word.  If the conversion fails for any
        reason, only zero (0) is returned on the stack.

        NOTE:  If the O/S library does not support the gmtime(3) and
        localtime(3) functions, the binary time is assumed to be GMT
        and the inLocal? argument is ignored.  If the O/S library
        does not support strftime(3), the format argument is ignored
        and the binary time is formatted as "YYYY-DOY-HR:MN:SC.MLS".
        Experiment with TV-SHOW to determine into which category your
        FINC build falls.


    Invocation:

        word_TV_SHOW (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TV_SHOW (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  allocated = false, inLocal ;
    char  *asciiTime, *format ;
    ficlCell  cell ;
    struct  timeval  binaryTime ;
    unsigned  long  length ;




    FICL_STACK_CHECK (vm->dataStack, 5, 2) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Format (length) */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Format (address) */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        format = NULL ;
    } else {
        format = (char *) cell.p ;
        if (format[length] != '\0') {
            format = strndup (format, length) ;
            allocated = true ;
        }
    }

    cell = ficlVmPop (vm) ;			/* inLocal? */
    inLocal = (cell.i != 0) ;

    cell = ficlVmPop (vm) ;			/* Time (microseconds) */
    binaryTime.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time (seconds) */
    binaryTime.tv_sec = (time_t) cell.i ;

/* Format the time in ASCII. */

    asciiTime = (char *) tvShow (binaryTime, inLocal, format) ;

    if (allocated)  free (format) ;

/* Return the result on the stack. */

    if (asciiTime != NULL) {
        cell.p = asciiTime ;
        ficlVmPush (vm, cell) ;
    }
    cell.u = (asciiTime == NULL) ? 0 : strlen (asciiTime) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TV_SUBTRACT ()

    Subtract Two Time Values.


Purpose:

    Function word_TV_SUBTRACT() implements the TV-SUBTRACT word, which
    subtracts two UNIX timevals:

        TV-SUBTRACT

            ( time1 time2 -- time3 )

        Subtract two UNIX timevals, time1 minus time2, and return the
        difference, time3.


    Invocation:

        word_TV_SUBTRACT (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TV_SUBTRACT (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    struct  timeval  time1, time2, time3 ;



    FICL_STACK_CHECK (vm->dataStack, 4, 2) ;

/* Get the operands from the stack. */

    cell = ficlVmPop (vm) ;			/* Time2 (microseconds). */
    time2.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time2 (seconds). */
    time2.tv_sec = (time_t) cell.i ;

    cell = ficlVmPop (vm) ;			/* Time1 (microseconds). */
    time1.tv_usec = (time_t) cell.i ;
    cell = ficlVmPop (vm) ;			/* Time1 (seconds). */
    time1.tv_sec = (time_t) cell.i ;

/* Subtract the two times and return the result on the stack. */

    time3 = tvSubtract (time1, time2) ;

    cell.i = (ficlInteger) time3.tv_sec ;	/* Time3 (seconds). */
    ficlVmPush (vm, cell) ;
    cell.i = (ficlInteger) time3.tv_usec ;	/* Time3 (microseconds). */
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_TV_TOD ()

    Get the Current Time of Day.


Purpose:

    Function word_TV_TOD() implements the TV-TOD word, which returns the
    current time-of-day:

        TV-TOD

            ( -- umicrosecond useconds )

        Get the current time-of-day in seconds and microsecond since the
        start of January 1, 1970.


    Invocation:

        word_TV_TOD (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_TV_TOD (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    ficlCell  cell ;
    struct  timeval  timeOfDay ;



    FICL_STACK_CHECK (vm->dataStack, 0, 2) ;

/* Return the current time-of-day on the stack. */

    timeOfDay = tvTOD () ;

    cell.i = (ficlInteger) timeOfDay.tv_sec ;		/* Seconds. */
    ficlVmPush (vm, cell) ;

    cell.i = (ficlInteger) timeOfDay.tv_usec ;		/* Microseconds. */
    ficlVmPush (vm, cell) ;

    return ;

}
