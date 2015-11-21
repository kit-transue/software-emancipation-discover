# Microsoft Developer Studio Project File - Name="log_presentation" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=log_presentation - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "log_presentation.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "log_presentation.mak" CFG="log_presentation - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "log_presentation - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "log_presentation - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "log_presentation"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "log_presentation - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /I "$(BUILD_SRC_ROOT)\stream_message\include" /I "$(BUILD_SRC_ROOT)\nameserver\include" /I "$(BUILD_SRC_ROOT)\thirdparty\xml4c\src" /D "NDEBUG" /D "WIN32" /D _WIN32_WINNT=0x0400 /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /D "TRANSPORT_SERVER" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "log_presentation - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "include" /I "$(BUILD_SRC_ROOT)\stream_message\include" /I "$(BUILD_SRC_ROOT)\nameserver\include" /I "$(BUILD_SRC_ROOT)\thirdparty\xml4c\src" /D "_DEBUG" /D "WIN32" /D _WIN32_WINNT=0x0400 /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /D "TRANSPORT_SERVER" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "log_presentation - Win32 Release"
# Name "log_presentation - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\filtersev.cxx
# End Source File
# Begin Source File

SOURCE=.\src\logpresent.cxx
# End Source File
# Begin Source File

SOURCE=.\src\msgreceiver.cxx
# End Source File
# Begin Source File

SOURCE=.\src\msgsender.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\filtersev.h
# End Source File
# Begin Source File

SOURCE=.\include\logpresent.h
# End Source File
# Begin Source File

SOURCE=.\include\lpinterface.h
# End Source File
# Begin Source File

SOURCE=.\include\msgreceiver.h
# End Source File
# Begin Source File

SOURCE=.\include\msgsender.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "MBDriver Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\aconnectn.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\mbmsgser.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\mbuildmsg.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\minidom.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\minisax.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\minixml.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\msg_comms.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\msg_format.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\nameserver\src\nameServCalls.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\service.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\startprocess.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\threads.cxx
# End Source File
# Begin Source File

SOURCE=$(BUILD_SRC_ROOT)\stream_message\src\transport.cxx
# End Source File
# End Group
# End Target
# End Project
