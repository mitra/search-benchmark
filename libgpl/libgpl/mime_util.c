/* $Id: mime_util.c,v 1.1 2009/09/10 10:58:43 alex Exp alex $ */
/*******************************************************************************

File:

    mime_util.c

    Mime Type Utilities.


Author:    Alex Measday


Purpose:

    The MIME_UTIL package ...


Public Procedures:

    mimeAdd() - adds an extension-type translation to a MIME map.
    mimeCount() - returns the number of entries in a MIME map.
    mimeCreate() - creates an empty MIME map.
    mimeDelete() - deletes an extension-type translation from a MIME map.
    mimeDestroy() - destroys a MIME map.
    mimeFind() - retrieves the MIME type associated with a file extension.
    mimeGet() - retrieves entries by index from a MIME map.
    mimeLoad() - loads type-extension(s) translations from a file.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "hash_util.h"			/* Hash table definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "mime_util.h"			/* MIME type functions. */


/*******************************************************************************
    Mime Map Data Structures.
*******************************************************************************/

typedef  struct  _MimeType {
    char  *name ;			/* Type name. */
    size_t  maxExtensions ;		/* Maximum # of extensions allocated. */
    size_t  numExtensions ;		/* Actual # of extensions used. */
    char  **extensions ;		/* Array of file extensions. */
    struct  _MimeType  *next ;		/* Link to next type in list. */
}  _MimeType, *MimeType ;

typedef  struct  _MimeMap {
    HashTable  extMap ;			/* Maps file extensions to MIME types. */
    HashTable  typeMap ;		/* Maps MIME types to MimeType objects. */
    MimeType  typeList ;		/* List of MIME types. */
}  _MimeMap ;


int  mime_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  mime_util_debug
/*!*****************************************************************************

Procedure:

    mimeCreate ()

    Create a Name/Value Pair List.


Purpose:

    Function mimeCreate() creates an empty list of name/value pairs.


    Invocation:

        status = mimeCreate (name, &list) ;

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


errno_t  mimeCreate (

#    if PROTOTYPES
        MimeMap  *map)
#    else
        map)

        MimeMap  *map ;
#    endif

{

/* Create an empty MIME map. */

    *map = (_MimeMap *) malloc (sizeof (_MimeMap)) ;
    if (*map == NULL) {
        LGE "(mimeCreate) Error creating an empty MIME map.\nmalloc: ") ;
        return (errno) ;
    }

    (*map)->extMap = NULL ;
    (*map)->typeMap = NULL ;
    (*map)->typeList = NULL ;

#define  MAX_ENTRIES  256

    if (hashCreate (MAX_ENTRIES, &(*map)->extMap)) {
        LGE "(mimeCreate) Error creating extension map.\nhashCreate: ") ;
        PUSH_ERRNO ;  mimeDestroy (*map) ;  POP_ERRNO ;
        return (errno) ;
    }

    if (hashCreate (MAX_ENTRIES, &(*map)->typeMap)) {
        LGE "(mimeCreate) Error creating type map.\nhashCreate: ") ;
        PUSH_ERRNO ;  mimeDestroy (*map) ;  POP_ERRNO ;
        return (errno) ;
    }

    LGI "(mimeCreate) Created empty MIME map %p.\n", (void *) *map) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    mimeDestroy ()

    Destroy a MIME Map.


Purpose:

    Function mimeDestroy() destroys a name/value pair list.  Each pair in
    the list is also destroyed.


    Invocation:

        status = mimeDestroy (list) ;

    where

        <list>		- I
            is the list handle returned by mimeCreate().
        <status>	- O
            returns the status of deleting the list, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  mimeDestroy (

#    if PROTOTYPES
        MimeMap  map)
#    else
        map)

        MimeMap  map ;
#    endif

{    /* Local variables. */
    MimeType  type ;
    size_t  i ;



    if (map == NULL)  return (0) ;

    LGI "(mimeDestroy) Destroying MIME map %p.\n", (void *) map) ;

/* Delete the two hash tables. */

    hashDestroy (map->extMap) ;  map->extMap = NULL ;
    hashDestroy (map->typeMap) ;  map->typeMap = NULL ;

/* Delete each of the MIME type objects in the list of MIME types. */

    while (map->typeList != NULL) {
        type = map->typeList ;
        map->typeList = type->next ;
        free (type->name) ;
        for (i = 0 ;  i < type->numExtensions ;  i++) {
            free (type->extensions[i]) ;
        }
        if (type->extensions != NULL) {
            free (type->extensions) ;
            type->extensions = NULL ;
        }
        free (type) ;
    }

    return (0) ;

}
