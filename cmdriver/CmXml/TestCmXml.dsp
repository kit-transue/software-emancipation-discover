# Microsoft Developer Studio Project File - Name="TestCmXml" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=TestCmXml - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TestCmXml.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TestCmXml.mak" CFG="TestCmXml - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TestCmXml - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "TestCmXml - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "TestCmXml"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TestCmXml - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "N:\paraset\src\xml4c\src" /I "N:\paraset\src\DeveloperXPress" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib xerces-c_1.lib /nologo /subsystem:console /machine:I386 /libpath:"N:\paraset\src\xml4c\Build\Win32\VC6\Release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cp N:\paraset\src\xml4c\Build\Win32\VC6\Release\xerces-c_1_2.dll .\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "TestCmXml - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "N:\paraset\src\xml4c\src" /I "N:\paraset\src\DeveloperXPress" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib xerces-c_1D.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"N:\paraset\src\xml4c\Build\Win32\VC6\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cp N:\paraset\src\xml4c\Build\Win32\VC6\Debug\xerces-c_1_2D.dll .\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "TestCmXml - Win32 Release"
# Name "TestCmXml - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\CmXml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CmXmlCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CmXmlErrorHandler.cpp

!IF  "$(CFG)" == "TestCmXml - Win32 Release"

# PROP Intermediate_Dir "Release\Obj"

!ELSEIF  "$(CFG)" == "TestCmXml - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\CmXmlException.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CmXmlNode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CmXmlStringTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CmXmlSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TestCmXml.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\CmXml.h
# End Source File
# Begin Source File

SOURCE=.\include\CmXmlCollection.h
# End Source File
# Begin Source File

SOURCE=.\include\CmXmlErrorHandler.h

!IF  "$(CFG)" == "TestCmXml - Win32 Release"

# PROP Intermediate_Dir "Release\Obj"

!ELSEIF  "$(CFG)" == "TestCmXml - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\include\CmXmlException.h
# End Source File
# Begin Source File

SOURCE=.\include\CmXmlNode.h
# End Source File
# Begin Source File

SOURCE=.\include\CmXmlStringTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\include\CmXmlSystem.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
