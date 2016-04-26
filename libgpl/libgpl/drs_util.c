/* $Id: drs_util.c,v 1.14 2011/07/18 17:50:18 alex Exp $ */
/*******************************************************************************

File:

    drs_util.c

    Directory Scanning Utilities.


Author:    Alex Measday


Purpose:

    The functions in the DRS_UTIL package are used to scan the names of the
    files in a directory.  Wildcards can be used to filter out unwanted files.

    The following example prints out the names of the ".c" files in a
    directory:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  "drs_util.h"		-- Directory scanning utilities.
        ...
        int  main (int argc, char *argv[])
        {
            char  *fileName ;
            DirectoryScan  scan ;

            drsCreate ("*.c", &scan) ;
            fileName = drsFirst (scan) ;
            while (fileName != NULL) {
                printf ("C File: %s\n", fileName) ;
                fileName = drsNext (scan) ;
            }
            drsDestroy (scan) ;
        }

    Alternatively, you can call drsGet() to get the I-th name in the directory:

            ...
            for (i = 0 ;  i < drsCount (scan) ;  i++)
                printf ("C File: %s\n", drsGet (scan, i)) ;
            ...


Notes:

    This package is derived from and supersedes my FSEARCH() routine.
    Farewell, VMS!


Public Procedures:

    drsCount() - returns the number of files in a directory scan.
    drsCreate() - creates a directory scan.
    drsDestroy() - destroys a directory scan.
    drsFirst() - gets the first entry in the directory.
    drsGet() - gets the I-th entry in the directory.
    drsNext() - gets the next entry in the directory.

Private Procedures:

    drsCompare() - compares two file names for sorting purposes.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* C Library string functions. */

#if defined(VMS)
#    include  <nam.h>			/* RMS name block definitions. */
#    include  <rmsdef.h>		/* RMS completion codes. */
#elif defined(_WIN32)
#    include  <winbase.h>		/* Directory scanning functions. */
#    include  <direct.h>		/* Directory manipulation functions. */
#else
#    include  <unistd.h>		/* UNIX-specific definitions. */
#    include  <dirent.h>		/* Directory entry definitions. */
#endif

#include  "rex_util.h"			/* Regular expression definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "drs_util.h"			/* Directory scanning utilities. */


/*******************************************************************************
    Directory Scan - contains information about a directory scan.
*******************************************************************************/

typedef  struct  _DirectoryScan {
    char  *pathname ;			/* Original wildcard pathname. */
#if defined(VMS)
    unsigned  long  directory ;		/* Directory handle. */
#elif defined(_WIN32)
    HANDLE  directory ;			/* Directory handle. */
#else
    DIR  *directory ;			/* Directory handle. */
#endif
    CompiledRE  compiledRE ;		/* Compiled wildcard specification. */
    int  numFiles ;			/* # of matching file names. */
    char  **fileName ;			/* List of matching file names. */
    int  nextFile ;			/* Index of next file in scan. */
}  _DirectoryScan ;


int  drs_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  drs_util_debug


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  int  drsCompare (
#    if PROTOTYPES
        const  void  *p1,
        const  void  *p2
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    drsCount ()

    Get the Number of Files in a Directory Scan.


Purpose:

    Function drsCount() returns the number of files in a directory scan that
    matched the wildcard file specification.


    Invocation:

        numFiles = drsCount (scan) ;

    where:

        <scan>	- I
            is the directory scan handle returned by drsCreate().
        <count>	- O
            returns the number of files that matched the wildcard file
            specification.

*******************************************************************************/


int  drsCount (

#    if PROTOTYPES
        DirectoryScan  scan)
#    else
        scan)

        DirectoryScan  scan ;
#    endif

{

    if (scan == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(drsCount) NULL scan handle: ") ;
        return (0) ;
    }

    return (scan->numFiles) ;

}

/*!*****************************************************************************

Procedure:

    drsCreate ()

    Create a Directory Scan.


Purpose:

    Function drsCreate() creates a directory scan.


    Invocation:

        status = drsCreate (pathname, &scan) ;

    where:

        <pathname>	- I
            is the wildcard specification for the directory being scanned.
        <scan>		- O
            returns a handle for the directory scan.  This handle is used
            in calls to the other DRS functions.
        <status>	- O
            returns the status of initiating the directory scan, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  drsCreate (

#    if PROTOTYPES
        const  char  *pathname,
        DirectoryScan  *scan)
#    else
        pathname, scan)

        char  *pathname ;
        DirectoryScan  *scan ;
#    endif

{    /* Local variables. */
    char  directoryName[PATH_MAX+1], *fileSpec ;
    char  fullFileName[PATH_MAX+1], *s ;
    int  maxFiles = 0 ;
#if defined(VMS)
    char  result[NAM$C_MAXRSS+1] ;
    $DESCRIPTOR (result_dsc, result) ;
    struct  dsc$descriptor_s  wildcard_dsc ;
    unsigned  long  status ;
#elif defined(_WIN32)
    WIN32_FIND_DATA  fileInfo ;
#else
    struct  dirent  *d ;
#endif




/* Create the directory scan context structure. */

    *scan = (_DirectoryScan *) malloc (sizeof (_DirectoryScan)) ;
    if (*scan == NULL) {
        LGE "(drsCreate) Error allocating scan context for %s.\nmalloc: ",
            pathname) ;
        return (errno) ;
    }

#if defined(VMS)
    (*scan)->directory = 0 ;
#elif defined(_WIN32)
    (*scan)->directory = INVALID_HANDLE_VALUE ;
#else
    (*scan)->directory = NULL ;
#endif
    (*scan)->compiledRE = NULL ;
    (*scan)->numFiles = 0 ;
    (*scan)->fileName = NULL ;
    (*scan)->nextFile = 0 ;

    strEnv (pathname, -1, fullFileName, sizeof fullFileName) ;
    (*scan)->pathname = strdup (fullFileName) ;
    if ((*scan)->pathname == NULL) {
        LGE "(drsCreate) Error duplicating pathname: %s\nstrdup: ",
            fullFileName) ;
        PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Separate the directory name from the wildcard file specification. */

    strcpy (directoryName, (*scan)->pathname) ;		/* Directory name. */
    s = strrchr (directoryName, '/') ;
    if (s == NULL)  s = strrchr (directoryName, '\\') ;
    if (s == NULL)
        getcwd (directoryName, sizeof directoryName) ;
    else
        *s = '\0' ;

    fileSpec = strrchr ((*scan)->pathname, '/') ;	/* File specification. */
    if (fileSpec == NULL)  fileSpec = strrchr ((*scan)->pathname, '\\') ;
    if (fileSpec == NULL)
        fileSpec = (*scan)->pathname ;
    else
        fileSpec++ ;

/*******************************************************************************
    Under VMS, use the system functions to scan the wildcard-selected files.
*******************************************************************************/

#if defined(VMS)

/* Get the first file. */

    ASSIGN (wildcard_dsc, (*scan)->pathname) ;
    result[(sizeof result)-1] = '\0' ;

    status = LIB$FIND_FILE (&wildcard_dsc, &result_dsc, &(*scan)->directory,
                            NULL, NULL, NULL, NULL) ;
    if (!(status & STS$M_SUCCESS)) {
        SET_ERRNO (EVMSERR) ;  vaxc$errno = status ;
        if (status == RMS$_NMF) {				/* No files? */
            LGI "(drsCreate) %s - %d files.\n",
                (*scan)->pathname, (*scan)->numFiles) ;
            return (0) ;
        } else {						/* Error. */
            LGE "(drsCreate) Error opening directory: %s\nLIB$FIND_FILE: ",
                (*scan)->pathname) ;
            PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
            vaxc$errno = status ;
            return (errno) ;
        }
    }

/* Construct the list of files. */

    do {

        char  *s = strchr (result, ' ') ;
        if (s != NULL)  *s = '\0' ;			/* Trim space fill. */

        if ((*scan)->numFiles >= maxFiles) {		/* Expand list? */
            size_t  newMax = maxFiles + 128 ;
            size_t  newSize = newMax * sizeof (char *) ;
            char  **names =
                ((*scan)->fileName == NULL)
                ? (char **) malloc (newSize)
                : (char **) realloc ((*scan)->fileName, newSize) ;
            if (names == NULL) {
                LGE "(drsCreate) Error expanding %s list to %d files.\nrealloc: ",
                    (*scan)->pathname, maxFiles) ;
                PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
                return (errno) ;
            }
            (*scan)->fileName = names ;
            maxFiles = newMax ;
        }
							/* Add new file. */
        (*scan)->fileName[(*scan)->numFiles] = strdup (result) ;
        if ((*scan)->fileName[(*scan)->numFiles] == NULL) {
            LGE "(drsCreate) Error duplicating file name: %s\nstrdup: ",
                result) ;
            PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
            return (errno) ;
        }
        (*scan)->numFiles++ ;

        status = LIB$FIND_FILE (&wildcard_dsc, &result_dsc, &(*scan)->directory,
                                NULL, NULL, NULL, NULL) ;

    } while (status & STS$M_SUCCESS) ;

/*******************************************************************************
    Under Windows, use the system functions to scan the wildcard-selected
    files.
*******************************************************************************/

#elif defined(_WIN32)

/* Get the first file. */

    (*scan)->directory = FindFirstFile ((*scan)->pathname, &fileInfo) ;
    if ((*scan)->directory == INVALID_HANDLE_VALUE) {
        if (GetLastError () == ERROR_NO_MORE_FILES) {		/* No files? */
            LGI "(drsCreate) %s - %d files.\n",
                (*scan)->pathname, (*scan)->numFiles) ;
            return (0) ;
        } else {						/* Error. */
            SET_ERRNO (EINVAL) ;
            LGE "(drsCreate) Error (%d) opening directory: %s\nFindFirstFile: ",
                (int) GetLastError (), (*scan)->pathname) ;
            PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
            return (errno) ;
        }
    }

/* Construct the list of files. */

    do {

        if ((strcmp (fileInfo.cFileName, ".") == 0) ||
            (strcmp (fileInfo.cFileName, "..") == 0))
            continue ;		/* Ignore current and parent entries. */

        if ((*scan)->numFiles >= maxFiles) {		/* Expand list? */
            size_t  newMax = maxFiles + 128 ;
            size_t  newSize = newMax * sizeof (char *) ;
            char  **names =
                ((*scan)->fileName == NULL)
                ? (char **) malloc (newSize)
                : (char **) realloc ((*scan)->fileName, newSize) ;
            if (names == NULL) {
                LGE "(drsCreate) Error expanding %s list to %d files.\nrealloc: ",
                    (*scan)->pathname, maxFiles) ;
                PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
                return (errno) ;
            }
            (*scan)->fileName = names ;
            maxFiles = newMax ;
        }
							/* Add new file. */
        sprintf (fullFileName, "%s/%s", directoryName, fileInfo.cFileName) ;
        (*scan)->fileName[(*scan)->numFiles] = strdup (fullFileName) ;
        if ((*scan)->fileName[(*scan)->numFiles] == NULL) {
            LGE "(drsCreate) Error duplicating file name: %s\nstrdup: ",
                fullFileName) ;
            PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
            return (errno) ;
        }
        (*scan)->numFiles++ ;

    } while (FindNextFile ((*scan)->directory, &fileInfo)) ;

/*******************************************************************************
    Under UNIX, compile and use a regular expression to match the wildcard
    specification against the files in the directory.
*******************************************************************************/

#else

/* Compile a regular expression (RE) for the wildcard file specification. */

    if (rex_compile (rex_wild (fileSpec), &(*scan)->compiledRE)) {
        LGE "(drsCreate) Error compiling regular expression for \"%s\": %s\nrex_compile: ",
            pathname, rex_error_text) ;
        PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Open the directory for scanning. */

    (*scan)->directory = opendir (directoryName) ;  s = (char *) "opendir" ;
    if ((*scan)->directory == NULL) {
        LGE "(drsCreate) Error opening directory: %s\nopendir: ",
            directoryName) ;
        PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Construct the list of files whose names match the wildcard specification. */

    (*scan)->numFiles = 0 ;
    while ((d = readdir ((*scan)->directory)) != NULL) {

        if ((strcmp (d->d_name, ".") == 0) || (strcmp (d->d_name, "..") == 0))
            continue ;		/* Ignore current and parent entries. */

        if (!rex_match (d->d_name, (*scan)->compiledRE, NULL, NULL, 0))
            continue ;		/* File not selected by wildcard. */

        if ((*scan)->numFiles >= maxFiles) {		/* Expand list? */
            size_t  newMax = maxFiles + 128 ;
            size_t  newSize = newMax * sizeof (char *) ;
            char  **names =
                ((*scan)->fileName == NULL)
                ? (char **) malloc (newSize)
                : (char **) realloc ((*scan)->fileName, newSize) ;
            if (names == NULL) {
                LGE "(drsCreate) Error expanding %s list to %d files.\nrealloc: ",
                    (*scan)->pathname, maxFiles) ;
                PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
                return (errno) ;
            }
            (*scan)->fileName = names ;
            maxFiles = newMax ;
        }
							/* Add new file. */
        sprintf (fullFileName, "%s/%s", directoryName, d->d_name) ;
        (*scan)->fileName[(*scan)->numFiles] = strdup (fullFileName) ;
        if ((*scan)->fileName[(*scan)->numFiles] == NULL) {
            LGE "(drsCreate) Error duplicating file name: %s\nstrdup: ",
                fullFileName) ;
            PUSH_ERRNO ;  drsDestroy (*scan) ;  POP_ERRNO ;
            return (errno) ;
        }
        (*scan)->numFiles++ ;

    }

#endif

/*******************************************************************************
    Depending on the file-system type, the file names may not be in alphabetical
    order.  Sort them.
*******************************************************************************/

#ifndef VMS
    qsort ((*scan)->fileName, (*scan)->numFiles, sizeof (char *), drsCompare) ;
#endif


    LGI "(drsCreate) %s - %d files.\n", (*scan)->pathname, (*scan)->numFiles) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    drsDestroy ()

    Destroy a Directory Scan.


Purpose:

    Function drsDestroy() destroys a directory scan.


    Invocation:

        status = drsDestroy (scan) ;

    where:

        <scan>		- I
            is the directory scan handle returned by drsCreate().
        <status>	- O
            returns the status of terminating the directory scan, zero
            if no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  drsDestroy (

#    if PROTOTYPES
        DirectoryScan  scan)
#    else
        scan)

        DirectoryScan  scan ;
#    endif

{    /* Local variables. */
    int  i ;



    if (scan == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(drsDestroy) NULL scan handle: ") ;
        return (errno) ;
    }

    LGI "(drsDestroy) Terminating scan of %s.\n", scan->pathname) ;

/* Delete it. */

    if (scan->pathname != NULL)  free (scan->pathname) ;
#if defined(VMS)
    if (scan->directory != 0)  LIB$FIND_FILE_END (&scan->directory) ;
#elif defined(_WIN32)
    if (scan->directory != INVALID_HANDLE_VALUE)  FindClose (scan->directory) ;
#else
    if (scan->directory != NULL)  closedir (scan->directory) ;
#endif
    if (scan->compiledRE != NULL)  rex_delete (scan->compiledRE) ;

/* Delete it. */

    if (scan->fileName != NULL) {
        for (i = 0 ;  i < scan->numFiles ;  i++)
            free (scan->fileName[i]) ;
        free (scan->fileName) ;
    }

/* Delete it. */

    free (scan) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    drsFirst ()

    Get the First File in a Directory Scan.


Purpose:

    Function drsFirst() returns the first matching file in a directory scan.


    Invocation:

        fileName = drsFirst (scan) ;

    where:

        <scan>		- I
            is the directory scan handle returned by drsCreate().
        <fileName>	- O
            returns the full pathname of the first matching file.  The file
            name is stored in memory belonging to the DRS_UTIL package and
            it should not be overwritten or deallocated by the caller.  NULL
            is returned if there are no matching files.

*******************************************************************************/


const  char  *drsFirst (

#    if PROTOTYPES
        DirectoryScan  scan)
#    else
        scan)

        DirectoryScan  scan ;
#    endif

{
    return (drsGet (scan, 0)) ;
}

/*!*****************************************************************************

Procedure:

    drsGet ()

    Get the I-th File in a Directory Scan.


Purpose:

    Function drsGet() returns the I-th matching file in a directory scan.


    Invocation:

        fileName = drsGet (scan, index) ;

    where:

        <scan>		- I
            is the directory scan handle returned by drsCreate().
        <index>		- I
            is the index, 0 .. N-1, of the desired file.
        <fileName>	- O
            returns the full pathname of the indicated matching file.
            The file name is stored in memory belonging to the DRS_UTIL
            package and it should not be overwritten or deallocated by
            the caller.  NULL is returned if an invalid index is specified.

*******************************************************************************/


const  char  *drsGet (

#    if PROTOTYPES
        DirectoryScan  scan,
        int  index)
#    else
        scan, index)

        DirectoryScan  scan ;
        int  index ;
#    endif

{

    if (scan == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(drsGet) NULL scan handle: ") ;
        return (NULL) ;
    }

    if ((index < 0) || (index >= scan->numFiles)) {
        return (NULL) ;
    } else {
        scan->nextFile = index + 1 ;
        return (scan->fileName[index]) ;
    }

}

/*!*****************************************************************************

Procedure:

    drsNext ()

    Get the Next File in a Directory Scan.


Purpose:

    Function drsNext() returns the next matching file in a directory scan.


    Invocation:

        fileName = drsNext (scan) ;

    where:

        <scan>		- I
            is the directory scan handle returned by drsCreate().
        <fileName>	- O
            returns the full pathname of the next matching file.  The file
            name is stored in memory belonging to the DRS_UTIL package and
            it should not be overwritten or deallocated by the caller.  NULL
            is returned if there are no matching files.

*******************************************************************************/


const  char  *drsNext (

#    if PROTOTYPES
        DirectoryScan  scan)
#    else
        scan)

        DirectoryScan  scan ;
#    endif

{
    return (drsGet (scan, (scan == NULL) ? 0 : scan->nextFile)) ;
}

/*!*****************************************************************************

Procedure:

    drsCompare ()

    Compare File Names.


Purpose:

    Function drsCompare() is called by the QSORT(3) function to compare
    two file names.


    Invocation:

        comparison = drsCompare (p1, p2) ;

    where:

        <p1>		- I
        <p2>		- I
            are (VOID *) pointers to the (CHAR *) pointers that point to
            the two file names being compared.
        <comparison>	- O
            returns -1, 0, or +1 if the first file name is lexicographically
            less than, equal to, or greater than the second file name.

*******************************************************************************/


static  int  drsCompare (

#    if PROTOTYPES
        const  void  *p1,
        const  void  *p2)
#    else
        p1, p2)

        void  *p1 ;
        void  *p2 ;
#    endif

{
#define  IGNORE_CASE
#ifdef IGNORE_CASE
    return (strcasecmp (*((char **) p1), *((char **) p2))) ;
#else
    return (strcmp (*((char **) p1), *((char **) p2))) ;
#endif
}
