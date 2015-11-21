# Microsoft Developer Studio Generated NMAKE File, Based on DevTCLauncher.dsp
!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to DevTCLauncher - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DevTCLauncher.mak" CFG="Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DevTCLauncher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DevTCLauncher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\DevTCLauncher.exe"

!ELSE 

ALL : "$(OUTDIR)\DevTCLauncher.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\DevTCLauncher.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\DevTCLauncher.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)\DevTCLauncher.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DevTCLauncher.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\DevTCLauncher.pdb" /machine:I386\
 /out:"$(OUTDIR)\DevTCLauncher.exe" 
LINK32_OBJS= \
	"$(INTDIR)\DevTCLauncher.obj"

"$(OUTDIR)\DevTCLauncher.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\DevTCLauncher.exe"

!ELSE 

ALL : "$(OUTDIR)\DevTCLauncher.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\DevTCLauncher.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\DevTCLauncher.exe"
	-@erase "$(OUTDIR)\DevTCLauncher.ilk"
	-@erase "$(OUTDIR)\DevTCLauncher.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)\DevTCLauncher.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /ZI /GZ /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DevTCLauncher.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\DevTCLauncher.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\DevTCLauncher.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\DevTCLauncher.obj"

"$(OUTDIR)\DevTCLauncher.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "Release" || "$(CFG)" == "Debug"
SOURCE=..\DevTCLauncher.cpp
DEP_CPP_DEVTC=\
	"..\stdafx.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_DEVTC=\
	"..\curses.h"\
	"..\sys\wait.h"\
	"..\unistd.h"\
	

"$(INTDIR)\DevTCLauncher.obj" : $(SOURCE) $(DEP_CPP_DEVTC) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

