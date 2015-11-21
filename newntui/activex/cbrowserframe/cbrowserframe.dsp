# Microsoft Developer Studio Project File - Name="CBrowserFrame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CBrowserFrame - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CBrowserFrame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CBrowserFrame.mak" CFG="CBrowserFrame - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CBrowserFrame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CBrowserFrame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "CBrowserFrame"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CBrowserFrame - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /I "..\..\CommonResources" /I "..\..\CommonResources\include" /I "..\..\DiscoverMDI\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /dll /machine:I386
# Begin Custom Build - Registering OLE control...
OutDir=.\Release
TargetPath=.\Release\CBrowserFrame.ocx
InputPath=.\Release\CBrowserFrame.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "CBrowserFrame - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\include" /I "..\..\CommonResources" /I "..\..\CommonResources\include" /I "..\..\DiscoverMDI\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /dll /incremental:no /debug /machine:I386
# Begin Custom Build - Registering OLE control...
OutDir=.\Debug
TargetPath=.\Debug\CBrowserFrame.ocx
InputPath=.\Debug\CBrowserFrame.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "CBrowserFrame - Win32 Release"
# Name "CBrowserFrame - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\src\AddSelectorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\CommonResources\Src\AttributesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CBrowserFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\CBrowserFrame.def
# End Source File
# Begin Source File

SOURCE=.\CBrowserFrame.odl
# End Source File
# Begin Source File

SOURCE=.\CBrowserFrame.rc
# End Source File
# Begin Source File

SOURCE=.\src\CBrowserFrameCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CBrowserFramePpg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\CommonResources\Src\ComplexSort.cpp
# End Source File
# Begin Source File

SOURCE=.\src\datasource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\CommonResources\Src\FilterList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\CommonResources\Src\FilterSheet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\CommonResources\Src\FilterWizard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\groupscombo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GroupsPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\discovermdi\src\newfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\queryresults.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\src\spreadsheet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\src\SubsystemsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tabselector.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\include\AddSelectorDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\addselectordlg.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\attributesdialog.h
# End Source File
# Begin Source File

SOURCE=.\include\CBrowserFrameCtl.h
# End Source File
# Begin Source File

SOURCE=.\include\CBrowserFramePpg.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\complexsort.h
# End Source File
# Begin Source File

SOURCE=.\include\datasource.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\filterdialog.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\filterlist.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\filtersheet.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\filterwizard.h
# End Source File
# Begin Source File

SOURCE=.\include\groupscombo.h
# End Source File
# Begin Source File

SOURCE=.\include\GroupsPage.h
# End Source File
# Begin Source File

SOURCE=..\..\commonresources\include\groupspage.h
# End Source File
# Begin Source File

SOURCE=.\include\newsubsystemdialog.h
# End Source File
# Begin Source File

SOURCE=.\include\QueryResults.h
# End Source File
# Begin Source File

SOURCE=.\include\resource.h
# End Source File
# Begin Source File

SOURCE=.\include\spreadsheet.h
# End Source File
# Begin Source File

SOURCE=.\include\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\include\SubsystemsPage.h
# End Source File
# Begin Source File

SOURCE=.\include\tabselector.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\And.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ArrowDown.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ArrowLeft.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ArrowRight.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ArrowUp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CBrowserFrame.ico
# End Source File
# Begin Source File

SOURCE=.\res\CBrowserFrameCtl.bmp
# End Source File
# Begin Source File

SOURCE=..\..\DiscoverMDI\res\DiscoverMDIDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\FilterList.bmp
# End Source File
# Begin Source File

SOURCE=.\res\FilterWizard.bmp
# End Source File
# Begin Source File

SOURCE=..\..\DiscoverMDI\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\..\DiscoverMDI\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Or.bmp
# End Source File
# End Group
# End Target
# End Project
# Section CBrowserFrame : {ADC2DFD4-1E61-11D2-AEF3-00A0C9B71DC4}
# 	0:15:TabSelector.cpp:C:\NewDiscover\ActiveX\CBrowserFrame\TabSelector.cpp
# 	0:13:TabSelector.h:C:\NewDiscover\ActiveX\CBrowserFrame\TabSelector.h
# 	2:21:DefaultSinkHeaderFile:tabselector.h
# 	2:16:DefaultSinkClass:CTabSelector
# End Section
# Section CBrowserFrame : {26C03863-0D33-11D2-AEE9-00A0C9B71DC4}
# 	0:16:QueryResults.cpp:C:\NewDiscover\ActiveX\CBrowserFrame\QueryResults.cpp
# 	0:14:QueryResults.h:C:\NewDiscover\ActiveX\CBrowserFrame\QueryResults.h
# 	2:21:DefaultSinkHeaderFile:queryresults.h
# 	2:16:DefaultSinkClass:CQueryResults
# End Section
# Section CBrowserFrame : {1487DD52-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	2:5:Class:CGroupsCombo
# 	2:10:HeaderFile:groupscombo.h
# 	2:8:ImplFile:groupscombo.cpp
# End Section
# Section CBrowserFrame : {077B20DC-0934-11D2-AEE6-00A0C9B71DC4}
# 	2:5:Class:CDataSource
# 	2:10:HeaderFile:datasource.h
# 	2:8:ImplFile:datasource.cpp
# End Section
# Section CBrowserFrame : {FC470417-2BF8-11D2-AF03-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:spreadsheet.h
# 	2:16:DefaultSinkClass:CSpreadsheet
# End Section
# Section CBrowserFrame : {ADC2DFD2-1E61-11D2-AEF3-00A0C9B71DC4}
# 	2:5:Class:CTabSelector
# 	2:10:HeaderFile:tabselector.h
# 	2:8:ImplFile:tabselector.cpp
# End Section
# Section CBrowserFrame : {26C03861-0D33-11D2-AEE9-00A0C9B71DC4}
# 	2:5:Class:CQueryResults
# 	2:10:HeaderFile:queryresults.h
# 	2:8:ImplFile:queryresults.cpp
# End Section
# Section CBrowserFrame : {7CA41F77-091C-11D2-AEE6-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:datasource.h
# 	2:16:DefaultSinkClass:CDataSource
# End Section
# Section OLE Controls
# 	{26C03863-0D33-11D2-AEE9-00A0C9B71DC4}
# 	{ADC2DFD4-1E61-11D2-AEF3-00A0C9B71DC4}
# End Section
# Section CBrowserFrame : {1487DD54-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:groupscombo.h
# 	2:16:DefaultSinkClass:CGroupsCombo
# End Section
# Section CBrowserFrame : {FC470415-2BF8-11D2-AF03-00A0C9B71DC4}
# 	2:5:Class:CSpreadsheet
# 	2:10:HeaderFile:spreadsheet.h
# 	2:8:ImplFile:spreadsheet.cpp
# End Section
