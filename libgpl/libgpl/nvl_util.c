/* $Id: nvl_util.c,v 1.8 2004/03/26 20:57:14 alex Exp $ */
/*******************************************************************************

File:

    nvl_util.c

    Name/Value Pair List Utilities.


Author:    Alex Measday


Purpose:

    The NVL_UTIL package manages lists of name/value pairs (see "nvp_util.h").
    A name/value pair list is essentially an associative array of name/value
    pairs.

    An empty list of name/value pairs is created as follows:

        #include  "nvl_util.h"			-- Name/value pair lists.
        NVList  list ;
        ...
        nvlCreate (NULL, &list) ;

    Name/value pairs are then added to the list using nvlAdd():

        nvlAdd (list, nvpNew ("THIS", NvpDouble, 123.45)) ;
        nvlAdd (list, nvpNew ("THAT", NvpLong, 6789)) ;

    The pairs in a list can be retrieved by index:

        int  i ;
        NVPair  pair ;
        ...
        for (i = 0 ;  i < nvlCount (list) ;  i++) {
            pair = nvlGet (list, i) ;
            printf ("%s = %s\n", nvpName (pair), nvpString (pair)) ;
        }

    or by name:

        pair = nvlFind (list, "THAT") ;		-- Prints "THAT = 6789".
        printf ("%s = %s\n", nvpName (pair), nvpString (pair)) ;

    Individual pairs can be deleted from a list:

        pair = nvlDelete (list, "THIS") ;

    Although the pair is removed from the list, the pair itself is not
    destroyed.  In contrast, deleting the entire list in one fell swoop:

        nvlDestroy (list) ;

    automatically destroys the remaining pairs via calls to nvpDestroy()
    (see "nvp_util.c").

    For my own convenience, I added the "Prop" functions, which implement
    LISP-like property lists using name/value lists and string-valued
    name/value pairs:

        NVList  myCar ;
        ...
        nvlCreate (NULL, &myCar) ;
        nvlPutProp (myCar, "MAKE", "Triumph") ;
        nvlPutProp (myCar, "MODEL", "TR-4") ;
        nvlPutProp (myCar, "COLOR", "green") ;
        printf ("I wish I had a %s %s %s.\n",
                nvlGetProp (myCar, "COLOR"),
                nvlGetProp (myCar, "MAKE"),
                nvlGetProp (myCar, "MODEL")) ;

    Using the "Prop" functions does not preclude using the regular NVL
    functions on the same list.


History:

    The name/value pair, name/value list, and version-independent message
    stream packages were inspired by Mike Maloney's C++ implementations of
    "named variables" and "named variable sets", and by Robert Martin's
    "attributed data trees" (see "Version-Independent Messages" in Appendix B
    of his DESIGNING OBJECTED-ORIENTED C++ APPLICATIONS USING THE BOOCH METHOD).


Public Procedures:

    nvlAdd() - adds a name/value pair to a list.
    nvlCount() - returns the number of name/value pairs in a list.
    nvlCreate() - creates an empty name/value pair list.
    nvlDelete() - deletes a name/value pair from a list.
    nvlDestroy() - destroys a list.
    nvlFind() - retrieves a pair by name from a list.
    nvlGet() - retrieves a pair by index from a list.
    nvlName() - returns a list's name.
    xdr_NVList() - encodes/decodes a name/value pair list in XDR format.

    nvlGetProp() - retrieves a property from a list.
    nvlPutProp() - adds a property to a list.
    nvlRemProp() - removes a property from a list.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "nvl_util.h"			/* Name/value pair lists. */


/*******************************************************************************
    Name/Value Pair List - contains zero or more name/value pairs.
*******************************************************************************/

typedef  struct  _NVList {
    char  *name ;			/* Name of list. */
    int  maxMembers ;			/* Maximum # of pairs in list. */
    int  numMembers ;			/* Actual # of pairs in list. */
    NVPair  *pairs ;			/* Array of name/value pair handles. */
}  _NVList ;


int  nvl_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  nvl_util_debug

/*!*****************************************************************************

Procedure:

    nvlAdd ()

    Add a Name/Value Pair to a List.


Purpose:

    Function nvlAdd() adds a name/value pair to a list.


    Invocation:

        status = nvlAdd (list, pair) ;

    where

        <list>		- I
            is the list handle returned by nvlCreate().
        <pair>		- I
            is a name/value pair handle returned by nvpCreate().
        <status>	- O
            returns the status of adding the pair to the list,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvlAdd (

#    if PROTOTYPES
        NVList  list,
        NVPair  pair)
#    else
        list, pair)

        NVList  list ;
        NVPair  pair ;
#    endif

{    /* Local variables. */
    int  newMax ;
    NVPair  *array ;



    if (list == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvlAdd) NULL list handle: ") ;
        return (errno) ;
    }
    if (pair == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvlAdd) NULL pair handle: ") ;
        return (errno) ;
    }

/* If there isn't enough room for the new member in the pair array,
   then expand the array. */

    if (list->numMembers >= list->maxMembers) {
        newMax = list->maxMembers + 8 ;
        array = (NVPair *) realloc (list->pairs, newMax * sizeof (NVPair)) ;
        if (array == NULL) {
            LGE "(nvlAdd) Error expanding %s list to %d members.\nrealloc: ",
                nvlName (list), newMax) ;
            return (errno) ;
        }
        list->pairs = array ;
        list->maxMembers = newMax ;
    }

/* Add the new pair to the array of pairs. */

    list->pairs[list->numMembers++] = pair ;

    LGI "(nvlAdd) Added name/value pair \"%s\" to list \"%s\".\n",
        nvpName (pair), nvlName (list)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvlCount ()

    Count the Number of Name/Value Pairs in a List.


Purpose:

    Function nvlCount() returns the number of name/value pairs in a list.


    Invocation:

        count = nvlCount (list) ;

    where

        <list>	- I
            is the list handle returned by nvlCreate().
        <count>	- O
            returns the number of name/value pairs in the list.

*******************************************************************************/


int  nvlCount (

#    if PROTOTYPES
        NVList  list)
#    else
        list)

        NVList  list ;
#    endif

{

    return ((list == NULL) ? 0 : list->numMembers) ;

}

/*!*****************************************************************************

Procedure:

    nvlCreate ()

    Create a Name/Value Pair List.


Purpose:

    Function nvlCreate() creates an empty list of name/value pairs.


    Invocation:

        status = nvlCreate (name, &list) ;

    where

        <name>		- I
            is the name that will be bound to the list; this argument
            can be NULL.
        <list>		- O
            returns a handle for the name/value pair list.
        <status>	- O
            returns the status of creating the name/value pair list,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvlCreate (

#    if PROTOTYPES
        const  char  *name,
        NVList  *list)
#    else
        name, list)

        char  *name ;
        NVList  *list ;
#    endif

{

/* Create a name/value pair list with no members. */

    *list = (_NVList *) malloc (sizeof (_NVList)) ;
    if (*list == NULL) {
        LGE "(nvlCreate) Error creating a name/value pair list.\nmalloc: ") ;
        return (errno) ;
    }

    (*list)->numMembers = 0 ;
    (*list)->maxMembers = 0 ;
    (*list)->pairs = NULL ;

    if (name == NULL) {
        (*list)->name = NULL ;
    } else {
        (*list)->name = strdup (name) ;
        if ((*list)->name == NULL) {
            LGE "(nvlCreate) Error duplicating \"%s\".\nstrdup: ", name) ;
            PUSH_ERRNO ;  nvlDestroy (*list) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

    LGI "(nvlCreate) Created an empty name/value pair list, \"%s\".\n",
        nvlName (*list)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvlDelete ()

    Delete a Name/Value Pair from a List.


Purpose:

    Function nvlDelete() deletes a name/value pair from a list; the caller
    is responsible for destroying the pair itself.  The string comparisons
    used to find the pair in the list are case-insensitive, so a pair named
    "abc" can be deleted from its list by calling nvlDelete() on "ABC".


    Invocation:

        pair = nvlDelete (list, name) ;

    where

        <list>	- I
            is the list handle returned by nvlCreate().
        <name>	- I
            is the name of the pair being deleted from the list.  The case
            of the name is ignored.
        <pair>	- O
            returns the handle of the deleted name/value pair; NULL is
            returned if no pair of the specified name is in the list.

*******************************************************************************/


NVPair  nvlDelete (

#    if PROTOTYPES
        NVList  list,
        const  char  *name)
#    else
        list, name)

        NVList  list ;
        char  *name ;
#    endif

{    /* Local variables. */
    int  i ;
    NVPair  pair ;



    if ((list == NULL) || (name == NULL))  return (NULL) ;

/* Locate the pair in the list. */

    pair = nvlFind (list, name) ;
    if (pair == NULL)  return (NULL) ;			/* Not found? */

    for (i = 0 ;  i < list->numMembers ;  i++)
        if (pair == list->pairs[i])  break ;

/* Delete the pair by compacting the list. */

    while (++i < list->numMembers)
        list->pairs[i-1] = list->pairs[i] ;
    list->numMembers-- ;

    LGI "(nvlDelete) Deleted name/value pair \"%s\" from list \"%s\".\n",
        nvpName (pair), nvlName (list)) ;

    return (pair) ;

}

/*!*****************************************************************************

Procedure:

    nvlDestroy ()

    Destroy a Name/Value Pair List.


Purpose:

    Function nvlDestroy() destroys a name/value pair list.  Each pair in
    the list is also destroyed.


    Invocation:

        status = nvlDestroy (list) ;

    where

        <list>		- I
            is the list handle returned by nvlCreate().
        <status>	- O
            returns the status of deleting the list, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvlDestroy (

#    if PROTOTYPES
        NVList  list)
#    else
        list)

        NVList  list ;
#    endif

{    /* Local variables. */
    int  i ;



    if (list == NULL)  return (0) ;

    LGI "(nvlDestroy) Destroying name/value pair list \"%s\".\n",
        nvlName (list)) ;

/* Delete the pairs in the list. */

    if (list->pairs != NULL) {
        for (i = 0 ;  i < list->numMembers ;  i++)
            nvpDestroy (list->pairs[i]) ;
        free (list->pairs) ;
        list->pairs = NULL ;
    }

/* Deallocate the list structure. */

    if (list->name != NULL)  free (list->name) ;
    free (list) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvlDump ()

    Dump a List.


Purpose:

    Function nvlDump() provides a formatted dump of a list.


    Invocation:

        status = nvlDump (file, indent, list) ;

    where:

        <file>		- I
            is the UNIX descriptor for the output file.  If NULL, the
            file pointer defaults to standard output (STDOUT).
        <indent>	- I
            is an optional text string used to indent the output.
        <list>		- I
            is the list handle returned by nvlCreate().
        <status>	- O
            returns the status of dumping the list, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  nvlDump (

#    if PROTOTYPES
        FILE  *file,
        const  char  *indent,
        NVList  list)
#    else
        file, indent, list)

        FILE  *file ;
        char  *indent ;
        NVList  list ;
#    endif

{    /* Local variables. */
    int  i, j ;
    NVList  *array ;
    NVPair  pair ;



    if (list == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvlDump) NULL list handle: ") ;
        return (errno) ;
    }

    if (file == NULL)  file = stdout ;
    if (indent == NULL)  indent = "" ;

/* Print the list name. */

    fprintf (file, "%s[%s]\n", indent, nvlName (list)) ;

/* Print each name/value pair in the list. */

    for (i = 0 ;  i < nvlCount (list) ;  i++) {
        pair = nvlGet (list, i) ;
        fprintf (file, "%s%s\n", indent, nvpEncode (pair)) ;
        if (nvpTypeOf (pair) == NvpList) {
            char  *s = strndup (indent, strlen (indent) + 5) ;
            strcat (s, "    ") ;
            array = nvpValue (pair) ;
            for (j = 0 ;  j < nvpCount (pair) ;  j++)
                nvlDump (file, s, array[j]) ;
            free (s) ;
        }
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvlFind ()

    Find a Name/Value Pair by Name in a List.


Purpose:

    Function nvlFind() retrieves a name/value pair by name from a list.
    The name comparisons are case-insensitive, so "ABC" will find a pair
    named "abc".


    Invocation:

        pair = nvlFind (list, name) ;

    where

        <list>	- I
            is the list handle returned by nvlCreate().
        <name>	- I
            is the name of the desired pair.  The case of the name is
            ignored.
        <pair>	- O
            returns the name/value pair handle if found and NULL otherwise.

*******************************************************************************/


NVPair  nvlFind (

#    if PROTOTYPES
        NVList  list,
        const  char  *name)
#    else
        list, name)

        NVList  list ;
        char  *name ;
#    endif

{    /* Local variables. */
    int  i ;



    if ((list == NULL) || (name == NULL))  return (NULL) ;

    for (i = 0 ;  i < list->numMembers ;  i++) {
        if (strcasecmp (nvpName (list->pairs[i]), name) == 0)
            return (list->pairs[i]) ;			/* Found. */
    }

    return (NULL) ;					/* Not found. */

}

/*!*****************************************************************************

Procedure:

    nvlGet ()

    Get a Name/Value Pair by Index from a List.


Purpose:

    Function nvlGet() retrieves a name/value pair by index (0..N-1) from a list.


    Invocation:

        pair = nvlGet (list, index) ;

    where

        <list>	- I
            is the list handle returned by nvlCreate().
        <index>	- I
            is the index (0..N-1) of the desired pair.
        <pair>	- O
            returns the name/value pair handle; NULL is returned if the index
            is out of bounds.

*******************************************************************************/


NVPair  nvlGet (

#    if PROTOTYPES
        NVList  list,
        int  index)
#    else
        list, index)

        NVList  list ;
        int  index ;
#    endif

{

    if ((list == NULL) || (index < 0) || (list->numMembers <= index))
        return (NULL) ;

    return (list->pairs[index]) ;

}

/*!*****************************************************************************

Procedure:

    nvlName ()

    Get a List's Name.


Purpose:

    Function nvlName() returns the name of a name/value pair list.


    Invocation:

        name = nvlName (list) ;

    where

        <list>	- I
            is the list handle returned by nvlCreate().
        <name>	- O
            returns the list's name.  The name is stored in memory local to
            the NVL utilities and it should not be modified or freed by the
            caller.

*******************************************************************************/


const  char  *nvlName (

#    if PROTOTYPES
        NVList  list)
#    else
        list)

        NVList  list ;
#    endif

{
    if (list == NULL)  return ("") ;
    if (list->name == NULL)  return ("") ;
    return (list->name) ;
}

/*!*****************************************************************************

Procedure:

    xdr_NVList ()

    Encode/Decode a Name/Value Pair List in XDR Format.


Purpose:

    Function xdr_NVList() is an XDR(3)-compatible function that encodes/decodes
    a name/value pair list into/from XDR format.


    Invocation:

        successful = xdr_NVList (xdrStream, &list) ;

    where

        <xdrStream>	- I
            is the XDR stream handle returned by one of the xdrTTT_create()
            functions.
        <list>		- I/O
            is the address of a name/value pair list.  When encoding a list,
            the handle must be for a valid name/value pair list.  Decoding
            a list is slightly more complicated.  If the handle is NULL, a
            brand new list will be created.  If the handle is *not* NULL,
            the incoming name/value pairs are added to the list's existing
            pairs; the decoded name is ignored.
        <successful>	- O
            returns TRUE if the XDR translation was successful and FALSE
            if it was not.

*******************************************************************************/


bool_t  xdr_NVList (

#    if PROTOTYPES
        XDR  *xdrStream,
        NVList  *list)
#    else
        xdrStream, list)

        XDR  *xdrStream ;
        NVList  *list ;
#    endif

{    /* Local pairs. */
    char  *name ;
    NVPair  *array ;
    unsigned  int  i, length ;




/*******************************************************************************
    Decode the list of name/value pairs from the XDR stream.
*******************************************************************************/

    if (xdrStream->x_op == XDR_DECODE) {

/* Decode the list's name.  If the caller passed in an existing list, then
   ignore the incoming name and add the new pairs to the list; otherwise,
   create a brand new list. */

        name = NULL ;
        if (!xdr_string (xdrStream, &name, UINT_MAX))  return (FALSE) ;

        if (*list == NULL) {				/* Create new one? */
            if (nvlCreate (name, list))  return (FALSE) ;
        }

        xdrStream->x_op = XDR_FREE ;			/* Free incoming name. */
        xdr_string (xdrStream, &name, UINT_MAX) ;
        xdrStream->x_op = XDR_DECODE ;

/* Decode the incoming array of name/value pairs. */

        array = NULL ;

        if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                        UINT_MAX, sizeof (NVPair), (xdrproc_t) xdr_NVPair))
            return (FALSE) ;

/* Add the new pairs to the list. */

        for (i = 0 ;  i < length ;  i++) {
            if (nvlAdd (*list, array[i]))  return (FALSE) ;
        }

/* Discard the array. */

        xdrStream->x_op = XDR_FREE ;
        xdr_array (xdrStream, (caddr_t *) &array, &length,
                   UINT_MAX, sizeof (NVPair), (xdrproc_t) xdr_void) ;
        xdrStream->x_op = XDR_DECODE ;

    }

/*******************************************************************************
    Encode the list of name/value pairs into the XDR stream.
*******************************************************************************/

    else if (xdrStream->x_op == XDR_ENCODE) {

/* Encode the list's name. */

        name = (char *) nvlName (*list) ;
        if (!xdr_string (xdrStream, &name, UINT_MAX))  return (FALSE) ;

/* Encode the list of name/value pairs. */

        array = (*list)->pairs ;
        length = nvlCount (*list) ;

        if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                        UINT_MAX, sizeof (NVPair), (xdrproc_t) xdr_NVPair))
            return (FALSE) ;

    }

/*******************************************************************************
    Free a previously allocated list.
*******************************************************************************/

    else if (xdrStream->x_op == XDR_FREE) {

        if (*list != NULL) {
            nvlDestroy (*list) ;
            *list = NULL ;
        }

    }


/*******************************************************************************
    Invalid XDR operation.
*******************************************************************************/

    else {

        return (FALSE) ;

    }


    return (TRUE) ;			/* Success! */

}

/*!*****************************************************************************

Procedure:

    nvlGetProp ()

    Get a Property from a List.


Purpose:

    Function nvlGetProp(), like the LISP GET function it is patterned after,
    is a simple means of getting a "property" (i.e., a name/value pair whose
    value is a string) from a list.  nvlGetProp() finds the name/value pair
    in the list and returns its string value.


    Invocation:

        value = nvlGetProp (list, name) ;

    where

        <list>	- I
            is the list handle returned by nvlCreate().
        <name>	- I
            is the name of the "property".
        <value>	- O
            returns the string value of the property; NULL is returned if
            the property is not found in the list.  The string belongs to
            the NVL utilities and it should not be modified or freed by the
            caller.  The string may need to be used or duplicated before
            calling nvlGetProp() again if the underlying name/value pair
            has a non-string value; this will not happen if nvlPutProp()
            is used to create the property.

*******************************************************************************/


const  char  *nvlGetProp (

#    if PROTOTYPES
        NVList  list,
        const  char  *name)
#    else
        list, name)

        NVList  list ;
        char  *name ;
#    endif

{    /* Local variables. */
    NVPair  pair ;



    if (list == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvlGetProp) NULL list handle: ") ;
        return (NULL) ;
    }

    pair = nvlFind (list, name) ;		/* Find property. */

    if (pair == NULL)				/* Property not found? */
        return (NULL) ;
    else					/* Property found. */
        return (nvpString (pair)) ;

}

/*!*****************************************************************************

Procedure:

    nvlPutProp ()

    Put a Property into a List.


Purpose:

    Function nvlPutProp(), like the LISP PUTPROP function it is patterned
    after, is a simple means of adding a "property" (i.e., a name/value pair
    whose value is a string) to a list.  nvlPutProp() creates the name/value
    pair for the property and adds the pair to the list.


    Invocation:

        status = nvlPutProp (list, name, value) ;

    where

        <list>		- I
            is the list handle returned by nvlCreate().
        <name>		- I
            is the name of the "property".
        <value>		- I
            is the string value of the property.
        <status>	- O
            returns the status of adding the property to the list,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvlPutProp (

#    if PROTOTYPES
        NVList  list,
        const  char  *name,
        const  char  *value)
#    else
        list, name, value)

        NVList  list ;
        char  *name ;
        char  *value ;
#    endif

{    /* Local variables. */
    NVPair  pair ;



    if (list == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvlPutProp) NULL list handle: ") ;
        return (errno) ;
    }

    pair = nvpNew (name, NvpString, value) ;
    if (pair == NULL) {
        LGE "(nvlPutProp) Error creating property %s = %s.\nnvpNew: ",
            (name == NULL) ? "<nil>" : name,
            (value == NULL) ? "<nil>" : value) ;
        return (errno) ;
    }

    if (nvlAdd (list, pair)) {
        LGE "(nvlPutProp) Error creating adding property %s to list %s.\nnvlAdd: ",
            (name == NULL) ? "<nil>" : name, nvlName (list)) ;
        PUSH_ERRNO ;  nvpDestroy (pair) ;  POP_ERRNO ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvlRemProp ()

    Remove a Property from a List.


Purpose:

    Function nvlRemProp(), like the LISP GET REMPROP function it is patterned
    after, is a simple means of removing a "property" (i.e., a name/value pair
    whose value is a string) from a list.  nvlRemProp() deletes the name/value
    pair from the list and then destroys the pair.


    Invocation:

        status = nvlRemProp (list, name) ;

    where

        <list>		- I
            is the list handle returned by nvlCreate().
        <name>		- I
            is the name of the "property".
        <status>	- O
            returns the status of removing the property from the list,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvlRemProp (

#    if PROTOTYPES
        NVList  list,
        const  char  *name)
#    else
        list, name)

        NVList  list ;
        char  *name ;
#    endif

{    /* Local variables. */
    NVPair  pair ;



    if (list == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvlRemProp) NULL list handle: ") ;
        return (errno) ;
    }

    pair = nvlDelete (list, name) ;		/* Delete property from list. */
    if (pair == NULL) {				/* Property not found? */
        SET_ERRNO (EINVAL) ;
        return (errno) ;
    }

    nvpDestroy (pair) ;				/* Destroy property. */

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the NVL_UTIL functions.

*******************************************************************************/

int  main (argc, argv)
    int  argc ;
    char  *argv[] ;
{    /* Local variables. */
    int  i ;
    NVPair  pair ;
    NVList  list, myCar, nest ;



    nvp_util_debug = 1 ;
    nvl_util_debug = 1 ;

    nvlCreate (NULL, &list) ;
    nvlAdd (list, nvpNew ("THIS", NvpDouble, 123.45)) ;
    nvlAdd (list, nvpNew ("THAT", NvpLong, 6789)) ;

    nvlCreate ("NESTED", &nest) ;
    nvlAdd (nest, nvpNew ("WHO", NvpDouble, 123.45)) ;
    nvlAdd (nest, nvpNew ("WHAT", NvpLong, 6789)) ;

    nvlAdd (list, nvpNew ("OTHER", NvpList, nest)) ;

    for (i = 0 ;  i < nvlCount (list) ;  i++) {
        pair = nvlGet (list, i) ;
        printf ("%s = %s\n", nvpName (pair), nvpString (pair)) ;
    }

    printf ("-----\n") ;
    nvlDump (NULL, NULL, list) ;
    printf ("-----\n") ;

    pair = nvlFind (list, "THAT") ;		/* Prints "THAT = 6789". */
    printf ("%s = %s\n", nvpName (pair), nvpString (pair)) ;

    pair = nvlDelete (list, "THIS") ;		/* Prints "THIS = 123.45". */
    printf ("%s = %s\n", nvpName (pair), nvpString (pair)) ;
    nvpDestroy (pair) ;

    nvlDestroy (list) ;

    printf ("-----\n") ;

    nvlCreate (NULL, &myCar) ;
    nvlPutProp (myCar, "MAKE", "Triumph") ;
    nvlPutProp (myCar, "MODEL", "TR-4") ;
    nvlPutProp (myCar, "COLOR", "green") ;
    printf ("I wish I had a %s %s %s.\n",
            nvlGetProp (myCar, "COLOR"),
            nvlGetProp (myCar, "MAKE"),
            nvlGetProp (myCar, "MODEL")) ;
    nvlRemProp (myCar, "MODEL") ;
    nvlDestroy (myCar) ;

    exit (0) ;

}

#endif  /* TEST */
