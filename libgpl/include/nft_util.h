/* $Id: nft_util.h,v 1.9 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    nft_util.h

    FTP Utility Definitions.

*******************************************************************************/

#ifndef  NFT_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  NFT_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "tcp_util.h"			/* TCP/IP networking utilities. */


/*******************************************************************************
    NFT Structures (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _NftSession  *NftSession ;	/* Session handle. */

typedef  errno_t  (NftCommandProc) P_((NftSession session,
                                       const char *command,
                                       char *arguments,
                                       void *userData)) ;

typedef  struct  NftSessionInfo {
    void  *userData ;			/* Arbitrary pointer passed to command callbacks. */
    char  *userName ;			/* User's name. */
    char  *currentDirectory ;		/* Pathname of current directory. */
    double  timeout ;			/* Idle timeout in seconds. */
    char  *dataPortName ;		/* "<server>@<host>" name for data port. */
    char  representation[2] ;		/* [0]=A|E|I, [1]=N|T|C */
    int  logout ;			/* Logout when transfer completes? */
    char  *oldPathname ;		/* Pathname from RNFR rename operation. */
}  NftSessionInfo ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  nft_util_debug  OCD ("nft_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  nftClose P_((NftSession session))
    OCD ("nft_util") ;

extern  errno_t  nftCreate P_((TcpEndpoint controlPoint,
                               const char *command[],
                               NftCommandProc *callback[],
                               void  *userData,
                               NftSession *session))
    OCD ("nft_util") ;

extern  errno_t  nftDestroy P_((NftSession session))
    OCD ("nft_util") ;

extern  errno_t  nftEvaluate P_((NftSession session,
                                 const char *command))
    OCD ("nft_util") ;

extern  IoFd  nftFd P_((NftSession session,
                        int which))
    OCD ("nft_util") ;

extern  errno_t  nftGetLine P_((NftSession session,
                                char **string))
    OCD ("nft_util") ;

extern  errno_t  nftIgnoreCmd P_((NftSession session,
                                  const char *command,
                                  char *arguments,
                                  void *userData))
    OCD ("nft_util") ;

extern  NftSessionInfo  *nftInfo P_((NftSession session))
    OCD ("nft_util") ;

extern  bool  nftIsReadable P_((NftSession session,
                                int which))
    OCD ("nft_util") ;

extern  bool  nftIsUp P_((NftSession session,
                          int which))
    OCD ("nft_util") ;

extern  bool  nftIsWriteable P_((NftSession session,
                                 int which))
    OCD ("nft_util") ;

extern  const  char  *nftName P_((NftSession session,
                                  int which))
    OCD ("nft_util") ;

extern  errno_t  nftOpen P_((NftSession session))
    OCD ("nft_util") ;

extern  errno_t  nftPASV P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftPORT P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftPutLine P_((NftSession session,
                                const char *format,
                                ...))
    OCD ("nft_util") ;

extern  errno_t  nftQUIT P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftRead P_((NftSession session,
                             ssize_t numBytesToRead,
                             char *buffer,
                             size_t *numBytesRead))
    OCD ("nft_util") ;

extern  errno_t  nftRegister P_((NftSession session,
                                 const char *command,
                                 NftCommandProc *callback))
    OCD ("nft_util") ;

extern  const  char  *nftSyntax P_((NftSession session,
                                    const char *command))
    OCD ("nft_util") ;

extern  errno_t  nftWrite P_((NftSession session,
                              size_t numBytesToWrite,
                              const char *buffer,
                              size_t *numBytesWritten))
    OCD ("nft_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
