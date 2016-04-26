/* $Id: nft_proc.h,v 1.6 2011/07/18 17:44:47 alex Exp $ */
/*******************************************************************************

    nft_proc.h

    FTP Command Processing Definitions.

*******************************************************************************/

#ifndef  NFT_PROC_H		/* Has the file been INCLUDE'd already? */
#define  NFT_PROC_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  "nft_util.h"			/* FTP utilties. */


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  errno_t  nftAccessCmds P_((NftSession session,
                                   const char *command,
                                   char *arguments,
                                   void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftCWD P_((NftSession session,
                            const char *command,
                            char *arguments,
                            void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftFileCmds P_((NftSession session,
                                 const char *command,
                                 char *arguments,
                                 void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftHELP P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftListCmds P_((NftSession session,
                                 const char *command,
                                 char *arguments,
                                 void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftMODE P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftPASS P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftPORT P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftRETR P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftServiceCmds P_((NftSession session,
                                    const char *command,
                                    char *arguments,
                                    void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftSTAT P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftStoreCmds P_((NftSession session,
                                  const char *command,
                                  char *arguments,
                                  void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftSTRU P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftTYPE P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;

extern  errno_t  nftUSER P_((NftSession session,
                             const char *command,
                             char *arguments,
                             void *userData))
    OCD ("nft_util") ;


#endif				/* If this file was not INCLUDE'd previously. */
