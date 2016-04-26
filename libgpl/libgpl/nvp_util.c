/* $Id: nvp_util.c,v 1.11 2011/07/18 17:47:24 alex Exp $ */
/*******************************************************************************

File:

    nvp_util.c

    Name/Value Pair Utilities.


Author:    Alex Measday


Purpose:

    The NVP_UTIL package implements name/value pairs; i.e., binding a name
    to a value.  A name/value pair is initially created without a value:

        #include  "nvp_util.h"			-- Name/value pairs.
        NVPair  pair ;
        ...
        nvpCreate ("pieceOfInfo", &pair) ;

    An unbound pair can then be assigned values of different types:

        #include  "tv_util.h"			-- UNIX timeval utilities.
        ...
        nvpAssign (pair, 1, NvpByte, 0x12) ;
        nvpAssign (pair, 1, NvpDouble, 123.45) ;
        nvpAssign (pair, 1, NvpLong, 678) ;
        nvpAssign (pair, 1, NvpTime, tvTOD ()) ;

    The pairs above have all been assigned scalar values.  Arrays of values
    can also be assigned to pairs:

        #define  MAX  50
        double  anArray[MAX] ;
        ...
        nvpAssign (pair, MAX, NvpDouble, anArray, NvpVolatile) ;

    Strings are null-terminated, so no length needs to be specified:

        nvpAssign (variable, -1, NvpString, "Hello", NvpStatic) ;

    An application can specify the "storage class" of a string or array:
    static, dynamic, or volatile.  In the volatile example above, the
    name/value pair variable is bound to a *copy* of the double array;
    changes made to the bound array are not reflected in the original
    array and the bound array is free(3)ed when the name/value pair is
    deleted.  Dynamic values are assumed to have already been "copied"
    (e.g., a strdup(3)ed string) and, like volatile values, will be
    free(3)ed when the name/value pair is deleted.  In contrast, a static
    value (e.g., the "Hello" string constant above, or a global variable)
    exists independently of the name/value pair to which it is bound; the
    name/value pair directly references the value and the value is not
    free(3)ed when the name/value pair is deleted.

    A shorthand means of creating name/value pairs bound to scalars and
    volatile strings is provided by nvpNew():

        NVPair  tic, tac, toe ;
        ...
        tic = nvpNew ("tic", NvpLong, 123) ;
        tac = nvpNew ("tac", NvpDouble, 4.56) ;
        toe = nvpNew ("toe", NvpString, "automatically duplicated") ;

    Various attributes of a pair's value can be retrieved:

        size_t  elementSize = nvpSizeOf (variable) ;
        size_t  numElements = nvpCount (variable) ;
        NvpDataType  type = nvpTypeOf (variable) ;

    A pointer to a pair's value is returned by nvpValue(); the application
    is responsible for casting the pointer to the correct data type before
    dereferencing it:

        printf ("%s = %ld\n", nvpName (tic), *((long *) nvpValue (tic))) ;
        printf ("%s = %g\n", nvpName (tac), *((double *) nvpValue (tac))) ;
        printf ("%s = %s\n", nvpName (toe), (char *) nvpValue (toe)) ;

    As with strings, an array value of any type is always returned as a
    pointer to the base of the array:

        struct  timeval  *array ;
        static  struct  timeval  tv[16] ;
        ...
        nvpAssign (pair, 16, NvpTime, tv, NvpStatic) ;
        nvpValue (pair) ;				-- Returns &tv[0]

    Finally, a name/value pair can be destroyed:

        nvpDestroy (pair) ;

    Depending on the storage class of the pair's value, the value may be
    free(3)ed by nvpDestroy().


History:

    The name/value pair, name/value list, and version-independent message
    stream packages were inspired by Mike Maloney's C++ implementations of
    "named variables" and "named variable sets", and by Robert Martin's
    "attributed data trees" (see "Version-Independent Messages" in Appendix B
    of his DESIGNING OBJECTED-ORIENTED C++ APPLICATIONS USING THE BOOCH METHOD).


Public Procedures:

    nvpAssign() - assigns a value to a name/value pair.
    nvpCount() - returns the number of elements in a pair's value.
    nvpCreate() - creates a name/value pair.
    nvpDecode() - creates a name/value pair from an ASCII specification.
    nvpDestroy() - destroys a name/value pair.
    nvpEncode() - returns the ASCII specification of a name/value pair.
    nvpName() - returns a variable's name.
    nvpNew() - creates a name/value pair with a value.
    nvpSizeOf() - returns the size of a scalar value or array element.
    nvpTypeOf() - returns the data type of a variable's value.
    nvpValue() - returns a variable's value.
    xdr_NVPair() - encodes/decodes a name/value pair in XDR format.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#if HAVE_STDARG_H
#    include  <stdarg.h>		/* Variable-length argument lists. */
#else
#    include  <varargs.h>		/* Variable-length argument lists. */
#endif
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "nvl_util.h"			/* Lists of name/value pairs. */
#include  "rex_util.h"			/* Regular expression definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "nvp_util.h"			/* Name/value pairs. */


/*******************************************************************************
    Name/Value Pair - binds a name to a data value.
*******************************************************************************/

typedef  struct  _NVPair {
    char  *name ;			/* Name of variable. */
    NvpDataType  dataType ;		/* Value's data type. */
    int  numElements ;			/* 1 for scalar, N for array. */
    NvpStorageClass  storageClass ;	/* Memory allocation type. */
    void  *value ;			/* Pointer to scalar value or array. */
}  _NVPair ;


int  nvp_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  nvp_util_debug

/*******************************************************************************
    Name/Type Table - maps type names to type and vice-versa.
*******************************************************************************/

typedef  struct  TypeInfo {
    NvpDataType  dataType ;
    const  char  *name ;
}  TypeInfo ;

static  TypeInfo  typeInfo[]  OCD ("nvp_util")  = {
  { NvpByte, "BYTE" },
  { NvpDouble, "DOUBLE" },
  { NvpLong, "LONG" },
  { NvpString, "STRING" },
  { NvpTime, "TIMEVAL" },
  { NvpList, "LIST" },
					/* Last entry in table. */
  { NvpUndefined, NULL }
} ;

/*!*****************************************************************************

Procedure:

    nvpAssign ()

    Assign a Value to a Name/Value Pair.


Purpose:

    Function nvpAssign() assigns a new value to a name/value pair.  The
    previous value, if any, is erased.


    Invocation:

        status = nvpAssign (pair, numElements, dataType, value[, storageType]) ;

    where

        <pair>		- I
            is the name/value pair handle returned by nvpCreate().
        <numElements>	- I
            is the number of elements in the value: 1 for scalar values,
            N for arrays, and -1 for strings.
        <dataType>	- I
            is the data type of the value; see the enumerated NvpDataType
            definition in "nvp_util.h".  If NvpUndefined is specified as
            the data type, the pair's current value is erased, but no new
            value is assigned; i.e., the pair is now unbound.
        <value>		- I
            is the new value for the name/value pair.  For all scalar values
            except strings, pass in the actual value.  For arrays and strings,
            pass in the address of the value; the disposition of the values
            passed in in this way depends on the storage type argument below.
        <storageType>	- I
            is the storage type of VALUE for arrays and strings:
                NvpStatic - the name/value pair is bound to the actual value.
                NvpDynamic - the pair is bound to the actual value,
                    which will be free(3)ed when the pair is deleted.
                NvpVolatile - the pair is bound to a *copy* of the value.
        <status>	- O
            returns the status of assigning a value to the name/value pair,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvpAssign (

#    if PROTOTYPES
        NVPair  pair,
        int  numElements,
        NvpDataType  dataType,
        ...)
#    else
        pair, numElements, dataType, va_alist)

        NVPair  pair ;
        int  numElements ;
        NvpDataType  dataType ;
        va_dcl
#    endif

{    /* Local variables. */
    int  i ;
    size_t  elementSize ;
    va_list  ap ;
    void  *value ;





    if (pair == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvpAssign) NULL pair handle: ") ;
        return (errno) ;
    }


/*******************************************************************************
    Erase the old value.
*******************************************************************************/

    if (pair->dataType != NvpUndefined) {
        if (pair->dataType == NvpList) {
            NVList  *array = pair->value ;
            for (i = 0 ;  i < pair->numElements ;  i++)
                nvlDestroy (array[i]) ;
        }
        if ((pair->storageClass == NvpDynamic) ||
            (pair->storageClass == NvpVolatile)) {
            if (pair->value != NULL)  free (pair->value) ;
        }
        pair->value = NULL ;
        pair->dataType = NvpUndefined ;
    }

    if (dataType == NvpUndefined)  return (0) ;		/* Erase only? */


/*******************************************************************************
    Determine the size in bytes of the new value.
*******************************************************************************/

    switch (dataType) {
    case NvpByte:
        elementSize = sizeof (unsigned char) ;  break ;
    case NvpDouble:
        elementSize = sizeof (double) ;  break ;
    case NvpLong:
        elementSize = sizeof (long) ;  break ;
    case NvpString:
        elementSize = sizeof (char) ;  break ;
    case NvpTime:
        elementSize = sizeof (struct timeval) ;  break ;
    case NvpList:
        elementSize = sizeof (NVList) ;  break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(nvpAssign) Invalid data type, %d, for %s.\n",
            dataType, nvpName (pair)) ;
        return (errno) ;
    }

/*******************************************************************************
    Scalar Value - make a copy of the value.
*******************************************************************************/

#if HAVE_STDARG_H
    va_start (ap, dataType) ;
#else
    va_start (ap) ;
#endif

    if ((numElements == 1) && (dataType != NvpString)) {

        pair->value = (void *) malloc (elementSize) ;
        if (pair->value == NULL) {
            LGE "(nvpAssign) Error allocating value for %s.\nmalloc: ",
                nvpName (pair)) ;
            return (errno) ;
        }
        pair->storageClass = NvpVolatile ;

        switch (dataType) {
        case NvpByte:
            *((unsigned char *) pair->value) = va_arg (ap, int) ;
            break ;
        case NvpDouble:
            *((double *) pair->value) = va_arg (ap, double) ;
            break ;
        case NvpLong:
            *((long *) pair->value) = va_arg (ap, long) ;
            break ;
        case NvpTime:
            *((struct timeval *) pair->value) = va_arg (ap, struct timeval) ;
            break ;
        case NvpList:
            *((NVList *) pair->value) = va_arg (ap, NVList) ;
            break ;
        default:
            break ;
        }

    }

/*******************************************************************************
    Strings and Arrays - for static and dynamic values, simply save the given
        pointer to the value.  For volatile values, make a copy of the value.
*******************************************************************************/

    else {

        value = va_arg (ap, void *) ;
        pair->storageClass = (NvpStorageClass) va_arg (ap, int) ;
        if (dataType == NvpString) {
            numElements = (value == NULL) ? 0 : strlen ((char *) value) ;
            numElements++ ;
        }

        switch (pair->storageClass) {
        case NvpDynamic:
        case NvpStatic:
            pair->value = value ;
            break ;
        case NvpVolatile:
            pair->value = calloc (numElements, elementSize) ;
            if (pair->value == NULL) {
                LGE "(nvpAssign) Error allocating value for %s.\ncalloc: ",
                    nvpName (pair)) ;
                return (errno) ;
            }
            if (value != NULL) {
                (void) memcpy (pair->value, value, numElements * elementSize) ;
                if (dataType == NvpString)
                    ((char *) (pair->value))[numElements-1] = '\0' ;
            }
            break ;
        default:
            SET_ERRNO (EINVAL) ;
            LGE "(nvpAssign) Invalid storage class, %d, for %s.\n",
                pair->storageClass, nvpName (pair)) ;
            return (errno) ;
        }

    }


    va_end (ap) ;

    pair->dataType = dataType ;
    pair->numElements = numElements ;

    LGI "(nvpAssign) %s = %s\n", nvpName (pair), nvpString (pair)) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvpCount ()

    Count the Number of Elements in a Name/Value Pair's Value.


Purpose:

    Function nvpCount() returns the number of elements in a name/value pair's
    value.


    Invocation:

        numElements = nvpCount (pair) ;

    where

        <pair>	- I
            is the name/value pair handle returned by nvpCreate().
        <count>	- O
            returns the number of elements in a name/value pair's value,
            1 for a scalar value, N for an array, and the length of the
            string for a string value.  -1 is returned if an invalid pair
            was specified or the pair is unbound.

*******************************************************************************/


int  nvpCount (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{

    if ((pair == NULL) || (pair->dataType == NvpUndefined))
        return (-1) ;
    else if (pair->dataType == NvpString)
        return (pair->numElements - 1) ;	/* Exclude null character. */
    else
        return (pair->numElements) ;

}

/*!*****************************************************************************

Procedure:

    nvpCreate ()

    Create a Name/Value Pair.


Purpose:

    Function nvpCreate() creates an unbound name/value pair.  nvpAssign()
    must be called to bind a value to the name.


    Invocation:

        status = nvpCreate (name, &pair) ;

    where

        <name>		- I
            is the name that will be bound to a value.
        <pair>		- O
            returns a handle for the name/value pair.
        <status>	- O
            returns the status of creating the name/value pair,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvpCreate (

#    if PROTOTYPES
        const  char  *name,
        NVPair  *pair)
#    else
        name, pair)

        char  *name ;
        NVPair  *pair ;
#    endif

{    /* Local pairs. */
    char  internalName[16] ;



/* Create a name/value pair that is NOT bound to a value (yet). */

    *pair = (_NVPair *) malloc (sizeof (_NVPair)) ;
    if (*pair == NULL) {
        LGE "(nvpCreate) Error creating \"%s\" name/value pair.\nmalloc: ",
            name) ;
        return (errno) ;
    }

    (*pair)->dataType = NvpUndefined ;
    (*pair)->value = NULL ;

    if (name == NULL) {
        sprintf (internalName, "NVP_%p", (void *) *pair) ;
        name = internalName ;
    }

    (*pair)->name = strdup (name) ;
    if ((*pair)->name == NULL) {
        LGE "(nvpCreate) Error duplicating \"%s\".\nstrdup: ", name) ;
        PUSH_ERRNO ;  nvpDestroy (*pair) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(nvpCreate) Created unbound name/value pair, \"%s\".\n", name) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvpDecode ()

    Decode a Name/Value Pair from an ASCII Specification.


Purpose:

    Function nvpDecode() creates a name/value pair from its specification in
    an ASCII string:

        <name> [=] <value>

    where the equals sign is optional and white space on either side of the
    equals sign is ignored.

    Only scalar values are currently decoded.  The data type of the value is
    determined according to the following rules, which are applied in order:

        NvpLong - if the value can be converted to an integer and the
            conversion consumes the entire value string

        NvpDouble - if the value can be converted to a real number and
            the conversion consumes the entire value string.

        NvpString - if all else fails.


    Invocation:

        pair = nvpDecode (spec) ;

    where

        <spec>		- I
            is the specification, "<name> [=] <value>", of a name/value pair.
        <pair>	- O
            returns a handle for the name/value pair; NULL is returned in the
            event of an error.

*******************************************************************************/


NVPair  nvpDecode (

#    if PROTOTYPES
        const  char  *spec)
#    else
        spec)

        char  *spec ;
#    endif

{    /* Local pairs. */
    char  *fieldStart[3], *matchStart, *name, *s, *value ;
    double  real ;
    int  fieldLength[3], i, matchLength, numElements, status ;
    long  integer ;
    NVList  list ;
    NVPair  pair ;
    NvpDataType  dataType ;
    static  CompiledRE  pattern  OCD ("nvp_util")  = NULL ;




/*******************************************************************************
    Parse the left side of the specification.
*******************************************************************************/

/* Compile the regular expression that parses the "<name>?(<type>?[<count>]?)?="
   portion of a specification.  <name> is assigned to $0, <type> to $1, and
   <count> to $2. */

#define  SPEC_PATTERN	\
    "^[:space:]*([:alpha:][^:space:\\(=]*)$0(\\([:space:]*([:alpha:_]+)$1([:space:]*\\[[:space:]*([:digit:]+)$2[:space:]*\\])?[:space:]*\\))?[:space:]*(=|[:space:])?[:space:]*"

    if ((pattern == NULL) && rex_compile (SPEC_PATTERN, &pattern)) {
        LGE "(nvpDecode) Error compiling regular expression: \"%s\"\nrex_compile: ",
            SPEC_PATTERN) ;
        pattern = NULL ;
        return (NULL) ;
    }

/* Parse the left side of the specification. */

    if (!rex_match (spec, pattern, &matchStart, &matchLength, 3,
                    &fieldStart[0], &fieldLength[0],
                    &fieldStart[1], &fieldLength[1],
                    &fieldStart[2], &fieldLength[2])) {
        SET_ERRNO (EINVAL) ;
        LGE "(nvpDecode) Invalid specification: %s\n",
            (spec == NULL) ? "<nil>" : spec) ;
        return (NULL) ;
    }

/* Duplicate the name. */

    name = strndup (fieldStart[0], fieldLength[0]) ;
    if (name == NULL) {
        LGE "(nvpDecode) Error duplicating name: %s\nstrndup: ", spec) ;
        return (NULL) ;
    }

/* Lookup the data type, if it was specified. */

    dataType = NvpUndefined ;
    if (fieldStart[1] != NULL) {
        for (i = 0 ;  typeInfo[i].name != NULL ;  i++) {
            if (strncasecmp (typeInfo[i].name,
                             fieldStart[1], fieldLength[1]) == 0)
                break ;
        }
        dataType = typeInfo[i].dataType ;
        if (dataType == NvpUndefined) {
            SET_ERRNO (EINVAL) ;
            LGE "(nvpDecode) Invalid data type: %s\n", spec) ;
            return (NULL) ;
        }
    }

/* Process the array size, if it was specified. */

    numElements = 1 ;
    if (fieldStart[2] != NULL) {
        numElements = strtol (fieldStart[2], &s, 0) ;
        if ((s != (fieldStart[2] + fieldLength[2])) || (numElements < 1)) {
            SET_ERRNO (EINVAL) ;
            LGE "(nvpDecode) Invalid array size: %s\n", spec) ;
            return (NULL) ;
        }
    }

/*******************************************************************************
    Create the name/value pair as specified by the caller.
*******************************************************************************/

/* Create an unbound pair for the name. */

    status = nvpCreate (name, &pair) ;  free (name) ;
    if (status) {
        SET_ERRNO (status) ;
        LGE "(nvpDecode) Error creating unbound pair: %s\nnvpCreate: ", spec) ;
        return (NULL) ;
    }

/* If a data type was specified, then construct a pair of that type. */

    if (numElements > 1) {

        status = nvpAssign (pair, numElements, dataType, (void *) NULL,
                            NvpVolatile) ;

    } else {

        switch (dataType) {
        case NvpByte:
            status = nvpAssign (pair, 1, dataType, (unsigned char) 0) ;
            break ;
        case NvpDouble:
            status = nvpAssign (pair, 1, dataType, (double) 0.0) ;
            break ;
        case NvpLong:
            status = nvpAssign (pair, 1, dataType, (long) 0) ;
            break ;
        case NvpString:
            status = nvpAssign (pair, 1, dataType, (char *) NULL, NvpVolatile) ;
            break ;
        case NvpTime:
            status = nvpAssign (pair, 1, dataType, tvCreate (0, 0)) ;
            break ;
        case NvpList:
            status = nvlCreate (NULL, &list) ;
            if (status)  break ;
            status = nvpAssign (pair, 1, dataType, list) ;
            break ;
        default:
            status = 0 ;
            break ;
        }

    }

    if (status) {
        LGE "(nvpDecode) Error assigning value: %s\nnvpAssign: ", spec) ;
        return (NULL) ;
    }

/*******************************************************************************
    Assign a value to the name/value pair.
*******************************************************************************/

/* Advance to the value field in the specification. */

    value = matchStart + matchLength ;

/* If there's no value specified, return the name/value pair unbound. */

    if (*value == '\0') {
        LGI "(nvpDecode) %s=\n", nvpName (pair)) ;
        return (pair) ;
    }

/* If an array was specified, then decode the individual elements' values. */

    if (numElements > 1) {

        switch (dataType) {
        case NvpByte:
          { unsigned  char  *array = (unsigned char *) nvpValue (pair) ;
            s = strtok (value, " \t,") ;
            while (s != NULL) {
                *array++ = (unsigned char) strtoul (s, NULL, 0) ;
                s = strtok (NULL, " \t,") ;
            }
            LGI "(nvpDecode) %s=%s\n", nvpName (pair), nvpString (pair)) ;
            return (pair) ;
          }
        case NvpDouble:
          { double  *array = (double *) nvpValue (pair) ;
            s = strtok (value, " \t,([") ;
            while (s != NULL) {
                *array++ = strtod (s, NULL) ;
                s = strtok (NULL, " \t,)]") ;
            }
            LGI "(nvpDecode) %s=%s\n", nvpName (pair), nvpString (pair)) ;
            return (pair) ;
          }
        case NvpLong:
          { long  *array = (long *) nvpValue (pair) ;
            s = strtok (value, " \t,([") ;
            while (s != NULL) {
                *array++ = strtol (s, NULL, 0) ;
                s = strtok (NULL, " \t,)]") ;
            }
            LGI "(nvpDecode) %s=%s\n", nvpName (pair), nvpString (pair)) ;
            return (pair) ;
          }
        default:
            break ;
        }

   }

/* If the value can be interpreted as an integer and the conversion consumes
   the entire value string, then bind the name to the integer value. */

    integer = strtol (value, &s, 0) ;
    if ((*s == '\0') &&
        ((dataType == NvpUndefined) || (dataType == NvpLong))) {
        if (nvpAssign (pair, 1, NvpLong, integer)) {
            LGE "(nvpDecode) Error assigning %ld to %s.\nnvpAssign: ",
                integer, nvpName (pair)) ;
            PUSH_ERRNO ;  nvpDestroy (pair) ;  POP_ERRNO ;
            return (NULL) ;
        }
        LGI "(nvpDecode) %s=%s\n", nvpName (pair), nvpString (pair)) ;
        return (pair) ;
    }


/* If the value can be interpreted as a real number and the conversion
   consumes the entire value string, then bind the name to the real value. */

    real = strtod (value, &s) ;
    if ((*s == '\0') &&
        ((dataType == NvpUndefined) || (dataType == NvpDouble))) {
        if (nvpAssign (pair, 1, NvpDouble, real)) {
            LGE "(nvpDecode) Error assigning %g to %s.\nnvpAssign: ",
                real, nvpName (pair)) ;
            PUSH_ERRNO ;  nvpDestroy (pair) ;  POP_ERRNO ;
            return (NULL) ;
        }
        LGI "(nvpDecode) %s=%s\n", nvpName (pair), nvpString (pair)) ;
        return (pair) ;
    }

/* If all else fails, treat the value as a string: resolve embedded quotes in
   the string and assign it to the name/value pair. */

    if (*value == '\"') {			/* Remove surrounding quotes. */
        value = strdup (&value[1]) ;
        if (value != NULL) {
            s = strrchr (value, '\"') ;
            if (s != NULL)  *s = '\0' ;
        }
    } else {
        value = strdup (value) ;
    }

    if (value == NULL) {
        LGE "(nvpDecode) Error duplicating value string: %s.\nstrdup: ", spec) ;
        PUSH_ERRNO ;  nvpDestroy (pair) ;  POP_ERRNO ;
        return (NULL) ;
    }

    strDestring (value, -1, "\"'{") ;

    if (((dataType == NvpUndefined) || (dataType == NvpString)) &&
        nvpAssign (pair, -1, NvpString, value, NvpDynamic)) {
        LGE "(nvpDecode) Error assigning \"%s\" to %s.\nnvpAssign: ",
            value, nvpName (pair)) ;
        PUSH_ERRNO ;  nvpDestroy (pair) ;
        free (value) ;  POP_ERRNO ;
        return (NULL) ;
    }

    LGI "(nvpDecode) %s=%s\n", nvpName (pair), nvpString (pair)) ;

    return (pair) ;

}

/*!*****************************************************************************

Procedure:

    nvpDestroy ()

    Destroy a Name/Value Pair.


Purpose:

    Function nvpDestroy() destroys a name/value pair.  If the pair's
    value was dynamically allocated, it is deallocated.


    Invocation:

        status = nvpDestroy (pair) ;

    where

        <pair>		- I
            is the name/value pair handle returned by nvpCreate().
        <status>	- O
            returns the status of deleting the pair, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  nvpDestroy (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{

    if (pair == NULL)  return (0) ;

    LGI "(nvpDestroy) Destroying \"%s\" ...\n", nvpName (pair)) ;

/* Erase the pair's value. */

    nvpAssign (pair, -1, NvpUndefined) ;

/* Deallocate the name/value pair structure. */

    if (pair->name != NULL)  free (pair->name) ;
    free ((char *) pair) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    nvpEncode ()

    Encode a Name/Value Pair as an ASCII Specification.


Purpose:

    Function nvpEncode() encodes the specification of a name/value pair in
    an ASCII string with the following format:

        <name>(<type>:<count>)=<value>


    Invocation:

        spec = nvpEncode (pair) ;

    where

        <pair>	- I
            is the name/value pair handle returned by nvpCreate().
        <spec>	- O
            returns the ASCII string specification of the name/value pair;
            NULL is returned in the event of an error.  The specification,
            stored in memory local to the NVP utilities, should be used or
            duplicated before calling nvpEncode() again.

*******************************************************************************/


const  char  *nvpEncode (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{    /* Local pairs. */
    int  i ;
    static  char  spec[1024]  OCD ("nvp_util") ;




    if (pair == NULL)  return (NULL) ;

    if ((pair->dataType == NvpUndefined) || (pair->value == NULL)) {

        sprintf (spec, "%s =", nvpName (pair)) ;

    } else if (pair->dataType == NvpString) {

        sprintf (spec, "%s = \"%s\"",
                 nvpName (pair), nvpString (pair)) ;

    } else {

        for (i = 0 ;  typeInfo[i].name != NULL ;  i++)
            if (typeInfo[i].dataType == pair->dataType)  break ;

        if (typeInfo[i].name == NULL)
            sprintf (spec, "%s = %s",
                     nvpName (pair), nvpString (pair)) ;
        else if (pair->numElements > 1)
            sprintf (spec, "%s(%s[%d]) = %s",
                     nvpName (pair), typeInfo[i].name,
                     pair->numElements, nvpString (pair)) ;
        else
            sprintf (spec, "%s(%s) = %s",
                     nvpName (pair), typeInfo[i].name,
                     nvpString (pair)) ;

    }

    return (spec) ;

}

/*!*****************************************************************************

Procedure:

    nvpName ()

    Get a Name/Value Pair's Name.


Purpose:

    Function nvpName() returns the name of a name/value pair.


    Invocation:

        name = nvpName (pair) ;

    where

        <pair>	- I
            is the name/value pair handle returned by nvpCreate().
        <name>	- O
            returns the pair's name.  The name is stored in memory local
            to the NVP utilities and it should not be modified or freed by
            the caller.

*******************************************************************************/


const  char  *nvpName (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{
    if (pair == NULL)  return ("") ;
    if (pair->name == NULL)  return ("") ;
    return (pair->name) ;
}

/*!*****************************************************************************

Procedure:

    nvpNew ()

    Create a Name/Value Pair with a Value.


Purpose:

    Function nvpNew() provides a shorthand method of creating a name/value pair
    and assigning it a value.


    Invocation:

        pair = nvpNew (name, dataType, value) ;

    where

        <name>		- I
            is the name that will be bound to a value.
        <dataType>	- I
            is the data type of the value; see the enumerated NvpDataType
            definition in "nvp_util.h".  NvpStrings are assumed to be volatile;
            all other data types are assumed to be scalars.
        <value>		- I
            is the new value for the name/value pair.  Pass in numbers by value
            and strings by address.
        <pair>		- O
            returns a handle for the name/value pair; a NULL handle is returned
            in the event of an error.

*******************************************************************************/


NVPair  nvpNew (

#    if PROTOTYPES
        const  char  *name,
        NvpDataType  dataType,
        ...)
#    else
        name, dataType, va_alist)

        char  *name ;
        NvpDataType  dataType ;
        va_dcl
#    endif

{    /* Local pairs. */
    NVPair  pair ;
    va_list  ap ;




/* Create an unbound name/value pair. */

    if (nvpCreate (name, &pair)) {
        LGE "(nvpNew) Error creating unbound pair for %s.\n", name) ;
        return (NULL) ;
    }

/* Assign the value to the pair. */

#if HAVE_STDARG_H
    va_start (ap, dataType) ;
#else
    va_start (ap) ;
#endif

    switch (dataType) {
    case NvpByte:
        if (nvpAssign (pair, 1, dataType, va_arg (ap, int))) {
            LGE "(nvpNew) Error assigning byte value to %s.\nnvpAssign: ",
                name) ;
            break ;
        }
        return (pair) ;
    case NvpDouble:
        if (nvpAssign (pair, 1, dataType, va_arg (ap, double))) {
            LGE "(nvpNew) Error assigning double value to %s.\nnvpAssign: ",
                name) ;
            break ;
        }
        return (pair) ;
    case NvpLong:
        if (nvpAssign (pair, 1, dataType, va_arg (ap, long))) {
            LGE "(nvpNew) Error assigning long value to %s.\nnvpAssign: ",
                name) ;
            break ;
        }
        return (pair) ;
    case NvpString:
        if (nvpAssign (pair, -1, dataType, va_arg (ap, char *), NvpVolatile)) {
            LGE "(nvpNew) Error assigning string value to %s.\nnvpAssign: ",
                name) ;
            break ;
        }
        return (pair) ;
    case NvpTime:
        if (nvpAssign (pair, 1, dataType, va_arg (ap, struct timeval))) {
            LGE "(nvpNew) Error assigning time value to %s.\nnvpAssign: ",
                name) ;
            break ;
        }
        return (pair) ;
    case NvpList:
        if (nvpAssign (pair, 1, dataType, va_arg (ap, NVList))) {
            LGE "(nvpNew) Error assigning tree to %s.\nnvpAssign: ", name) ;
            break ;
        }
        return (pair) ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(nvpNew) Data type %d unsupported by nvpNew().\n",
            dataType, name) ;
        break ;
    }

/* An error occurred; destroy the unbound pair and return a NULL handle. */

    PUSH_ERRNO ;  nvpDestroy (pair) ;  POP_ERRNO ;

    return (NULL) ;

}

/*!*****************************************************************************

Procedure:

    nvpSizeOf ()

    Get the Size of the Value in a Name/Value Pair.


Purpose:

    Function nvpSizeOf() returns the size in bytes of a name/value pair's
    value; in the case of an array, the size of an individual element is
    returned.


    Invocation:

        numBytes = nvpSizeOf (pair) ;

    where

        <pair>	- I
            is the name/value pair handle returned by nvpCreate().
        <size>	- O
            returns the size in bytes of an individual element of a named
            pair's value.

*******************************************************************************/


size_t  nvpSizeOf (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{

    if (pair == NULL)  return (0) ;

    switch (pair->dataType) {
    case NvpByte:
        return (sizeof (unsigned char)) ;
    case NvpString:
        return (sizeof (char)) ;
    case NvpDouble:
        return (sizeof (double)) ;
    case NvpLong:
        return (sizeof (long)) ;
    case NvpTime:
        return (sizeof (struct timeval)) ;
    case NvpList:
        return (sizeof (NVList)) ;
    default:
        return (0) ;
    }

}

/*!*****************************************************************************

Procedure:

    nvpString ()

    Format a Name/Value Pair's Value as a String.


Purpose:

    Function nvpString() formats a name/value pair's value as an ASCII string.


    Invocation:

        asciiValue = nvpString (pair) ;

    where

        <pair>		- I
            is the name/value pair handle returned by nvpCreate().
        <asciiValue>	- O
            returns the name/value pair's value, formatted as an ASCII string.

*******************************************************************************/


const  char  *nvpString (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{    /* Local pairs. */
    char  *s ;
    int  count, i, maxLength ;
#define  MAXLEN  1024
    static  char  asciiValue[MAXLEN]  OCD ("nvp_util") ;




    if (pair == NULL)  return ("<none>") ;

/* Format the value. */

    asciiValue[0] = '\0' ;
    s = asciiValue ;

    count = nvpCount (pair) ;

    switch (pair->dataType) {
    case NvpByte:
        maxLength = MAXLEN - 3 ;
        if (maxLength/2 < count)  count = maxLength / 2 ;
        strcpy (s, "0x") ;  s = s + strlen (s) ;
        for (i = 0 ;  i < count ;  i++) {
            sprintf (&s[i*2], "%02X",
                     (int) ((unsigned char *) pair->value)[i]) ;
        }
        break ;
    case NvpDouble:
        maxLength = MAXLEN - 1 ;
        if (maxLength/12 < count)  count = maxLength / 12 ;
        for (i = 0 ;  i < count ;  i++) {
            if (i > 0)  *s++ = ',' ;
            sprintf (s, "%g", ((double *) pair->value)[i]) ;
            s = s + strlen (s) ;
        }
        break ;
    case NvpLong:
        maxLength = MAXLEN - 1 ;
        if (maxLength/12 < count)  count = maxLength / 12 ;
        for (i = 0 ;  i < count ;  i++) {
            if (i > 0)  *s++ = ',' ;
            sprintf (s, "%ld", ((long *) pair->value)[i]) ;
            s = s + strlen (s) ;
        }
        break ;
    case NvpString:
        return ((pair->value == NULL) ? "<nil>" : pair->value) ;
    case NvpTime:
        strcpy (s, tvShow (*((struct timeval *) pair->value), 0, NULL)) ;
        break ;
    case NvpList:
        return ("<list>") ;
    default:
        return ("<undef>") ;
    }

    return (asciiValue) ;

}

/*!*****************************************************************************

Procedure:

    nvpTypeOf ()

    Get the Data Type of a Name/Value Pair's Value.


Purpose:

    Function nvpTypeOf() returns the data type of a name/value pair's value.


    Invocation:

        dataType = nvpTypeOf (pair) ;

    where

        <pair>		- I
            is the name/value pair handle returned by nvpCreate().
        <dataType>	- O
            returns the data type of the pair's value; see the enumerated
            NvpDataType definition in "nvp_util.h".

*******************************************************************************/


NvpDataType  nvpTypeOf (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{
    return ((pair == NULL) ? NvpUndefined : pair->dataType) ;
}

/*!*****************************************************************************

Procedure:

    nvpValue ()

    Get the Value of a Name/Value Pair.


Purpose:

    Function nvpValue() returns a pointer to a name/value pair's value;
    the application is responsible for casting the pointer to the correct
    data type before dereferencing it.


    Invocation:

        pointer = nvpValue (pair) ;

    where

        <pair>		- I
            is the name/value pair handle returned by nvpCreate().
        <pointer>	- O
            returns a (void *) pointer to the pair's value; NULL is returned
            if the pair has no value.

*******************************************************************************/


void  *nvpValue (

#    if PROTOTYPES
        NVPair  pair)
#    else
        pair)

        NVPair  pair ;
#    endif

{
    if ((pair == NULL) || (pair->dataType == NvpUndefined))
        return (NULL) ;
    else
        return (pair->value) ;
}

/*!*****************************************************************************

Procedure:

    xdr_NVPair ()

    Encode/Decode a Name/Value Pair in XDR Format.


Purpose:

    Function xdr_NVPair() is an XDR(3)-compatible function that encodes/decodes
    a name/value pair into/from XDR format.


    Invocation:

        successful = xdr_NVPair (xdrStream, &pair) ;

    where

        <xdrStream>	- I
            is the XDR stream handle returned by one of the xdrTTT_create()
            functions.
        <pair>		- I/O
            is the address of a name/value pair handle.  When encoding a pair,
            the handle must be for a valid name/value pair.  Decoding a
            name/value pair is slightly more complicated.  If the handle is
            NULL, a brand new name/value pair will be created.  If the handle
            is *not* NULL, the existing pair's value will be replaced by the
            decoded value; the decoded name is ignored.
        <successful>	- O
            returns TRUE if the XDR translation was successful and FALSE
            if it was not.

*******************************************************************************/


bool_t  xdr_NVPair (

#    if PROTOTYPES
        XDR  *xdrStream,
        NVPair  *pair)
#    else
        xdrStream, pair)

        XDR  *xdrStream ;
        NVPair  *pair ;
#    endif

{    /* Local pairs. */
    char  *name ;
    NvpDataType  dataType ;




/*******************************************************************************
    Decode the name/value pair from the XDR stream.
*******************************************************************************/

    if (xdrStream->x_op == XDR_DECODE) {

/* Decode the pair's name.  If the caller passed in an existing pair, then
   erase its value and ignore the incoming name; otherwise, create a brand
   new name/value pair. */

        name = NULL ;
        if (!xdr_string (xdrStream, &name, UINT_MAX))  return (FALSE) ;

        if (*pair == NULL) {				/* Create new one? */
            if (nvpCreate (name, pair))  return (FALSE) ;
        } else {
            nvpAssign (*pair, 0, NvpUndefined) ;	/* Erase old value. */
        }

        xdrStream->x_op = XDR_FREE ;			/* Free incoming name. */
        xdr_string (xdrStream, &name, UINT_MAX) ;
        xdrStream->x_op = XDR_DECODE ;

/* Decode the data type of the pair's value. */

        if (!xdr_enum (xdrStream, (enum_t *) &dataType))  return (FALSE) ;

/* Decode the pair's value. */

        switch (dataType) {

        case NvpByte:
          { unsigned  char  *array = NULL ;
            unsigned  int  length ;
            if (!xdr_bytes (xdrStream, (char **) &array, &length, UINT_MAX))
                return (FALSE) ;
            if (length == 1) {
                if (nvpAssign (*pair, 1, dataType, *array))  return (FALSE) ;
                xdrStream->x_op = XDR_FREE ;
                xdr_array (xdrStream, (caddr_t *) &array, &length, UINT_MAX,
                           sizeof (unsigned char), (xdrproc_t) xdr_void) ;
                xdrStream->x_op = XDR_DECODE ;
            } else if (nvpAssign (*pair, length, dataType, array, NvpDynamic)) {
                return (FALSE) ;
            }
            break ;
          }

        case NvpDouble:
          { double  *array = NULL ;
            unsigned  int  length ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                            UINT_MAX, sizeof (double), (xdrproc_t) xdr_double))
                return (FALSE) ;
            if (length == 1) {
                if (nvpAssign (*pair, 1, dataType, *array))  return (FALSE) ;
                xdrStream->x_op = XDR_FREE ;
                xdr_array (xdrStream, (caddr_t *) &array, &length,
                           UINT_MAX, sizeof (double), (xdrproc_t) xdr_void) ;
                xdrStream->x_op = XDR_DECODE ;
            } else if (nvpAssign (*pair, length, dataType, array, NvpDynamic)) {
                return (FALSE) ;
            }
            break ;
          }

        case NvpLong:
          { unsigned  int  length ;
            long  *array = NULL ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                            UINT_MAX, sizeof (long), (xdrproc_t) xdr_long))
                return (FALSE) ;
            if (length == 1) {
                if (nvpAssign (*pair, 1, dataType, *array))  return (FALSE) ;
                xdrStream->x_op = XDR_FREE ;
                xdr_array (xdrStream, (caddr_t *) &array, &length,
                           UINT_MAX, sizeof (long), (xdrproc_t) xdr_void) ;
                xdrStream->x_op = XDR_DECODE ;
            } else if (nvpAssign (*pair, length, dataType, array, NvpDynamic)) {
                return (FALSE) ;
            }
            break ;
          }

        case NvpString:
          { char  *string = NULL ;
            if (!xdr_string (xdrStream, &string, UINT_MAX))  return (FALSE) ;
            if (nvpAssign (*pair, -1, dataType, string, NvpDynamic))
                return (FALSE) ;
            break ;
          }

        case NvpTime:
          { struct  timeval  *array = NULL ;
            unsigned  int  length ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length, UINT_MAX,
                            sizeof (struct timeval), (xdrproc_t) xdr_timeval))
                return (FALSE) ;
            if (length == 1) {
                if (nvpAssign (*pair, 1, dataType, *array))  return (FALSE) ;
                xdrStream->x_op = XDR_FREE ;
                xdr_array (xdrStream, (caddr_t *) &array, &length, UINT_MAX,
                           sizeof (struct timeval), (xdrproc_t) xdr_void) ;
                xdrStream->x_op = XDR_DECODE ;
            } else if (nvpAssign (*pair, length, dataType, array, NvpDynamic)) {
                return (FALSE) ;
            }
            break ;
          }

        case NvpList:
          { unsigned  int  length ;
            NVList  *array = NULL ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                            UINT_MAX, sizeof (NVList), (xdrproc_t) xdr_NVList))
                return (FALSE) ;
            if (length == 1) {
                if (nvpAssign (*pair, 1, dataType, *array))  return (FALSE) ;
                xdrStream->x_op = XDR_FREE ;
                xdr_array (xdrStream, (caddr_t *) &array, &length,
                           UINT_MAX, sizeof (NVList), (xdrproc_t) xdr_void) ;
                xdrStream->x_op = XDR_DECODE ;
            } else if (nvpAssign (*pair, length, dataType, array, NvpDynamic)) {
                return (FALSE) ;
            }
            break ;
          }

        default:
            LGE "(xdr_NVPair) Unrecognized data type %d for \"%s\".\n",
                dataType, nvpName (*pair)) ;
            return (FALSE) ;

        }

    }

/*******************************************************************************
    Encode the name/value pair into the XDR stream.
*******************************************************************************/

    else if (xdrStream->x_op == XDR_ENCODE) {

/* Encode the pair's name. */

        name = (char *) nvpName (*pair) ;
        if (!xdr_string (xdrStream, &name, UINT_MAX))  return (FALSE) ;

/* Encode the data type of the pair's value. */

        dataType = nvpTypeOf (*pair) ;
        if (!xdr_enum (xdrStream, (enum_t *) &dataType))  return (FALSE) ;

/* Encode the value. */

        switch (dataType) {

        case NvpByte:
          { unsigned  char  *array = nvpValue (*pair) ;
            unsigned  int  length = nvpCount (*pair) ;
            if (!xdr_bytes (xdrStream, (char **) &array, &length, UINT_MAX))
                return (FALSE) ;
            return (TRUE) ;
          }

        case NvpDouble:
          { double  *array = nvpValue (*pair) ;
            unsigned  int  length = nvpCount (*pair) ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                            UINT_MAX, sizeof (double), (xdrproc_t) xdr_double))
                return (FALSE) ;
            return (TRUE) ;
          }

        case NvpLong:
          { long  *array = nvpValue (*pair) ;
            unsigned  int  length = nvpCount (*pair) ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                            UINT_MAX, sizeof (long), (xdrproc_t) xdr_long))
                return (FALSE) ;
            return (TRUE) ;
          }

        case NvpString:
          { char  *string = nvpValue (*pair) ;
            if (!xdr_string (xdrStream, &string, UINT_MAX))  return (FALSE) ;
            return (TRUE) ;
          }

        case NvpTime:
          { unsigned  int  length = nvpCount (*pair) ;
            struct  timeval  *array = nvpValue (*pair) ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length, UINT_MAX,
                            sizeof (struct timeval), (xdrproc_t) xdr_timeval))
                return (FALSE) ;
            return (TRUE) ;
          }

        case NvpList:
          { unsigned  int  length = nvpCount (*pair) ;
            NVList  *array = nvpValue (*pair) ;
            if (!xdr_array (xdrStream, (caddr_t *) &array, &length,
                            UINT_MAX, sizeof (NVList), (xdrproc_t) xdr_NVList))
                return (FALSE) ;
            return (TRUE) ;
          }

        default:
            LGE "(xdr_NVPair) Unrecognized data type %d for \"%s\".\n",
                dataType, nvpName (*pair)) ;
            return (FALSE) ;

        }

    }

/*******************************************************************************
    Free a previously allocated name/value pair.
*******************************************************************************/

    else if (xdrStream->x_op == XDR_FREE) {

        if (*pair != NULL) {
            nvpDestroy (*pair) ;
            *pair = NULL ;
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

#ifdef  TEST

/*******************************************************************************

    Program to test the NVP_UTIL functions.

*******************************************************************************/

int  main (argc, argv)
    int  argc ;
    char  *argv[] ;
{    /* Local pairs. */
    double  anArray[50], *pArray ;
    long  number ;
    NVPair  pair ;
    struct  timeval  now ;



    nvp_util_debug = 1 ;

    pair = nvpDecode ("oth-abc(list[123])") ;
    printf ("[TEST] %s\n", nvpEncode (pair)) ;

    nvpCreate ("pieceOfInfo", &pair) ;
    printf ("[TEST] %s\n", nvpEncode (pair)) ;

    nvpAssign (pair, 1, NvpByte, (unsigned char) 0x12) ;
    number = *((unsigned char *) nvpValue (pair)) ;
    printf ("[TEST] Variable = %ld\n", number) ;

    nvpAssign (pair, 1, NvpDouble, (double) 123.45) ;
    printf ("[TEST] %s\n", nvpEncode (pair)) ;

    nvpAssign (pair, 1, NvpLong, (long) 678) ;
    number = *((long *) nvpValue (pair)) ;
    printf ("[TEST] Variable = %ld\n", number) ;

    nvpAssign (pair, 1, NvpTime, tvTOD ()) ;
    now = *((struct timeval *) nvpValue (pair)) ;
    printf ("[TEST] Variable = %ld seconds, %ld microseconds\n", now.tv_sec, now.tv_usec) ;

    anArray[0] = 12345.6789 ;
    nvpAssign (pair, 50, NvpDouble, (void *) anArray, NvpVolatile) ;
    pArray = (double *) nvpValue (pair) ;
    printf ("[TEST] anArray = %p, pArray = %p\n", anArray, pArray) ;

    nvpAssign (pair, -1, NvpString, "Hello", NvpStatic) ;
    printf ("[TEST] %s\n", nvpEncode (pair)) ;

    nvpDestroy (pair) ;

    pair = nvpDecode ("this=123.45xyz") ;
    printf ("[TEST] %s\n", nvpEncode (pair)) ;

    pair = nvpDecode ("that 123.45") ;
    printf ("[TEST] %s\n", nvpEncode (pair)) ;

    exit (0) ;

}

#endif  /* TEST */
