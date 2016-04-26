# Microsoft Developer Studio Generated NMAKE File, Based on libxdr.dsp
!IF "$(CFG)" == ""
CFG=libxdr - Win32 Debug
!MESSAGE No configuration specified. Defaulting to libxdr - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "libxdr - Win32 Release" && "$(CFG)" != "libxdr - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libxdr.mak" CFG="libxdr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libxdr - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libxdr - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "libxdr - Win32 Release"

OUTDIR=.\..\lib
INTDIR=.\..\obj\libxdr
# Begin Custom Macros
OutDir=.\..\lib
# End Custom Macros

ALL : "$(OUTDIR)\libxdr.lib"


CLEAN :
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xdr.obj"
	-@erase "$(INTDIR)\xdr_array.obj"
	-@erase "$(INTDIR)\xdr_float.obj"
	-@erase "$(INTDIR)\xdr_mem.obj"
	-@erase "$(INTDIR)\xdr_rec.obj"
	-@erase "$(INTDIR)\xdr_reference.obj"
	-@erase "$(INTDIR)\xdr_stdio.obj"
	-@erase "$(OUTDIR)\libxdr.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\libxdr.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libxdr.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libxdr.lib" 
LIB32_OBJS= \
	"$(INTDIR)\xdr.obj" \
	"$(INTDIR)\xdr_array.obj" \
	"$(INTDIR)\xdr_float.obj" \
	"$(INTDIR)\xdr_mem.obj" \
	"$(INTDIR)\xdr_rec.obj" \
	"$(INTDIR)\xdr_reference.obj" \
	"$(INTDIR)\xdr_stdio.obj"

"$(OUTDIR)\libxdr.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libxdr - Win32 Debug"

OUTDIR=.\..\lib
INTDIR=.\..\obj\libxdr
# Begin Custom Macros
OutDir=.\..\lib
# End Custom Macros

ALL : "$(OUTDIR)\libxdr.lib"


CLEAN :
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xdr.obj"
	-@erase "$(INTDIR)\xdr_array.obj"
	-@erase "$(INTDIR)\xdr_float.obj"
	-@erase "$(INTDIR)\xdr_mem.obj"
	-@erase "$(INTDIR)\xdr_rec.obj"
	-@erase "$(INTDIR)\xdr_reference.obj"
	-@erase "$(INTDIR)\xdr_stdio.obj"
	-@erase "$(OUTDIR)\libxdr.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\libxdr.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /TC /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libxdr.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libxdr.lib" 
LIB32_OBJS= \
	"$(INTDIR)\xdr.obj" \
	"$(INTDIR)\xdr_array.obj" \
	"$(INTDIR)\xdr_float.obj" \
	"$(INTDIR)\xdr_mem.obj" \
	"$(INTDIR)\xdr_rec.obj" \
	"$(INTDIR)\xdr_reference.obj" \
	"$(INTDIR)\xdr_stdio.obj"

"$(OUTDIR)\libxdr.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("libxdr.dep")
!INCLUDE "libxdr.dep"
!ELSE 
!MESSAGE Warning: cannot find "libxdr.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "libxdr - Win32 Release" || "$(CFG)" == "libxdr - Win32 Debug"
SOURCE=.\xdr.c

"$(INTDIR)\xdr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdr_array.c

"$(INTDIR)\xdr_array.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdr_float.c

"$(INTDIR)\xdr_float.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdr_mem.c

"$(INTDIR)\xdr_mem.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdr_rec.c

"$(INTDIR)\xdr_rec.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdr_reference.c

"$(INTDIR)\xdr_reference.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\xdr_stdio.c

"$(INTDIR)\xdr_stdio.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

