# Microsoft Developer Studio Generated NMAKE File, Based on libgpl.dsp
!IF "$(CFG)" == ""
CFG=libgpl - Win32 Debug
!MESSAGE No configuration specified. Defaulting to libgpl - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "libgpl - Win32 Release" && "$(CFG)" != "libgpl - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

!IF  "$(CFG)" == "libgpl - Win32 Release"

OUTDIR=.\..\lib
INTDIR=.\..\obj\libgpl
# Begin Custom Macros
OutDir=.\..\lib
# End Custom Macros

ALL : "$(OUTDIR)\libgpl.lib"


CLEAN :
	-@erase "$(INTDIR)\aperror.obj"
	-@erase "$(INTDIR)\bio_util.obj"
	-@erase "$(INTDIR)\bit_util.obj"
	-@erase "$(INTDIR)\bmw_util.obj"
	-@erase "$(INTDIR)\coli_util.obj"
	-@erase "$(INTDIR)\comx_util.obj"
	-@erase "$(INTDIR)\cosn_util.obj"
	-@erase "$(INTDIR)\crlf_util.obj"
	-@erase "$(INTDIR)\damx_util.obj"
	-@erase "$(INTDIR)\ddmx_util.obj"
	-@erase "$(INTDIR)\drs_util.obj"
	-@erase "$(INTDIR)\f1750a_util.obj"
	-@erase "$(INTDIR)\ffs.obj"
	-@erase "$(INTDIR)\fnm_util.obj"
	-@erase "$(INTDIR)\get_util.obj"
	-@erase "$(INTDIR)\gimx_util.obj"
	-@erase "$(INTDIR)\gsc_util.obj"
	-@erase "$(INTDIR)\hash_util.obj"
	-@erase "$(INTDIR)\id3_util.obj"
	-@erase "$(INTDIR)\ieee_util.obj"
	-@erase "$(INTDIR)\iiop_util.obj"
	-@erase "$(INTDIR)\iox_util.obj"
	-@erase "$(INTDIR)\lemx_util.obj"
	-@erase "$(INTDIR)\lfn_util.obj"
	-@erase "$(INTDIR)\list_util.obj"
	-@erase "$(INTDIR)\log_util.obj"
	-@erase "$(INTDIR)\meo_util.obj"
	-@erase "$(INTDIR)\net_util.obj"
	-@erase "$(INTDIR)\nft_proc.obj"
	-@erase "$(INTDIR)\nft_util.obj"
	-@erase "$(INTDIR)\nvl_util.obj"
	-@erase "$(INTDIR)\nvp_util.obj"
	-@erase "$(INTDIR)\opt_util.obj"
	-@erase "$(INTDIR)\port_util.obj"
	-@erase "$(INTDIR)\rex_util.obj"
	-@erase "$(INTDIR)\rex_util_y.obj"
	-@erase "$(INTDIR)\skt_util.obj"
	-@erase "$(INTDIR)\str_util.obj"
	-@erase "$(INTDIR)\tcp_util.obj"
	-@erase "$(INTDIR)\tpl_util.obj"
	-@erase "$(INTDIR)\ts_util.obj"
	-@erase "$(INTDIR)\tv_util.obj"
	-@erase "$(INTDIR)\udp_util.obj"
	-@erase "$(INTDIR)\utf_util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vim_util.obj"
	-@erase "$(INTDIR)\wcs_util.obj"
	-@erase "$(INTDIR)\xnet_util.obj"
	-@erase "$(OUTDIR)\libgpl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\libgpl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libgpl.bsc"
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libgpl.lib"
LIB32_OBJS= \
	"$(INTDIR)\aperror.obj" \
	"$(INTDIR)\bio_util.obj" \
	"$(INTDIR)\bit_util.obj" \
	"$(INTDIR)\bmw_util.obj" \
	"$(INTDIR)\coli_util.obj" \
	"$(INTDIR)\comx_util.obj" \
	"$(INTDIR)\cosn_util.obj" \
	"$(INTDIR)\crlf_util.obj" \
	"$(INTDIR)\damx_util.obj" \
	"$(INTDIR)\ddmx_util.obj" \
	"$(INTDIR)\drs_util.obj" \
	"$(INTDIR)\f1750a_util.obj" \
	"$(INTDIR)\ffs.obj" \
	"$(INTDIR)\fnm_util.obj" \
	"$(INTDIR)\get_util.obj" \
	"$(INTDIR)\gimx_util.obj" \
	"$(INTDIR)\gsc_util.obj" \
	"$(INTDIR)\hash_util.obj" \
	"$(INTDIR)\id3_util.obj" \
	"$(INTDIR)\ieee_util.obj" \
	"$(INTDIR)\iiop_util.obj" \
	"$(INTDIR)\iox_util.obj" \
	"$(INTDIR)\lemx_util.obj" \
	"$(INTDIR)\lfn_util.obj" \
	"$(INTDIR)\list_util.obj" \
	"$(INTDIR)\log_util.obj" \
	"$(INTDIR)\meo_util.obj" \
	"$(INTDIR)\net_util.obj" \
	"$(INTDIR)\nft_proc.obj" \
	"$(INTDIR)\nft_util.obj" \
	"$(INTDIR)\nvl_util.obj" \
	"$(INTDIR)\nvp_util.obj" \
	"$(INTDIR)\opt_util.obj" \
	"$(INTDIR)\port_util.obj" \
	"$(INTDIR)\rex_util.obj" \
	"$(INTDIR)\rex_util_y.obj" \
	"$(INTDIR)\skt_util.obj" \
	"$(INTDIR)\str_util.obj" \
	"$(INTDIR)\tcp_util.obj" \
	"$(INTDIR)\tpl_util.obj" \
	"$(INTDIR)\ts_util.obj" \
	"$(INTDIR)\tv_util.obj" \
	"$(INTDIR)\udp_util.obj" \
	"$(INTDIR)\utf_util.obj" \
	"$(INTDIR)\vim_util.obj" \
	"$(INTDIR)\wcs_util.obj" \
	"$(INTDIR)\xnet_util.obj"

"$(OUTDIR)\libgpl.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libgpl - Win32 Debug"

OUTDIR=.\..\lib
INTDIR=.\..\obj\libgpl
# Begin Custom Macros
OutDir=.\..\lib
# End Custom Macros

ALL : "$(OUTDIR)\libgpl.lib"


CLEAN :
	-@erase "$(INTDIR)\aperror.obj"
	-@erase "$(INTDIR)\bio_util.obj"
	-@erase "$(INTDIR)\bit_util.obj"
	-@erase "$(INTDIR)\bmw_util.obj"
	-@erase "$(INTDIR)\coli_util.obj"
	-@erase "$(INTDIR)\comx_util.obj"
	-@erase "$(INTDIR)\cosn_util.obj"
	-@erase "$(INTDIR)\crlf_util.obj"
	-@erase "$(INTDIR)\damx_util.obj"
	-@erase "$(INTDIR)\ddmx_util.obj"
	-@erase "$(INTDIR)\drs_util.obj"
	-@erase "$(INTDIR)\f1750a_util.obj"
	-@erase "$(INTDIR)\ffs.obj"
	-@erase "$(INTDIR)\fnm_util.obj"
	-@erase "$(INTDIR)\get_util.obj"
	-@erase "$(INTDIR)\gimx_util.obj"
	-@erase "$(INTDIR)\gsc_util.obj"
	-@erase "$(INTDIR)\hash_util.obj"
	-@erase "$(INTDIR)\id3_util.obj"
	-@erase "$(INTDIR)\ieee_util.obj"
	-@erase "$(INTDIR)\iiop_util.obj"
	-@erase "$(INTDIR)\iox_util.obj"
	-@erase "$(INTDIR)\lemx_util.obj"
	-@erase "$(INTDIR)\lfn_util.obj"
	-@erase "$(INTDIR)\list_util.obj"
	-@erase "$(INTDIR)\log_util.obj"
	-@erase "$(INTDIR)\meo_util.obj"
	-@erase "$(INTDIR)\net_util.obj"
	-@erase "$(INTDIR)\nft_proc.obj"
	-@erase "$(INTDIR)\nft_util.obj"
	-@erase "$(INTDIR)\nvl_util.obj"
	-@erase "$(INTDIR)\nvp_util.obj"
	-@erase "$(INTDIR)\opt_util.obj"
	-@erase "$(INTDIR)\port_util.obj"
	-@erase "$(INTDIR)\rex_util.obj"
	-@erase "$(INTDIR)\rex_util_y.obj"
	-@erase "$(INTDIR)\skt_util.obj"
	-@erase "$(INTDIR)\str_util.obj"
	-@erase "$(INTDIR)\tcp_util.obj"
	-@erase "$(INTDIR)\tpl_util.obj"
	-@erase "$(INTDIR)\ts_util.obj"
	-@erase "$(INTDIR)\tv_util.obj"
	-@erase "$(INTDIR)\udp_util.obj"
	-@erase "$(INTDIR)\utf_util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vim_util.obj"
	-@erase "$(INTDIR)\wcs_util.obj"
	-@erase "$(INTDIR)\xnet_util.obj"
	-@erase "$(OUTDIR)\libgpl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\libgpl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /TC /D /I /GZ /c

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libgpl.bsc"
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libgpl.lib"
LIB32_OBJS= \
	"$(INTDIR)\aperror.obj" \
	"$(INTDIR)\bio_util.obj" \
	"$(INTDIR)\bit_util.obj" \
	"$(INTDIR)\bmw_util.obj" \
	"$(INTDIR)\coli_util.obj" \
	"$(INTDIR)\comx_util.obj" \
	"$(INTDIR)\cosn_util.obj" \
	"$(INTDIR)\crlf_util.obj" \
	"$(INTDIR)\damx_util.obj" \
	"$(INTDIR)\ddmx_util.obj" \
	"$(INTDIR)\drs_util.obj" \
	"$(INTDIR)\f1750a_util.obj" \
	"$(INTDIR)\ffs.obj" \
	"$(INTDIR)\fnm_util.obj" \
	"$(INTDIR)\get_util.obj" \
	"$(INTDIR)\gimx_util.obj" \
	"$(INTDIR)\gsc_util.obj" \
	"$(INTDIR)\hash_util.obj" \
	"$(INTDIR)\id3_util.obj" \
	"$(INTDIR)\ieee_util.obj" \
	"$(INTDIR)\iiop_util.obj" \
	"$(INTDIR)\iox_util.obj" \
	"$(INTDIR)\lemx_util.obj" \
	"$(INTDIR)\lfn_util.obj" \
	"$(INTDIR)\list_util.obj" \
	"$(INTDIR)\log_util.obj" \
	"$(INTDIR)\meo_util.obj" \
	"$(INTDIR)\net_util.obj" \
	"$(INTDIR)\nft_proc.obj" \
	"$(INTDIR)\nft_util.obj" \
	"$(INTDIR)\nvl_util.obj" \
	"$(INTDIR)\nvp_util.obj" \
	"$(INTDIR)\opt_util.obj" \
	"$(INTDIR)\port_util.obj" \
	"$(INTDIR)\rex_util.obj" \
	"$(INTDIR)\rex_util_y.obj" \
	"$(INTDIR)\skt_util.obj" \
	"$(INTDIR)\str_util.obj" \
	"$(INTDIR)\tcp_util.obj" \
	"$(INTDIR)\tpl_util.obj" \
	"$(INTDIR)\ts_util.obj" \
	"$(INTDIR)\tv_util.obj" \
	"$(INTDIR)\udp_util.obj" \
	"$(INTDIR)\utf_util.obj" \
	"$(INTDIR)\vim_util.obj" \
	"$(INTDIR)\wcs_util.obj" \
	"$(INTDIR)\xnet_util.obj"

"$(OUTDIR)\libgpl.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("libgpl.dep")
!INCLUDE "libgpl.dep"
!ELSE
!MESSAGE Warning: cannot find "libgpl.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "libgpl - Win32 Release" || "$(CFG)" == "libgpl - Win32 Debug"
SOURCE=.\aperror.c

"$(INTDIR)\aperror.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\bio_util.c

"$(INTDIR)\bio_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\bit_util.c

"$(INTDIR)\bit_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\bmw_util.c

"$(INTDIR)\bmw_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\coli_util.c

"$(INTDIR)\coli_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\comx_util.c

"$(INTDIR)\comx_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cosn_util.c

"$(INTDIR)\cosn_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\crlf_util.c

"$(INTDIR)\crlf_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\damx_idl.c
SOURCE=.\damx_util.c

"$(INTDIR)\damx_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ddmx_idl.c
SOURCE=.\ddmx_util.c

"$(INTDIR)\ddmx_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\drs_util.c

"$(INTDIR)\drs_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\f1750a_util.c

"$(INTDIR)\f1750a_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ffs.c

"$(INTDIR)\ffs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fnm_util.c

"$(INTDIR)\fnm_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\get_util.c

"$(INTDIR)\get_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\getopt.c
SOURCE=.\gimx_idl.c
SOURCE=.\gimx_util.c

"$(INTDIR)\gimx_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gsc_util.c

"$(INTDIR)\gsc_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hash_util.c

"$(INTDIR)\hash_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\id3_util.c

"$(INTDIR)\id3_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ieee_util.c

"$(INTDIR)\ieee_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\iiop_util.c

"$(INTDIR)\iiop_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\iox_util.c

"$(INTDIR)\iox_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lemx_idl.c
SOURCE=.\lemx_util.c

"$(INTDIR)\lemx_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lfn_util.c

"$(INTDIR)\lfn_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\list_util.c

"$(INTDIR)\list_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\log_util.c

"$(INTDIR)\log_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\meo_util.c

"$(INTDIR)\meo_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\net_util.c

"$(INTDIR)\net_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\nft_proc.c

"$(INTDIR)\nft_proc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\nft_util.c

"$(INTDIR)\nft_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\nvl_util.c

"$(INTDIR)\nvl_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\nvp_util.c

"$(INTDIR)\nvp_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\opt_util.c

"$(INTDIR)\opt_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\port_util.c

"$(INTDIR)\port_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rex_util.c

"$(INTDIR)\rex_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rex_util_y.c

"$(INTDIR)\rex_util_y.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\skt_util.c

"$(INTDIR)\skt_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\str_util.c

"$(INTDIR)\str_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tcp_util.c

"$(INTDIR)\tcp_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tpl_util.c

"$(INTDIR)\tpl_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ts_util.c

"$(INTDIR)\ts_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tv_util.c

"$(INTDIR)\tv_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\udp_util.c

"$(INTDIR)\udp_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utf_util.c

"$(INTDIR)\utf_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vim_util.c

"$(INTDIR)\vim_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wcs_util.c

"$(INTDIR)\wcs_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xnet_util.c

"$(INTDIR)\xnet_util.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF
