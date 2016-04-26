/* $Id: gsc_util.h,v 1.8 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    gsc_util.h

    Graph/Structure Chart Utility definitions.

*******************************************************************************/

#ifndef  GSC_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  GSC_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Graph (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _Graph  *Graph ;	/* Graph handle. */

typedef  enum  GscVisitStatus {
    GscFIRST,				/* First visit to this vertex. */
    GscPREVIOUS,			/* This vertex has already been visited. */
    GscRECURSIVE			/* This vertex is part of a cycle in the graph. */
}  GscVisitStatus ;

					/* Utility function prototypes. */
typedef  int  (*GscCompareF) P_((const char *, const char *)) ;
typedef  char  *(*GscDuplicateF) P_((const char *)) ;
typedef  void  (*GscDeleteF) P_((void *)) ;
typedef  char  *(*GscDisplayF) P_((const char *)) ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  gsc_util_debug ;		/* Global debug switch (1/0 = yes/no). */


/*******************************************************************************
    Public Functions
*******************************************************************************/

extern  errno_t  gscAdd P_((Graph graph,
                            const char *vertex_1,
                            const char *vertex_2)) ;

extern  errno_t  gscCreate P_((GscCompareF compare,
                               GscDuplicateF duplicate,
                               GscDeleteF delete,
                               GscDisplayF display,
                               Graph *graph)) ;

extern  errno_t  gscDelete P_((Graph graph)) ;

extern  errno_t  gscDump P_((FILE *file,
                             const char *header,
                             Graph graph)) ;

extern  errno_t  gscFirst P_((Graph graph,
                              char **name,
                              int *depth,
                              GscVisitStatus *visit)) ;

extern  errno_t  gscMark P_((Graph graph,
                             const char *root,
                             bool bfs)) ;

extern  errno_t  gscNext P_((Graph graph,
                             char **name,
                             int *depth,
                             GscVisitStatus *visit)) ;

extern  char  *gscRoot P_((Graph graph,
                           int which)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
