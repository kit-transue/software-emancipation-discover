# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=Integrator - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Integrator - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Integrator - Win32 Release" && "$(CFG)" !=\
 "Integrator - Win32 Debug" && "$(CFG)" != "DisRegistry - Win32 Release" &&\
 "$(CFG)" != "DisRegistry - Win32 Debug" && "$(CFG)" !=\
 "IDEServerClient - Win32 Release" && "$(CFG)" !=\
 "IDEServerClient - Win32 Debug" && "$(CFG)" != "MapNet - Win32 Release" &&\
 "$(CFG)" != "MapNet - Win32 Debug" && "$(CFG)" != "CList - Win32 Release" &&\
 "$(CFG)" != "CList - Win32 Debug" && "$(CFG)" != "MenuChooser - Win32 Release"\
 && "$(CFG)" != "MenuChooser - Win32 Debug" && "$(CFG)" !=\
 "IDESpy - Win32 Release" && "$(CFG)" != "IDESpy - Win32 Debug" && "$(CFG)" !=\
 "IDESpyLoader - Win32 Release" && "$(CFG)" != "IDESpyLoader - Win32 Debug" &&\
 "$(CFG)" != "IDEConsole - Win32 Release" && "$(CFG)" !=\
 "IDEConsole - Win32 Debug" && "$(CFG)" != "IDEParamReciever - Win32 Release" &&\
 "$(CFG)" != "IDEParamReciever - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "integrator.mak" CFG="Integrator - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Integrator - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Integrator - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "DisRegistry - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DisRegistry - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "IDEServerClient - Win32 Release" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "IDEServerClient - Win32 Debug" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "MapNet - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MapNet - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "CList - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CList - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "MenuChooser - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MenuChooser - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "IDESpy - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IDESpy - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IDESpyLoader - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "IDESpyLoader - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "IDEConsole - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "IDEConsole - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "IDEParamReciever - Win32 Release" (based on\
 "Win32 (x86) Application")
!MESSAGE "IDEParamReciever - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Integrator - Win32 Debug"

!IF  "$(CFG)" == "Integrator - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\gObj\Integrator"
# PROP Intermediate_Dir "c:\Integrator\Obj\Integrator"
# PROP Target_Dir ""
OUTDIR=c:\Integrator\gObj\Integrator
INTDIR=c:\Integrator\Obj\Integrator

ALL : "IDEParamReciever - Win32 Release" "IDEConsole - Win32 Release"\
 "IDESpy - Win32 Release" "MenuChooser - Win32 Release" "MapNet - Win32 Release"\
 "IDEServerClient - Win32 Release" "DisRegistry - Win32 Release"\
 "CList - Win32 Release" "$(OUTDIR)\IntegratorServer.exe"\
 "c:\Integrator\Browse\Integrator.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\Integrator.bsc"
	-@erase "c:\Integrator\Obj\Integrator\ServerAdapter.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerIDEInfo.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerWin.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerDDE.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerQuery.sbr"
	-@erase "c:\Integrator\Obj\Integrator\readsock.sbr"
	-@erase "c:\Integrator\Bin\IntegratorServer.exe"
	-@erase "c:\Integrator\Obj\Integrator\readsock.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerAdapter.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerIDEInfo.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerWin.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerDDE.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerQuery.obj"
	-@erase "c:\Integrator\Obj\Integrator\server.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/integrator.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\Integrator/
CPP_SBRS=c:\Integrator\Obj\Integrator/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/server.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\Integrator.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\Integrator.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/ServerAdapter.sbr" \
	"$(INTDIR)/ServerIDEInfo.sbr" \
	"$(INTDIR)/ServerWin.sbr" \
	"$(INTDIR)/ServerDDE.sbr" \
	"$(INTDIR)/ServerQuery.sbr" \
	"$(INTDIR)/readsock.sbr"

"c:\Integrator\Browse\Integrator.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 IDEServerClient.lib MapNet.lib CList.lib MenuChooser.lib DisRegistry.lib wsock32.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"c:\Integrator\Bin\IntegratorServer.exe"
LINK32_FLAGS=IDEServerClient.lib MapNet.lib CList.lib MenuChooser.lib\
 DisRegistry.lib wsock32.lib mpr.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/IntegratorServer.pdb" /machine:I386\
 /out:"c:\Integrator\Bin\IntegratorServer.exe" 
LINK32_OBJS= \
	"$(INTDIR)/readsock.obj" \
	"$(INTDIR)/ServerAdapter.obj" \
	"$(INTDIR)/ServerIDEInfo.obj" \
	"$(INTDIR)/ServerWin.obj" \
	"$(INTDIR)/ServerDDE.obj" \
	"$(INTDIR)/ServerQuery.obj" \
	"$(INTDIR)/server.res" \
	"..\lib\tcl74.lib" \
	"c:\Integrator\Lib\CList.lib" \
	"c:\Integrator\Lib\DisRegistry.lib" \
	"c:\Integrator\Lib\IDEServerClient.lib" \
	"c:\Integrator\Lib\MapNet.lib" \
	"c:\Integrator\Lib\MenuChooser.lib" \
	"c:\Integrator\Obj\IDESpy\IDESpy.lib"

"$(OUTDIR)\IntegratorServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\Integrator"
# PROP Intermediate_Dir "c:\Integrator\Obj\Integrator"
# PROP Target_Dir ""
OUTDIR=c:\Integrator\Obj\Integrator
INTDIR=c:\Integrator\Obj\Integrator

ALL : "IDEParamReciever - Win32 Debug" "IDEConsole - Win32 Debug"\
 "IDESpy - Win32 Debug" "MenuChooser - Win32 Debug" "MapNet - Win32 Debug"\
 "IDEServerClient - Win32 Debug" "DisRegistry - Win32 Debug"\
 "CList - Win32 Debug" "$(OUTDIR)\IntegratorServer.exe"\
 "c:\Integrator\Browse\Integrator.bsc"

CLEAN : 
	-@erase "c:\Integrator\Obj\Integrator\vc40.pdb"
	-@erase "c:\Integrator\Obj\Integrator\vc40.idb"
	-@erase "c:\Integrator\Browse\Integrator.bsc"
	-@erase "c:\Integrator\Obj\Integrator\ServerAdapter.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerIDEInfo.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerWin.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerDDE.sbr"
	-@erase "c:\Integrator\Obj\Integrator\ServerQuery.sbr"
	-@erase "c:\Integrator\Obj\Integrator\readsock.sbr"
	-@erase "c:\Integrator\Bin\IntegratorServer.exe"
	-@erase "c:\Integrator\Obj\Integrator\readsock.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerAdapter.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerIDEInfo.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerWin.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerDDE.obj"
	-@erase "c:\Integrator\Obj\Integrator\ServerQuery.obj"
	-@erase "c:\Integrator\Obj\Integrator\server.res"
	-@erase "c:\Integrator\Bin\IntegratorServer.ilk"
	-@erase "c:\Integrator\Obj\Integrator\IntegratorServer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/integrator.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\Integrator/
CPP_SBRS=c:\Integrator\Obj\Integrator/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/server.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\Integrator.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\Integrator.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/ServerAdapter.sbr" \
	"$(INTDIR)/ServerIDEInfo.sbr" \
	"$(INTDIR)/ServerWin.sbr" \
	"$(INTDIR)/ServerDDE.sbr" \
	"$(INTDIR)/ServerQuery.sbr" \
	"$(INTDIR)/readsock.sbr"

"c:\Integrator\Browse\Integrator.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 IDEServerClient.lib MapNet.lib CList.lib MenuChooser.lib DisRegistry.lib netapi32.lib wsock32.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\Integrator\Bin\IntegratorServer.exe"
LINK32_FLAGS=IDEServerClient.lib MapNet.lib CList.lib MenuChooser.lib\
 DisRegistry.lib netapi32.lib wsock32.lib mpr.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/IntegratorServer.pdb" /debug /machine:I386\
 /out:"c:\Integrator\Bin\IntegratorServer.exe" 
LINK32_OBJS= \
	"$(INTDIR)/readsock.obj" \
	"$(INTDIR)/ServerAdapter.obj" \
	"$(INTDIR)/ServerIDEInfo.obj" \
	"$(INTDIR)/ServerWin.obj" \
	"$(INTDIR)/ServerDDE.obj" \
	"$(INTDIR)/ServerQuery.obj" \
	"$(INTDIR)/server.res" \
	"..\lib\tcl74.lib" \
	"c:\Integrator\Lib\CList.lib" \
	"c:\Integrator\Lib\DisRegistry.lib" \
	"c:\Integrator\Lib\IDEServerClient.lib" \
	"c:\Integrator\Lib\MapNet.lib" \
	"c:\Integrator\Lib\MenuChooser.lib" \
	"c:\Integrator\Obj\IDESpy\IDESpy.lib"

"$(OUTDIR)\IntegratorServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DisRegistry - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DisRegistry\Release"
# PROP BASE Intermediate_Dir "DisRegistry\Release"
# PROP BASE Target_Dir "DisRegistry"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\DisRegistry"
# PROP Intermediate_Dir "c:\Integrator\Obj\DisRegistry"
# PROP Target_Dir "DisRegistry"
OUTDIR=c:\Integrator\Obj\DisRegistry
INTDIR=c:\Integrator\Obj\DisRegistry

ALL : "$(OUTDIR)\DisRegistry.lib" "c:\Integrator\Browse\DisRegistry.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\DisRegistry.bsc"
	-@erase "c:\Integrator\Obj\DisRegistry\DisRegistry.sbr"
	-@erase "c:\Integrator\Lib\DisRegistry.lib"
	-@erase "c:\Integrator\Obj\DisRegistry\DisRegistry.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/DisRegistry.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\DisRegistry/
CPP_SBRS=c:\Integrator\Obj\DisRegistry/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\DisRegistry.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\DisRegistry.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/DisRegistry.sbr"

"c:\Integrator\Browse\DisRegistry.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\DisRegistry.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\DisRegistry.lib" 
LIB32_OBJS= \
	"$(INTDIR)/DisRegistry.obj"

"$(OUTDIR)\DisRegistry.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DisRegistry - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DisRegistry\DisRegis"
# PROP BASE Intermediate_Dir "DisRegistry\DisRegis"
# PROP BASE Target_Dir "DisRegistry"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\DisRegistry"
# PROP Intermediate_Dir "c:\Integrator\Obj\DisRegistry"
# PROP Target_Dir "DisRegistry"
OUTDIR=c:\Integrator\Obj\DisRegistry
INTDIR=c:\Integrator\Obj\DisRegistry

ALL : "$(OUTDIR)\DisRegistry.lib" "c:\Integrator\Browse\DisRegistry.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\DisRegistry.bsc"
	-@erase "c:\Integrator\Obj\DisRegistry\DisRegistry.sbr"
	-@erase "c:\Integrator\Lib\DisRegistry.lib"
	-@erase "c:\Integrator\Obj\DisRegistry\DisRegistry.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/DisRegistry.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\DisRegistry/
CPP_SBRS=c:\Integrator\Obj\DisRegistry/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\DisRegistry.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\DisRegistry.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/DisRegistry.sbr"

"c:\Integrator\Browse\DisRegistry.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\DisRegistry.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\DisRegistry.lib" 
LIB32_OBJS= \
	"$(INTDIR)/DisRegistry.obj"

"$(OUTDIR)\DisRegistry.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDEServerClient - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IDEServerClient\Release"
# PROP BASE Intermediate_Dir "IDEServerClient\Release"
# PROP BASE Target_Dir "IDEServerClient"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\IDEServerClient"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDEServerClient"
# PROP Target_Dir "IDEServerClient"
OUTDIR=c:\Integrator\Obj\IDEServerClient
INTDIR=c:\Integrator\Obj\IDEServerClient

ALL : "$(OUTDIR)\IDEServerClient.lib"\
 "c:\Integrator\Browse\IDEServerClient.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\IDEServerClient.bsc"
	-@erase "c:\Integrator\Obj\IDEServerClient\IDEServerClient.sbr"
	-@erase "c:\Integrator\Lib\IDEServerClient.lib"
	-@erase "c:\Integrator\Obj\IDEServerClient\IDEServerClient.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDEServerClient.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDEServerClient/
CPP_SBRS=c:\Integrator\Obj\IDEServerClient/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDEServerClient.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDEServerClient.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/IDEServerClient.sbr"

"c:\Integrator\Browse\IDEServerClient.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\IDEServerClient.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\IDEServerClient.lib" 
LIB32_OBJS= \
	"$(INTDIR)/IDEServerClient.obj"

"$(OUTDIR)\IDEServerClient.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDEServerClient - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "IDEServerClient\Debug"
# PROP BASE Intermediate_Dir "IDEServerClient\Debug"
# PROP BASE Target_Dir "IDEServerClient"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\IDEServerClient"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDEServerClient"
# PROP Target_Dir "IDEServerClient"
OUTDIR=c:\Integrator\Obj\IDEServerClient
INTDIR=c:\Integrator\Obj\IDEServerClient

ALL : "$(OUTDIR)\IDEServerClient.lib"\
 "c:\Integrator\Browse\IDEServerClient.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\IDEServerClient.bsc"
	-@erase "c:\Integrator\Obj\IDEServerClient\IDEServerClient.sbr"
	-@erase "c:\Integrator\Lib\IDEServerClient.lib"
	-@erase "c:\Integrator\Obj\IDEServerClient\IDEServerClient.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDEServerClient.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDEServerClient/
CPP_SBRS=c:\Integrator\Obj\IDEServerClient/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDEServerClient.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDEServerClient.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/IDEServerClient.sbr"

"c:\Integrator\Browse\IDEServerClient.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\IDEServerClient.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\IDEServerClient.lib" 
LIB32_OBJS= \
	"$(INTDIR)/IDEServerClient.obj"

"$(OUTDIR)\IDEServerClient.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MapNet - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MapNet\Release"
# PROP BASE Intermediate_Dir "MapNet\Release"
# PROP BASE Target_Dir "MapNet"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\MapNet"
# PROP Intermediate_Dir "c:\Integrator\Obj\MapNet"
# PROP Target_Dir "MapNet"
OUTDIR=c:\Integrator\Obj\MapNet
INTDIR=c:\Integrator\Obj\MapNet

ALL : "$(OUTDIR)\MapNet.lib" "c:\Integrator\Browse\MapNet.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\MapNet.bsc"
	-@erase "c:\Integrator\Obj\MapNet\mapnet.sbr"
	-@erase "c:\Integrator\Lib\MapNet.lib"
	-@erase "c:\Integrator\Obj\MapNet\mapnet.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/MapNet.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\MapNet/
CPP_SBRS=c:\Integrator\Obj\MapNet/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\MapNet.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\MapNet.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/mapnet.sbr"

"c:\Integrator\Browse\MapNet.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\MapNet.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\MapNet.lib" 
LIB32_OBJS= \
	"$(INTDIR)/mapnet.obj"

"$(OUTDIR)\MapNet.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MapNet - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MapNet\Debug"
# PROP BASE Intermediate_Dir "MapNet\Debug"
# PROP BASE Target_Dir "MapNet"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\MapNetug"
# PROP Intermediate_Dir "c:\Integrator\Obj\MapNet"
# PROP Target_Dir "MapNet"
OUTDIR=c:\Integrator\Obj\MapNetug
INTDIR=c:\Integrator\Obj\MapNet

ALL : "$(OUTDIR)\MapNet.lib" "c:\Integrator\Browse\MapNet.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\MapNet.bsc"
	-@erase "c:\Integrator\Obj\MapNet\mapnet.sbr"
	-@erase "c:\Integrator\Lib\MapNet.lib"
	-@erase "c:\Integrator\Obj\MapNet\mapnet.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/MapNet.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\MapNet/
CPP_SBRS=c:\Integrator\Obj\MapNet/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\MapNet.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\MapNet.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/mapnet.sbr"

"c:\Integrator\Browse\MapNet.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\MapNet.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\MapNet.lib" 
LIB32_OBJS= \
	"$(INTDIR)/mapnet.obj"

"$(OUTDIR)\MapNet.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CList - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CList\Release"
# PROP BASE Intermediate_Dir "CList\Release"
# PROP BASE Target_Dir "CList"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\CList"
# PROP Intermediate_Dir "c:\Integrator\Obj\CList"
# PROP Target_Dir "CList"
OUTDIR=c:\Integrator\Obj\CList
INTDIR=c:\Integrator\Obj\CList

ALL : "$(OUTDIR)\CList.lib" "c:\Integrator\Browse\CList.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\CList.bsc"
	-@erase "c:\Integrator\Obj\CList\clist.sbr"
	-@erase "c:\Integrator\Lib\CList.lib"
	-@erase "c:\Integrator\Obj\CList\clist.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/CList.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\CList/
CPP_SBRS=c:\Integrator\Obj\CList/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\CList.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\CList.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/clist.sbr"

"c:\Integrator\Browse\CList.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\CList.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\CList.lib" 
LIB32_OBJS= \
	"$(INTDIR)/clist.obj"

"$(OUTDIR)\CList.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CList - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CList\Debug"
# PROP BASE Intermediate_Dir "CList\Debug"
# PROP BASE Target_Dir "CList"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\CList"
# PROP Intermediate_Dir "c:\Integrator\Obj\CList"
# PROP Target_Dir "CList"
OUTDIR=c:\Integrator\Obj\CList
INTDIR=c:\Integrator\Obj\CList

ALL : "$(OUTDIR)\CList.lib" "c:\Integrator\Browse\CList.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\CList.bsc"
	-@erase "c:\Integrator\Obj\CList\clist.sbr"
	-@erase "c:\Integrator\Lib\CList.lib"
	-@erase "c:\Integrator\Obj\CList\clist.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/CList.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\CList/
CPP_SBRS=c:\Integrator\Obj\CList/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\CList.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\CList.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/clist.sbr"

"c:\Integrator\Browse\CList.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\CList.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\CList.lib" 
LIB32_OBJS= \
	"$(INTDIR)/clist.obj"

"$(OUTDIR)\CList.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MenuChooser - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MenuChooser\Release"
# PROP BASE Intermediate_Dir "MenuChooser\Release"
# PROP BASE Target_Dir "MenuChooser"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\MenuChooser"
# PROP Intermediate_Dir "c:\Integrator\Obj\MenuChooser"
# PROP Target_Dir "MenuChooser"
OUTDIR=c:\Integrator\Obj\MenuChooser
INTDIR=c:\Integrator\Obj\MenuChooser

ALL : "$(OUTDIR)\MenuChooser.lib" "c:\Integrator\Browse\MenuChooser.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\MenuChooser.bsc"
	-@erase "c:\Integrator\Obj\MenuChooser\MenuChooser.sbr"
	-@erase "c:\Integrator\Lib\MenuChooser.lib"
	-@erase "c:\Integrator\Obj\MenuChooser\MenuChooser.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/MenuChooser.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\MenuChooser/
CPP_SBRS=c:\Integrator\Obj\MenuChooser/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\MenuChooser.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\MenuChooser.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/MenuChooser.sbr"

"c:\Integrator\Browse\MenuChooser.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\MenuChooser.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\MenuChooser.lib" 
LIB32_OBJS= \
	"$(INTDIR)/MenuChooser.obj"

"$(OUTDIR)\MenuChooser.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MenuChooser - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MenuChooser\Debug"
# PROP BASE Intermediate_Dir "MenuChooser\Debug"
# PROP BASE Target_Dir "MenuChooser"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\MenuChooser"
# PROP Intermediate_Dir "c:\Integrator\Obj\MenuChooser"
# PROP Target_Dir "MenuChooser"
OUTDIR=c:\Integrator\Obj\MenuChooser
INTDIR=c:\Integrator\Obj\MenuChooser

ALL : "$(OUTDIR)\MenuChooser.lib" "c:\Integrator\Browse\MenuChooser.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\MenuChooser.bsc"
	-@erase "c:\Integrator\Obj\MenuChooser\MenuChooser.sbr"
	-@erase "c:\Integrator\Lib\MenuChooser.lib"
	-@erase "c:\Integrator\Obj\MenuChooser\MenuChooser.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/MenuChooser.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\MenuChooser/
CPP_SBRS=c:\Integrator\Obj\MenuChooser/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\MenuChooser.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\MenuChooser.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/MenuChooser.sbr"

"c:\Integrator\Browse\MenuChooser.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\Integrator\Lib\MenuChooser.lib"
LIB32_FLAGS=/nologo /out:"c:\Integrator\Lib\MenuChooser.lib" 
LIB32_OBJS= \
	"$(INTDIR)/MenuChooser.obj"

"$(OUTDIR)\MenuChooser.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDESpy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IDESpy\Release"
# PROP BASE Intermediate_Dir "IDESpy\Release"
# PROP BASE Target_Dir "IDESpy"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\IDESpy"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDESpy"
# PROP Target_Dir "IDESpy"
OUTDIR=c:\Integrator\Obj\IDESpy
INTDIR=c:\Integrator\Obj\IDESpy

ALL : "IDESpyLoader - Win32 Release" "$(OUTDIR)\IDESpy.dll"\
 "c:\Integrator\Browse\IDESpy.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\IDESpy.bsc"
	-@erase "c:\Integrator\Obj\IDESpy\idespy.sbr"
	-@erase "c:\Integrator\Obj\IDESpy\procs.sbr"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpyDDEServer.sbr"
	-@erase "c:\Integrator\Bin\IDESpy.dll"
	-@erase "c:\Integrator\Obj\IDESpy\idespy.obj"
	-@erase "c:\Integrator\Obj\IDESpy\procs.obj"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpyDDEServer.obj"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.res"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.lib"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDESpy.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDESpy/
CPP_SBRS=c:\Integrator\Obj\IDESpy/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IDESpy.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDESpy.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDESpy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/idespy.sbr" \
	"$(INTDIR)/procs.sbr" \
	"$(INTDIR)/IDESpyDDEServer.sbr"

"c:\Integrator\Browse\IDESpy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 IDEServerClient.lib MenuChooser.lib DisRegistry.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"c:\Integrator\Bin\IDESpy.dll"
LINK32_FLAGS=IDEServerClient.lib MenuChooser.lib DisRegistry.lib comctl32.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/IDESpy.pdb"\
 /machine:I386 /out:"c:\Integrator\Bin\IDESpy.dll"\
 /implib:"$(OUTDIR)/IDESpy.lib" 
LINK32_OBJS= \
	"$(INTDIR)/idespy.obj" \
	"$(INTDIR)/procs.obj" \
	"$(INTDIR)/IDESpyDDEServer.obj" \
	"$(INTDIR)/IDESpy.res"

"$(OUTDIR)\IDESpy.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDESpy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "IDESpy\Debug"
# PROP BASE Intermediate_Dir "IDESpy\Debug"
# PROP BASE Target_Dir "IDESpy"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\IDESpy"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDESpy"
# PROP Target_Dir "IDESpy"
OUTDIR=c:\Integrator\Obj\IDESpy
INTDIR=c:\Integrator\Obj\IDESpy

ALL : "IDESpyLoader - Win32 Debug" "$(OUTDIR)\IDESpy.dll"\
 "c:\Integrator\Browse\IDESpy.bsc"

CLEAN : 
	-@erase "c:\Integrator\Obj\IDESpy\vc40.pdb"
	-@erase "c:\Integrator\Obj\IDESpy\vc40.idb"
	-@erase "c:\Integrator\Browse\IDESpy.bsc"
	-@erase "c:\Integrator\Obj\IDESpy\idespy.sbr"
	-@erase "c:\Integrator\Obj\IDESpy\procs.sbr"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpyDDEServer.sbr"
	-@erase "c:\Integrator\Bin\IDESpy.dll"
	-@erase "c:\Integrator\Obj\IDESpy\idespy.obj"
	-@erase "c:\Integrator\Obj\IDESpy\procs.obj"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpyDDEServer.obj"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.res"
	-@erase "c:\Integrator\Bin\IDESpy.ilk"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.lib"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.exp"
	-@erase "c:\Integrator\Obj\IDESpy\IDESpy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDESpy.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=c:\Integrator\Obj\IDESpy/
CPP_SBRS=c:\Integrator\Obj\IDESpy/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/IDESpy.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDESpy.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDESpy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/idespy.sbr" \
	"$(INTDIR)/procs.sbr" \
	"$(INTDIR)/IDESpyDDEServer.sbr"

"c:\Integrator\Browse\IDESpy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 IDEServerClient.lib MenuChooser.lib DisRegistry.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"c:\Integrator\Bin\IDESpy.dll"
LINK32_FLAGS=IDEServerClient.lib MenuChooser.lib DisRegistry.lib comctl32.lib\
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib\
 shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/IDESpy.pdb" /debug\
 /machine:I386 /out:"c:\Integrator\Bin\IDESpy.dll"\
 /implib:"$(OUTDIR)/IDESpy.lib" 
LINK32_OBJS= \
	"$(INTDIR)/idespy.obj" \
	"$(INTDIR)/procs.obj" \
	"$(INTDIR)/IDESpyDDEServer.obj" \
	"$(INTDIR)/IDESpy.res"

"$(OUTDIR)\IDESpy.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDESpyLoader - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IDESpyLoader\Release"
# PROP BASE Intermediate_Dir "IDESpyLoader\Release"
# PROP BASE Target_Dir "IDESpyLoader"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\IDESpyLoader"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDESpyLoader"
# PROP Target_Dir "IDESpyLoader"
OUTDIR=c:\Integrator\Obj\IDESpyLoader
INTDIR=c:\Integrator\Obj\IDESpyLoader

ALL : "$(OUTDIR)\IDESpyLoader.exe" "c:\Integrator\Browse\IDESpyLoader.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\IDESpyLoader.bsc"
	-@erase "c:\Integrator\Obj\IDESpyLoader\hooker.sbr"
	-@erase "c:\Integrator\Bin\IDESpyLoader.exe"
	-@erase "c:\Integrator\Obj\IDESpyLoader\hooker.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDESpyLoader.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDESpyLoader/
CPP_SBRS=c:\Integrator\Obj\IDESpyLoader/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDESpyLoader.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDESpyLoader.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/hooker.sbr"

"c:\Integrator\Browse\IDESpyLoader.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"c:\Integrator\Bin\IDESpyLoader.exe"
LINK32_FLAGS=DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/IDESpyLoader.pdb" /machine:I386\
 /out:"c:\Integrator\Bin\IDESpyLoader.exe" 
LINK32_OBJS= \
	"$(INTDIR)/hooker.obj"

"$(OUTDIR)\IDESpyLoader.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDESpyLoader - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "IDESpyLoader\Debug"
# PROP BASE Intermediate_Dir "IDESpyLoader\Debug"
# PROP BASE Target_Dir "IDESpyLoader"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\IDESpyLoader"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDESpyLoader"
# PROP Target_Dir "IDESpyLoader"
OUTDIR=c:\Integrator\Obj\IDESpyLoader
INTDIR=c:\Integrator\Obj\IDESpyLoader

ALL : "$(OUTDIR)\IDESpyLoader.exe" "c:\Integrator\Browse\IDESpyLoader.bsc"

CLEAN : 
	-@erase "c:\Integrator\Obj\IDESpyLoader\vc40.pdb"
	-@erase "c:\Integrator\Obj\IDESpyLoader\vc40.idb"
	-@erase "c:\Integrator\Browse\IDESpyLoader.bsc"
	-@erase "c:\Integrator\Obj\IDESpyLoader\hooker.sbr"
	-@erase "c:\Integrator\Bin\IDESpyLoader.exe"
	-@erase "c:\Integrator\Obj\IDESpyLoader\hooker.obj"
	-@erase "c:\Integrator\Bin\IDESpyLoader.ilk"
	-@erase "c:\Integrator\Obj\IDESpyLoader\IDESpyLoader.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDESpyLoader.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDESpyLoader/
CPP_SBRS=c:\Integrator\Obj\IDESpyLoader/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDESpyLoader.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDESpyLoader.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/hooker.sbr"

"c:\Integrator\Browse\IDESpyLoader.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"c:\Integrator\Bin\IDESpyLoader.exe"
LINK32_FLAGS=DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/IDESpyLoader.pdb" /debug /machine:I386\
 /out:"c:\Integrator\Bin\IDESpyLoader.exe" 
LINK32_OBJS= \
	"$(INTDIR)/hooker.obj"

"$(OUTDIR)\IDESpyLoader.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDEConsole - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IDEConsole\Release"
# PROP BASE Intermediate_Dir "IDEConsole\Release"
# PROP BASE Target_Dir "IDEConsole"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\IDEConsole"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDEConsole"
# PROP Target_Dir "IDEConsole"
OUTDIR=c:\Integrator\Obj\IDEConsole
INTDIR=c:\Integrator\Obj\IDEConsole

ALL : "$(OUTDIR)\DEConsole.exe" "c:\Integrator\Browse\IDEConsole.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\IDEConsole.bsc"
	-@erase "c:\Integrator\Obj\IDEConsole\idecon.sbr"
	-@erase "c:\Integrator\Bin\DEConsole.exe"
	-@erase "c:\Integrator\Obj\IDEConsole\idecon.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDEConsole.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDEConsole/
CPP_SBRS=c:\Integrator\Obj\IDEConsole/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDEConsole.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDEConsole.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/idecon.sbr"

"c:\Integrator\Browse\IDEConsole.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"c:\Integrator\Bin\DEConsole.exe"
LINK32_FLAGS=IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console\
 /incremental:no /pdb:"$(OUTDIR)/DEConsole.pdb" /machine:I386\
 /out:"c:\Integrator\Bin\DEConsole.exe" 
LINK32_OBJS= \
	"$(INTDIR)/idecon.obj"

"$(OUTDIR)\DEConsole.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDEConsole - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "IDEConsole\Debug"
# PROP BASE Intermediate_Dir "IDEConsole\Debug"
# PROP BASE Target_Dir "IDEConsole"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\IDEConsole"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDEConsole"
# PROP Target_Dir "IDEConsole"
OUTDIR=c:\Integrator\Obj\IDEConsole
INTDIR=c:\Integrator\Obj\IDEConsole

ALL : "$(OUTDIR)\IDEConsole.exe" "c:\Integrator\Browse\IDEConsole.bsc"

CLEAN : 
	-@erase "c:\Integrator\Obj\IDEConsole\vc40.pdb"
	-@erase "c:\Integrator\Obj\IDEConsole\vc40.idb"
	-@erase "c:\Integrator\Browse\IDEConsole.bsc"
	-@erase "c:\Integrator\Obj\IDEConsole\idecon.sbr"
	-@erase "c:\Integrator\Bin\IDEConsole.exe"
	-@erase "c:\Integrator\Obj\IDEConsole\idecon.obj"
	-@erase "c:\Integrator\Bin\IDEConsole.ilk"
	-@erase "c:\Integrator\Obj\IDEConsole\IDEConsole.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDEConsole.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDEConsole/
CPP_SBRS=c:\Integrator\Obj\IDEConsole/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDEConsole.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDEConsole.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/idecon.sbr"

"c:\Integrator\Browse\IDEConsole.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"c:\Integrator\Bin\IDEConsole.exe"
LINK32_FLAGS=IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console\
 /incremental:yes /pdb:"$(OUTDIR)/IDEConsole.pdb" /debug /machine:I386\
 /out:"c:\Integrator\Bin\IDEConsole.exe" 
LINK32_OBJS= \
	"$(INTDIR)/idecon.obj"

"$(OUTDIR)\IDEConsole.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDEParamReciever - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IDEParamReciever\Release"
# PROP BASE Intermediate_Dir "IDEParamReciever\Release"
# PROP BASE Target_Dir "IDEParamReciever"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\IDEParamReciever"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDEParamReciever"
# PROP Target_Dir "IDEParamReciever"
OUTDIR=c:\Integrator\Obj\IDEParamReciever
INTDIR=c:\Integrator\Obj\IDEParamReciever

ALL : "$(OUTDIR)\IDEParamReciever.exe"\
 "c:\Integrator\Browse\IDEParamReciever.bsc"

CLEAN : 
	-@erase "c:\Integrator\Browse\IDEParamReciever.bsc"
	-@erase "c:\Integrator\Obj\IDEParamReciever\IDEParamReciever.sbr"
	-@erase "c:\Integrator\Bin\IDEParamReciever.exe"
	-@erase "c:\Integrator\Obj\IDEParamReciever\IDEParamReciever.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDEParamReciever.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDEParamReciever/
CPP_SBRS=c:\Integrator\Obj\IDEParamReciever/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDEParamReciever.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDEParamReciever.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/IDEParamReciever.sbr"

"c:\Integrator\Browse\IDEParamReciever.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"c:\Integrator\Bin\IDEParamReciever.exe"
LINK32_FLAGS=IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/IDEParamReciever.pdb" /machine:I386\
 /out:"c:\Integrator\Bin\IDEParamReciever.exe" 
LINK32_OBJS= \
	"$(INTDIR)/IDEParamReciever.obj"

"$(OUTDIR)\IDEParamReciever.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IDEParamReciever - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "IDEParamReciever\Debug"
# PROP BASE Intermediate_Dir "IDEParamReciever\Debug"
# PROP BASE Target_Dir "IDEParamReciever"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\IDEParamReciever"
# PROP Intermediate_Dir "c:\Integrator\Obj\IDEParamReciever"
# PROP Target_Dir "IDEParamReciever"
OUTDIR=c:\Integrator\Obj\IDEParamReciever
INTDIR=c:\Integrator\Obj\IDEParamReciever

ALL : "$(OUTDIR)\IDEParamReciever.exe"\
 "c:\Integrator\Browse\IDEParamReciever.bsc"

CLEAN : 
	-@erase "c:\Integrator\Obj\IDEParamReciever\vc40.pdb"
	-@erase "c:\Integrator\Obj\IDEParamReciever\vc40.idb"
	-@erase "c:\Integrator\Browse\IDEParamReciever.bsc"
	-@erase "c:\Integrator\Obj\IDEParamReciever\IDEParamReciever.sbr"
	-@erase "c:\Integrator\Bin\IDEParamReciever.exe"
	-@erase "c:\Integrator\Obj\IDEParamReciever\IDEParamReciever.obj"
	-@erase "c:\Integrator\Bin\IDEParamReciever.ilk"
	-@erase "c:\Integrator\Obj\IDEParamReciever\IDEParamReciever.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/IDEParamReciever.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\IDEParamReciever/
CPP_SBRS=c:\Integrator\Obj\IDEParamReciever/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\IDEParamReciever.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\IDEParamReciever.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/IDEParamReciever.sbr"

"c:\Integrator\Browse\IDEParamReciever.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\Integrator\Bin\IDEParamReciever.exe"
LINK32_FLAGS=IDEServerClient.lib DisRegistry.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/IDEParamReciever.pdb" /debug /machine:I386\
 /out:"c:\Integrator\Bin\IDEParamReciever.exe" 
LINK32_OBJS= \
	"$(INTDIR)/IDEParamReciever.obj"

"$(OUTDIR)\IDEParamReciever.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "Integrator - Win32 Release"
# Name "Integrator - Win32 Debug"

!IF  "$(CFG)" == "Integrator - Win32 Release"

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\readsock.cpp
DEP_CPP_READS=\
	".\integratorserver\readsock.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\readsock.obj" : $(SOURCE) $(DEP_CPP_READS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\readsock.sbr" : $(SOURCE) $(DEP_CPP_READS) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\ServerWin.cpp

!IF  "$(CFG)" == "Integrator - Win32 Release"

DEP_CPP_SERVE=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerWin.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerWin.sbr" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

DEP_CPP_SERVE=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerWin.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerWin.sbr" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\ServerQuery.cpp

!IF  "$(CFG)" == "Integrator - Win32 Release"

DEP_CPP_SERVER=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\mapnet.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerQuery.obj" : $(SOURCE) $(DEP_CPP_SERVER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerQuery.sbr" : $(SOURCE) $(DEP_CPP_SERVER) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

DEP_CPP_SERVER=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\mapnet.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerQuery.obj" : $(SOURCE) $(DEP_CPP_SERVER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerQuery.sbr" : $(SOURCE) $(DEP_CPP_SERVER) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\ServerIDEInfo.cpp

!IF  "$(CFG)" == "Integrator - Win32 Release"

DEP_CPP_SERVERI=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerIDEInfo.obj" : $(SOURCE) $(DEP_CPP_SERVERI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerIDEInfo.sbr" : $(SOURCE) $(DEP_CPP_SERVERI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

DEP_CPP_SERVERI=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerIDEInfo.obj" : $(SOURCE) $(DEP_CPP_SERVERI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerIDEInfo.sbr" : $(SOURCE) $(DEP_CPP_SERVERI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\ServerDDE.cpp

!IF  "$(CFG)" == "Integrator - Win32 Release"

DEP_CPP_SERVERD=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerDDE.obj" : $(SOURCE) $(DEP_CPP_SERVERD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerDDE.sbr" : $(SOURCE) $(DEP_CPP_SERVERD) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

DEP_CPP_SERVERD=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerDDE.obj" : $(SOURCE) $(DEP_CPP_SERVERD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerDDE.sbr" : $(SOURCE) $(DEP_CPP_SERVERD) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\ServerAdapter.cpp

!IF  "$(CFG)" == "Integrator - Win32 Release"

DEP_CPP_SERVERA=\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\tcl.h"\
	".\integratorserver\readsock.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerAdapter.obj" : $(SOURCE) $(DEP_CPP_SERVERA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerAdapter.sbr" : $(SOURCE) $(DEP_CPP_SERVERA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

DEP_CPP_SERVERA=\
	".\integratorserver\readsock.h"\
	".\integratorserver\server.h"\
	{$(INCLUDE)}"\clist.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\tcl.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ServerAdapter.obj" : $(SOURCE) $(DEP_CPP_SERVERA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ServerAdapter.sbr" : $(SOURCE) $(DEP_CPP_SERVERA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "CList"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"CList - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="CList - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"CList - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="CList - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "DisRegistry"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"DisRegistry - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="DisRegistry - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"DisRegistry - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="DisRegistry - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "IDEServerClient"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"IDEServerClient - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak\
 CFG="IDEServerClient - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"IDEServerClient - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak\
 CFG="IDEServerClient - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "MapNet"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"MapNet - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="MapNet - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"MapNet - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="MapNet - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "MenuChooser"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"MenuChooser - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="MenuChooser - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"MenuChooser - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="MenuChooser - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "IDESpy"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"IDESpy - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="IDESpy - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"IDESpy - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="IDESpy - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "IDEConsole"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"IDEConsole - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="IDEConsole - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"IDEConsole - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="IDEConsole - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "IDEParamReciever"

!IF  "$(CFG)" == "Integrator - Win32 Release"

"IDEParamReciever - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak\
 CFG="IDEParamReciever - Win32 Release" 

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

"IDEParamReciever - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak\
 CFG="IDEParamReciever - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\IntegratorServer\server.rc
DEP_RSC_SERVER_=\
	".\IntegratorServer\server.ico"\
	".\IntegratorServer\bitmap1.bmp"\
	

!IF  "$(CFG)" == "Integrator - Win32 Release"


"$(INTDIR)\server.res" : $(SOURCE) $(DEP_RSC_SERVER_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/server.res" /i "IntegratorServer" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"


"$(INTDIR)\server.res" : $(SOURCE) $(DEP_RSC_SERVER_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/server.res" /i "IntegratorServer" /d "_DEBUG"\
 $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\lvku\paraset\src\clients\integrator\msdev\lib\tcl74.lib

!IF  "$(CFG)" == "Integrator - Win32 Release"

!ELSEIF  "$(CFG)" == "Integrator - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "DisRegistry - Win32 Release"
# Name "DisRegistry - Win32 Debug"

!IF  "$(CFG)" == "DisRegistry - Win32 Release"

!ELSEIF  "$(CFG)" == "DisRegistry - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\DisRegistry\DisRegistry.cpp
DEP_CPP_DISRE=\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\DisRegistry.obj" : $(SOURCE) $(DEP_CPP_DISRE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DisRegistry.sbr" : $(SOURCE) $(DEP_CPP_DISRE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
################################################################################
# Begin Target

# Name "IDEServerClient - Win32 Release"
# Name "IDEServerClient - Win32 Debug"

!IF  "$(CFG)" == "IDEServerClient - Win32 Release"

!ELSEIF  "$(CFG)" == "IDEServerClient - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\IDEServerClient\IDEServerClient.cpp
DEP_CPP_IDESE=\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\IDEServerClient.obj" : $(SOURCE) $(DEP_CPP_IDESE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\IDEServerClient.sbr" : $(SOURCE) $(DEP_CPP_IDESE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
################################################################################
# Begin Target

# Name "MapNet - Win32 Release"
# Name "MapNet - Win32 Debug"

!IF  "$(CFG)" == "MapNet - Win32 Release"

!ELSEIF  "$(CFG)" == "MapNet - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\MapNet\mapnet.cpp
DEP_CPP_MAPNE=\
	{$(INCLUDE)}"\mapnet.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\mapnet.obj" : $(SOURCE) $(DEP_CPP_MAPNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\mapnet.sbr" : $(SOURCE) $(DEP_CPP_MAPNE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
################################################################################
# Begin Target

# Name "CList - Win32 Release"
# Name "CList - Win32 Debug"

!IF  "$(CFG)" == "CList - Win32 Release"

!ELSEIF  "$(CFG)" == "CList - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\CList\clist.cpp
DEP_CPP_CLIST=\
	{$(INCLUDE)}"\clist.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\clist.obj" : $(SOURCE) $(DEP_CPP_CLIST) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\clist.sbr" : $(SOURCE) $(DEP_CPP_CLIST) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
################################################################################
# Begin Target

# Name "MenuChooser - Win32 Release"
# Name "MenuChooser - Win32 Debug"

!IF  "$(CFG)" == "MenuChooser - Win32 Release"

!ELSEIF  "$(CFG)" == "MenuChooser - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\MenuChooser\MenuChooser.cpp

!IF  "$(CFG)" == "MenuChooser - Win32 Release"

DEP_CPP_MENUC=\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\MenuChooser.obj" : $(SOURCE) $(DEP_CPP_MENUC) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MenuChooser.sbr" : $(SOURCE) $(DEP_CPP_MENUC) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "MenuChooser - Win32 Debug"

DEP_CPP_MENUC=\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\MenuChooser.obj" : $(SOURCE) $(DEP_CPP_MENUC) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MenuChooser.sbr" : $(SOURCE) $(DEP_CPP_MENUC) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "IDESpy - Win32 Release"
# Name "IDESpy - Win32 Debug"

!IF  "$(CFG)" == "IDESpy - Win32 Release"

!ELSEIF  "$(CFG)" == "IDESpy - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\IDESpy\procs.cpp
DEP_CPP_PROCS=\
	{$(INCLUDE)}"\Integrator.h"\
	".\idespy\idespy.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\procs.obj" : $(SOURCE) $(DEP_CPP_PROCS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\procs.sbr" : $(SOURCE) $(DEP_CPP_PROCS) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IDESpy\IDESpyDDEServer.cpp
DEP_CPP_IDESP=\
	".\IDESpy\..\Include\integrator.h"\
	".\idespy\idespy.h"\
	".\Include\ErrMessages.h"\
	".\Include\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\IDESpyDDEServer.obj" : $(SOURCE) $(DEP_CPP_IDESP) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\IDESpyDDEServer.sbr" : $(SOURCE) $(DEP_CPP_IDESP) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\IDESpy\idespy.cpp
DEP_CPP_IDESPY=\
	{$(INCLUDE)}"\Integrator.h"\
	".\idespy\idespy.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\idespy.obj" : $(SOURCE) $(DEP_CPP_IDESPY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\idespy.sbr" : $(SOURCE) $(DEP_CPP_IDESPY) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "IDESpyLoader"

!IF  "$(CFG)" == "IDESpy - Win32 Release"

"IDESpyLoader - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="IDESpyLoader - Win32 Release" 

!ELSEIF  "$(CFG)" == "IDESpy - Win32 Debug"

"IDESpyLoader - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\integrator.mak CFG="IDESpyLoader - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\IDESpy\IDESpy.rc
DEP_RSC_IDESPY_=\
	".\IDESpy\toolbar2.bmp"\
	".\IDESpy\bitmap2.bmp"\
	

!IF  "$(CFG)" == "IDESpy - Win32 Release"


"$(INTDIR)\IDESpy.res" : $(SOURCE) $(DEP_RSC_IDESPY_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IDESpy.res" /i "IDESpy" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "IDESpy - Win32 Debug"


"$(INTDIR)\IDESpy.res" : $(SOURCE) $(DEP_RSC_IDESPY_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/IDESpy.res" /i "IDESpy" /d "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "IDESpyLoader - Win32 Release"
# Name "IDESpyLoader - Win32 Debug"

!IF  "$(CFG)" == "IDESpyLoader - Win32 Release"

!ELSEIF  "$(CFG)" == "IDESpyLoader - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\idespy\idespyloader\hooker.cpp
DEP_CPP_HOOKE=\
	{$(INCLUDE)}"\Integrator.h"\
	".\idespy\idespyloader\hooker.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\hooker.obj" : $(SOURCE) $(DEP_CPP_HOOKE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\hooker.sbr" : $(SOURCE) $(DEP_CPP_HOOKE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
################################################################################
# Begin Target

# Name "IDEConsole - Win32 Release"
# Name "IDEConsole - Win32 Debug"

!IF  "$(CFG)" == "IDEConsole - Win32 Release"

!ELSEIF  "$(CFG)" == "IDEConsole - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\IDEConsole\idecon.cpp

!IF  "$(CFG)" == "IDEConsole - Win32 Release"

DEP_CPP_IDECO=\
	".\ideconsole\idecon.h"\
	{$(INCLUDE)}"\Integrator.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\idecon.obj" : $(SOURCE) $(DEP_CPP_IDECO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\idecon.sbr" : $(SOURCE) $(DEP_CPP_IDECO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "IDEConsole - Win32 Debug"

DEP_CPP_IDECO=\
	".\ideconsole\idecon.h"\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\idecon.obj" : $(SOURCE) $(DEP_CPP_IDECO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\idecon.sbr" : $(SOURCE) $(DEP_CPP_IDECO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "IDEParamReciever - Win32 Release"
# Name "IDEParamReciever - Win32 Debug"

!IF  "$(CFG)" == "IDEParamReciever - Win32 Release"

!ELSEIF  "$(CFG)" == "IDEParamReciever - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\IDEParamReciever\IDEParamReciever.cpp

!IF  "$(CFG)" == "IDEParamReciever - Win32 Release"

DEP_CPP_IDEPA=\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\Integrator.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\IDEParamReciever.obj" : $(SOURCE) $(DEP_CPP_IDEPA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\IDEParamReciever.sbr" : $(SOURCE) $(DEP_CPP_IDEPA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "IDEParamReciever - Win32 Debug"

DEP_CPP_IDEPA=\
	{$(INCLUDE)}"\Integrator.h"\
	{$(INCLUDE)}"\ErrMessages.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\IDEParamReciever.obj" : $(SOURCE) $(DEP_CPP_IDEPA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\IDEParamReciever.sbr" : $(SOURCE) $(DEP_CPP_IDEPA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
