/* $Id$ */
/*******************************************************************************

File:

    cosn_util.c

    CORBAservices (COS) Naming Service Utilities.


Author:    Alex Measday


Purpose:

    The COSN utilities ...


Public Procedures:

    cosnN2S() - converts a Naming Service name to a string.
    cosnN2URL() - converts an IOR and a Naming Service name to a URL.
    cosnS2N() - converts a string to a Naming Service name.
    cosnURL2N() - converts a URL to an IOR and a Naming Service name.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "coli_util.h"			/* CORBA-Lite utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "cosn_util.h"			/* CosNaming utilities. */

					/* Global debug switch (1/0 = yes/no). */
int  cosn_util_debug  OCD ("CosNaming")  = 0 ;
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  cosn_util_debug

/*!*****************************************************************************

Procedure:

    cosnN2S ()

    Convert a Naming Service Name to a String.


Purpose:

    Function cosnN2S() converts a Naming Service name to its string
    representation.


    Invocation:

        string = cosnN2S (name, dynamic) ;

    where

        <name>		- I
            is the CosNaming name to be converted.
        <dynamic>	- I
            controls the disposition of the result.  If this argument is
            true (non-zero), cosnN2S() returns a dynamically allocated
            string that the caller is responsible for free(3)ing.  If this
            argument is false (zero), the string is stored in memory local
            to cosnN2S() and it should be used or duplicated before calling
            cosnN2S() again.
        <string>	- O
            returns the name's string representation; NULL is returned in
            the event of an error.

*******************************************************************************/


char  *cosnN2S (

#    if PROTOTYPES
        const  Name  *name,
        bool  dynamic)
#    else
        name, dynamic)

        Name  *name ;
        bool  dynamic ;
#    endif

{    /* Local variables. */
    char  *nameString, *s, *t ;
    NameComponent  *component ;
    size_t  length ;
    unsigned  long  i ;
    static  char  *result = NULL ;
    static  size_t  resultMax = 0 ;



/* Determine the length of the string representation. */

    length = 0 ;
    for (i = 0 ;  i < name->count ;  i++) {
        component = &name->elements[i] ;
        if (component->id != NULL)
            length += strlen (component->id) * 2 ;
        if (component->kind != NULL)
            length += strlen (".") + strlen (component->kind) * 2 ;
        length += strlen ("/") ;
    }
    length++ ;				/* NUL terminator. */

/* Dynamically allocate a string to hold the result. */

    if (dynamic || (length > resultMax)) {
        nameString = (char *) malloc (length) ;
        if (nameString == NULL) {
            LGE "(cosnN2S) Error allocating %d-byte string.\nmalloc: ",
                length) ;
            return (NULL) ;
        }
        if (!dynamic) {
            if (result != NULL)  free (result) ;
            result = nameString ;  resultMax = length ;
        }
    } else {
        nameString = result ;
    }

/* Generate the string representation of the name:
   "<id1>[.<kind1>]/<id2>[.<kind2>]/..." */

    t = nameString ;

    for (i = 0 ;  i < name->count ;  i++) {
        component = &name->elements[i] ;
        if (component->id != NULL) {
            for (s = component->id ;  *s != '\0' ;  *t++ = *s++) {
                if ((*s == '.') || (*s == '/') || (*s == '\\')) {
                    *t++ = '\\' ;
                }
            }
        }
        if (component->kind != NULL) {
            *t++ = '.' ;
            for (s = component->kind ;  *s != '\0' ;  *t++ = *s++) {
                if ((*s == '.') || (*s == '/') || (*s == '\\')) {
                    *t++ = '\\' ;
                }
            }
        }
        *t++ = '/' ;
    }

    if (t == nameString)
        *t = '\0' ;
    else
        *--t = '\0' ;

    return (nameString) ;

}
