# Microsoft Developer Studio Project File - Name="QADriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=QADriver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QAEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QAEngine.mak" CFG="QADriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QADriver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "QADriver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "QAEngine"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QADriver - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "n:\paraset\src\stream_message\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "QADriver - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "n:\paraset\src\stream_message\include" /D "_DEBUG" /D "ISO_CPP_HEADERS" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "QADriver - Win32 Release"
# Name "QADriver - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Windows\BatchBuild.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\ClientInterface.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\Hit.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\HitsTable.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\nameserver\api\nameServCalls.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\PolicyTree.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\ProjectTree.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\ProjectTreeNode.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\QAEngine.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\ServerInterface.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\dish\src\SocketComm.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\TaskDriver.cxx
# End Source File
# Begin Source File

SOURCE=.\Windows\XMLTree.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Windows\BatchBuild.h
# End Source File
# Begin Source File

SOURCE=.\Windows\ClientInterface.h
# End Source File
# Begin Source File

SOURCE=.\Windows\globals.h
# End Source File
# Begin Source File

SOURCE=.\Windows\Hit.h
# End Source File
# Begin Source File

SOURCE=.\Windows\HitsTable.h
# End Source File
# Begin Source File

SOURCE=.\Windows\PolicyTree.h
# End Source File
# Begin Source File

SOURCE=.\Windows\ProjectTree.h
# End Source File
# Begin Source File

SOURCE=.\Windows\ProjectTreeNode.h
# End Source File
# Begin Source File

SOURCE=.\Windows\ServerInterface.h
# End Source File
# Begin Source File

SOURCE=.\Windows\TaskDriver.h
# End Source File
# Begin Source File

SOURCE=.\Windows\XMLTree.h
# End Source File
# End Group
# Begin Group "Messaging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\stream_message\src\aconnectn.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\mbmsgser.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\mbuildmsg.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\minidom.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\minisax.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\minixml.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\msg_comms.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\msg_format.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\service.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\startprocess.cxx
# End Source File
# Begin Source File

SOURCE=..\stream_message\src\transport.cxx
# End Source File
# End Group
# End Target
# End Project
