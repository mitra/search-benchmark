/* $Id: dirtree.c,v 1.3 2005/02/12 00:44:18 alex Exp alex $ */
/*******************************************************************************

    dirtree.c


    Directory Tree Utility.


    Program DIRTREE outputs a directory tree structure to standard output.


    Invocation:

        % dirtree [-debug] [-full] [top_directory]

    where:

        "-debug"
            turns debug on.
        "-full"
            prints out the full pathnames for subdirectories.  Normally,
            only the top, top-level directory name is displayed as a full
            pathname; the subdirectories are displayed as simple file names.
        <top_directory>
            specifies the pathname for the top ("root") of the directory
            structure, e.g., "/usr/alex" for Alex's directory structure.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <sys/stat.h>			/* File status definitions. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */

#ifdef  PRE_SUN_OS_40
#    include  <sys/dir.h>		/* Directory entry definitions. */
#    define  dirent  direct		/* Renames directory structure type. */
#else
#    include  <dirent.h>		/* Directory entry definitions. */
#endif

int  dirtree_debug = 0 ;		/* Global debug switch (-1/0 = yes/no). */
int  dirtree_full_path = 0 ;		/* Output format switch, where
					-1 = full pathname for subdirectories;
					0 = full name for top directory only. */


/*******************************************************************************
    Internal functions.
*******************************************************************************/

static  int  print_dir_tree (
#    if PROTOTYPES
        FILE  *outfile,
        char  *top_directory,
        int  level
#    endif
    ) ;

/*******************************************************************************
    Main Program.
*******************************************************************************/

int  main (

#    if PROTOTYPES
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{  /* Local variables. */
    char  *argument, *top_directory ;
    int  errflg, option ;
    OptContext  scan ;




/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    top_directory = NULL ;

    opt_init (argc, argv, "{debug}{full}", NULL, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            dirtree_debug = 1 ;
            break ;
        case 2:			/* "-full" */
            dirtree_full_path = 1 ;
            break ;
        case NONOPT:
            top_directory = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  dirtree [-debug] [-full] <top_directory>\n") ;
        exit (EINVAL) ;
    }

    top_directory = strdup (fnmBuild (FnmPath, top_directory, NULL)) ;


/*******************************************************************************
    Print out the full, top-level directory name.  Then, recursively descend
    the directory tree, printing out each new level of directories at increasing
    indentation.
*******************************************************************************/

    if (dirtree_debug)  printf ("(main) Top Directory = \"%s\"\n", top_directory) ;

    print_dir_tree (stdout, top_directory, 0) ;

    exit (0) ;

}

/*******************************************************************************

    print_dir_tree ()

    Print Directory Tree.


    Function PRINT_DIR_TREE recursively descends through a directory tree and
    prints it out with indentation reflecting the tree structure.


    Invocation:

        status = print_dir_tree (file, top_directory, level) ;

    where

        <file>
            is the UNIX file descriptor for the output file.
        <top_directory>
            is the UNIX pathname of the "root" directory in the tree.
        <level>
            is the level of descent in the directory tree.  The level starts
            at zero at the very top level of the tree and increases by one as
            PRINT_DIR_TREE descends each level.
        <status>
            returns the status of the tree print operation, zero if no errors
            occurred and ERRNO if an error occurred.

*******************************************************************************/


int  print_dir_tree (

#    if PROTOTYPES
        FILE  *outfile,
        char  *top_directory,
        int  level)
#    else
        outfile, top_directory, level)

        FILE  *outfile ;
        char  *top_directory ;
        int  level ;
#    endif

{    /* Local variables. */
    char  pathname[PATH_MAX], *s ;
    int  length ;
    DIR  *dir_stream ;
    struct  dirent  *d ;
    struct  stat  file_info ;




/* Construct the top-level directory's full name; remove any trailing '/'. */

    strcpy (pathname, fnmBuild (FnmPath, top_directory, NULL)) ;
    length = strlen (pathname) ;
    if (pathname[length-1] == '/')  pathname[length-1] = '\0' ;


/* Print out the top-level directory's name. */

    if (level == 0) {
        fprintf (outfile, "%s\n", pathname) ;
    } else if (dirtree_full_path) {
        fprintf (outfile, "%*s%s\n", level*4, " ", pathname) ;
    } else {
        fprintf (outfile, "%*s%s\n", level*4, " ",
                 fnmBuild (FnmFile, pathname, NULL)) ;
    }


/* Open the directory so we can scan it for subdirectories. */

    if (dirtree_debug)  printf ("(print_dir_tree) Opening directory \"%s\" at level %d.\n", pathname, level) ;
    dir_stream = opendir (pathname) ;
    if (dir_stream == NULL) {
        fprintf (stderr, "(print_dir_tree) Error opening directory stream for \"%s\".\n", pathname) ;
        perror ("opendir") ;  return (errno) ;
    }


/* For each subdirectory, print out the subdirectory's subtree.  Variable
   PATHNAME initially contains the directory pathname, e.g., "/usr/alex/".
   Variable S points to the first character after the directory pathname;
   by copying the name of each file in our scan to position S, we can
   construct the full pathname of the file; e.g., "/usr/alex/file1",
   "/usr/alex/file2", etc. */

    strcat (pathname, "/") ;		/* Restore trailing '/'. */
    s = pathname + strlen (pathname) ;

    for (d = readdir (dir_stream) ;  d != NULL ;  d = readdir (dir_stream)) {

        if (dirtree_debug)  printf ("(print_dir_tree) Entry \"%s\"\n", d->d_name) ;

        if (strcmp (d->d_name, ".") == 0)  continue ;	/* Skip "." and ".." directories. */
        if (strcmp (d->d_name, "..") == 0)  continue ;

        strcpy (s, d->d_name) ;		/* Append file name to directory name. */

        if (stat (pathname, &file_info)) {
            fprintf (stderr, "(print_dir_tree) Error obtaining information for file \"%s\".\n", pathname) ;
            perror ("stat") ;  continue ;
        }

        if (dirtree_debug)  printf ("(print_dir_tree) st_mode = %.4X\n", (int) file_info.st_mode) ;

        if ((file_info.st_mode & S_IFDIR) != 0) {
            print_dir_tree (outfile, pathname, level+1) ;
            /* if (status)  return (status) ; */
        }

    }


/* The directory scan is complete; close it. */

    *--s = '\0' ;			/* Remove '/<last_filename>'. */
    if (dirtree_debug)  printf ("(print_dir_tree) Closing directory \"%s\" at level %d.\n", pathname, level) ;
    closedir (dir_stream) ;

    return (0) ;

}
