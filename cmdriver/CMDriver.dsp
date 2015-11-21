# Microsoft Developer Studio Project File - Name="CMDriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=CMDriver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CMDriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CMDriver.mak" CFG="CMDriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CMDriver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "CMDriver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "CMDriver"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CMDriver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\CmXml\include" /I "$(BUILD_SRC_ROOT)\thirdparty\xml4c\src" /I "$(BUILD_SRC_ROOT)/nameserver/include" /I "$(BUILD_SRC_ROOT)/DevXLauncher/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 advapi32.lib wsock32.lib xerces-c_1.lib  /nologo /subsystem:console /machine:I386 /libpath:"..\thirdparty\xerces\NT"

!ELSEIF  "$(CFG)" == "CMDriver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\CmXml\include" /I "$(BUILD_SRC_ROOT)\thirdparty\xml4c\src" /I "$(BUILD_SRC_ROOT)/nameserver/include" /I "$(BUILD_SRC_ROOT)/DevXLauncher/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib wsock32.lib xerces-c_1D.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\thirdparty\xerces\NT"

!ENDIF 

# Begin Target

# Name "CMDriver - Win32 Release"
# Name "CMDriver - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CmXml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CmXml\src\CmXml.cpp
# End Source File
# Begin Source File

SOURCE=.\CmXml\src\CmXmlCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\CmXml\src\CmXmlErrorHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\CmXml\src\CmXmlException.cpp
# End Source File
# Begin Source File

SOURCE=.\CmXml\src\CmXmlNode.cpp
# End Source File
# Begin Source File

SOURCE=.\CmXml\src\CmXmlStringTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\CmXml\src\CmXmlSystem.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\CMDriver.cxx
# End Source File
# Begin Source File

SOURCE=q:\nameserver\src\nameServCalls.cxx
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\DevXLauncher\src\startproc.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "CmXml_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CmXml\include\CmXml.h
# End Source File
# Begin Source File

SOURCE=.\CmXml\include\CmXmlCollection.h
# End Source File
# Begin Source File

SOURCE=.\CmXml\include\CmXmlErrorHandler.h
# End Source File
# Begin Source File

SOURCE=.\CmXml\include\CmXmlException.h
# End Source File
# Begin Source File

SOURCE=.\CmXml\include\CmXmlNode.h
# End Source File
# Begin Source File

SOURCE=.\CmXml\include\CmXmlStringTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\CmXml\include\CmXmlSystem.h
# End Source File
# End Group
# Begin Source File

SOURCE="$(PATH2BS)\DevXLauncher\include\debug.h"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\DevXLauncher\include\startproc.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
