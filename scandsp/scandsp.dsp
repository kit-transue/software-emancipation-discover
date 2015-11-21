# Microsoft Developer Studio Project File - Name="scandsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=scandsp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scandsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scandsp.mak" CFG="scandsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scandsp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "scandsp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "scandsp"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scandsp - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "n:\paraset\src\scandsp\include" /I "n:\paraset\src\pdfwiz\parsemake" /I "n:\paraset\src\stream_message\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "scandsp - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "n:\paraset\src\scandsp\include" /I "n:\paraset\src\pdfwiz\parsemake" /I "n:\paraset\src\stream_message\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ISO_CPP_HEADERS" /YX /FD /GZ /c
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

# Name "scandsp - Win32 Release"
# Name "scandsp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\paraset\src\scandsp\Windows\circlebuf.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\dspparser.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\dswparser.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\entity.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\line.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\main.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\scandsp\Windows\nametracker.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\scandsp\Windows\optparser.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\parseline.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\parser.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\pdfwiz\parsemake\registry_if.cxx
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\Windows\token.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\paraset\src\scandsp\include\circlebuf.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\dspparser.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\dswparser.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\entity.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\line.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\scandsp\include\nametracker.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\scandsp\include\optparser.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\parseline.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\parser.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\pdfwiz\parsemake\registry_if.h
# End Source File
# Begin Source File

SOURCE=N:\paraset\src\scandsp\include\token.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "MBDriver Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\aconnectn.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\mbmsgser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\mbuildmsg.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\minidom.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\minisax.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\minixml.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\msg_comms.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\msg_format.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\nameserver\api\nameServCalls.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\service.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\startprocess.cxx
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\src\transport.cxx
# End Source File
# End Group
# Begin Group "MBDriver Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\aconnectn.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\mbmsgser.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\mbuildmsg.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\msg.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\msg_comms.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\msg_format.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\nameserver\include\nameServCalls.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\service.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\startprocess.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\tptinterfaces.h
# End Source File
# Begin Source File

SOURCE=..\..\paraset\src\stream_message\include\transport.h
# End Source File
# End Group
# End Target
# End Project
