/* $Id: opt_util.h,v 1.10 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    opt_util.h

    Command Line Option Processing Definitions.

*******************************************************************************/

#ifndef  OPT_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  OPT_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Option Scan (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _OptContext  *OptContext ;	/* Option scan context. */

/* Values returned by OPT_GET(). */

#define  OPTEND  0			/* End of command line. */
#define  OPTERR  -1			/* Invalid option or missing argument. */
#define  NONOPT  -2			/* Non-option argument. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  opt_util_debug  OCD ("opt_util") ;

/*******************************************************************************
    Function prototypes and external definitions for OPT_UTIL functions.
*******************************************************************************/

extern  int  opt_create_argv P_((const char *program,
                                 const char *command,
                                 int *argc,
                                 char *(*argv[])))
    OCD ("opt_util") ;

extern  int  opt_delete_argv P_((int argc,
                                 char *argv[]))
    OCD ("opt_util") ;

extern  int  opt_errors P_((OptContext scan,
                            bool display_flag))
    OCD ("opt_util") ;

extern  int  opt_get P_((OptContext scan,
                         char **argument))
    OCD ("opt_util") ;

extern  int  opt_index P_((OptContext scan))
    OCD ("opt_util") ;

extern  int  opt_init P_((int argc,
                          char *argv[],
                          const char *optionString,
                          const char *optionList[],
                          OptContext *scan))
    OCD ("opt_util") ;

extern  const  char  *opt_name P_((OptContext scan,
                                   int index))
    OCD ("opt_util") ;

extern  int  opt_reset P_((OptContext scan,
                           int argc,
                           char *argv[]))
    OCD ("opt_util") ;

extern  int  opt_set P_((OptContext scan,
                         int new_index))
    OCD ("opt_util") ;

extern  int  opt_term P_((OptContext scan))
    OCD ("opt_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
