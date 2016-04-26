/* $Id: ftw_util.c,v 1.9 2011/07/18 17:37:46 alex Exp $ */
/*******************************************************************************

File:

    ftw_util.c

    File Tree Walk Utilities.


Author:    Alex Measday


Purpose:

    The FTW_UTIL package ...


Public Procedures:

    fileTreeWalk() - walks a directory tree.

Private Procedures:

    ftwScanDirectory() - scans a directory.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#if !defined(NO_GETGID) && !defined(NO_GETUID)
#    include  <unistd.h>		/* UNIX I/O definitions. */
#endif
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
#include  "rex_util.h"			/* Regular expression definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "ftw_util.h"			/* File tree walk definitions. */


int  ftw_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  ftw_util_debug


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  int  ftwScanDirectory (
#    if PROTOTYPES
        const  char  *root,
        CompiledRE  wildcardRE,
        FtwFlag  flags,
        FtwFileCallback  callbackF,
        void  *clientData,
        int  level
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    fileTreeWalk ()

    Walk a File Tree.


Purpose:

    Function fileTreeWalk() walks a directory tree, calling a user-specified
    callback function for each file encountered in the hierarchy.


    Invocation:

        status = fileTreeWalk (root, fileSpec, flags,
                               callbackFunction, clientData) ;

    where:

        <root>			- I
            is the name of the root of the directory tree being walked;
            "." (the current directory) is the default if this argument
            is NULL.
        <fileSpec>		- I
            specifies which files to visit in each directory; "*" (all the
            files) is the default if this argument is NULL.
        <flags>			- I
            is the bit-wise OR of zero or more of the values specified for
            the FtwFlag enumeration defined in "ftw_util.h".
        <callbackFunction>	- I
            specifies a function to be called for each file encountered in
            the tree walk.
        <clientData>		- I
            is a (VOID *) pointer passed to the callback function.
        <status>		- O
            returns the status of walking the file tree, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


int  fileTreeWalk (

#    if PROTOTYPES
        const  char  *root,
        const  char  *fileSpec,
        FtwFlag  flags,
        FtwFileCallback  callbackF,
        void  *clientData)
#    else
        root, fileSpec, flags, callbackF, clientData)

        char  *root ;
        char  *fileSpec ;
        FtwFlag  flags ;
        FtwFileCallback  callbackF ;
        void  *clientData ;
#    endif

{    /* Local variables. */
    CompiledRE  wildcardRE ;
    int  status ;



    if (fileSpec == NULL)  fileSpec = "*" ;
    if (root == NULL)  root = "./" ;

/* Compile a regular expression (RE) for the wildcard file specification. */

    if (rex_compile (rex_wild (fileSpec), &wildcardRE)) {
        LGE "(fileTreeWalk) Error compiling regular expression for \"%s\": %s\nrex_compile: ",
            fileSpec, rex_error_text) ;
        return (errno) ;
    }

/* Visit each file in the directory tree, invoking the file callback function
   for each file whose name matches the wildcard file specification. */

    status = ftwScanDirectory (root, wildcardRE, flags,
                               callbackF, clientData, 0) ;
    rex_delete (wildcardRE) ;
    SET_ERRNO (status) ;

    return (errno) ;

}

/*!*****************************************************************************

Procedure:

    ftwScanDirectory ()

    Scan the Files in a Directory.


Purpose:

    Function ftwScanDirectory() scans all of the files in a directory,
    invoking the file callback function for each file whose name matches
    the wildcard file specification and recursively descending into any
    subdirectories.


    Invocation:

        status = ftwScanDirectory (directory, wildcardRE, flags,
                                   callbackFunction, clientData, level) ;

    where:

        <directory>		- I
            is the name of the directory being scanned; "." (the current
            directory) is the default if this argument is NULL.
        <wildcardRE>		- I
            is the compiled regular expression for the wildcard file
            specification.
        <flags>			- I
            is the bit-wise OR of zero or more of the values specified for
            the FtwFlag enumeration defined in "ftw_util.h".
        <callbackFunction>	- I
            specifies a function to be called for each file encountered in
            the directory scan.
        <clientData>		- I
            is a (VOID *) pointer passed to the callback function.
        <level>			- I
            specifies the depth of nesting in the file tree being walked.
        <status>		- O
            returns the status of scanning the directory, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


static  int  ftwScanDirectory (

#    if PROTOTYPES
        const  char  *directory,
        CompiledRE  wildcardRE,
        FtwFlag  flags,
        FtwFileCallback  callbackF,
        void  *clientData,
        int  level)
#    else
        directory, wildcardRE, flags, callbackF, clientData, level)

        char  *directory ;
        CompiledRE  wildcardRE ;
        FtwFlag  flags ;
        FtwFileCallback  callbackF ;
        void  *clientData ;
        int  level ;
#    endif

{    /* Local variables. */
    char  *fileName, *fullFileName, pathname[PATH_MAX+1] ;
    DirectoryScan  scan ;
    FtwFileType  fileType ;
    int  i, status = 0 ;
    struct  stat  fileInfo ;
#ifndef NO_GETGID
    gid_t  groupID = getgid () ;
#endif
#ifndef NO_GETUID
    uid_t  userID = getuid () ;
#endif




/* Construct a list of all the files in the directory. */

    strcpy (pathname, directory) ;
    if (pathname[strlen (pathname) - 1] == '/')
        strcat (pathname, "*") ;
    else
        strcat (pathname, "/*") ;

    LGI "(ftwScanDirectory) %s\n", pathname) ;

    if (drsCreate (pathname, &scan)) {
        LGE "(ftwScanDirectory) Error scanning directory: %s\ndrsCreate: ",
            pathname) ;
        return (errno) ;
    }


/*******************************************************************************
    Step through each file in the list ...
*******************************************************************************/

    for (i = 0 ;  i < drsCount (scan) ;  i++) {

        fullFileName = (char *) drsGet (scan, i) ;

/* Query the operating system for information about the file. */

#ifdef NO_LSTAT
        if (stat (fullFileName, &fileInfo)) {
#else
        if (((flags & FtwPhysical) && lstat (fullFileName, &fileInfo)) ||
            (!(flags & FtwPhysical) && stat (fullFileName, &fileInfo))) {
#endif

            switch (errno) {
            case EACCES:			/* No search permission. */
                fileType = FtwNoStat ;
                break ;
            default:				/* Unexpected errors. */
                LGE "(ftwScanDirectory) Error getting information for %s.\nstat: ",
                    fullFileName) ;
                continue ;			/* Skip on error. */
            }

        }

/* Determine the type of the current file. */

        else {

            if ((flags & FtwPhysical) && S_ISLNK (fileInfo.st_mode))
                fileType = FtwSymbolicLink ;
            else if (S_ISDIR (fileInfo.st_mode))
                fileType = FtwDirectory ;
            else
                fileType = FtwFile ;

        }

/* If the file is a directory, then check if we have permission to search it. */

        if (fileType == FtwDirectory) {
            if (!(fileInfo.st_mode & S_IXOTH)
#ifndef NO_GETGID
                &&
                !((fileInfo.st_mode & S_IXGRP) && (fileInfo.st_gid == groupID))
#endif
#ifndef NO_GETUID
                &&
                !((fileInfo.st_mode & S_IXUSR) && (fileInfo.st_uid == userID))
#endif
               ) {
                fileType = FtwDirNoRead ;
            }
        }

/* If the wildcard file specification matches the current file name, then
   invoke the file callback function. */

        fileName = strrchr (fullFileName, '/') ;
        if (fileName == NULL)
            fileName = fullFileName ;
        else
            fileName++ ;

        if ((fileType == FtwDirectory) ||
            (fileType == FtwDirNoRead) ||
            rex_match (fileName, wildcardRE, NULL, NULL, 0)) {
            status = callbackF (fullFileName, fileName, fileType,
                                &fileInfo, level, clientData) ;
            if (status)  break ;
        }

/* If the current file is a directory, then recursively scan that directory. */

        if ((fileType == FtwDirectory) &&
            (status = ftwScanDirectory (fullFileName, wildcardRE, flags,
                                        callbackF, clientData, level + 1))) {
            break ;
        }

    }


/*******************************************************************************
    Clean up.
*******************************************************************************/

    SET_ERRNO ((i < drsCount (scan)) ? status : 0) ;
    PUSH_ERRNO ;  drsDestroy (scan) ;  POP_ERRNO ;
    return (errno) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the FTW_UTIL() functions.

    Under UNIX,
        compile and link as follows:
            % cc -g -DTEST ftw_util.c -I<... includes ...> <libraries ...>
        and run with the following command line:
            % a.out <files>

    Under VxWorks,
        compile and link as follows:
            % cc -g -c -DTEST -DVXWORKS ftw_util.c -I<... includes ...> \
                       -o test_ftw.o
            % ld -r test_ftw.o <libraries ...> -o test_info.vx.o
        load as follows:
            -> ld <test_ftw.vx.o
        and run with the following command line:
            -> test_ftw.vx.o "<files>"

*******************************************************************************/


static  int  ftwCallback (pathname, fileName, type, info, level, clientData)
    char  *pathname ;
    char  *fileName ;
    FtwFileType  type ;
    struct  stat  *info ;
    int  level ;
    void  *clientData ;
{
    while (level-- > 0)
        printf ("  ") ;
    printf ("%s%c\n", fileName, (type == FtwDirectory) ? '/' : ' ') ;
    return (0) ;
}


#ifdef VXWORKS

test_ftw (files)
    char  *files ;
{    /* Local variables. */

#else

main (argc, argv)
    int  argc ;
    char  *argv[] ;
{    /* Local variables. */
    char  *files = argv[1] ;

#endif

    aperror_print = 1 ;
    ftw_util_debug = 1 ;

    fileTreeWalk (NULL, files, 0, ftwCallback, NULL) ;

}
#endif
