# Microsoft Developer Studio Project File - Name="client" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=client - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "client.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "client.mak" CFG="client - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "client - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "client - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "client"
# PROP Scc_LocalPath "..\..\..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "client - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".\include" /I "..\include" /I "../../dislite/include" /I "$(PATH2BS)/dislite/include" /I "../../dish/include" /I "$(PATH2BS)/dish/include" /I "../../nameserver/include/" /I "$(PATH2BS)/nameserver/include/" /I "../../DevXLauncher/include" /I "$(PATH2BS)/DevXLauncher/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 Ws2_32.lib Advapi32.lib User32.lib /nologo /subsystem:console /machine:I386 /out:"Release/emacsclient.exe"

!ELSEIF  "$(CFG)" == "client - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "..\..\src\\" /I ".\include" /I "..\include" /I "../../dislite/include" /I "$(PATH2BS)/dislite/include" /I "../../dish/include" /I "$(PATH2BS)/dish/include" /I "../../nameserver/include/" /I "$(PATH2BS)/nameserver/include/" /I "../../DevXLauncher/include" /I "$(PATH2BS)/DevXLauncher/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ws2_32.lib Advapi32.lib User32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/emacsclient.exe" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "client - Win32 Release"
# Name "client - Win32 Debug"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\connect_mgr.h
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\DevXLauncher\include\debug.h"
# End Source File
# Begin Source File

SOURCE=.\include\editor_api.h
# End Source File
# Begin Source File

SOURCE=..\include\lib.h
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\DevXLauncher\include\startproc.h"
# End Source File
# End Group
# Begin Group "external_src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\CommProto.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\nameserver\src\nameServCalls.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\nameserver\src\winmain.cxx"
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\dish\src\SocketComm.cxx"
# End Source File
# End Group
# Begin Group "Common Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\connect_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\driver.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lib.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\DevXLauncher\src\startproc.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\emacs_api.cpp
# End Source File
# End Target
# End Project
