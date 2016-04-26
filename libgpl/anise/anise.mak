# Microsoft Developer Studio Generated NMAKE File, Based on anise.dsp
!IF "$(CFG)" == ""
CFG=anise - Win32 Debug
!MESSAGE No configuration specified. Defaulting to anise - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "anise - Win32 Release" && "$(CFG)" != "anise - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "anise.mak" CFG="anise - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "anise - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "anise - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

!IF  "$(CFG)" == "anise - Win32 Release"

OUTDIR=.\..\bin
INTDIR=.\..\obj
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\anise.exe"


CLEAN :
	-@erase "$(INTDIR)\anise.obj"
	-@erase "$(INTDIR)\ftpClient.obj"
	-@erase "$(INTDIR)\http_util.obj"
	-@erase "$(INTDIR)\passClient.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wwwClient.obj"
	-@erase "$(OUTDIR)\anise.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\anise.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\anise.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=..\lib\libgpl.lib ..\lib\libxdr.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\anise.pdb" /machine:I386 /out:"$(OUTDIR)\anise.exe"
LINK32_OBJS= \
	"$(INTDIR)\anise.obj" \
	"$(INTDIR)\ftpClient.obj" \
	"$(INTDIR)\http_util.obj" \
	"$(INTDIR)\passClient.obj" \
	"$(INTDIR)\wwwClient.obj"

"$(OUTDIR)\anise.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "anise - Win32 Debug"

OUTDIR=.\..\bin
INTDIR=.\..\obj
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\anise.exe"


CLEAN :
	-@erase "$(INTDIR)\anise.obj"
	-@erase "$(OUTDIR)\anise.exe"
	-@erase "$(OUTDIR)\anise.ilk"
	-@erase "$(OUTDIR)\anise.pdb"
	-@erase "$(INTDIR)\ftpClient.obj"
	-@erase "$(INTDIR)\http_util.obj"
	-@erase "$(INTDIR)\passClient.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wwwClient.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\anise.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\anise.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=..\lib\libgpl.lib ..\lib\libxdr.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\anise.pdb" /debug /machine:I386 /out:"$(OUTDIR)\anise.exe" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\ftpClient.obj" \
	"$(INTDIR)\http_util.obj" \
	"$(INTDIR)\passClient.obj" \
	"$(INTDIR)\anise.obj" \
	"$(INTDIR)\wwwClient.obj"

"$(OUTDIR)\anise.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("anise.dep")
!INCLUDE "anise.dep"
!ELSE
!MESSAGE Warning: cannot find "anise.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "anise - Win32 Release" || "$(CFG)" == "anise - Win32 Debug"
SOURCE=.\anise.c

"$(INTDIR)\anise.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ftpClient.c

"$(INTDIR)\ftpClient.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\http_util.c

"$(INTDIR)\http_util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\passClient.c

"$(INTDIR)\passClient.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wwwClient.c

"$(INTDIR)\wwwClient.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF
