# Microsoft Developer Studio Project File - Name="ServerStart" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ServerStart - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "serverstart.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "serverstart.mak" CFG="ServerStart - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ServerStart - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ServerStart - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ServerStart"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ServerStart - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\gala\extern\tset\include" /I "..\gala\extern\osport\include" /I "..\gala\extern\ads\include" /I "..\nameserver\include" /I "..\dish\include" /I "$(PATH2BS)\gala\extern\tset\include" /I "$(PATH2BS)\gala\extern\osport\include" /I "$(PATH2BS)\gala\extern\ads\include" /I "$(PATH2BS)\nameserver\include" /I "$(PATH2BS)\dish\include" /I ".\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "PCxOS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "ServerStart - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm- /GX /ZI /Od /I "../gala/extern/osport/include" /I "../gala/ads/osport/include" /I "..\gala\extern\tset\include" /I "..\gala\extern\osport\include" /I "..\gala\extern\ads\include" /I "..\nameserver\include" /I "..\dish\include" /I "$(PATH2BS)\gala\extern\tset\include" /I "$(PATH2BS)\gala\extern\osport\include" /I "$(PATH2BS)\gala\extern\ads\include" /I "$(PATH2BS)\nameserver\include" /I "$(PATH2BS)\dish\include" /I ".\include" /D "_DEBUG" /D "PCxOS" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 WSock32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ServerStart - Win32 Release"
# Name "ServerStart - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\bitMEM.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\tset\settings\item.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\nameserver\src\nameServCalls.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\ads\src\parray.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\pdufile.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\pduio.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\pdumem.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\pdupath.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\pdustring.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\tset\settings\persistent.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\raFile.cxx"
# End Source File
# Begin Source File

SOURCE=.\src\ServerStart.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerStart.rc
# End Source File
# Begin Source File

SOURCE=.\src\ServerStartDlg.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\tset\settings\settings.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\dish\src\SocketComm.cxx"
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="$(PATH2BS)\nameserver\include\nameServCalls.h"
# End Source File
# Begin Source File

SOURCE=.\include\Resource.h
# End Source File
# Begin Source File

SOURCE=.\include\ServerStart.h
# End Source File
# Begin Source File

SOURCE=.\include\ServerStartDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\ServerStart.ico
# End Source File
# Begin Source File

SOURCE=.\res\ServerStart.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
