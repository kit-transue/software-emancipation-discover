# Microsoft Developer Studio Project File - Name="Impact" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Impact - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Impact.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Impact.mak" CFG="Impact - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Impact - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Impact - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Impact - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Impact - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Impact"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Impact - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Ext "ocx"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /dll /machine:I386
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Release
TargetPath=.\Release\Impact.ocx
InputPath=.\Release\Impact.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Impact - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Debug
TargetPath=.\Debug\Impact.ocx
InputPath=.\Debug\Impact.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Impact - Win32 Unicode Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugU"
# PROP BASE Intermediate_Dir "DebugU"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugU"
# PROP Intermediate_Dir "DebugU"
# PROP Target_Ext "ocx"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\DebugU
TargetPath=.\DebugU\Impact.ocx
InputPath=.\DebugU\Impact.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Impact - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseU"
# PROP BASE Intermediate_Dir "ReleaseU"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseU"
# PROP Intermediate_Dir "ReleaseU"
# PROP Target_Ext "ocx"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /dll /machine:I386
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\ReleaseU
TargetPath=.\ReleaseU\Impact.ocx
InputPath=.\ReleaseU\Impact.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "Impact - Win32 Release"
# Name "Impact - Win32 Debug"
# Name "Impact - Win32 Unicode Debug"
# Name "Impact - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\AccessCpp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AccessCppDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AccessJava.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AccessJavaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AddBaseClassAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AddBaseInterfaceAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AddEnumValueAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AddFieldAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AddMethodAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Argument.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ArgumentsInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ArgumentsTableCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AttributesQueryResult.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeArgumentsAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeBaseClassAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeBodyAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeClassPermissionsAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeFieldPermissionsCppAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeFieldPermissionsJavaAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeFunctionPermissionsCppAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeFunctionPermissionsJavaAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeTypeAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeVariablePermissionsAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CheckedTree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ClassAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ClassAccessDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ClassActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DeleteEntityAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DeleteVariableAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Entity.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EnumActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EnumValueDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FieldActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FieldInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FunctionAccessCpp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FunctionAccessCppDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FunctionAccessJava.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FunctionAccessJavaDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FunctionActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\HtmlCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Impact.cpp
# End Source File
# Begin Source File

SOURCE=.\Impact.def
# End Source File
# Begin Source File

SOURCE=.\Impact.odl
# End Source File
# Begin Source File

SOURCE=.\Impact.rc
# End Source File
# Begin Source File

SOURCE=.\src\ImpactAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactActionsSet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactItem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactPageSelector.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactPpg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactReport.cpp
# End Source File
# Begin Source File

SOURCE=.\src\InplaceEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\InterfaceActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LocalVariableActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MacroActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MethodInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ModuleActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MoveToPackageAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NewTypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PackageActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RemoveBaseClassAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RemoveBaseInterfaceAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RemoveEnumValueAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RemoveFieldAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RemoveMethodAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RenameAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RenameDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ReportViewer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ResultsTree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SelectorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\src\TemplateActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TypedefActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UnionActions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\VariableAccessDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\VariableActions.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\AccessCpp.h
# End Source File
# Begin Source File

SOURCE=.\include\AccessCppDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\AccessJava.h
# End Source File
# Begin Source File

SOURCE=.\include\AccessJavaDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\AddBaseClassAction.h
# End Source File
# Begin Source File

SOURCE=.\include\AddBaseInterfaceAction.h
# End Source File
# Begin Source File

SOURCE=.\include\AddEnumValueAction.h
# End Source File
# Begin Source File

SOURCE=.\include\AddFieldAction.h
# End Source File
# Begin Source File

SOURCE=.\include\AddMethodAction.h
# End Source File
# Begin Source File

SOURCE=.\include\Argument.h
# End Source File
# Begin Source File

SOURCE=.\include\ArgumentsInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\ArgumentsTableCtrl.h
# End Source File
# Begin Source File

SOURCE=.\include\AttributesQueryResult.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeArgumentsAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeBaseClassAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeBodyAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeClassPermissionsAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeFieldPermissionsCppAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeFieldPermissionsJavaAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeFunctionPermissionsCppAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeFunctionPermissionsJavaAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeTypeAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ChangeVariablePermissionsAction.h
# End Source File
# Begin Source File

SOURCE=.\include\CheckedTree.h
# End Source File
# Begin Source File

SOURCE=.\include\ClassAccess.h
# End Source File
# Begin Source File

SOURCE=.\include\ClassAccessDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\ClassActions.h
# End Source File
# Begin Source File

SOURCE=.\include\custommsgs.h
# End Source File
# Begin Source File

SOURCE=.\include\DeleteEntityAction.h
# End Source File
# Begin Source File

SOURCE=.\include\DeleteVariableAction.h
# End Source File
# Begin Source File

SOURCE=.\include\Entity.h
# End Source File
# Begin Source File

SOURCE=.\include\EnumActions.h
# End Source File
# Begin Source File

SOURCE=.\include\EnumValueDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\FieldActions.h
# End Source File
# Begin Source File

SOURCE=.\include\FieldInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\FunctionAccessCpp.h
# End Source File
# Begin Source File

SOURCE=.\include\FunctionAccessCppDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\FunctionAccessJava.h
# End Source File
# Begin Source File

SOURCE=.\include\FunctionAccessJavaDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\FunctionActions.h
# End Source File
# Begin Source File

SOURCE=.\include\HtmlCtrl.h
# End Source File
# Begin Source File

SOURCE=.\include\Impact.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactAction.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactActionsSet.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactCtl.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactItem.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactPageSelector.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactPpg.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactReport.h
# End Source File
# Begin Source File

SOURCE=.\include\InplaceEdit.h
# End Source File
# Begin Source File

SOURCE=.\include\InterfaceActions.h
# End Source File
# Begin Source File

SOURCE=.\include\LocalVariableActions.h
# End Source File
# Begin Source File

SOURCE=.\include\MacroActions.h
# End Source File
# Begin Source File

SOURCE=.\include\MethodInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\ModuleActions.h
# End Source File
# Begin Source File

SOURCE=.\include\MoveToPackageAction.h
# End Source File
# Begin Source File

SOURCE=.\include\NewTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\PackageActions.h
# End Source File
# Begin Source File

SOURCE=.\include\RemoveBaseClassAction.h
# End Source File
# Begin Source File

SOURCE=.\include\RemoveBaseInterfaceAction.h
# End Source File
# Begin Source File

SOURCE=.\include\RemoveEnumValueAction.h
# End Source File
# Begin Source File

SOURCE=.\include\RemoveFieldAction.h
# End Source File
# Begin Source File

SOURCE=.\include\RemoveMethodAction.h
# End Source File
# Begin Source File

SOURCE=.\include\RenameAction.h
# End Source File
# Begin Source File

SOURCE=.\include\RenameDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\ReportViewer.h
# End Source File
# Begin Source File

SOURCE=.\include\Resource.h
# End Source File
# Begin Source File

SOURCE=.\include\ResultsTree.h
# End Source File
# Begin Source File

SOURCE=.\include\SelectorDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\include\TemplateActions.h
# End Source File
# Begin Source File

SOURCE=.\include\TypedefActions.h
# End Source File
# Begin Source File

SOURCE=.\include\UnionActions.h
# End Source File
# Begin Source File

SOURCE=.\include\VariableAccessDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\VariableActions.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\action.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ast.bmp
# End Source File
# Begin Source File

SOURCE=.\res\categ.bmp
# End Source File
# Begin Source File

SOURCE=.\res\changes.bmp
# End Source File
# Begin Source File

SOURCE=.\res\classes.bmp
# End Source File
# Begin Source File

SOURCE=.\res\const.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor.bmp
# End Source File
# Begin Source File

SOURCE=.\res\defect.bmp
# End Source File
# Begin Source File

SOURCE=.\res\enum.bmp
# End Source File
# Begin Source File

SOURCE=.\res\eval.bmp
# End Source File
# Begin Source File

SOURCE=.\res\field2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\files.bmp
# End Source File
# Begin Source File

SOURCE=.\res\function.bmp
# End Source File
# Begin Source File

SOURCE=.\res\groups.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ImpactCtl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\impactpr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\impactre.bmp
# End Source File
# Begin Source File

SOURCE=.\res\interfac.bmp
# End Source File
# Begin Source File

SOURCE=.\res\label.bmp
# End Source File
# Begin Source File

SOURCE=.\res\locvar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\macroses.bmp
# End Source File
# Begin Source File

SOURCE=.\res\packages.bmp
# End Source File
# Begin Source File

SOURCE=.\res\QueryResultsCtl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rootr_projects.bmp
# End Source File
# Begin Source File

SOURCE=.\res\semtype.bmp
# End Source File
# Begin Source File

SOURCE=.\res\state.bmp
# End Source File
# Begin Source File

SOURCE=.\res\strings.bmp
# End Source File
# Begin Source File

SOURCE=.\res\struct.bmp
# End Source File
# Begin Source File

SOURCE=.\res\table.bmp
# End Source File
# Begin Source File

SOURCE=.\res\template.bmp
# End Source File
# Begin Source File

SOURCE=.\res\typedef.bmp
# End Source File
# Begin Source File

SOURCE=.\res\union.bmp
# End Source File
# Begin Source File

SOURCE=.\res\variable.bmp
# End Source File
# Begin Source File

SOURCE=.\res\variables.bmp
# End Source File
# Begin Source File

SOURCE=.\res\warning.bmp
# End Source File
# Begin Source File

SOURCE=.\res\xref.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
