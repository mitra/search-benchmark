/* $Id: gflow.c,v 1.2 2005/02/12 00:31:39 alex Exp $ */
/*******************************************************************************

Process:

    gflow

    Graph Flow.


Author:    Alex Measday


Purpose:

    Program GFLOW ...


    Invocation:

        % gflow [-debug] [<file(s)>]

    where

        "-debug"
            enables debug output (written to STDOUT).
        "<file(s)>"
            are zero or more input files; if no files are specified, input
            is read from standard input.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "gsc_util.h"			/* Graph/structure chart definitions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */


/*******************************************************************************
    Internal functions.
*******************************************************************************/

static  void  readFile (
#    if PROTOTYPES
        FILE  *file,
        Graph  graph
#    endif
    ) ;

/*!*****************************************************************************
    GFLOW's Main Program.
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
    char  *argument ;
    FILE  *file ;
    Graph  graph ;
    int  errflg, i, option ;
    OptContext  scan ;

    const  char  *optionList[] = {	/* Command line options. */
        "{debug}", NULL
    } ;




    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    file = NULL ;
    gscCreate (NULL, NULL, NULL, NULL, &graph) ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-debug" */
            gsc_util_debug = 1 ;
            break ;
        case NONOPT:		/* "<file>" */
            file = fopen (argument, "r") ;
            if (file == NULL) {
                LGE "[%s] Error opening input file: %s\nfopen: ",
                    argv[0], argument) ;
                errflg++ ;
            } else {
                readFile (file, graph) ;
                fclose (file) ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  gflow [-debug] [<file(s)>]\n") ;
        exit (EINVAL) ;
    }

/* If no files were specified, read input from standard input. */

    if (file == NULL)  readFile (stdin, graph) ;

/*******************************************************************************
    Output the graph(s).
*******************************************************************************/

    for (i = 1 ;  ;  i++) {		/* For each possible root vertex. */
        char  *name ;
        GscVisitStatus  visit ;
        int  depth, j ;

        name = gscRoot (graph, i) ;
        if (name == NULL)  break ;
        gscMark (graph, name, 0) ;	/* Mark graph starting at root. */
					/* Traverse the graph. */
        gscFirst (graph, &name, &depth, &visit) ;
        while (name != NULL) {
            for (j = 0 ;  j < depth ;  j++)
                printf ("    ") ;
            printf ("%s%s\n", name,
                    (visit == GscPREVIOUS) ? "  +" :
                    (visit == GscRECURSIVE) ? "  *" : "") ;
            gscNext (graph, &name, &depth, &visit) ;
        }

    }


    exit (errno) ;

}

/*!*****************************************************************************
    readFile() - reads an input file, storing the arcs in the graph.
*******************************************************************************/

static  void  readFile (

#    if PROTOTYPES
        FILE  *file,
        Graph  graph)
#    else
        file, graph)

        FILE  *file ;
        Graph  graph ;
#    endif

{    /* Local variables. */
    char  buffer[1024], *s ;



    while (fgets (buffer, sizeof buffer, file) != NULL) {
        s = strchr (buffer, '#') ;
        if (s != NULL)  *s = '\0' ;
        strTrim (buffer, -1) ;
        if (strlen (buffer) == 0)  continue ;
        s = strchr (buffer, '\t') ;
        if (s == NULL)  continue ;
        *s++ = '\0' ;
        s = s + strspn (s, "\t ") ;
        gscAdd (graph, buffer, s) ;
    }

    return ;

}
