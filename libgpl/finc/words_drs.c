/* $Id: words_drs.c,v 1.1 2009/08/17 05:25:38 alex Exp alex $ */
/*******************************************************************************

File:

    words_drs.c

    Directory Scanning.


Author:    Alex Measday


Purpose:

    The WORDS_DRS package defines words for scanning files in a directory.

        "<pathname>" DRS-CREATE
        <scan> DRS-DESTROY
        "<pathname>" DRS-DIRECTORY?
        <scan> DRS-FIRST
        <scan> DRS-NEXT
        <scan> DRS-COUNT
        <scan> <index> DRS-GET

    The DRS-FIRST and DRS-NEXT words are useful for sequencing through
    files in a BEGIN loop.  DRS-COUNT and DRS-GET are suited for DO loops.


Public Procedures:

    buildWordsDRS() - registers the words with the FICL system.

Private Procedures:

    word_DRS_CREATE() - implements the DRS-CREATE word.
    word_DRS_DESTROY() - implements the DRS-DESTROY word.
    word_DRS_DIRECTORYq() - implements the DRS-DIRECTORY? word.
    word_DRS_FIRST() - implements the DRS-FIRST word.
    word_DRS_NEXT() - implements the DRS-NEXT word.
    word_DRS_COUNT() - implements the DRS-COUNT word.
    word_DRS_GET() - implements the DRS-GET word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#if defined(VMS)
#    include  <file.h>			/* File definitions. */
#    include  <unixio.h>		/* VMS-emulation of UNIX I/O. */
#    define  S_ISDIR(mode)  ((((mode) & S_IFMT) & S_IFDIR) != 0)
#    define  S_ISLNK(mode)  false
#elif defined(VXWORKS)
#    include  <ioLib.h>			/* I/O library definitions. */
#    define  HAVE_LSTAT  0
#else
#    include  <fcntl.h>			/* File control definitions. */
#    ifdef _WIN32
#        include  <io.h>		/* UNIX I/O definitions. */
#        define  S_ISDIR(mode)  ((((mode) & _S_IFMT) & _S_IFDIR) != 0)
#        define  S_ISLNK(mode)  false
#    else
#        include  <unistd.h>		/* UNIX I/O definitions. */
#    endif
#endif
#include  <sys/stat.h>			/* File status definitions. */
#if defined(HAVE_LSTAT) && !HAVE_LSTAT
#    define  lstat  stat		/* OS doesn't support links. */
#endif
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_DRS_CREATE P_((ficlVm *vm)) ;
static  void  word_DRS_DESTROY P_((ficlVm *vm)) ;
static  void  word_DRS_DIRECTORYq P_((ficlVm *vm)) ;
static  void  word_DRS_FIRST P_((ficlVm *vm)) ;
static  void  word_DRS_NEXT P_((ficlVm *vm)) ;
static  void  word_DRS_COUNT P_((ficlVm *vm)) ;
static  void  word_DRS_GET P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsDRS ()

    Enter the DRS Words into the Dictionary.


Purpose:

    Function buildWordsDRS() enters the DRS words into the system dictionary.


    Invocation:

        buildWordsDRS (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsDRS (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-CREATE", word_DRS_CREATE,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-DESTROY", word_DRS_DESTROY,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-DIRECTORY?", word_DRS_DIRECTORYq,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-FIRST", word_DRS_FIRST,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-NEXT", word_DRS_NEXT,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-COUNT", word_DRS_COUNT,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "DRS-GET", word_DRS_GET,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_CREATE ()

    Create a Directory Scan.


Purpose:

    Function word_DRS_CREATE() implements the DRS-CREATE word, which
    creates a directory scan.

        DRS-CREATE

            ( c-addr u -- scan 0 | ior )

        Create a directory scan for the directory specified in the c-addr/u
        pathname string.  The pathname may contain wildcard characters for
        the files in the directory.  If the scan is successfully created,
        the scan is returned on the stack with a status of zero.  If there
        was an error, only a non-zero I/O result is returned on the stack.


    Invocation:

        word_DRS_CREATE (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_CREATE (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *pathname ;
    DirectoryScan  scan ;
    ficlCell  cell ;
    int  ior ;
    unsigned  long  length ;



    FICL_STACK_CHECK (vm->dataStack, 2, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Pathname. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        pathname = NULL ;
    } else {
        pathname = (char *) cell.p ;
        if (pathname[length] != '\0')  pathname = strndup (pathname, length) ;
    }

/* Create the directory scan. */

    ior = drsCreate (pathname, &scan) ;

    if (pathname != cell.p)  free (pathname) ;

/* Return the scan and the I/O result on the stack. */

    if (ior == 0) {			/* Successfully created scan? */
        cell.p = scan ;
        ficlVmPush (vm, cell) ;
    }
    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_DESTROY ()

    Destroy a Directory Scan.


Purpose:

    Function word_DRS_DESTROY() implements the DRS-DESTROY word, which
    destroys a directory scan.

        DRS-DESTROY

            ( scan -- ior )

        Destroy the directory scan and return the I/O result ior on the stack.


    Invocation:

        word_DRS_DESTROY (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_DESTROY (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    DirectoryScan  scan ;
    ficlCell  cell ;
    int  ior ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Scan. */
    scan = (DirectoryScan) cell.p ;

/* Destroy the scan. */

    ior = drsDestroy (scan) ;

/* Return the I/O result on the stack. */

    cell.i = ior ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_DIRECTORYq ()

    Does a Pathname Refer to a Directory?


Purpose:

    Function word_DRS_DIRECTORYq() implements the DRS-DIRECTORY? word, which
    determines if a pathname refers to a directory.

        DRS-DIRECTORY?

            ( c-addr u -- flag )

        Determine if the c-addr/u pathname string refers to a directory.
        Return (i) true if the pathname does refer to a directory and
        (ii) false if it doesn't or if there was an error.

        Ficl does have the DPANS94 FILE-STATUS word (from the optional
        File-Access word set), but FILE-STATUS returns an
        implementation-defined status value.  In particular, Ficl returns
        different values under Windows than it does on other platforms
        (e.g., UNIX).  DRS-DIRECTORY? always uses the UNIX stat(2) call,
        which *is* available on Windows.


    Invocation:

        word_DRS_DIRECTORYq (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_DIRECTORYq (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *pathname ;
    ficlCell  cell ;
    int  flag, length ;
    struct  stat  info ;



    FICL_STACK_CHECK (vm->dataStack, 2, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Pathname. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        pathname = NULL ;
    } else {
        pathname = strndup ((char *) cell.p, length) ;
    }

/* Get the system's information about the file. */

#ifdef _WIN32			/* Windows thinks trailing "/" is a file. */
    if ((pathname != NULL) && (pathname[length-1] == '/')) {
        pathname[length-1] = '\0' ;
    }
#endif

    if (pathname == NULL) {
        flag = 0 ;
    } else {
        flag = stat (pathname, &info) ? 0 : (S_ISDIR (info.st_mode) ? ~0 : 0) ;
        free (pathname) ;
    }

/* Return the is-directory? flag on the stack. */

    cell.i = flag ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_FIRST ()

    Get the First File in a Directory Scan.


Purpose:

    Function word_DRS_FIRST() implements the DRS-FIRST word, which returns
    the first matching file in a directory scan.

        DRS-FIRST

            ( scan -- c-addr u | 0 )

        Get the first matching file in a directory scan.  The file's full
        pathname is returned on the stack as c-addr/u.  Zero (0) is returned
        if there are no matching files in the scan.


    Invocation:

        word_DRS_FIRST (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_FIRST (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;
    ficlCell  cell ;



    FICL_STACK_CHECK (vm->dataStack, 1, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Scan. */
    scan = (DirectoryScan) cell.p ;

/* Get the first matching file in the scan. */

    fileName = (char *) drsFirst (scan) ;

/* Return the file name on the stack. */

    if (fileName != NULL) {
        cell.p = fileName ;
        ficlVmPush (vm, cell) ;
    }
    cell.u = (fileName == NULL) ? 0 : strlen (fileName) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_NEXT ()

    Get the Next File in a Directory Scan.


Purpose:

    Function word_DRS_NEXT() implements the DRS-NEXT word, which returns
    the next matching file in a directory scan.

        DRS-NEXT

            ( scan -- c-addr u | 0 )

        Get the next matching file in a directory scan.  The file's full
        pathname is returned on the stack as c-addr/u.  Zero (0) is returned
        if there are no more matching files in the scan.


    Invocation:

        word_DRS_NEXT (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_NEXT (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;
    ficlCell  cell ;



    FICL_STACK_CHECK (vm->dataStack, 1, 2) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Scan. */
    scan = (DirectoryScan) cell.p ;

/* Get the next matching file in the scan. */

    fileName = (char *) drsNext (scan) ;

/* Return the file name on the stack. */

    if (fileName != NULL) {
        cell.p = fileName ;
        ficlVmPush (vm, cell) ;
    }
    cell.u = (fileName == NULL) ? 0 : strlen (fileName) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_COUNT ()

    Get the Number of Files in a Directory Scan.


Purpose:

    Function word_DRS_COUNT() implements the DRS-COUNT word, which returns
    the number of files that match the pathname specification in a directory
    scan.

        DRS-COUNT

            ( scan -- u )

        Get the number of files, u, in a directory scan that matched the
        wildcard file specification supplied to DRS-CREATE.


    Invocation:

        word_DRS_COUNT (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_COUNT (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    DirectoryScan  scan ;
    ficlCell  cell ;



    FICL_STACK_CHECK (vm->dataStack, 1, 1) ;

/* Get the argument from the stack. */

    cell = ficlVmPop (vm) ;			/* Scan. */
    scan = (DirectoryScan) cell.p ;

/* Get the count of files in the scan and return it on the stack. */

    cell.u = (ficlUnsigned) drsCount (scan) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_DRS_GET ()

    Get the I-th File in a Directory Scan.


Purpose:

    Function word_DRS_GET() implements the DRS-GET word, which returns
    the I-th matching file in a directory scan.

        DRS-GET

            ( scan n -- c-addr u | 0 )

        Get the indexed, n, matching file in a directory scan. Indices are
        numbered from 1 to the value returned by DRS-COUNT.  The file's full
        pathname is returned on the stack as c-addr/u.  Zero (0) is returned
        if the index is out of range.  Getting a file name by index does not
        affect the sequence of file names returned by DRS-FIRST and DRS-NEXT.


    Invocation:

        word_DRS_GET (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_DRS_GET (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;
    ficlCell  cell ;
    int  index ;



    FICL_STACK_CHECK (vm->dataStack, 2, 2) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Index. */
    index = cell.i ;
    cell = ficlVmPop (vm) ;			/* Scan. */
    scan = (DirectoryScan) cell.p ;

/* Get the I-th matching file in the scan. */

    fileName = (char *) drsGet (scan, index - 1) ;

/* Return the file name on the stack. */

    if (fileName != NULL) {
        cell.p = fileName ;
        ficlVmPush (vm, cell) ;
    }
    cell.u = (fileName == NULL) ? 0 : strlen (fileName) ;
    ficlVmPush (vm, cell) ;

    return ;

}
