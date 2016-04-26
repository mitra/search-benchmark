/* $Id: words_misc.c,v 1.1 2009/08/17 05:26:35 alex Exp alex $ */
/*******************************************************************************

File:

    words_misc.c

    Miscellaneous Utilities.


Author:    Alex Measday


Purpose:

    The WORDS_MISC package defines a miscellaneous collection of unrelated
    words:

        "<name>" GETENV


Public Procedures:

    buildWordsMISC() - registers the words with the FICL system.

Private Procedures:

    word_MISC_GETENV() - implements the GETENV word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_MISC_GETENV P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsMISC ()

    Enter the MISC Words into the Dictionary.


Purpose:

    Function buildWordsMISC() enters the MISC words into the system dictionary.


    Invocation:

        buildWordsMISC (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsMISC (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "GETENV", word_MISC_GETENV,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_MISC_GETENV ()

    Get an Environment Variable's Value.


Purpose:

    Function word_MISC_GETENV() implements the GETENV word, which looks up
    an environment variable and returns its value.

        GETENV

            ( c-addr1 u1 -- c-addr2 u2 | 0 )

        Lookup the environment variable name represented by c-addr1/u1 and
        return its value, c-addr2/u2.  If the environment variable is not
        defined, then return 0 instead of c-addr2/u2.


    Invocation:

        word_MISC_GETENV (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_MISC_GETENV (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *name, *value ;
    ficlCell  cell ;
    unsigned  long  length ;



    FICL_STACK_CHECK (vm->dataStack, 2, 2) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Environment variable name. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        name = NULL ;
    } else {
        name = (char *) cell.p ;
        if (name[length] != '\0')  name = strndup (name, length) ;
    }

/* Get the environment variable's value. */

    value = getenv (name) ;

    if (name != cell.p)  free (name) ;

/* Return the variable's value on the stack. */

    if (value != NULL) {
        cell.p = value ;
        ficlVmPush (vm, cell) ;
    }
    cell.u = (value == NULL) ? 0 : strlen (value) ;
    ficlVmPush (vm, cell) ;

    return ;

}
