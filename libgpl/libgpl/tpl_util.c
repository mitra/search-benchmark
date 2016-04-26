/* $Id: tpl_util.c,v 1.6 2004/04/02 23:02:16 alex Exp $ */
/*******************************************************************************

File:

    tpl_util.c

    Tuple Utilities.


Author:    Alex Measday


Purpose:

    The TPL_UTIL package is used for quickly creating N-tuples of information,
    where each element of a tuple is represented as a (VOID *) pointer:

        #include  "tpl_util.h"			-- Tuple utilities.
        Tuple  tuple ;
        ...
        tuple = tplCreate (3,
                           (void *) thisInfo,
                           (void *) thatInfo,
                           (void *) otherInfo) ;

    The following code fragments retrieves and prints out the individual
    elements in a tuple:

        #include  <stdio.h>			-- Standard I/O definitions.
        int  i ;
        ...
        for (i = 0 ;  i < tplArity (tuple) ;  i++)
            printf ("Element %d: %p\n", i, tplGet (tuple, i)) ;

    When a tuple is no longer needed, it should be deleted:

        tplDestroy (tuple) ;


Public Procedures:

    tplArity() - returns the number of elements in a tuple.
    tplCreate() - creates a tuple of N elements.
    tplDestroy() - deletes a tuple.
    tplGet() - gets an individual element from a tuple.
    tplSet() - sets the value of an individual element in a tuple.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  "tpl_util.h"			/* Tuple utilities. */


/*******************************************************************************
    Tuple - contains the elements of a tuple.
*******************************************************************************/

typedef  struct  _Tuple {
    int  numElements ;			/* # of elements in tuple. */
    void  *element[2] ;			/* First 2 elements. */
}  _Tuple ;

/*!*****************************************************************************

Procedure:

    tplArity ()

    Get Number of Elements in Tuple.


Purpose:

    The tplArity() function returns the number of elements in a tuple.


    Invocation:

        numElements = tplArity (tuple) ;

    where

        <tuple>		- I
            is the tuple handle returned by tplCreate().
        <numElements>	- O
            returns the number of elements in the tuple.

*******************************************************************************/


int  tplArity (

#    if PROTOTYPES
        Tuple  tuple)
#    else
        tuple)

        Tuple  tuple ;
#    endif

{

    return ((tuple == NULL) ? 0 : tuple->numElements) ;

}

/*!*****************************************************************************

Procedure:

    tplCreate ()

    Create a Tuple.


Purpose:

    The tplCreate() function creates a tuple and assigns values to each of
    its elements.


    Invocation:

        tuple = tplCreate (numElements, value0, value1, ...) ;

    where

        <numElements>		- I
            is the number of elements in the tuple.
        <value0>, <value1>, ...	- I
            are the (VOID *) values of the individual elements.
        <tuple>			- I
            returns a tuple handle that can be used in other TPL_UTIL calls.

*******************************************************************************/


Tuple  tplCreate (

#    if PROTOTYPES
        int  numElements,
        ...)
#    else
        numElements, va_alist)

        int  numElements ;
        va_dcl
#    endif

{    /* Local variables. */
    int  i ;
    Tuple  tuple ;
    va_list  ap ;



/* Allocate storage for the tuple. */

    if (numElements > 2) {
        tuple = (Tuple) malloc (sizeof (struct _Tuple) +
                                ((numElements - 2) * sizeof (void *))) ;
    } else {
        tuple = (Tuple) malloc (sizeof (struct _Tuple)) ;
    }

    if (tuple == NULL) {
        LGE "(tplCreate) Error allocating %d-ary tuple.\nmalloc: ",
            numElements) ;
        return (NULL) ;
    }

    tuple->numElements = (numElements < 0) ? 0 : numElements ;

/* Assign values to the individual elements. */

#if HAVE_STDARG_H
    va_start (ap, numElements) ;
#else
    va_start (ap) ;
#endif

    for (i = 0 ;  i < tuple->numElements ;  i++)
        tuple->element[i] = (void *) va_arg (ap, void *) ;

    va_end (ap) ;

    return (tuple) ;

}

/*!*****************************************************************************

Procedure:

    tplDestroy ()

    Destroy a Tuple.


Purpose:

    Function tplDestroy() destroy a tuple; it does NOT destroy the elements
    of the tuple.


    Invocation:

        status = tplDestroy (tuple) ;

    where

        <tuple>		- I
            is the tuple handle returned by tplCreate().
        <status>	- O
            returns the status of destroying the tuple, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  tplDestroy (

#    if PROTOTYPES
        Tuple  tuple)
#    else
        tuple)

        Tuple  tuple ;
#    endif

{

    if (tuple == NULL)  return (0) ;

/* Deallocate the tuple structure. */

    free (tuple) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    tplGet ()

    Get the Value of an Element From a Tuple.


Purpose:

    Function tplGet() returns the value of the I-th element of a tuple, where
    I ranges from 0 for the first element to N-1 for the N-th element.


    Invocation:

        value = tplGet (tuple, index) ;

    where

        <tuple>	- I
            is the tuple handle returned by tplCreate().
        <index>	- I
            is the index (0..N-1) of the desired element.
        <value>	- O
            returns the value, a (VOID *) pointer, of the specified element;
            NULL is returned in the event of an error.

*******************************************************************************/


void  *tplGet (

#    if PROTOTYPES
        Tuple  tuple,
        int  index)
#    else
        tuple, index)

        Tuple  tuple ;
        int  index ;
#    endif

{

    if (tuple == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(tplGet) NULL tuple handle: ") ;
        return (NULL) ;
    } else if ((index < 0) || (index >= tuple->numElements)) {
        SET_ERRNO (EINVAL) ;
        LGE "(tplGet) Invalid index %d for %d-ary tuple.\n",
            index, tuple->numElements) ;
        return (NULL) ;
    }

    return (tuple->element[index]) ;

}

/*!*****************************************************************************

Procedure:

    tplSet ()

    Set the Value of an Element in a Tuple.


Purpose:

    Function tplSet() sets the value of the I-th element in a tuple, where
    I ranges from 0 for the first element to N-1 for the N-th element.


    Invocation:

        status = tplSet (tuple, index, value) ;

    where

        <tuple>		- I
            is the tuple handle returned by tplCreate().
        <index>		- I
            is the index (0..N-1) of the target element.
        <value>		- I
            is the (VOID *) value of the specified element.
        <status>	- O
            returns the status of setting the element's value,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  tplSet (

#    if PROTOTYPES
        Tuple  tuple,
        int  index,
        void  *value)
#    else
        tuple, index, value)

        Tuple  tuple ;
        int  index ;
        void  *value ;
#    endif

{

    if (tuple == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(tplSet) NULL tuple handle: ") ;
        return (errno) ;
    } else if ((index < 0) || (index >= tuple->numElements)) {
        SET_ERRNO (EINVAL) ;
        LGE "(tplSet) Invalid index %d for %d-ary tuple.\n",
            index, tuple->numElements) ;
        return (errno) ;
    }

    tuple->element[index] = value ;

    return (0) ;

}
