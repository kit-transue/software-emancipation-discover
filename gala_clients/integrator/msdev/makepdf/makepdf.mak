# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=PDFOpt - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to PDFOpt - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "MakePDF - Win32 Debug" && "$(CFG)" != "PDFOpt - Win32 Release"\
 && "$(CFG)" != "PDFOpt - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "makepdf.mak" CFG="PDFOpt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MakePDF - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "PDFOpt - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PDFOpt - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "MakePDF - Win32 Debug"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "MakePDF - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\MakePDF"
# PROP Intermediate_Dir "c:\Integrator\Obj\MakePDF"
# PROP Target_Dir ""
OUTDIR=c:\Integrator\Obj\MakePDF
INTDIR=c:\Integrator\Obj\MakePDF

ALL : "PDFOpt - Win32 Debug" "$(OUTDIR)\MakePDF.exe"

CLEAN : 
	-@erase "c:\Integrator\Bin\MakePDF.exe"
	-@erase "c:\Integrator\Obj\MakePDF\MakePDF.obj"
	-@erase "c:\Integrator\Obj\MakePDF\TargetNode.obj"
	-@erase "c:\Integrator\Obj\MakePDF\TargetNodeFiles.obj"
	-@erase "c:\Integrator\Obj\MakePDF\TargetNodeOptions.obj"
	-@erase "c:\Integrator\Obj\MakePDF\TargetNodeTree.obj"
	-@erase "c:\Integrator\Obj\MakePDF\WritePDF.obj"
	-@erase "c:\Integrator\Obj\MakePDF\TargetFileH.obj"
	-@erase "c:\Integrator\Obj\MakePDF\MakePDFCommon.obj"
	-@erase "c:\Integrator\Obj\MakePDF\ProcessNMake.obj"
	-@erase "c:\Integrator\Bin\MakePDF.ilk"
	-@erase "c:\Integrator\Obj\MakePDF\MakePDF.pdb"
	-@erase "c:\Integrator\Obj\MakePDF\vc40.pdb"
	-@erase "c:\Integrator\Obj\MakePDF\vc40.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/makepdf.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\MakePDF/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\MakePDF.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\MakePDF.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 DisRegistry.lib MapNet.lib netapi32.lib mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"c:\Integrator\Bin\MakePDF.exe"
LINK32_FLAGS=DisRegistry.lib MapNet.lib netapi32.lib mpr.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/MakePDF.pdb" /debug\
 /machine:I386 /out:"c:\Integrator\Bin\MakePDF.exe" 
LINK32_OBJS= \
	"$(INTDIR)/MakePDF.obj" \
	"$(INTDIR)/TargetNode.obj" \
	"$(INTDIR)/TargetNodeFiles.obj" \
	"$(INTDIR)/TargetNodeOptions.obj" \
	"$(INTDIR)/TargetNodeTree.obj" \
	"$(INTDIR)/WritePDF.obj" \
	"$(INTDIR)/TargetFileH.obj" \
	"$(INTDIR)/MakePDFCommon.obj" \
	"$(INTDIR)/ProcessNMake.obj" \
	"..\lib\mapnet.lib" \
	"..\lib\disregistry.lib"

"$(OUTDIR)\MakePDF.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PDFOpt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PDFOpt\Release"
# PROP BASE Intermediate_Dir "PDFOpt\Release"
# PROP BASE Target_Dir "PDFOpt"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\Integrator\Obj\Release"
# PROP Intermediate_Dir "c:\Integrator\Obj\Release"
# PROP Target_Dir "PDFOpt"
OUTDIR=c:\Integrator\Obj\Release
INTDIR=c:\Integrator\Obj\Release

ALL : "$(OUTDIR)\PDFOpt.exe"

CLEAN : 
	-@erase "c:\Integrator\Bin\PDFOpt.exe"
	-@erase "c:\Integrator\Obj\Release\PDFOpt.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/PDFOpt.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=c:\Integrator\Obj\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\PDFOpt.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\PDFOpt.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"c:\Integrator\Bin\PDFOpt.exe"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/PDFOpt.pdb" /machine:I386 /out:"c:\Integrator\Bin\PDFOpt.exe" 
LINK32_OBJS= \
	"$(INTDIR)/PDFOpt.obj"

"$(OUTDIR)\PDFOpt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PDFOpt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PDFOpt\Debug"
# PROP BASE Intermediate_Dir "PDFOpt\Debug"
# PROP BASE Target_Dir "PDFOpt"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\Integrator\Obj\PDFOpt"
# PROP Intermediate_Dir "c:\Integrator\Obj\PDFOpt"
# PROP Target_Dir "PDFOpt"
OUTDIR=c:\Integrator\Obj\PDFOpt
INTDIR=c:\Integrator\Obj\PDFOpt

ALL : "$(OUTDIR)\PDFOpt.exe" "c:\Integrator\Browse\PDFOpt.bsc"

CLEAN : 
	-@erase "c:\Integrator\Bin\PDFOpt.exe"
	-@erase "c:\Integrator\Obj\PDFOpt\PDFOpt.obj"
	-@erase "c:\Integrator\Bin\PDFOpt.ilk"
	-@erase "c:\Integrator\Browse\PDFOpt.bsc"
	-@erase "c:\Integrator\Obj\PDFOpt\PDFOpt.sbr"
	-@erase "c:\Integrator\Obj\PDFOpt\PDFOpt.pdb"
	-@erase "c:\Integrator\Obj\PDFOpt\vc40.pdb"
	-@erase "c:\Integrator\Obj\PDFOpt\vc40.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/PDFOpt.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=c:\Integrator\Obj\PDFOpt/
CPP_SBRS=c:\Integrator\Obj\PDFOpt/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"c:\Integrator\Browse\PDFOpt.bsc"
BSC32_FLAGS=/nologo /o"c:\Integrator\Browse\PDFOpt.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/PDFOpt.sbr"

"c:\Integrator\Browse\PDFOpt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"c:\Integrator\Bin\PDFOpt.exe"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/PDFOpt.pdb" /debug /machine:I386\
 /out:"c:\Integrator\Bin\PDFOpt.exe" 
LINK32_OBJS= \
	"$(INTDIR)/PDFOpt.obj"

"$(OUTDIR)\PDFOpt.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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

################################################################################
# Begin Target

# Name "MakePDF - Win32 Debug"
################################################################################
# Begin Source File

SOURCE=.\MakePDF.cpp
DEP_CPP_MAKEP=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\mapnet.h"\
	

"$(INTDIR)\MakePDF.obj" : $(SOURCE) $(DEP_CPP_MAKEP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TargetNode.cpp
DEP_CPP_TARGE=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\TargetNode.obj" : $(SOURCE) $(DEP_CPP_TARGE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TargetNodeFiles.cpp
DEP_CPP_TARGET=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\TargetNodeFiles.obj" : $(SOURCE) $(DEP_CPP_TARGET) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TargetNodeOptions.cpp
DEP_CPP_TARGETN=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\TargetNodeOptions.obj" : $(SOURCE) $(DEP_CPP_TARGETN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TargetNodeTree.cpp
DEP_CPP_TARGETNO=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\TargetNodeTree.obj" : $(SOURCE) $(DEP_CPP_TARGETNO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\WritePDF.cpp
DEP_CPP_WRITE=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\WritePDF.obj" : $(SOURCE) $(DEP_CPP_WRITE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TargetFileH.cpp
DEP_CPP_TARGETF=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\TargetFileH.obj" : $(SOURCE) $(DEP_CPP_TARGETF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MakePDFCommon.cpp
DEP_CPP_MAKEPD=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	{$(INCLUDE)}"\mapnet.h"\
	

"$(INTDIR)\MakePDFCommon.obj" : $(SOURCE) $(DEP_CPP_MAKEPD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "PDFOpt"

!IF  "$(CFG)" == "MakePDF - Win32 Debug"

"PDFOpt - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F .\makepdf.mak CFG="PDFOpt - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ProcessNMake.cpp
DEP_CPP_PROCE=\
	".\MakePDF.h"\
	{$(INCLUDE)}"\DisRegistry.h"\
	

"$(INTDIR)\ProcessNMake.obj" : $(SOURCE) $(DEP_CPP_PROCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=\lvku\paraset\src\clients\integrator\msdev\lib\mapnet.lib
# End Source File
################################################################################
# Begin Source File

SOURCE=\lvku\paraset\src\clients\integrator\msdev\lib\disregistry.lib
# End Source File
# End Target
################################################################################
# Begin Target

# Name "PDFOpt - Win32 Release"
# Name "PDFOpt - Win32 Debug"

!IF  "$(CFG)" == "PDFOpt - Win32 Release"

!ELSEIF  "$(CFG)" == "PDFOpt - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\PDFOpt\PDFOpt.cpp

!IF  "$(CFG)" == "PDFOpt - Win32 Release"


"$(INTDIR)\PDFOpt.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PDFOpt - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\PDFOpt.obj" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\PDFOpt.sbr" : $(SOURCE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
