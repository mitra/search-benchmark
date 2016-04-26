# Microsoft Developer Studio Project File - Name="libgpl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libgpl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "libgpl.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "libgpl.mak" CFG="libgpl - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "libgpl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libgpl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libgpl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\obj\libgpl"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp8 /W3 /Gi /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libgpl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\obj\libgpl"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /TC /D /I /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF

# Begin Target

# Name "libgpl - Win32 Release"
# Name "libgpl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aperror.c
# End Source File
# Begin Source File

SOURCE=.\bio_util.c
# End Source File
# Begin Source File

SOURCE=.\bit_util.c
# End Source File
# Begin Source File

SOURCE=.\bmw_util.c
# End Source File
# Begin Source File

SOURCE=.\coli_util.c
# End Source File
# Begin Source File

SOURCE=.\comx_util.c
# End Source File
# Begin Source File

SOURCE=.\cosn_util.c
# End Source File
# Begin Source File

SOURCE=.\crlf_util.c
# End Source File
# Begin Source File

SOURCE=.\damx_idl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\damx_util.c
# End Source File
# Begin Source File

SOURCE=.\ddmx_idl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ddmx_util.c
# End Source File
# Begin Source File

SOURCE=.\drs_util.c
# End Source File
# Begin Source File

SOURCE=.\f1750a_util.c
# End Source File
# Begin Source File

SOURCE=.\ffs.c
# End Source File
# Begin Source File

SOURCE=.\fnm_util.c
# End Source File
# Begin Source File

SOURCE=.\get_util.c
# End Source File
# Begin Source File

SOURCE=.\getopt.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\gimx_idl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\gimx_util.c
# End Source File
# Begin Source File

SOURCE=.\gsc_util.c
# End Source File
# Begin Source File

SOURCE=.\hash_util.c
# End Source File
# Begin Source File

SOURCE=.\id3_util.c
# End Source File
# Begin Source File

SOURCE=.\ieee_util.c
# End Source File
# Begin Source File

SOURCE=.\iiop_util.c
# End Source File
# Begin Source File

SOURCE=.\iox_util.c
# End Source File
# Begin Source File

SOURCE=.\lemx_idl.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\lemx_util.c
# End Source File
# Begin Source File

SOURCE=.\lfn_util.c
# End Source File
# Begin Source File

SOURCE=.\list_util.c
# End Source File
# Begin Source File

SOURCE=.\log_util.c
# End Source File
# Begin Source File

SOURCE=.\meo_util.c
# End Source File
# Begin Source File

SOURCE=.\net_util.c
# End Source File
# Begin Source File

SOURCE=.\nft_proc.c
# End Source File
# Begin Source File

SOURCE=.\nft_util.c
# End Source File
# Begin Source File

SOURCE=.\nvl_util.c
# End Source File
# Begin Source File

SOURCE=.\nvp_util.c
# End Source File
# Begin Source File

SOURCE=.\opt_util.c
# End Source File
# Begin Source File

SOURCE=.\port_util.c
# End Source File
# Begin Source File

SOURCE=.\rex_util.c
# End Source File
# Begin Source File

SOURCE=.\rex_util_y.c
# End Source File
# Begin Source File

SOURCE=.\skt_util.c
# End Source File
# Begin Source File

SOURCE=.\str_util.c
# End Source File
# Begin Source File

SOURCE=.\tcp_util.c
# End Source File
# Begin Source File

SOURCE=.\tpl_util.c
# End Source File
# Begin Source File

SOURCE=.\ts_util.c
# End Source File
# Begin Source File

SOURCE=.\tv_util.c
# End Source File
# Begin Source File

SOURCE=.\udp_util.c
# End Source File
# Begin Source File

SOURCE=.\utf_util.c
# End Source File
# Begin Source File

SOURCE=.\vim_util.c
# End Source File
# Begin Source File

SOURCE=.\wcs_util.c
# End Source File
# Begin Source File

SOURCE=.\xnet_util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\aperror.h
# End Source File
# Begin Source File

SOURCE=..\include\bio_util.h
# End Source File
# Begin Source File

SOURCE=..\include\bit_util.h
# End Source File
# Begin Source File

SOURCE=..\include\bmw_util.h
# End Source File
# Begin Source File

SOURCE=..\include\coli_util.h
# End Source File
# Begin Source File

SOURCE=..\include\comx_util.h
# End Source File
# Begin Source File

SOURCE=..\include\cosn_util.h
# End Source File
# Begin Source File

SOURCE=..\include\crlf_util.h
# End Source File
# Begin Source File

SOURCE=..\include\damx_idl.h
# End Source File
# Begin Source File

SOURCE=..\include\damx_util.h
# End Source File
# Begin Source File

SOURCE=..\include\ddmx_idl.h
# End Source File
# Begin Source File

SOURCE=..\include\ddmx_util.h
# End Source File
# Begin Source File

SOURCE=..\include\drs_util.h
# End Source File
# Begin Source File

SOURCE=..\include\f1750a_util.h
# End Source File
# Begin Source File

SOURCE=..\include\ftw_util.h
# End Source File
# Begin Source File

SOURCE=..\include\get_util.h
# End Source File
# Begin Source File

SOURCE=..\include\gimx_idl.h
# End Source File
# Begin Source File

SOURCE=..\include\gimx_util.h
# End Source File
# Begin Source File

SOURCE=..\include\gsc_util.h
# End Source File
# Begin Source File

SOURCE=..\include\hash_util.h
# End Source File
# Begin Source File

SOURCE=..\include\id3_util.h
# End Source File
# Begin Source File

SOURCE=..\include\ieee_util.h
# End Source File
# Begin Source File

SOURCE=..\include\iiop_util.h
# End Source File
# Begin Source File

SOURCE=..\include\iox_util.h
# End Source File
# Begin Source File

SOURCE=..\include\lemx_idl.h
# End Source File
# Begin Source File

SOURCE=..\include\lemx_util.h
# End Source File
# Begin Source File

SOURCE=..\include\lfn_util.h
# End Source File
# Begin Source File

SOURCE=..\include\list_util.h
# End Source File
# Begin Source File

SOURCE=..\include\log_util.h
# End Source File
# Begin Source File

SOURCE=..\include\meo_util.h
# End Source File
# Begin Source File

SOURCE=..\include\net_util.h
# End Source File
# Begin Source File

SOURCE=.\nft_proc.h
# End Source File
# Begin Source File

SOURCE=..\include\nft_util.h
# End Source File
# Begin Source File

SOURCE=..\include\nvl_util.h
# End Source File
# Begin Source File

SOURCE=..\include\nvp_util.h
# End Source File
# Begin Source File

SOURCE=..\include\opt_util.h
# End Source File
# Begin Source File

SOURCE=..\include\port_util.h
# End Source File
# Begin Source File

SOURCE=..\include\pragmatics.h
# End Source File
# Begin Source File

SOURCE=.\rex_internals.h
# End Source File
# Begin Source File

SOURCE=..\include\skt_util.h
# End Source File
# Begin Source File

SOURCE=..\include\str_util.h
# End Source File
# Begin Source File

SOURCE=..\include\tcp_util.h
# End Source File
# Begin Source File

SOURCE=..\include\tpl_util.h
# End Source File
# Begin Source File

SOURCE=..\include\ts_util.h
# End Source File
# Begin Source File

SOURCE=..\include\tv_util.h
# End Source File
# Begin Source File

SOURCE=..\include\udp_util.h
# End Source File
# Begin Source File

SOURCE=..\include\utf_util.h
# End Source File
# Begin Source File

SOURCE=..\include\vim_util.h
# End Source File
# Begin Source File

SOURCE=..\include\wcs_util.h
# End Source File
# Begin Source File

SOURCE=..\include\xnet_util.h
# End Source File
# End Group
# End Target
# End Project
