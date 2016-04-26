/* $Id: fnm_util.h,v 1.11 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    fnm_util.h

    Filename Utility Definitions.

*******************************************************************************/

#ifndef  FNM_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  FNM_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Filename (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _FileName  *FileName ;	/* Filename handle. */

typedef  enum  FnmPart {
    FnmPath = 0,			/* The whole pathname. */
    FnmNode,				/* Node name, if specified. */
    FnmDirectory,			/* Directory only. */
    FnmFile,				/* Name, extension, and version. */
    FnmName,				/* File name. */
    FnmExtension,			/* File extension. */
    FnmVersion				/* Version number. */
}  FnmPart ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  const  char  *fnmBuild P_((FnmPart part,
                                   const char *path,
                                   ...))
    OCD ("fnm_util") ;

extern  FileName  fnmCreate P_((const char *path,
                                ...))
    OCD ("fnm_util") ;

extern  errno_t  fnmDestroy P_((FileName filename))
    OCD ("fnm_util") ;

extern  bool  fnmExists P_((const FileName filename))
    OCD ("fnm_util") ;

extern  const  char  *fnmFind P_((const char *path,
                                  ...))
    OCD ("fnm_util") ;

extern  const  char  *fnmParse P_((const FileName fname,
                                   FnmPart part))
    OCD ("fnm_util") ;

#define  fnmPath(fname)  fnmParse (fname, FnmPath)
#define  fnmNode(fname)  fnmParse (fname, FnmNode)
#define  fnmDirectory(fname)  fnmParse (fname, FnmDirectory)
#define  fnmFile(fname)  fnmParse (fname, FnmFile)
#define  fnmName(fname)  fnmParse (fname, FnmName)
#define  fnmExtension(fname)  fnmParse (fname, FnmExtension)
#define  fnmVersion(fname)  fnmParse (fname, FnmVersion)


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
