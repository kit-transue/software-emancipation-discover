# Microsoft Developer Studio Project File - Name="pdf_wizard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=pdf_wizard - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pdf_wizard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pdf_wizard.mak" CFG="pdf_wizard - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pdf_wizard - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "pdf_wizard - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pdf_wizard - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../parsemake ../messages" /I "../parsemake" /I "../messages" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "pdf_wizard - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "../parsemake" /I "../messages" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../messages/gui_message.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "pdf_wizard - Win32 Release"
# Name "pdf_wizard - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\doneproppage.cpp
# End Source File
# Begin Source File

SOURCE=.\messagedialog.cpp
# End Source File
# Begin Source File

SOURCE=.\msg_gui.cxx
# End Source File
# Begin Source File

SOURCE=.\parsemake_if.cxx
# End Source File
# Begin Source File

SOURCE=.\parseprogressproppage.cpp
# End Source File
# Begin Source File

SOURCE=.\pdf_wizard.cpp
# End Source File
# Begin Source File

SOURCE=.\pdf_wizard.rc
# End Source File
# Begin Source File

SOURCE=.\pdf_wizardDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectPropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\PW_propSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\settingsdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StoragePropPage.cpp
# End Source File
# Begin Source File

SOURCE=.\TargetsPropPage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\doneproppage.h
# End Source File
# Begin Source File

SOURCE=.\messagedialog.h
# End Source File
# Begin Source File

SOURCE=.\msg_gui.h
# End Source File
# Begin Source File

SOURCE=.\parsemake_if.h
# End Source File
# Begin Source File

SOURCE=.\parseprogressproppage.h
# End Source File
# Begin Source File

SOURCE=.\pdf_wizard.h
# End Source File
# Begin Source File

SOURCE=.\pdf_wizardDlg.h
# End Source File
# Begin Source File

SOURCE=.\ProjectPropPage.h
# End Source File
# Begin Source File

SOURCE=.\PW_propSheet.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\settingsdialog.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StoragePropPage.h
# End Source File
# Begin Source File

SOURCE=.\TargetsPropPage.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pdf_wizard.ico
# End Source File
# Begin Source File

SOURCE=.\res\pdf_wizard.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=..\messages\gui_message.lib
# End Source File
# Begin Source File

SOURCE=..\parsemake\parsemake.lib
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
