/* $Id: hls.c,v 1.2 2011/03/31 22:31:19 alex Exp $ */
/*******************************************************************************

Program:

    hls

    Hierarchical Directory Listing.


Author:    Alex Measday


Purpose:

    Program HLS generates a recursive directory listing.


    Invocation:

        % hls [-debug] [-directory <pathname>] [-full]
              [-greater <numBytes>] [-less <numBytes>] [-pathname]
              <wildcardSpec>

    where

        "-debug"
            enables debug output (written to STDOUT).
        "-directory <pathname>"
            specifies the directory at which the listing is to start; the
            default is the current directory.
        "-full"
            generates a full listing.
        "-greater <numBytes>"
            generates a listing of only those files whose size is greater
            than or equal to the specified number of bytes.
        "-less <numBytes>"
            generates a listing of only those files whose size is less than
            or equal to the specified number of bytes.
        "-pathname"
            causes the full pathname of each file to be displayed.
        "<wildcardSpec>"
            specifies which files are to be listed.  The standard shell
            wildcard characters are allowed, but the specification should
            be quoted to prevent its expansion by the shell.

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <signal.h>			/* Signal definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#ifdef sun
#    define  strtoul  strtol		/* STRTOUL(3) is not supported. */
#endif
#include  <sys/stat.h>			/* File status definitions. */
#ifdef VXWORKS
#    define  exit  return
#endif
#include  "ftw_util.h"			/* File tree walk definitions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "aperror.h"			/* APERROR() definitions. */

					/* Parameters passed to file callback. */
static  int  fullListing = 0 ;
static  int  fullPathname = 0 ;
static  unsigned  long  greaterThan = 0 ;
static  unsigned  long  lessThan = ~0 ;


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  char  *accessString (
#    if __STDC__
        mode_t  mode
#    endif
    ) ;

static  int  examineFile (
#    if __STDC__
        const char *fullFileName,
        const char *fileName,
        FtwFileType fileType,
        struct stat *fileInfo,
        int level,
        void *clientData
#    endif
    ) ;

/*******************************************************************************
    HLS's Main Program.
*******************************************************************************/

#ifdef VXWORKS

int  hls (

#    if __STDC__
        char  *commandLine)
#    else
        commandLine)

        char  *commandLine ;
#    endif

#else

int  main (

#    if __STDC__
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

#endif

{  /* Local variables. */
    char  *argument, *directory, *wildcardSpec ;
    int  errflg, option ;
    OptContext  context ;

    const  char  *optionList[] = {	/* Command line options. */
        "{debug}", "{directory:}", "{full}",
        "{greater:}", "{less:}", "{pathname}",
        NULL
    } ;





#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("hls", commandLine, &argc, &argv) ;
#endif

    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    directory = "." ;  wildcardSpec = "*" ;
    fullListing = 0 ;  fullPathname = 0 ;
    greaterThan = 0 ;  lessThan = ~0 ;

    opt_init (argc, argv, NULL, optionList, &context) ;
    errflg = 0 ;

    while ((option = opt_get (context, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            ftw_util_debug = 1 ;
            break ;
        case 2:			/* "-directory <pathname>" */
            directory = argument ;
            break ;
        case 3:			/* "-full" */
            fullListing = 1 ;
            break ;
        case 4:			/* "-greater <numBytes>" */
            greaterThan = strtoul (argument, NULL, 0) ;
            break ;
        case 5:			/* "-less <numBytes>" */
            lessThan = strtoul (argument, NULL, 0) ;
            break ;
        case 6:			/* "-pathname" */
            fullPathname = 1 ;
            break ;
        case NONOPT:		/* "<wildcardSpec>" */
            wildcardSpec = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (context) ;

    if (errflg) {
        fprintf (stderr, "Usage:  hls [-debug] [-directory <pathname>] [-full]\n") ;
        fprintf (stderr, "            [-greater <numBytes>] [-less <numBytes>] [-pathname]\n") ;
        fprintf (stderr, "            <wildcardSpec>\n") ;
        exit (EINVAL) ;
    }


/*******************************************************************************
    List the files in the directory.
*******************************************************************************/

    exit (fileTreeWalk (directory, wildcardSpec, FtwPhysical,
                        examineFile, NULL)) ;

}

/*******************************************************************************

Procedure:

    accessString ()

    Convert Access Modes to String.


Purpose:

    Function accessString() returns a string showing a file's access modes.


    Invocation:

        string = accessString (mode) ;

    where:

        <mode>		- I
            is the mode field from the file's STAT(2) structure.
        <string>	- O
            returns a string containing the ASCII representation of the
            access modes.  The string is stored in memory local to this
            function and the string should be used or duplicated before
            calling accessString() again.

*******************************************************************************/


static  char  *accessString (

#    if  __STDC__
        mode_t  mode)
#    else
        mode)

        mode_t  mode ;
#    endif

{    /* Local variables. */
    static  char  string[11] ;



    if (S_ISDIR (mode))
        string[0] = 'd' ;
#ifdef S_ISLNK
    else if (S_ISLNK (mode))
        string[0] = 'l' ;
#endif
    else
        string[0] = '-' ;

    string[1] = (mode & S_IRUSR) ? 'r' : '-' ;
    string[2] = (mode & S_IWUSR) ? 'w' : '-' ;
    string[3] = (mode & S_IXUSR) ? 'x' : '-' ;

    string[4] = (mode & S_IRGRP) ? 'r' : '-' ;
    string[5] = (mode & S_IWGRP) ? 'w' : '-' ;
    string[6] = (mode & S_IXGRP) ? 'x' : '-' ;

    string[7] = (mode & S_IROTH) ? 'r' : '-' ;
    string[8] = (mode & S_IWOTH) ? 'w' : '-' ;
    string[9] = (mode & S_IXOTH) ? 'x' : '-' ;

    return (string) ;

}

/*******************************************************************************

Procedure:

    examineFile ()

    Examine a File Entry.


Purpose:

    Function examineFile(), a callback function invoked for each file
    encountered in the directory hierarchy, examines the directory
    entry for a file.


    Invocation:

        status = examineFile (fullFileName, fileName, fileType,
                              fileInfo, level, clientData) ;

    where:

        <fullFileName>	- I
            is the full pathname for the file.
        <fileName>	- I
            is the file's name, excluding the directory.
        <fileType>	- I
            is the file type; see the FtwFileType enumeration in "ftw_util.h".
        <fileInfo>	- I
            is the system "stat" structure for the file; see STAT(2).
        <level>		- I
            is the level in the directory hierarchy at which the file is found.
        <clientData>		- I
            is the (VOID *) pointer specified in the call to fileTreeWalk().
        <status>		- O
            returns the status of examining the file entry, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


static  int  examineFile (

#    if  __STDC__
        const  char  *fullFileName,
        const  char  *fileName,
        FtwFileType fileType,
        struct  stat  *fileInfo,
        int  level,
        void  *clientData)
#    else
        fullFileName, fileName, fileType, fileInfo, level, clientData)

        char  *fullFileName ;
        char  *fileName ;
        FtwFileType fileType ;
        struct  stat  *fileInfo ;
        int  level ;
        void  *clientData ;
#    endif

{

/* If the file is a regular file and its size is not within the size limits
   specified by the user, then ignore the file. */

    if ((fileType == FtwFile) &&
        ((fileInfo->st_size < greaterThan) || (lessThan < fileInfo->st_size))) {
        return (0) ;
    }

/* Indent the listing according to the depth within the directory tree. */

    if (!fullListing && !fullPathname) {
        while (level-- > 0)
            printf ("    ") ;
    }

/* If a full listing was requested, then display the additional information. */

    if (fullListing) {
        switch (fileType) {
        case FtwDirectory:
            printf ("\n") ;
        case FtwDirNoRead:
        case FtwFile:
        case FtwSymbolicLink:
            printf ("%s  %12lu  ",
                    accessString (fileInfo->st_mode),
                    (unsigned long) fileInfo->st_size) ;
            break ;
        default:
            break ;
        }
    }

/* List the file. */

    switch (fileType) {
    case FtwDirectory:
        printf ("%s/\n",
                (fullListing || fullPathname) ? fullFileName : fileName) ;
        break ;
    case FtwDirNoRead:
        printf ("%s/ permission denied\n", fileName) ;
        break ;
    case FtwNoStat:
        printf ("%s inscrutable\n", fileName) ;
        break ;
    case FtwFile:
        printf ("%s\n", fullPathname ? fullFileName : fileName) ;
        break ;
    case FtwSymbolicLink:
        printf ("%s@\n", fullPathname ? fullFileName : fileName) ;
        break ;
    case FtwVisited:
        printf ("%s visited\n", fileName) ;
        break ;
    default:
        printf ("%s ?\n", fullPathname ? fullFileName : fileName) ;
        break ;
    }

    return (0) ;

}
