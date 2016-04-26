# Microsoft Developer Studio Generated NMAKE File, Based on talknet.dsp
!IF "$(CFG)" == ""
CFG=talknet - Win32 Debug
!MESSAGE No configuration specified. Defaulting to talknet - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "talknet - Win32 Release" && "$(CFG)" != "talknet - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "talknet.mak" CFG="talknet - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "talknet - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "talknet - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "talknet - Win32 Release"

OUTDIR=.\..\bin
INTDIR=.\..\obj
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\talknet.exe"


CLEAN :
	-@erase "$(INTDIR)\talknet.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\talknet.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\talknet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\talknet.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\lib\libgpl.lib ..\lib\libxdr.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\talknet.pdb" /machine:I386 /out:"$(OUTDIR)\talknet.exe" 
LINK32_OBJS= \
	"$(INTDIR)\talknet.obj"

"$(OUTDIR)\talknet.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "talknet - Win32 Debug"

OUTDIR=.\..\bin
INTDIR=.\..\obj
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\talknet.exe"


CLEAN :
	-@erase "$(INTDIR)\talknet.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\talknet.exe"
	-@erase "$(OUTDIR)\talknet.ilk"
	-@erase "$(OUTDIR)\talknet.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\talknet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /TC /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\talknet.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\lib\libgpl.lib ..\lib\libxdr.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\talknet.pdb" /debug /machine:I386 /out:"$(OUTDIR)\talknet.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\talknet.obj"

"$(OUTDIR)\talknet.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("talknet.dep")
!INCLUDE "talknet.dep"
!ELSE 
!MESSAGE Warning: cannot find "talknet.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "talknet - Win32 Release" || "$(CFG)" == "talknet - Win32 Debug"
SOURCE=.\talknet.c

"$(INTDIR)\talknet.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

