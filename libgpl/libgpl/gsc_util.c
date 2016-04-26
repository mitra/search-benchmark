/* $Id: gsc_util.c,v 1.16 2004/04/23 21:47:28 alex Exp $ */
/*******************************************************************************

File:

    gsc_util.c

    Graph/Structure Chart Utilities.


Author:    Alex Measday


Purpose:

    The Graph/Structure Chart Utilities provide a simple means of constructing
    and traversing directed graphs.  A graph is web of connected points.  Each
    point is known as a "vertex"; the connection between two points is known
    as an "edge".  In a directed graph, edges have a direction.  An edge, A->B,
    goes from vertex A to vertex B, and not in the reverse direction (although
    there could be another edge, B->A).  Given an edge, A-B, vertex B is said
    to be adjacent to A.  A simple example of a directed graph is a program's
    structure chart, where each edge represents the "calls" relationship
    between a routine and a subroutine.

    Usually, graphs are built in order to traverse or search them, to step
    from vertex to vertex in a certain order.  The vertex at which a traversal
    starts is called the root vertex.  The main routine of a program would be
    the root of a structure chart; an arbitrary graph might have more than one
    possible root vertex.  There are two basic traversal/search strategies for
    graphs: depth-first and breadth-first.  A depth-first traversal descends
    into lower levels of a graph whenever it can; a breadth-first traversal
    visits each vertex at a given level before descending to the next lower
    level.

    The classical graph search algorithms only visit each vertex in a graph
    once during the search.  The GSC utilities use these same algorithms,
    but, when stepping through the vertices using gscFirst() and gscNext()
    (see below), vertices with multiple incoming edges will be "visited"
    multiple times.  For example, given two edges, "A->C" and "B->C",
    vertex C will be returned twice by gscNext(): the first time with
    a FIRST visit indicator and the second time with a PREVIOUSly-visited
    indicator.  The first vertex visited in a cycle will be flagged the
    second time as a RECURSIVEly-visited vertex.  If you only want to visit
    each vertex once, simply ignore the PREVIOUS and RECURSIVE vertices
    returned by gscFirst() and gscNext().

    The GSC utilities are easy to use:

        int  i ;
        char  *name, *vertex_A, *vertex_B ;
        Graph  graph ;

        gscCreate (..., &graph) ;	-- Create an empty graph.
        while (more_edges) {		-- Add edges to the graph.
	    ... get edge A->B ...
            gscAdd (graph, vertex_A, vertex_B) ;
        }

        for (i = 1 ;  ;  i++) {		-- For each possible root vertex ...

            name = gscRoot (graph, i) ;
            if (name == NULL)  break;

            gscMark (graph, name, 0) ;	-- Mark graph starting at root.

					-- Traverse the graph.
            gscFirst (graph, &name, ...) ;
            while (name != NULL) {
                ... process vertex "name" ...
                gscNext (graph, &name, ...) ;
            }

        }

    Vertex names are normal ASCII strings.  You can use other types of
    "names", but, to do so, you need to supply gscCreate() with the
    following functions for your name type: COMPARE, DELETE, DISPLAY,
    and DUPLICATE.  See gscCreate() for more information about how
    to do this.


Notes:

    These functions are reentrant under VxWorks (except for the global
    debug flag).


Public Procedures:

    gscAdd() - adds an edge to a graph.
    gscCreate() - creates an empty graph.
    gscDelete() - deletes a graph.
    gscDump() - dumps a graph.
    gscFirst() - begins a caller traversal of a graph and returns the root
        vertex of the graph.
    gscMark() - performs a complete traversal of a graph, marking it in
        preparation for a caller's traversal.
    gscNext() - returns the next vertex in a caller traversal of a graph.
    gscRoot() - returns potential root nodes in a graph.

Private Procedures:

    gscLocate() - locates a vertex by name in a graph's vertex list.
    gscMarkBFS() - performs a breadth-first traversal of a subgraph.
    gscMarkDFS() - performs a depth-first traversal of a subgraph.
    gscNextBFS() - returns the next vertex in a BFS traversal of a graph.
    gscNextDFS() - returns the next vertex in a DFS traversal of a graph.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "gsc_util.h"			/* Graph/structure chart definitions. */


/*******************************************************************************
    Graphs - are represented using adjacency lists.  The graph header structure
        has a pointer to a list of all the vertices in the graph.  Each vertex,
        in turn, has a pointer to a list of the vertices adjacent to the vertex
        in question; i.e., vertex A's adjacency list will have entries for all
        vertices X such that edge A->X is in the graph.  For example, a graph
        with edges A->B, A->C, C->B,and C->D will be represented as follows:

                               A -> B -> C
                               B
                               C -> B -> D
                               D

        where the vertical list (A, B, C, D) is the vertex list and the
        horizontal lists are the adjacency lists.
*******************************************************************************/

typedef  struct  CallEntry {
    struct  Vertex  *vertex ;		/* The vertex referenced by this entry. */
    GscVisitStatus  visit ;		/* Status of called vertex. */
    struct  CallEntry  *next ;
}  CallEntry ;

typedef  struct  Vertex {
    char  *name ;			/* Name of vertex. */
    CallEntry  *calls ;			/* List of all vertices "called" by this vertex. */
    bool  isCalled ;			/* Is this vertex "called" by any other? */
    bool  wasVisited ;			/* Visited yet? */
    bool  isLocked ;			/* On current search path? */
    struct  Vertex  *link ;		/* BFS: Next vertex/adjacency list. */
#define  parent  link			/* DFS: Parent vertex on first visit. */
    int  depth ;			/* Depth on first visit. */
    struct  Vertex  *next ;
}  Vertex ;

typedef  struct  _Graph {
#if PROTOTYPES
    int  (*compare)(const char *,	/* Vertex name comparison function. */
                    const char *) ;
    void  (*delete)(void *) ;		/* Vertex name deletion function. */
    char  *(*display)(const char *) ;	/* Vertex name display function. */
    char  *(*duplicate)(const char *) ;	/* Vertex name duplication function. */
#else
    int  (*compare)() ;			/* Vertex name comparison function. */
    void  (*delete)() ;			/* Vertex name deletion function. */
    char  *(*display)() ;		/* Vertex name display function. */
    char  *(*duplicate)() ;		/* Vertex name duplication function. */
#endif
    Vertex  *vertexList ;		/* List of vertices in the graph. */
    Vertex  *root ;			/* Root vertex - set by gscMark(). */
    bool  isBFS ;			/* BFS or DFS - set by gscMark(). */
    Vertex  *lastVertex ;		/* Last vertex visited by gscNext(). */
    CallEntry  *lastCall ;		/* Last adjacency list entry visited. */
}  _Graph ;

int  gsc_util_debug = 0 ;		/* Global debug switch (-1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  gsc_util_debug

					/* CPP macro to display vertex names. */
#define  gscDisplay(graph, name) \
    ((graph->display == NULL) ? name : graph->display (name))


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  Vertex  *gscLocate (
#    if PROTOTYPES
        Graph  graph,
        const  char  *name
#    endif
    ) ;

static  errno_t  gscMarkBFS (
#    if PROTOTYPES
        Graph  graph,
        Vertex  *root,
        int  depth
#    endif
    ) ;

static  errno_t  gscMarkDFS (
#    if PROTOTYPES
        Graph  graph,
        Vertex  *root,
        int  depth
#    endif
    ) ;

static  errno_t  gscNextBFS (
#    if PROTOTYPES
        Graph  graph,
        char  **name,
        int  *depth,
        GscVisitStatus  *visit
#    endif
    ) ;

static  errno_t  gscNextDFS (
#    if PROTOTYPES
        Graph  graph,
        char  **name,
        int  *depth,
        GscVisitStatus  *visit
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    gscAdd ()

    Add an Edge to a Graph.


Purpose:

    Function gscAdd() adds an edge to a graph.  A stand-alone vertex can be
    added to the graph by calling gscAdd() with a NULL destination vertex.


    Invocation:

        status = gscAdd (graph, vertex_1, vertex_2) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <vertex_1>	- I
        <vertex_2>	- I
            are the "names" of the source and destination vertices (i.e., the
            edge is directed from VERTEX_1 to VERTEX_2).  If VERTEX_2 is NULL,
            VERTXEX_1 is added to the graph as a stand-alone vertex (if it is
            not already present in the graph).  If you specified a special
            comparison function in the call to gscCreate(), then the vertex
            "names" s" should be compatible with that function; otherwise, you
            should pass in normal C strings.
        <status>	- O
            returns the status of adding the edge to the graph, zero if no
            errors occurred and ERRNO otherwise.

*******************************************************************************/

/*PDL----------------------------PDL--------------------------PDL**

CALL gscLocate() to locate the source vertex in the graph's list of vertices.
IF a destination vertex was not specfied THEN
    RETURN
ENDIF
CALL gscLocate() to locate the destination vertex in the graph's vertex list.
Tag the destination vertex as being called by another vertex.
Scan the source vertex's list of adjacent vertices.
IF the destination vertex is not in the source vertex's adjacency list THEN
    CALL MALLOC(3) to allocate an adjacency list entry.
    Set the adjacency list entry to point to the destination vertex.
    Add the new entry to the end of source vertex's adjacency list.
ENDIF

RETURN

**PDL----------------------------PDL--------------------------PDL*/


errno_t  gscAdd (

#    if PROTOTYPES
        Graph  graph,
        const  char  *vertex_1,
        const  char  *vertex_2)
#    else
        graph, vertex_1, vertex_2)

        Graph  graph ;
        char  *vertex_1 ;
        char  *vertex_2 ;
#    endif

{    /* Local variables. */
    CallEntry  *adj, *callee, *prev ;
    Vertex  *destination, *source ;




    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscAdd) NULL graph handle: ") ;
        return (errno) ;
    }

/* Locate the source and destination vertices in the graph's list of vertices.
   If a vertex is not found in the list, gscLocate() will automatically add
   the vertex to the list. */

    source = gscLocate (graph, vertex_1) ;
    if (source == NULL) {
        LGE "(gscAdd) Error locating \"%s\" in graph %8.8X.\ngscLocate: ",
            gscDisplay (graph, vertex_1), graph) ;
        return (errno) ;
    }

    if (vertex_2 == NULL) {		/* Vertex 1 stands by itself? */
        LGI "(gscAdd) Added vertex \"%s\" to graph %p.\n",
            gscDisplay (graph, vertex_1), (void *) graph) ;
        return (0) ;
    }

    destination = gscLocate (graph, vertex_2) ;
    if (destination == NULL) {
        LGE "(gscAdd) Error locating \"%s\" in graph %p.\ngscLocate: ",
            gscDisplay (graph, vertex_2), graph) ;
        return (errno) ;
    }
    destination->isCalled = true ;

/* Scan the source vertex's adjacency list, looking for the destination
   vertex.  If a "call" to the destination vector is not found, then add
   a new entry to the adjacency list. */

    prev = NULL ;
    for (adj = source->calls ;  adj != NULL ;  adj = adj->next) {
        if (adj->vertex == destination)  break ;
        prev = adj ;
    }

    if (adj == NULL) {
        callee = (CallEntry *) malloc (sizeof (CallEntry)) ;
        if (callee == NULL) {
            LGE "(gscAdd) Error allocating adjacency list entry for \"%s -> %s\"\nmalloc: ",
                gscDisplay (graph, vertex_1), gscDisplay (graph, vertex_2)) ;
            return (errno) ;
        }
        callee->vertex = destination ;
        callee->next = NULL ;
        if (prev == NULL)		/* First entry in list? */
            source->calls = callee ;
        else				/* Add to end of list. */
            prev->next = callee ;
    }

    LGI "(gscAdd) Added edge \"%s\" -> \"%s\" to graph %p.\n",
        gscDisplay (graph, vertex_1),
        gscDisplay (graph, vertex_2),
        (void *) graph) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscCreate ()

    Create an Empty Graph.


Purpose:

    Function gscCreate() creates an empty graph.


    Invocation:

        status = gscCreate (compare, duplicate, delete, display, &graph) ;

    where:

        <compare>	- I
            is a pointer to a name comparison function.  This function is used
            to compare vertex "names" and should be declared as follows:
                int  compare (char *name1, char *name2) { ... } ;
            COMPARE() should return zero if the two "names" are equal and a
            non-zero value if the names are not equal.  If the names in your
            graph are normal C strings, then pass in NULL for this argument;
            the graph routines will use the Standard C Library's STRCMP(3)
            string comparison function.
        <duplicate>	- I
            is a pointer to a name duplication function.  This function is
            used to duplicate vertex "names" and should be declared as follows:
                char  *duplicate (char *name) { ... } ;
            DUPLICATE() should return a MALLOC(3)ed copy of NAME.  If the
            names in your graph are normal C strings, then pass in NULL for
            this argument; the graph routines will use the Standard C Library's
            STRDUP(3) string duplication function.
        <delete>	- I
            is a pointer to a name deletion function.  This function is used
            to delete vertex "names" when a graph is deleted; DELETE() should
            be declared as follows:
                void  delete (void *name) { ... } ;
            If the names in your graph are normal C strings, then pass in
            NULL for this argument; the graph routines will use the Standard
            C Library's FREE(3) function to deallocate the names.
        <display>	- I
            is a pointer to a name display function.  This function is used
            to display vertex "names" and should be declared as follows:
                char  *display (char *name) { ... } ;
            DISPLAY() should return a printable, ASCII string version of NAME.
            If the names in your graph are normal C strings, then pass in NULL
            for this argument; the graph routines will use the names directly
            as ASCII strings.
        <graph>		- O
            returns a handle that can be used in other GSC_UTIL calls.
        <status>	- O
            returns the status of creating the graph, zero if there were no
            errors and ERRNO if there were.

*******************************************************************************/


errno_t  gscCreate (

#    if PROTOTYPES
        GscCompareF  compareF,
        GscDuplicateF  duplicateF,
        GscDeleteF  deleteF,
        GscDisplayF  displayF,
        Graph  *graph)
#    else
        compareF, duplicateF, deleteF, displayF, graph)

        GscCompareF  compareF ;
        GscDuplicateF  duplicateF ;
        GscDeleteF  deleteF ;
        GscDisplayF  displayF ;
        Graph  *graph ;
#    endif

{

    *graph = (Graph) malloc (sizeof (_Graph)) ;
    if (*graph == NULL) {
        LGE "(gscCreate) Error allocating graph header.\nmalloc: ") ;
        return (errno) ;
    }

/* Save function pointers.  Substitute default system functions for NULL
   pointers when they're actually needed.  (To avoid PalmOS compilation
   problems.) */

    (*graph)->compare = compareF ;
    (*graph)->duplicate = duplicateF ;
    (*graph)->delete = deleteF ;
    (*graph)->display = displayF ;
    (*graph)->vertexList = NULL ;
    (*graph)->root = NULL ;
    (*graph)->isBFS = false ;
    (*graph)->lastVertex = NULL ;
    (*graph)->lastCall = NULL ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscDelete ()

    Delete a Graph.


Purpose:

    Function gscDelete() deletes a graph.


    Invocation:

        status = gscDelete (graph) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <status>	- O
            returns the status of deleting the graph, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  gscDelete (

#    if PROTOTYPES
        Graph  graph)
#    else
        graph)

        Graph  graph ;
#    endif

{    /* Local variables. */
    CallEntry  *adj, *nextAdj ;
    Vertex  *nextVertex, *vertex ;



    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscDelete) NULL graph handle: ") ;
        return (errno) ;
    }

/* For each vertex in the graph, delete the vertex's adjacency list and the
   vertex itself. */

    for (vertex = graph->vertexList ;  vertex != NULL ;  vertex = nextVertex) {
        nextVertex = vertex->next ;
        LGI "(gscDelete) Deleting vertex \"%s\" ...\n",
            gscDisplay (graph, vertex->name)) ;
					/* Delete the vertex's adjacency list. */
        for (adj = vertex->calls ;  adj != NULL ;  adj = nextAdj) {
            nextAdj = adj->next ;  free (adj) ;
        }
        if (graph->delete == NULL)
            free (vertex->name) ;		/* Delete the vertex's name. */
        else
            graph->delete (vertex->name) ;	/* Delete the vertex's name. */
        free (vertex) ;				/* Delete the vertex structure. */
    }

/* Finally, delete the graph header. */

    free (graph) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscDump ()

    Dump a Graph.


Purpose:

    Function gscDump() provides a formatted dump of a graph.


    Invocation:

        status = gscDump (file, header, graph) ;

    where:

        <file>		- I
            is the UNIX descriptor for the output file.  If NULL, the
            file pointer defaults to standard output (STDOUT).
        <header>	- I
            is a text string to be output as a header.  The string is
            actually used as the format string in an FPRINTF statement,
            so you need to include any newlines ("\n"), etc. that you
            need.  This argument can be NULL.
        <graph>		- I
            is the graph handle returned by gscCreate().
        <status>	- O
            returns the status of dumping the graph, zero if no errors occurred
            and ERRNO otherwise.

*******************************************************************************/


errno_t  gscDump (

#    if PROTOTYPES
        FILE  *file,
        const  char  *header,
        Graph  graph)
#    else
        file, header, graph)

        FILE  *file ;
        char  *header ;
        Graph  graph ;
#    endif

{    /* Local variables. */
    CallEntry  *callee ;
    Vertex  *vertex ;



    if (file == (FILE *) NULL)  file = stdout ;

/* Print the header text. */

    if (header != NULL)  fprintf (file, header) ;

    if ((graph == NULL) || (graph->vertexList == NULL)) {
        fprintf (file, "<empty>\n") ;
        return (0) ;
    }

/* For each vertex in the graph, print out a list of the vertices it "calls". */

    for (vertex = graph->vertexList ;  vertex != NULL ;
         vertex = vertex->next) {
        fprintf (file, "Vertex %s%s%s%s%s\n",
                 gscDisplay (graph, vertex->name),
                 vertex->wasVisited ? "  V" : "",
                 (vertex->link == NULL) ? "" : "  (",
                 (vertex->link == NULL) ? "" :
                    gscDisplay (graph, (vertex->link)->name),
                 (vertex->link == NULL) ? "" : ")") ;
        for (callee = vertex->calls ;  callee != NULL ;  callee = callee->next)
            fprintf (file, "    -> %s\n",
                     gscDisplay (graph, (callee->vertex)->name)) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscFirst ()

    Get Name of First Vertex in Graph Traversal.


Purpose:

    Function gscFirst() initiates a user-controlled traversal of a graph and
    returns the name of the first vertex (i.e., the root) visited during the
    traversal.  Before calling gscFirst(), gscMark() should be called to
    perform an internal traversal of the graph, after which gscFirst() and
    gscNext() can be called to read out the names of the vertices visited
    by gscMark().


    Invocation:

        status = gscFirst (graph, &name, &depth, &visit) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <name>		- O
            returns the "name" of the first vertex visited during the graph
            traversal; i.e., the root vertex specified in the call to
            gscMark().  Vertex "names" are normally C strings, although
            other types can be used; see gscCreate().  NULL is returned
            if the marked graph being traversed is empty.
        <depth>		- O
            returns the depth (0..N) in the graph of this visit to the vertex.
            The root vertex is at depth 0; its adjacent vertices are at depth
            1, and so on.
        <visit>		- O
            returns the status of the visit to this vertex:
                GscFIRST - This is the first visit to the vertex.
                GscPREVIOUS - This vertex was visited previously during
                    the traversal.
                GscRECURSIVE - This is a recursive visit to the vertex;
                    i.e., there is a cycle in the graph.
            On the first visit to a vertex, the VISIT argument returns
            GscFIRST and the vertex's subgraph is or will be traversed.
            On subsequent visits to the same vertex, the VISIT argument
            will return GscPREVIOUS (or GscRECURSIVE!) and the vertex's
            subgraph will not be traversed again.
        <status>	- O
            returns zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  gscFirst (

#    if PROTOTYPES
        Graph  graph,
        char  **name,
        int  *depth,
        GscVisitStatus  *visit)
#    else
        graph, name, depth, visit)

        Graph  graph ;
        char  **name ;
        int  *depth ;
        GscVisitStatus  *visit ;
#    endif

{

    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscFirst) NULL graph handle: ") ;
        return (errno) ;
    }
    if (graph->root == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscFirst) Graph %p hasn't been marked yet.\n", graph) ;
        return (errno) ;
    }

/* Reset the last vertex pointers in the graph header and return the root
   vertex's name to the calling routine. */

    graph->lastVertex = graph->root ;
    graph->lastCall = NULL ;

    *name = (graph->root)->name ;
    *depth = 0 ;
    *visit = GscFIRST ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscMark ()

    Traverse and Mark a Graph.


Purpose:

    Function gscMark() performs a depth-first or breadth-first traversal of a
    graph and marks the graph in preparation for a user-controlled traversal
    using gscFirst() and gscNext().  gscMark() should be called before
    calling the latter two routines.


    Invocation:

        status = gscMark (graph, root, bfs) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <root>		- I
            is the "name" of the root vertex at which the traversal is to
            start.  If you specified a special comparison function in the
            call to gscCreate(), this "name" should be compatible with
            that function; otherwise, you should pass in a normal C string.
        <bfs>		- I
            indicates the type of traversal to perform: 0 means depth-first,
            non-zero means breadth-first.
        <status>	- O
            returns the status of traversing and marking the graph, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/


errno_t  gscMark (

#    if PROTOTYPES
        Graph  graph,
        const  char  *root,
        bool  bfs)
#    else
        graph, root, bfs)

        Graph  graph ;
        char  *root ;
        bool  bfs ;
#    endif

{    /* Local variables. */
    Vertex  *vertex ;




    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscMark) NULL graph handle: ") ;
        return (errno) ;
    }

/* Initialize all the traversal information in the graph. */

    for (vertex = graph->vertexList ;  vertex != NULL ;
         vertex = vertex->next) {
        vertex->wasVisited = false ;  vertex->isLocked = false ;
        vertex->link = NULL ;  vertex->depth = 0 ;
    }
    graph->isBFS = bfs ;
    graph->lastVertex = NULL ;
    graph->lastCall = NULL ;

/* Locate the root vertex in the graph's list of vertices. */

    graph->root = gscLocate (graph, root) ;
    if (graph->root == NULL) {
        LGE "(gscMark) Error locating root \"%s\" in graph %p.\ngscLocate: ",
            root, graph) ;
        return (errno) ;
    }

/* Depending on the type of traversal requested, traverse the graph, beginning
   with the root vertex. */

    if (bfs)
        return (gscMarkBFS (graph, graph->root, 0)) ;
    else
        return (gscMarkDFS (graph, graph->root, 0)) ;

}

/*!*****************************************************************************

Procedure:

    gscNext ()

    Get Name of Next Vertex in Graph Traversal.


Purpose:

    Function gscNext() returns the name of the next vertex visited during a
    graph traversal.  gscMark() and gscFirst() should be called before
    gscNext() is called: gscMark() to perform an internal traversal of
    the graph, and gscFirst() to begin reading out the names of the vertices
    visited by gscMark().


    Invocation:

        status = gscNext (graph, &name, &depth, &visit) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <name>		- O
            returns the "name" of the next vertex visited during the graph
            traversal.  Vertex "names" are normally C strings, although other
            types can be used; see gscCreate().  NULL is returned when the
            traversal is complete and there are no more vertices to visit.
        <depth>		- O
            returns the depth (0..N) in the graph of this visit to the vertex.
            The root vertex is at depth 0; its adjacent vertices are at depth
            1, and so on.
        <visit>		- O
            returns the status of the visit to this vertex:
                GscFIRST - This is the first visit to the vertex.
                GscPREVIOUS - This vertex was visited previously during
                    the traversal.
                GscRECURSIVE - This is a recursive visit to the vertex;
                    i.e., there is a cycle in the graph.
            On the first visit to a vertex, the VISIT argument returns
            GscFIRST and the vertex's subgraph is or will be traversed.
            On subsequent visits to the same vertex, the VISIT argument
            will return GscPREVIOUS (or GscRECURSIVE!) and the vertex's
            subgraph will not be traversed again.
        <status>	- O
            returns zero if no errors occurred and ERRNO otherwise.  When
            the traversal is complete and there are no more vertices to visit,
            STATUS returns zero and NAME returns NULL.

*******************************************************************************/


errno_t  gscNext (

#    if PROTOTYPES
        Graph  graph,
        char  **name,
        int  *depth,
        GscVisitStatus  *visit)
#    else
        graph, name, depth, visit)

        Graph  graph ;
        char  **name ;
        int  *depth ;
        GscVisitStatus  *visit ;
#    endif

{

    *name = NULL ;  *depth = 0 ;  *visit = GscFIRST ;

    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscNext) NULL graph handle: ") ;
        return (errno) ;
    }
    if (graph->root == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscNext) Graph %p hasn't been marked yet.\n", graph) ;
        return (errno) ;
    }

/* If gscFirst() hasn't been called yet, then call it and return. */

    if (graph->lastVertex == NULL) {
        return (gscFirst (graph, name, depth, visit)) ;
    }

/* Otherwise, step to the next vertex in the traversal. */

    if (graph->isBFS)
        return (gscNextBFS (graph, name, depth, visit)) ;
    else
        return (gscNextDFS (graph, name, depth, visit)) ;

}

/*!*****************************************************************************

Procedure:

    gscRoot ()

    Find Possible Roots in a Graph.


Purpose:

    Function gscRoot() returns the name of a vertex that could be the root of
    a graph.  A vertex can be the root of a graph if it has no incoming edges
    (i.e., no parent vertices).  For example, a tree-structured graph has only
    a single root node.  An arbitrary graph could have more than one possible
    root.  To see all the potential roots of a graph, you must call gscRoot()
    in a loop:

                for (i = 1 ;  ;  i++) {
                    name = gscRoot (graph, i) ;
                    if (name == NULL)  break ;
                    ... process root vertex <name> ...
                }


    Invocation:

        name = gscRoot (graph, which) ;

    where:

        <graph>	- I
            is the graph handle returned by gscCreate().
        <which>	- I
            specifies which root (the first through the Nth) you want returned.
        <name>	- O
            returns the "name" of the selected root vertex.  If the index is
            greater than the number of possible roots in the graph, NULL is
            returned.

*******************************************************************************/


char  *gscRoot (

#    if PROTOTYPES
        Graph  graph,
        int  which)
#    else
        graph, which)

        Graph  graph ;
        int  which ;
#    endif

{    /* Local variables. */
    Vertex  *vertex ;



    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscRoot) NULL graph handle: ") ;
        return (NULL) ;
    }

/* Scan the list of vertices, looking for vertices which have no
   incoming edges; i.e., they are not called by any other vertex. */

    for (vertex = graph->vertexList ;  vertex != NULL ;
         vertex = vertex->next) {
        if (!vertex->isCalled)  which-- ;
        if (which == 0)  break ;
    }

    if ((vertex == NULL) || vertex->isCalled)
        return (NULL) ;			/* No more roots. */
    else
        return (vertex->name) ;		/* Possible root. */

}

/*!*****************************************************************************

Procedure:

    gscLocate ()

    Locate a Vertex by Name.


Purpose:

    Function gscLocate() locates a vertex by name in a graph's list of
    vertices.  If the vertex is not found in the list, it is added to
    the list.  A pointer to the new or existing vertex's list entry is
    returned to the calling routine.


    Invocation:

        vertex = gscLocate (graph, name) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <name>		- I
            is the "name" of the vertex you wish to locate.  If you specified
            a special comparison function in the call to gscCreate(), this
            "name" should be compatible with that function; otherwise, you
            should pass in a normal C string.
        <vertex>	- O
            returns a pointer to the vertex's "Vertex" structure.
            (If the vertex was not found in the list, it is added to the
            list and a pointer to the new "Vertex" structure is
            returned to the caller.)  NULL is returned if an error occurs.

*******************************************************************************/


static  Vertex  *gscLocate (

#    if PROTOTYPES
        Graph  graph,
        const  char  *name)
#    else
        graph, name)

        Graph  graph ;
        char  *name ;
#    endif

{    /* Local variables. */
    Vertex  *prev, *vertex ;



    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscLocate) NULL graph handle: ") ;
        return (NULL) ;
    }

/* Scan the graph's list of vertices, looking for a vertex with the desired
   name. */

    prev = NULL ;
    for (vertex = graph->vertexList ;  vertex != NULL ;
         vertex = vertex->next) {
        if (graph->compare == NULL) {
            if (strcmp (vertex->name, name) == 0)  break ;
        } else {
            if (graph->compare (vertex->name, name) == 0)  break ;
        }
        prev = vertex ;
    }

/* If a vertex of the desired name was not found in the list, then add a new
   entry for that name. */

    if (vertex == NULL) {

        vertex = (Vertex *) malloc (sizeof (Vertex)) ;
        if (vertex == NULL) {
            LGE "(gscLocate) Error allocating vertex for \"%s\".\nmalloc: ",
                gscDisplay (graph, name)) ;
            return (NULL) ;
        }

        vertex->name = (graph->duplicate == NULL) ? strdup (name)
                                                  : graph->duplicate (name) ;
        if (vertex->name == NULL) {
            LGE "(gscLocate) Error duplicating vertex name: \"%s\".\n",
                gscDisplay (graph, name)) ;
            free ((char *) vertex) ;
            return (NULL) ;
        }
        vertex->calls = NULL ;
        vertex->isCalled = false ;
        vertex->next = NULL ;

        if (prev == NULL)			/* First in list? */
            graph->vertexList = vertex ;
        else					/* Add to end of list. */
            prev->next = vertex ;

        LGI "(gscLocate) Added vertex \"%s\" to graph %p.\n",
            gscDisplay (graph, name), (void *) graph) ;

    }

    return (vertex) ;

}

/*!*****************************************************************************

Procedure:

    gscMarkBFS ()

    Perform a Breadth-First Traversal of a Graph.


Purpose:

    Function gscMarkBFS() performs a breadth-first traversal of a graph.
    Since a breadth-first search of a graph cannot detect cycles in the
    graph, a depth-first traversal is initially performed to detect and
    flag recursively called vertices.  The breadth-first traversal of the
    graph is based upon the following, iterative algorithm:

            PROC bfs (root)
                ClearQ ()
                AddQ (root)
                visited[root] = true
                WHILE NOT IsEmptyQ () DO
                    X = DeleteQ ()
                    FOR each vertex Y adjacent to X DO
                        IF NOT visited[Y] THEN
                            visited[Y] = true
                            AddQ (Y)
                        ENDIF
                    ENDDO
                ENDDO
            ENDPROC


    Invocation:

        status = gscMarkBFS (graph, vertex, depth) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <vertex>	- I
            is a pointer to the root vertex of the graph being searched.
        <depth>		- I
            is the current search depth, 0..N.
        <status>	- O
            returns the status of traversing and marking the graph, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/

/*PDL----------------------------PDL--------------------------PDL**

CALL gscMarkDFS() to perform an initial depth-first traversal that will
    detect and mark cycles in the graph.

Reinitialize the traversal-related fields in the vertex structures.

Mark the root vertex as "visited" and set its depth to 0.
Add the root vertex to the BFS queue.

DOWHILE there are more vertices in the BFS queue
    Link the last vertex processed to the vertex at the front of the queue.
        This link will be used by gscFirst() and gscNext() to step through
        the graph.
    Remove the vertex from the front of the queue.
    DOFOR each vertex adjacent to the that vertex
        IF the adjacent vertex has already been visited THEN
            Mark the adjacency list entry as PREVIOUS_VISIT
                (unless it's already marked as RECURSIVE_VISIT).
        ELSE
            Mark the adjacency list entry as FIRST_VISIT.
            Mark the adjacent vertex as visited.
            Set its depth to that of the current vertex plus one.
            Add the adjacent vertex to the rear of the BFS queue.
        ENDIF
    ENDDO
ENDDO

RETURN

**PDL----------------------------PDL--------------------------PDL*/


static  errno_t  gscMarkBFS (

#    if PROTOTYPES
        Graph  graph,
        Vertex  *root,
        int  depth)
#    else
        graph, root, depth)

        Graph  graph ;
        Vertex  *root ;
        int  depth ;
#    endif

{    /* Local variables. */
    CallEntry  *adj ;
    Vertex  *bfsQueue, *vertex ;

/*******************************************************************************
    Macro to add a vertex to the BFS queue.
*******************************************************************************/

#define  ADDQ(vertex)						\
  { if (bfsQueue == NULL) {	/* Only item in queue? */	\
        bfsQueue = vertex ;  (vertex)->link = vertex ;		\
    } else {			/* Append to queue. */		\
        (vertex)->link = bfsQueue->link ;			\
        bfsQueue->link = vertex ;				\
        bfsQueue = vertex ;					\
    }								\
  }

/*******************************************************************************
    Macro to delete a vertex from the BFS queue.
*******************************************************************************/

#define  DELETEQ(vertex)					\
  { vertex = bfsQueue->link ;					\
    if (bfsQueue == bfsQueue->link)				\
        bfsQueue = NULL ;	/* Queue is now empty. */	\
    else							\
        bfsQueue->link = vertex->link ;			\
    vertex->link = NULL ;	/* Erase queue link. */		\
  }

/*******************************************************************************
    Macro to examine the vertex at the front of the BFS queue.
*******************************************************************************/

#define  EXAMINEQ						\
    ((bfsQueue == NULL) ? NULL : bfsQueue->link)




/* Before beginning the breadth-first traversal, perform an initial
   depth-first traversal that will detect and mark cycles in the graph.
   The cycle will be marked by a RECURSIVE visit flag in the appropriate
   adjacency list. */

    if (gscMarkDFS (graph, graph->root, 0)) {
        LGE "(gscMarkBFS) Error performing depth-first traversal to detect cycles.\ngscMarkDFS: ") ;
        return (errno) ;
    }


/* Reinitialize some of the fields set by the depth-first traversal.
   NOTE that the "visit" fields in the adjacency list entries should
   not be touched; the BFS traversal algorithm will be looking for
   RECURSIVE visits flagged by the DFS traversal. */

    for (vertex = graph->vertexList ;  vertex != NULL ;
         vertex = vertex->next) {
        vertex->wasVisited = false ;  vertex->isLocked = false ;
        vertex->link = NULL ;  vertex->depth = 0 ;
    }


/*******************************************************************************
    Beginning at the root vertex, perform the breadth-first traversal.
*******************************************************************************/

    root->wasVisited = true ;  root->depth = 0 ;
    bfsQueue = NULL ;  ADDQ (root) ;
    vertex = NULL ;

/* While there are more vertices to visit, get the next vertex from the BFS
   queue (which initially contains the root vertex).  In a BFS traversal,
   the vertices adjacent to the new vertex will follow those adjacent to
   the previously processed vertex; therefore, link the previous vertex to
   the new vertex.  Then, scan the new vertex's adjacency list.  Adjacent
   vertices being visited for the first time are added to the BFS queue for
   later processing. */

    while (bfsQueue != NULL) {

        if (vertex != NULL)		/* Link previous vertex to next. */
            vertex->link = EXAMINEQ ;

        DELETEQ (vertex) ;		/* Get next vertex. */

        LGI "(gscMarkBFS) Visiting \"%s\" at depth %d.\n",
            gscDisplay (graph, vertex->name), vertex->depth) ;

			/* Add adjacent, unvisited nodes to the BFS queue. */
        for (adj = vertex->calls ;  adj != NULL ;  adj = adj->next) {
            if ((adj->vertex)->wasVisited) {
                if (adj->visit != GscRECURSIVE)  adj->visit = GscPREVIOUS ;
            } else {
                adj->visit = GscFIRST ;
                (adj->vertex)->wasVisited = true ;
                (adj->vertex)->depth = vertex->depth + 1 ;
                ADDQ (adj->vertex) ;
            }
        }     /* For each adjacent vertex */

    }     /* While there are more vertices in the BFS queue */


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscMarkDFS ()

    Perform a Depth-First Traversal of a Subgraph.


Purpose:

    Function gscMarkDFS() performs a depth-first traversal of a subgraph.
    The traversal is based on the following, recursive algorithm:

            PROC dfs (X)
                IF visited[X]  RETURN
                visited[X] = true
                FOR each vertex Y adjacent to X DO
                    CALL dfs (Y)
                ENDDO
            ENDPROC


    Invocation:

        status = gscMarkDFS (graph, vertex, depth) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <vertex>	- I
            is a pointer to the root vertex of the subgraph being searched.
        <depth>		- I
            is the current search depth, 0..N.
        <status>	- O
            returns the status of traversing and marking the subgraph,
            zero if no errors occurred and ERRNO otherwise.

*******************************************************************************/

/*PDL----------------------------PDL--------------------------PDL**

Mark the vertex as VISITED and LOCKED.
DOFOR each entry in the vertex's adjacency list
    IF the adjacent vertex is locked THEN
        Mark the adjacency list entry as RECURSIVE_CALL.
    ELSEIF the adjacent vertex has already been visited THEN
        Mark the adjacency list entry as PREVIOUSLY_EXPANDED.
    ELSE
        Mark the adjacency list entry as FIRST_VISIT.
        CALL gscMarkDFS() to traverse the subgraph rooted at the adjacent
            vertex.
    ENDIF
ENDDO
Mark the vertex as UNLOCKED.

RETURN

**PDL----------------------------PDL--------------------------PDL*/


static  errno_t  gscMarkDFS (

#    if PROTOTYPES
        Graph  graph,
        Vertex  *root,
        int  depth)
#    else
        graph, root, depth)

        Graph  graph ;
        Vertex  *root ;
        int  depth ;
#    endif

{    /* Local variables. */
    CallEntry  *adj ;




    if (graph == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(gscMarkDFS) The graph hasn't been created yet.\n") ;
        return (errno) ;
    }
    if (root == NULL)  return (0) ;

    LGI "(gscMarkDFS) Visiting \"%s\" at depth %d.\n",
        gscDisplay (graph, root->name), depth) ;

/* Mark the current vertex as "visited" (so its subgraph won't be traversed
   on subsequent visits) and "locked" (so that cycles in the graph can be
   detected).  Also, tag the vertex with the current depth; i.e., the depth
   of the first visit to this vertex. */

    root->wasVisited = true ;
    root->isLocked = true ;
    root->depth = depth ;

/* Scan the list of vertices adjacent to the current vertex.  For each entry
   in the list, recursively call gscMarkDFS() to traverse the subgraph rooted
   at the adjacent vertex.  Don't revisit adjacent vertices that are locked
   (indicating a cycle in the graph) or that have already been visited. */

    for (adj = root->calls ;  adj != NULL ;  adj = adj->next) {
        if ((adj->vertex)->isLocked)
            adj->visit = GscRECURSIVE ;
        else if ((adj->vertex)->wasVisited)
            adj->visit = GscPREVIOUS ;
        else {
            adj->visit = GscFIRST ;
            (adj->vertex)->parent = root ;
            gscMarkDFS (graph, adj->vertex, depth+1) ;
        }
    }

/* All done!  Unlock the vertex, thus removing it from the current search
   path. */

    root->isLocked = false ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscNextBFS ()

    Get Name of Next Vertex in Breadth-First Graph Traversal.


Purpose:

    Function gscNextBFS() returns the name of the next vertex visited during
    a breadth-first graph traversal.  gscNextBFS() is an internal function
    called by gscNext().


    Invocation:

        status = gscNextBFS (graph, &name, &depth, &visit) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <name>		- O
            returns the "name" of the next vertex visited during the graph
            traversal.  Vertex "names" are normally C strings, although other
            types can be used; see gscCreate().  NULL is returned when the
            traversal is complete and there are no more vertices to visit.
        <depth>		- O
            returns the depth (0..N) in the graph of this visit to the vertex.
            The root vertex is at depth 0; its adjacent vertices are at depth
            1, and so on.
        <visit>		- O
            returns the status of the visit to this vertex:
                GscFIRST - This is the first visit to the vertex.
                GscPREVIOUS - This vertex was visited previously during
                    the traversal.
                GscRECURSIVE - This is a recursive visit to the vertex;
                    i.e., there is a cycle in the graph.
            On the first visit to a vertex, the VISIT argument returns
            GscFIRST and the vertex's subgraph is or will be traversed.
            On subsequent visits to the same vertex, the VISIT argument
            will return GscPREVIOUS (or GscRECURSIVE!) and the vertex's
            subgraph will not be traversed again.
        <status>	- O
            returns zero if no errors occurred and ERRNO otherwise.  When
            the traversal is complete and there are no more vertices to visit,
            STATUS returns zero and NAME returns NULL.

*******************************************************************************/

/*PDL----------------------------PDL--------------------------PDL**

Step to the next vertex in the current vertex's adjacency list.
DOWHILE positioned at the end of the current vertex's adjacency list
    IF the current vertex is the root of the graph THEN
        RETURN a null name to the calling routine.
    ENDIF
    Make the parent of the current vertex the new current vertex.
    Locate the child vertex's call entry in the parent's adjacency list.
    Step to the next entry in the parent's adjacency list.
ENDDO

DOCASE adjacency list entry's visit status
CASE first visit:  Set the last vertex pointers in the graph header to point
    to the adjacent vertex.
CASE previously-visited:  Set the last vertex pointers in the graph header to
    point to this adjacency list entry.
CASE recursively-visited:  Set the last vertex pointers in the graph header
    to point to this adjacency list entry.
ENDDO

RETURN the name of the vertex designated by the adjacency list entry.

**PDL----------------------------PDL--------------------------PDL*/


static  errno_t  gscNextBFS (

#    if PROTOTYPES
        Graph  graph,
        char  **name,
        int  *depth,
        GscVisitStatus  *visit)
#    else
        graph, name, depth, visit)

        Graph  graph ;
        char  **name ;
        int  *depth ;
        GscVisitStatus  *visit ;
#    endif

{    /* Local variables. */
    CallEntry  *adj ;
    Vertex  *vertex ;




    *name = NULL ;  *depth = 0 ;  *visit = GscFIRST ;

/* Step to the next vertex in the traversal.  Normally, this is the next
   vertex adjacent to the current vertex (i.e., the next call entry in
   the current vertex's adjacency list) - try that first. */

    vertex = graph->lastVertex ;  adj = graph->lastCall ;
    if (adj == NULL)
        adj = vertex->calls ;	/* Advance to first entry in adjacency list. */
    else
        adj = adj->next ;	/* Advance to next entry in adjacency list. */

/* If there are no more vertices adjacent to the current vertex (i.e., we're
   at the end of the adjacency list), then step forward to the next non-empty
   adjacency list that follows the current list in the traversal. */

    while (adj == NULL) {
        vertex = vertex->link ;
        if (vertex == NULL)  return (0) ;	/* End of traversal. */
        adj = vertex->calls ;
    }

/* Now we're at the call entry (in its parent's list) for the next vertex in
   the traversal. */

    *name = (adj->vertex)->name ;
    *depth = vertex->depth + 1 ;
    *visit = adj->visit ;

    graph->lastVertex = vertex ;  graph->lastCall = adj ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    gscNextDFS ()

    Get Name of Next Vertex in Depth-First Graph Traversal.


Purpose:

    Function gscNextDFS() returns the name of the next vertex visited during
    a depth-first graph traversal.  gscNextDFS() is an internal function
    called by gscNext().


    Invocation:

        status = gscNextDFS (graph, &name, &depth, &visit) ;

    where:

        <graph>		- I
            is the graph handle returned by gscCreate().
        <name>		- O
            returns the "name" of the next vertex visited during the graph
            traversal.  Vertex "names" are normally C strings, although other
            types can be used; see gscCreate().  NULL is returned when the
            traversal is complete and there are no more vertices to visit.
        <depth>		- O
            returns the depth (0..N) in the graph of this visit to the vertex.
            The root vertex is at depth 0; its adjacent vertices are at depth
            1, and so on.
        <visit>		- O
            returns the status of the visit to this vertex:
                GscFIRST - This is the first visit to the vertex.
                GscPREVIOUS - This vertex was visited previously during
                    the traversal.
                GscRECURSIVE - This is a recursive visit to the vertex;
                    i.e., there is a cycle in the graph.
            On the first visit to a vertex, the VISIT argument returns
            GscFIRST and the vertex's subgraph is or will be traversed.
            On subsequent visits to the same vertex, the VISIT argument
            will return GscPREVIOUS (or GscRECURSIVE!) and the vertex's
            subgraph will not be traversed again.
        <status>	- O
            returns zero if no errors occurred and ERRNO otherwise.  When
            the traversal is complete and there are no more vertices to visit,
            STATUS returns zero and NAME returns NULL.

*******************************************************************************/

/*PDL----------------------------PDL--------------------------PDL**

Step to the next vertex in the current vertex's adjacency list.
DOWHILE positioned at the end of the current vertex's adjacency list
    IF the current vertex is the root of the graph THEN
        RETURN a null name to the calling routine.
    ENDIF
    Make the parent of the current vertex the new current vertex.
    Locate the child vertex's call entry in the parent's adjacency list.
    Step to the next entry in the parent's adjacency list.
ENDDO

DOCASE adjacency list entry's visit status
CASE first visit:  Set the last vertex pointers in the graph header to point
    to the adjacent vertex.
CASE previously-visited:  Set the last vertex pointers in the graph header to
    point to this adjacency list entry.
CASE recursively-visited:  Set the last vertex pointers in the graph header
    to point to this adjacency list entry.
ENDDO

RETURN the name of the vertex designated by the adjacency list entry.

**PDL----------------------------PDL--------------------------PDL*/


static  errno_t  gscNextDFS (

#    if PROTOTYPES
        Graph  graph,
        char  **name,
        int  *depth,
        GscVisitStatus  *visit)
#    else
        graph, name, depth, visit)

        Graph  graph ;
        char  **name ;
        int  *depth ;
        GscVisitStatus  *visit ;
#    endif

{    /* Local variables. */
    CallEntry  *adj ;
    Vertex  *child, *vertex ;




    *name = NULL ;  *depth = 0 ;  *visit = GscFIRST ;

/* Step to the next vertex in the traversal.  Normally, this is the next
   vertex adjacent to the current vertex (i.e., the next call entry in
   the current vertex's adjacency list) - try that first. */

    vertex = graph->lastVertex ;  adj = graph->lastCall ;
    if (adj == NULL)
        adj = vertex->calls ;	/* Advance to first entry in adjacency list. */
    else
        adj = adj->next ;	/* Advance to next entry in adjacency list. */

/* If there are no more vertices adjacent to the current vertex (i.e., we're
   at the end of the adjacency list), then "return" to the next higher level
   of the traversal.  In other words, return to the parent of the current
   vertex and advance to the next vertex adjacent to the parent.  This is
   done in a loop, since the parent (and its parent, etc.) might be at the
   end of its adjacency list. */

    while (adj == NULL) {
        child = vertex ;  vertex = child->parent ;
        if (vertex == NULL)  break ;
			/* Locate current vertex in parent's adjacency list. */
        for (adj = vertex->calls ;  adj != NULL ;  adj = adj->next)
            if (adj->vertex == child)  break ;
        adj = adj->next ;			/* Is this dangerous? */
    }

    if (vertex == NULL)  return (0) ;		/* End of traversal. */

/* Now we're at the call entry (in its parent's list) for the next vertex in
   the traversal.  If this is the traversal's first visit to the next vertex,
   then "descend" to that vertex; the next vertex's subgraph will be traversed
   by subsequent calls to gscNext().  If the next vertex has already been
   visited (because the vertex has multiple parents or is part of a cycle),
   its subgraph will not be traversed. */

    *name = (adj->vertex)->name ;
    *depth = vertex->depth + 1 ;
    *visit = adj->visit ;

    switch (*visit) {
    case GscFIRST:				/* Descend to new level. */
        graph->lastVertex = adj->vertex ;  graph->lastCall = NULL ;
        break ;
    case GscPREVIOUS:
    case GscRECURSIVE:				/* Already visited, one way or another. */
        graph->lastVertex = vertex ;  graph->lastCall = adj ;
        break ;
    default:
        graph->lastVertex = NULL ;  graph->lastCall = NULL ;
        break ;
    }

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the GSC_UTIL functions.

    Under VMS,
        compile and link as follows:
            $ CC/DEFINE=TEST/INCLUDE=?? gsc_util.c
            $ LINK gsc_util, <libraries>
        invoke with one of the following command lines:
            $ gsc_util		! For a DFS traversal.
            $ gsc_util 1	! For a BFS traversal.

*******************************************************************************/

main (argc, argv)

    int  argc ;
    char  *argv[] ;

{    /* Local variables. */
    char  inbuf[128], *name1, *name2, *tag ;
    int  bfs, depth, i, visit ;
    Graph  graph ;
    static  char  *test_lines[] = {
        "A -> B",
        "A -> C",
        "A -> D",
        "C -> E",
        "E -> F",
        "E -> G",
        "G -> C		C is recursively called.",
        "AA -> BB",
        "AA -> CC",
        "BB -> DD",
        "BB -> EE	EE is called by BB and CC.",
        "CC -> EE",
        "CC -> FF",
        "EE -> GG",
        "EE -> HH",
        NULL
    } ;




    gsc_util_debug = 1 ;

    bfs = (argc > 1) ? atoi (argv[1]) : 0 ;

    if (gscCreate (NULL, NULL, NULL, NULL, &graph))  exit (errno) ;

    for (i = 0 ;  test_lines[i] != NULL ;  i++) {
        printf ("Test Line: \"%s\"\n", test_lines[i]) ;
        name1 = strtok (test_lines[i], " \t->") ;
        name2 = strtok (NULL, " \t->") ;
        if (gscAdd (graph, name1, name2))  break ;
    }

    for (i = 1 ;  ;  i++) {
        name1 = gscRoot (graph, i) ;
        if (name1 == NULL)  break ;
        printf ("\nMarking the graph at root %s...\n", name1) ;
        gscMark (graph, name1, bfs) ;
        gscDump (NULL, "\nDump of Graph:\n", graph) ;
        printf ("\nTraversing the graph ...\n") ;
        gscFirst (graph, &name2, &depth, &visit) ;
        while (name2 != NULL) {
            switch (visit) {
            case GscFIRST:  tag = "" ;  break ;
            case GscPREVIOUS:  tag = " *" ;  break ;
            case GscRECURSIVE:  tag = " is recursively called." ;  break ;
            default:
                tag = " illegal tag" ;  break ;
            }
            if (depth > 0)
                printf ("%*s%s%s\n", depth*4, " ", name2, tag) ;
            else
                printf ("%s%s\n", name2, tag) ;
            gscNext (graph, &name2, &depth, &visit) ;
        }
    }

    gscDelete (graph) ;

}

#endif  /* TEST */
