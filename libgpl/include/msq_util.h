/* $Id: msq_util.h,v 1.7 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    msq_util.h

    Message Queue Utility Definitions.

*******************************************************************************/

#ifndef  MSQ_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  MSQ_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Message Queue (Client View).
*******************************************************************************/

typedef  struct  _MessageQueue  *MessageQueue ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  msq_util_debug ;		/* Global debug switch (1/0 = yes/no). */
extern  int  msq_max_messages ;		/* Maximum # of messages in queue. */
extern  int  msq_max_length ;		/* Maximum length of each message in queue. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  msqClose P_((MessageQueue queue)) ;

extern  int  msqId P_((MessageQueue queue)) ;

extern  int  msqOpen P_((const char *name,
                         MessageQueue *queue)) ;

extern  int  msqPoll P_((MessageQueue queue)) ;

extern  int  msqRead P_((MessageQueue queue,
                         double timeout,
                         int maxLength,
                         char *message,
                         int *length)) ;

extern  int  msqWrite P_((MessageQueue queue,
                          double timeout,
                          int length,
                          const char *message)) ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
