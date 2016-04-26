/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

    yip.c


    Yank IPC Utility.


    Program YIP interactively deletes existing IPC entities, such as message
    queues, shared memories, and semaphores.


    Invocation:

        % yip  [-m] [-q] [-s]

    where:

        "-m"    specifies that shared memories ONLY are to be deleted.
        "-q"    specifies that message queues ONLY are to be deleted.
        "-s"    specifies that semaphores ONLY are to be deleted.

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  "getopt.h"			/* GETOPT(3) definitions. */
#ifdef  BSD				/* Define for BSD variants of UNIX. */
#    include  <strings.h>		/* C Library string functions. */
#    define  strchr  index
#    define  strrchr  rindex
#else
#    ifdef  VXWORKS
#        include  <strLib.h>
#        define  strchr  index
#        define  strrchr  rindex
#    else
#        include  <string.h>		/* C Library string functions. */
#    endif
#endif

					/* External functions. */
extern  char  *getword (), *set_handler () ;
extern  int  delete_msq (), delete_sem (), delete_shmem () ;


main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    char  inbuf[128], *s ;
    int  errflg ;
    int  ipc_id, length, only_msq, only_sem, only_shm, option ;
    FILE  *infile ;





/*******************************************************************************

  Scan the command line options.

*******************************************************************************/

    only_msq = only_sem = only_shm = 0 ;

    for (optind = 1, opterr = -1, errflg = 0 ;  optind < argc ;  ) {

        while ((option = getopt (argc, argv, "mqs")) != NONOPT) {
            switch (option) {
            case 'm':  only_shm = -1 ;  break ;
            case 'q':  only_msq = -1 ;  break ;
            case 's':  only_sem = -1 ;  break ;
            case '?':  errflg++ ;  break ;
            default :  break ;
            }
        }

        if (optind < argc) {
            optind++ ;
        }

    }

    if (errflg) {
        fprintf (stderr, "Usage:  yip  [-mqs]\n") ;
        exit (-1) ;
    }


/*******************************************************************************

    Execute the UNIX command "ipcs" (IPC Status) and pipe its output into our
    input.

*******************************************************************************/

    infile = popen ("ipcs", "r") ;
    if (infile == NULL) {
        fprintf (stderr, "[YIP] Error piping \"ipcs\" command as input.\n") ;
        perror ("popen") ;  exit (errno) ;
    }

/*******************************************************************************

    Read each line of input.  Look for status lines for IPC objects, indicated
    by an "m", "q", or "s" (for shared memories, message queues, and semaphores,
    respectively) at the beginning of the line.  For each status line, ask the
    user if the object should be deleted.  Other lines (headings, etc.), are
    simply echoed to the screen.

*******************************************************************************/

    while (fgets (inbuf, (sizeof inbuf), infile) != NULL) {

        if ((s = strchr (inbuf, '\n')) != NULL)  *s = '\0' ;	/* Trim trailing newline. */
        str_trim (inbuf, -1) ;

        length = 0 ;
        s = getword (inbuf, " \t", &length) ;

        if (strncmp (s, "m", length) == 0) {		/* Shared memory? */

            if (!only_shm && (only_msq || only_sem)) {
                printf ("%s\n", inbuf) ;  continue ;
            }
            s = getword (inbuf, " \t", &length) ;
            ipc_id = atoi (s) ;
            printf ("%s\tDelete (y/cr)? ", inbuf) ;
            if (fgets (inbuf, (sizeof inbuf), stdin) == NULL) {
                printf ("\n") ;  exit (0) ;
            }
            length = 0 ;  s = getword (inbuf, " \t\n", &length) ;
            if ((*s == 'y') || (*s == 'Y'))  delete_shmem (ipc_id, NULL) ;

        } else if (strncmp (s, "q", length) == 0) {	/* Message queue? */

            if (!only_msq && (only_sem || only_shm)) {
                printf ("%s\n", inbuf) ;  continue ;
            }
            s = getword (inbuf, " \t", &length) ;
            ipc_id = atoi (s) ;
            printf ("%s\tDelete (y/cr)? ", inbuf) ;
            if (fgets (inbuf, (sizeof inbuf), stdin) == NULL) {
                printf ("\n") ;  exit (0) ;
            }
            length = 0 ;  s = getword (inbuf, " \t\n", &length) ;
            if ((*s == 'y') || (*s == 'Y'))  delete_msq (ipc_id) ;

        } else if (strncmp (s, "s", length) == 0) {	/* Semaphore? */

            if (!only_sem && (only_msq || only_shm)) {
                printf ("%s\n", inbuf) ;  continue ;
            }
            s = getword (inbuf, " \t", &length) ;
            ipc_id = atoi (s) ;
            printf ("%s\tDelete (y/cr)? ", inbuf) ;
            if (fgets (inbuf, (sizeof inbuf), stdin) == NULL) {
                printf ("\n") ;  exit (0) ;
            }
            length = 0 ;  s = getword (inbuf, " \t\n", &length) ;
            if ((*s == 'y') || (*s == 'Y'))  delete_sem (ipc_id) ;

        } else {					/* Fluff? */

            printf ("%s\n", inbuf) ;

        }

    }

}
