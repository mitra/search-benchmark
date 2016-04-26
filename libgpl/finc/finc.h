/* $Id: finc.h,v 1.1 2007/04/30 19:31:44 alex Exp alex $ */
/*******************************************************************************

    finc.h

    Forth-Inspired Network Commands (FINC).

*******************************************************************************/

#ifndef  FINC_H			/* Has the file been INCLUDE'd already? */
#define  FINC_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <ficl.h>			/* Forth-Inspired Command Language. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  void  buildWordsDRS P_((ficlSystem *sys)) ;

extern  void  buildWordsLFN P_((ficlSystem *sys)) ;

extern  void  buildWordsMISC P_((ficlSystem *sys)) ;

extern  void  buildWordsNET P_((ficlSystem *sys)) ;

extern  void  buildWordsIOX P_((ficlSystem *sys)) ;

extern  void  buildWordsSKT P_((ficlSystem *sys)) ;

extern  void  buildWordsTCP P_((ficlSystem *sys)) ;

extern  void  buildWordsTV P_((ficlSystem *sys)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
