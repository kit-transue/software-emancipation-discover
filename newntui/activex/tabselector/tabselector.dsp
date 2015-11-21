# Microsoft Developer Studio Project File - Name="TabSelector" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TabSelector - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TabSelector.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TabSelector.mak" CFG="TabSelector - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TabSelector - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TabSelector - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "TabSelector"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TabSelector - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /FD /c
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
TargetPath=.\Release\TabSelector.ocx
InputPath=.\Release\TabSelector.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "TabSelector - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "n:/paraset/src/paraset/gala/include" /I "n:/paraset/src/paraset/gala/extern/osport/include" /I ".\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_WINDLL" /FD /c
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
TargetPath=.\Debug\TabSelector.ocx
InputPath=.\Debug\TabSelector.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "TabSelector - Win32 Release"
# Name "TabSelector - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\src\categoriescombo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\groupscombo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\queriescombo.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\src\TabSelector.cpp
# End Source File
# Begin Source File

SOURCE=.\TabSelector.def
# End Source File
# Begin Source File

SOURCE=.\TabSelector.odl
# End Source File
# Begin Source File

SOURCE=.\TabSelector.rc
# End Source File
# Begin Source File

SOURCE=.\src\TabSelectorCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TabSelectorPpg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\include\categoriescombo.h
# End Source File
# Begin Source File

SOURCE=.\include\groupscombo.h
# End Source File
# Begin Source File

SOURCE=.\include\queriescombo.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\include\TabSelector.h
# End Source File
# Begin Source File

SOURCE=.\include\TabSelectorCtl.h
# End Source File
# Begin Source File

SOURCE=.\include\TabSelectorPpg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\TabSelector.ico
# End Source File
# Begin Source File

SOURCE=.\res\TabSelectorCtl.bmp
# End Source File
# End Group
# End Target
# End Project
# Section OLE Controls
# 	{2451D66C-1203-11D2-AEED-00A0C9B71DC4}
# 	{1487DD54-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	{6E672C25-1543-11D2-AEED-00A0C9B71DC4}
# End Section
# Section TabSelector : {2451D66A-1203-11D2-AEED-00A0C9B71DC4}
# 	2:5:Class:CCategoriesCombo
# 	2:10:HeaderFile:categoriescombo.h
# 	2:8:ImplFile:categoriescombo.cpp
# End Section
# Section TabSelector : {1487DD52-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	2:5:Class:CGroupsCombo
# 	2:10:HeaderFile:groupscombo.h
# 	2:8:ImplFile:groupscombo.cpp
# End Section
# Section TabSelector : {2451D66C-1203-11D2-AEED-00A0C9B71DC4}
# 	0:19:CategoriesCombo.cpp:C:\NewDiscover\ActiveX\TabSelector\CategoriesCombo.cpp
# 	0:17:CategoriesCombo.h:C:\NewDiscover\ActiveX\TabSelector\CategoriesCombo.h
# 	2:21:DefaultSinkHeaderFile:categoriescombo.h
# 	2:16:DefaultSinkClass:CCategoriesCombo
# End Section
# Section TabSelector : {6E672C25-1543-11D2-AEED-00A0C9B71DC4}
# 	0:16:QueriesCombo.cpp:C:\NewDiscover\ActiveX\TabSelector\QueriesCombo.cpp
# 	0:14:QueriesCombo.h:C:\NewDiscover\ActiveX\TabSelector\QueriesCombo.h
# 	2:21:DefaultSinkHeaderFile:queriescombo.h
# 	2:16:DefaultSinkClass:CQueriesCombo
# End Section
# Section TabSelector : {1487DD54-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	0:15:GroupsCombo.cpp:C:\NewDiscover\ActiveX\TabSelector\GroupsCombo.cpp
# 	0:13:GroupsCombo.h:C:\NewDiscover\ActiveX\TabSelector\GroupsCombo.h
# 	2:21:DefaultSinkHeaderFile:groupscombo.h
# 	2:16:DefaultSinkClass:CGroupsCombo
# End Section
# Section TabSelector : {6E672C23-1543-11D2-AEED-00A0C9B71DC4}
# 	2:5:Class:CQueriesCombo
# 	2:10:HeaderFile:queriescombo.h
# 	2:8:ImplFile:queriescombo.cpp
# End Section
