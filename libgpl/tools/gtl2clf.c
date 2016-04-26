/*******************************************************************************

    gtl2clf.c


    GTL2CLF converts a GENTLE-format HTTP log to a Combined Log File-format log.

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */

#define  MAX_STRING  32768

char  *monthName[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
} ;




int  main (

#    if __STDC__
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{  /* Local variables. */
    char  *agent, *date, *host, *referer, *s, *url ;
    char  buffer[MAX_STRING], header[MAX_STRING] ;
    FILE  *infile ;
    int  day, month ;




/* Open the GENTLE-format log file for input. */

    if (argc > 1) {
        infile = fopen (argv[1], "r") ;
        if (infile == NULL) {
            perror (argv[1]) ;
            exit (errno) ;
        }
    } else {
        infile = stdin ;
    }


/*******************************************************************************
    Read each entry in the log file, convert it to CLF format, and output it
    to standard output.
*******************************************************************************/

    for ( ; ; ) {

        if (fgets (buffer, sizeof buffer, infile) == NULL)  break ;

        if (strncmp (buffer, "<DT>", 4) != 0) {
            fprintf (stderr, "Skipping: %s", buffer) ;
            continue ;
        }

        s = buffer + 4 ;
        host = s ;  s = s + strcspn (s, ":") ;  *s = '\0' ;
        s = s + 5 ;
        url = s ;  s = s + strcspn (s, "<") ;  *s = '\0' ;
        s = s + 6 ;
        date = s ;  s = strchr (s, ' ') ;  *s = ':' ;
                    s = strchr (s, ']') ;  *s = '\0' ;

        month = atoi (date) ;
        day = atoi (&date[3]) ;
        date = date + 6 ;

        if (fgets (header, sizeof header, infile) == NULL)  break ;

        if (strncmp (header, "    <DD>", 8) != 0) {
            fprintf (stderr, "Skipping: %s", header) ;
            continue ;
        }

        s = strstr (header, "Referer:") ;
        referer = (s == NULL) ? NULL : (s + 9) ;
        s = strstr (header, "User-Agent:") ;
        agent = (s == NULL) ? NULL : (s + 12) ;

        if (referer == NULL) {
            referer = "-" ;
        } else {
            s = strchr (referer, '}') ;  if (s != NULL)  *s = '\0' ;
        }

        if (agent == NULL) {
            agent = "-" ;
        } else {
            s = strchr (agent, '}') ;  if (s != NULL)  *s = '\0' ;
            s = strchr (agent, ' ') ;  if (s != NULL)  *s = '\0' ;
        }

        printf ("%s - - %.2d/%s/%s \"GET %s HTTP/1.0\" - - %s %s\n",
                host, day, monthName[month-1], date, url, referer, agent) ;

    }

}
