# Microsoft Developer Studio Project File - Name="DiscoverMDI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DiscoverMDI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DiscoverMDI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DiscoverMDI.mak" CFG="DiscoverMDI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DiscoverMDI - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DiscoverMDI - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "DiscoverMDI"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DiscoverMDI - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\gala\extern\osport\include" /I "..\..\dish\include" /I "..\..\gala/extern/osport/include" /I "..\nameserver\include" /I "..\..\gala\extern\ads\include" /I "..\..\gala\extern\tset\include" /I "$(PATH2BS)/gala\extern\ads\include" /I "$(PATH2BS)/gala\extern\tset\include" /I "$(PATH2BS)/gala\extern\osport\include" /I "$(PATH2BS)/dish\include" /I "$(PATH2BS)/gala/extern/osport/include" /I "$(PATH2BS)\nameserver\include" /I "..\CommonResources" /I "..\CommonResources\include" /I ".\include" /I "..\activex\msdevintegrator\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "PCxOS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "DiscoverMDI - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\..\gala\extern\osport\include" /I "..\..\dish\include" /I "..\..\gala/extern/osport/include" /I "..\nameserver\include" /I "..\..\gala\extern\ads\include" /I "..\..\gala\extern\tset\include" /I "$(PATH2BS)/gala\extern\ads\include" /I "$(PATH2BS)/gala\extern\tset\include" /I "$(PATH2BS)/gala\extern\osport\include" /I "$(PATH2BS)/dish\include" /I "$(PATH2BS)/gala/extern/osport/include" /I "$(PATH2BS)\nameserver\include" /I "..\CommonResources" /I "..\CommonResources\include" /I ".\include" /I "..\activex\msdevintegrator\include" /D "_DEBUG" /D "PCxOS" /D "WIN32" /D "_WINDOWS" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /stack:0x1000000 /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DiscoverMDI - Win32 Release"
# Name "DiscoverMDI - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\accesschildframe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\accessedit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\accesswnd.cpp
# End Source File
# Begin Source File

SOURCE=..\commonresources\src\addselectordlg.cpp
# End Source File
# Begin Source File

SOURCE=..\CommonResources\Src\AttributesDialog.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\bitMEM.cxx"
# End Source File
# Begin Source File

SOURCE=.\src\callgraphdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\categoriescombo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\checkinpromptdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\checkoutpromptdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMAttributesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cmintegrator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CMSelectionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CntrItem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\coderoverintegrator.cpp
# End Source File
# Begin Source File

SOURCE=..\CommonResources\Src\ComplexSort.cpp
# End Source File
# Begin Source File

SOURCE=.\src\datasource.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DiscoverMDI.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\DiscoverMDI.hpj

!IF  "$(CFG)" == "DiscoverMDI - Win32 Release"

USERDEP__DISCO="$(ProjDir)\hlp\AfxCore.rtf"	"$(ProjDir)\hlp\AfxPrint.rtf"	
# Begin Custom Build - Making help file...
OutDir=.\Release
ProjDir=.
TargetName=DiscoverMDI
InputPath=.\hlp\DiscoverMDI.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call "$(ProjDir)\makehelp.bat"

# End Custom Build

!ELSEIF  "$(CFG)" == "DiscoverMDI - Win32 Debug"

USERDEP__DISCO="$(ProjDir)\hlp\AfxCore.rtf"	"$(ProjDir)\hlp\AfxPrint.rtf"	
# Begin Custom Build - Making help file...
OutDir=.\Debug
ProjDir=.
TargetName=DiscoverMDI
InputPath=.\hlp\DiscoverMDI.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call "$(ProjDir)\makehelp.bat"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DiscoverMDI.odl
# End Source File
# Begin Source File

SOURCE=.\discovermdi.rc

!IF  "$(CFG)" == "DiscoverMDI - Win32 Release"

# ADD BASE RSC /l 0x409 /i "Release"
# ADD RSC /l 0x409 /i "Release" /i "include" /i "res"

!ELSEIF  "$(CFG)" == "DiscoverMDI - Win32 Debug"

# ADD BASE RSC /l 0x409 /i "Debug"
# ADD RSC /l 0x409 /i "Debug" /i "include" /i "res"
# SUBTRACT RSC /x

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\DiscoverMDIDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DiscoverMDIView.cpp
# End Source File
# Begin Source File

SOURCE=..\activex\msdevintegrator\disregistry\disregistry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dockablebrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DormantChildFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DormantProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dtpicker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditorSelectionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\errorsbrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\CommonResources\Src\FilterDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\CommonResources\Src\FilterList.cpp
# End Source File
# Begin Source File

SOURCE=..\CommonResources\Src\FilterSheet.cpp
# End Source File
# Begin Source File

SOURCE=..\CommonResources\Src\FilterWizard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FlowTab.cpp
# End Source File
# Begin Source File

SOURCE=.\src\font.cpp
# End Source File
# Begin Source File

SOURCE=.\src\groupscombo1.cpp
# End Source File
# Begin Source File

SOURCE=..\commonresources\src\groupspage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\IMAttachDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\impact.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactChildFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpactForm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\IpFrame.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\tset\settings\item.cxx"
# End Source File
# Begin Source File

SOURCE=.\src\listseldlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lockpromptdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\msdevintegrator.cpp
# End Source File
# Begin Source File

SOURCE=..\nameserver\api\nameservcalls.cxx
# End Source File
# Begin Source File

SOURCE=.\src\NewFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NoCollapseTreeCtrl.cpp
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

SOURCE=.\src\picture.cpp
# End Source File
# Begin Source File

SOURCE=.\src\projectmodules.cpp
# End Source File
# Begin Source File

SOURCE=.\src\projecttree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\promptdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QAChildFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\qaerrors.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QAParams.cpp
# End Source File
# Begin Source File

SOURCE=.\src\QATree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\queriescombo1.cpp
# End Source File
# Begin Source File

SOURCE=.\src\queryresults.cpp
# End Source File
# Begin Source File

SOURCE=.\src\questiondlg.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\osport\src\raFile.cxx"
# End Source File
# Begin Source File

SOURCE=.\src\reporttemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Sash.cpp
# End Source File
# Begin Source File

SOURCE=.\src\servicechooser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ServiceManagerDlg.cpp
# End Source File
# Begin Source File

SOURCE="$(PATH2BS)\gala\extern\tset\settings\settings.cxx"
# End Source File
# Begin Source File

SOURCE=.\src\sevopendlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sevsavedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\simplifychildframe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\simplifyselection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\simplifytext.cpp
# End Source File
# Begin Source File

SOURCE=.\src\simplifytree.cpp
# End Source File
# Begin Source File

SOURCE=..\sockets\src\socketcomm.cxx
# End Source File
# Begin Source File

SOURCE=.\src\spreadsheet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SrvrItem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\src\submitinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tabselector.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TaskFlow.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TaskNewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TPMEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TpmProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\uncheckoutpromptdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unlockpromptdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\updatewaitdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WaitDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\webbrowser2.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\accesschildframe.h
# End Source File
# Begin Source File

SOURCE=.\include\accessedit.h
# End Source File
# Begin Source File

SOURCE=.\include\accesswnd.h
# End Source File
# Begin Source File

SOURCE=.\include\callgraphdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\categoriescombo.h
# End Source File
# Begin Source File

SOURCE=.\include\cbrowserframe.h
# End Source File
# Begin Source File

SOURCE=.\include\checkinpromptdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\checkoutpromptdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\CMAttributesDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\CMCommand.h
# End Source File
# Begin Source File

SOURCE=.\include\cmintegrator.h
# End Source File
# Begin Source File

SOURCE=.\include\CMSelectionDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\CntrItem.h
# End Source File
# Begin Source File

SOURCE=.\include\coderoverintegrator.h
# End Source File
# Begin Source File

SOURCE=..\commonresources\commondialogs.rh
# End Source File
# Begin Source File

SOURCE=.\include\datasource.h
# End Source File
# Begin Source File

SOURCE=.\include\DiscoverMDI.h
# End Source File
# Begin Source File

SOURCE=.\include\DiscoverMDIDoc.h
# End Source File
# Begin Source File

SOURCE=.\include\DiscoverMDIView.h
# End Source File
# Begin Source File

SOURCE=.\include\dockablebrowser.h
# End Source File
# Begin Source File

SOURCE=.\include\DormantChildFrame.h
# End Source File
# Begin Source File

SOURCE=.\include\DormantProcessor.h
# End Source File
# Begin Source File

SOURCE=.\include\dtpicker.h
# End Source File
# Begin Source File

SOURCE=.\include\EditorSelectionDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\errorsbrowser.h
# End Source File
# Begin Source File

SOURCE=.\include\FlowTab.h
# End Source File
# Begin Source File

SOURCE=.\include\font.h
# End Source File
# Begin Source File

SOURCE=.\include\groupscombo.h
# End Source File
# Begin Source File

SOURCE=.\include\IMAttachDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\impact.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactChildFrame.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpactForm.h
# End Source File
# Begin Source File

SOURCE=.\include\IpFrame.h
# End Source File
# Begin Source File

SOURCE=.\include\listseldlg.h
# End Source File
# Begin Source File

SOURCE=.\include\lockpromptdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\include\msdevintegrator.h
# End Source File
# Begin Source File

SOURCE=..\..\NameServer\include\nameServCalls.h
# End Source File
# Begin Source File

SOURCE=.\include\NewFilter.h
# End Source File
# Begin Source File

SOURCE=.\include\NoCollapseTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\include\picture.h
# End Source File
# Begin Source File

SOURCE=.\include\projectmodules.h
# End Source File
# Begin Source File

SOURCE=.\include\projecttree.h
# End Source File
# Begin Source File

SOURCE=.\include\promptdialog.h
# End Source File
# Begin Source File

SOURCE=.\include\QAChildFrame.h
# End Source File
# Begin Source File

SOURCE=.\include\qaerrors.h
# End Source File
# Begin Source File

SOURCE=.\include\QAParams.h
# End Source File
# Begin Source File

SOURCE=.\include\QATree.h
# End Source File
# Begin Source File

SOURCE=.\include\queriescombo1.h
# End Source File
# Begin Source File

SOURCE=.\include\queryresults.h
# End Source File
# Begin Source File

SOURCE=.\include\questiondlg.h
# End Source File
# Begin Source File

SOURCE=.\include\reporttemplate.h
# End Source File
# Begin Source File

SOURCE=.\include\Resource.h
# End Source File
# Begin Source File

SOURCE=.\include\Sash.h
# End Source File
# Begin Source File

SOURCE=.\include\servicechooser.h
# End Source File
# Begin Source File

SOURCE=.\include\ServiceManagerDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\sevopendlg.h
# End Source File
# Begin Source File

SOURCE=.\include\sevsavedlg.h
# End Source File
# Begin Source File

SOURCE=.\include\simplifychildframe.h
# End Source File
# Begin Source File

SOURCE=.\include\simplifyselection.h
# End Source File
# Begin Source File

SOURCE=.\include\simplifytext.h
# End Source File
# Begin Source File

SOURCE=.\include\simplifytree.h
# End Source File
# Begin Source File

SOURCE=.\include\spreadsheet.h
# End Source File
# Begin Source File

SOURCE=.\include\SrvrItem.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\include\submitinfo.h
# End Source File
# Begin Source File

SOURCE=.\include\tabselector.h
# End Source File
# Begin Source File

SOURCE=.\include\TaskFlow.h
# End Source File
# Begin Source File

SOURCE=.\include\TaskNewDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\TPMEditor.h
# End Source File
# Begin Source File

SOURCE=.\include\TpmProcessor.h
# End Source File
# Begin Source File

SOURCE=.\include\uncheckoutpromptdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\unlockpromptdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\updatewaitdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\WaitDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\webbrowser2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\about.bmp
# End Source File
# Begin Source File

SOURCE=.\res\And.bmp
# End Source File
# Begin Source File

SOURCE=.\res\arright1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ArrowDown.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ArrowLeft.bmp
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

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00005.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00007.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00008.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00009.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00010.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00011.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00012.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00013.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00014.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00015.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00016.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00017.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00018.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00019.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00020.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00021.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00022.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_categories.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_groups.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_next.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_prev.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_projects.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_queries.bmp
# End Source File
# Begin Source File

SOURCE=.\res\browser_split.bmp
# End Source File
# Begin Source File

SOURCE=.\res\btot.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cl_bmp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CodeRover.ico
# End Source File
# Begin Source File

SOURCE=.\res\copy1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\datachar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\datacharttools.bmp
# End Source File
# Begin Source File

SOURCE=.\res\DiscoverMDI.rc2
# End Source File
# Begin Source File

SOURCE=.\res\DiscoverMDIDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\erd_tools.bmp
# End Source File
# Begin Source File

SOURCE=.\res\erdtools.bmp
# End Source File
# Begin Source File

SOURCE=.\res\filesnee.bmp
# End Source File
# Begin Source File

SOURCE=.\res\FilterList.bmp
# End Source File
# Begin Source File

SOURCE=.\res\FilterWizard.bmp
# End Source File
# Begin Source File

SOURCE=.\res\flowcharttools.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_m_cl_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_m_op_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_mark_closed.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_mark_opened.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_o.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_w_cl_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_w_closed.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_w_op_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\folder_w_open.bmp
# End Source File
# Begin Source File

SOURCE=.\res\graph_collapse.bmp
# End Source File
# Begin Source File

SOURCE=.\res\graph_expand.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_impa.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_tpm1.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_xml1.ico
# End Source File
# Begin Source File

SOURCE=.\res\impact.bmp
# End Source File
# Begin Source File

SOURCE=.\res\impact.ico
# End Source File
# Begin Source File

SOURCE=.\res\IToolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ltor.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mainframvies.bmp
# End Source File
# Begin Source File

SOURCE=.\res\modelim.bmp
# End Source File
# Begin Source File

SOURCE=.\res\myprojects.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_bub_prop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_pack_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_pack_prop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_priv_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_priv_prop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_prot_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_prot_prop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nv_pub_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\one2many_bottom.bmp
# End Source File
# Begin Source File

SOURCE=.\res\one2many_left.bmp
# End Source File
# Begin Source File

SOURCE=.\res\one2many_right.bmp
# End Source File
# Begin Source File

SOURCE=.\res\one2many_top.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Or.bmp
# End Source File
# Begin Source File

SOURCE=.\res\outdated.bmp
# End Source File
# Begin Source File

SOURCE=.\res\outlinetools.bmp
# End Source File
# Begin Source File

SOURCE=.\res\paste1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\private_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\protect_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\public_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\qa1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\query1_c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\query1_e.bmp
# End Source File
# Begin Source File

SOURCE=.\res\query1_o.bmp
# End Source File
# Begin Source File

SOURCE=.\res\query2_o.bmp
# End Source File
# Begin Source File

SOURCE=.\res\query_di.bmp
# End Source File
# Begin Source File

SOURCE=.\res\query_op.bmp
# End Source File
# Begin Source File

SOURCE=.\res\report1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rtol.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sbchk_rt_warn.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_bar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_check.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_check_sel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_permanent.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_processing.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_root.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_root_sel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_uncheck.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_uncheck_sel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_unprocessed.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_updated.bmp
# End Source File
# Begin Source File

SOURCE=.\res\simplify_visited.bmp
# End Source File
# Begin Source File

SOURCE=.\res\stop1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sub_c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\suberror_sel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\suberror_sel_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\suberror_unsel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\suberror_unsel_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\submit_o.bmp
# End Source File
# Begin Source File

SOURCE=.\res\suboksel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subokseldis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subokunsel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subokunseldis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subwarn_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subwarn_unsel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subwarn_unsel_dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\subwarnsel.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\treetool.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ttob.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vv_pack_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vv_priv_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vv_prot_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vv_pub_func.bmp
# End Source File
# Begin Source File

SOURCE=.\res\wait.bmp
# End Source File
# Begin Source File

SOURCE=.\res\xml1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\xmlcompo.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxOleCl.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxOleSv.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxPrint.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\DiscoverMDI.cnt
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\MakeHelp.bat
# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# Begin Group "ServerScripts"

# PROP Default_Filter "*.dis"
# Begin Source File

SOURCE=..\scripts\dormant.dis
# End Source File
# Begin Source File

SOURCE=..\scripts\inheritanceviewscripts.dis
# End Source File
# Begin Source File

SOURCE=..\scripts\menu_cmds.dis
# End Source File
# Begin Source File

SOURCE=..\scripts\sel2sym.dis
# End Source File
# Begin Source File

SOURCE=..\scripts\simplify_nt.dis
# End Source File
# Begin Source File

SOURCE=..\scripts\source.dis
# End Source File
# Begin Source File

SOURCE=..\scripts\treeviewscripts.dis
# End Source File
# Begin Source File

SOURCE="..\scripts\xml-parse.tcl"
# End Source File
# Begin Source File

SOURCE=..\scripts\xmlreport.dis
# End Source File
# End Group
# Begin Group "Documents"

# PROP Default_Filter "*.doc"
# Begin Source File

SOURCE=.\project.doc
# End Source File
# End Group
# Begin Group "Report templates"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DiscoverMDI.reg
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Group
# End Target
# End Project
# Section DiscoverMDI : {1DE4F492-201B-11D2-AEF3-00A0C9B71DC4}
# 	2:5:Class:CCBrowserFrame
# 	2:10:HeaderFile:cbrowserframe.h
# 	2:8:ImplFile:cbrowserframe.cpp
# End Section
# Section DiscoverMDI : {29FEA4AE-16B9-4A8D-B9BE-BF0B63A9E72B}
# 	2:5:Class:CCMIntegrator
# 	2:10:HeaderFile:cmintegrator.h
# 	2:8:ImplFile:cmintegrator.cpp
# End Section
# Section DiscoverMDI : {D30C1661-CDAF-11D0-8A3E-00C04FC9E26E}
# 	2:5:Class:CWebBrowser2
# 	2:10:HeaderFile:webbrowser2.h
# 	2:8:ImplFile:webbrowser2.cpp
# End Section
# Section DiscoverMDI : {FC470415-2BF8-11D2-AF03-00A0C9B71DC4}
# 	2:5:Class:CSpreadsheet
# 	2:10:HeaderFile:spreadsheet.h
# 	2:8:ImplFile:spreadsheet.cpp
# End Section
# Section DiscoverMDI : {5743FFDB-0A20-11D2-AEE7-00A0C9B71DC4}
# 	2:5:Class:CProjectModules
# 	2:10:HeaderFile:projectmodules.h
# 	2:8:ImplFile:projectmodules.cpp
# End Section
# Section DiscoverMDI : {1487DD54-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:groupscombo1.h
# 	2:16:DefaultSinkClass:CGroupsCombo
# End Section
# Section DiscoverMDI : {2047B32D-6D1B-4538-9AF5-41F7B8D0BDBE}
# 	2:21:DefaultSinkHeaderFile:impact.h
# 	2:16:DefaultSinkClass:CImpactCtrl
# End Section
# Section DiscoverMDI : {ADC2DFD4-1E61-11D2-AEF3-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:tabselector.h
# 	2:16:DefaultSinkClass:CTabSelector
# End Section
# Section DiscoverMDI : {6E672C23-1543-11D2-AEED-00A0C9B71DC4}
# 	2:5:Class:CQueriesCombo
# 	2:10:HeaderFile:queriescombo1.h
# 	2:8:ImplFile:queriescombo1.cpp
# End Section
# Section DiscoverMDI : {C5267913-392E-11D2-AF11-00A0C9B71DC4}
# 	2:5:Class:CMSDEVIntegrator
# 	2:10:HeaderFile:msdevintegrator.h
# 	2:8:ImplFile:msdevintegrator.cpp
# End Section
# Section DiscoverMDI : {B0A92BA6-41AC-11D2-AF18-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:reporttemplate.h
# 	2:16:DefaultSinkClass:CReportTemplate
# End Section
# Section DiscoverMDI : {8856F961-340A-11D0-A96B-00C04FD705A2}
# 	2:21:DefaultSinkHeaderFile:webbrowser2.h
# 	2:16:DefaultSinkClass:CWebBrowser2
# End Section
# Section DiscoverMDI : {2451D66A-1203-11D2-AEED-00A0C9B71DC4}
# 	2:5:Class:CCategoriesCombo
# 	2:10:HeaderFile:categoriescombo.h
# 	2:8:ImplFile:categoriescombo.cpp
# End Section
# Section DiscoverMDI : {3CA23651-0B2F-4E42-9C35-941DCBC95480}
# 	2:21:DefaultSinkHeaderFile:cmintegrator.h
# 	2:16:DefaultSinkClass:CCMIntegrator
# End Section
# Section DiscoverMDI : {077B20D5-0934-11D2-AEE6-00A0C9B71DC4}
# 	2:5:Class:CProjectTree
# 	2:10:HeaderFile:projecttree.h
# 	2:8:ImplFile:projecttree.cpp
# End Section
# Section DiscoverMDI : {1487DD52-1CB6-11D2-AEF0-00A0C9B71DC4}
# 	2:5:Class:CGroupsCombo
# 	2:10:HeaderFile:groupscombo1.h
# 	2:8:ImplFile:groupscombo1.cpp
# End Section
# Section DiscoverMDI : {20DD1B9B-87C4-11D1-8BE3-0000F8754DA1}
# 	2:5:Class:CDTPicker
# 	2:10:HeaderFile:dtpicker.h
# 	2:8:ImplFile:dtpicker.cpp
# End Section
# Section DiscoverMDI : {26C03863-0D33-11D2-AEE9-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:queryresults.h
# 	2:16:DefaultSinkClass:CQueryResults
# End Section
# Section DiscoverMDI : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture.h
# 	2:8:ImplFile:picture.cpp
# End Section
# Section DiscoverMDI : {AA00DC08-36DE-11D2-AF0F-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:msdevintegrator.h
# 	2:16:DefaultSinkClass:CMSDEVIntegrator
# End Section
# Section DiscoverMDI : {077B20DC-0934-11D2-AEE6-00A0C9B71DC4}
# 	2:5:Class:CDataSource
# 	2:10:HeaderFile:datasource.h
# 	2:8:ImplFile:datasource.cpp
# End Section
# Section DiscoverMDI : {5743FFDD-0A20-11D2-AEE7-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:projectmodules.h
# 	2:16:DefaultSinkClass:CProjectModules
# End Section
# Section DiscoverMDI : {1DE4F494-201B-11D2-AEF3-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:cbrowserframe.h
# 	2:16:DefaultSinkClass:CCBrowserFrame
# End Section
# Section DiscoverMDI : {ADC2DFD2-1E61-11D2-AEF3-00A0C9B71DC4}
# 	2:5:Class:CTabSelector
# 	2:10:HeaderFile:tabselector.h
# 	2:8:ImplFile:tabselector.cpp
# End Section
# Section DiscoverMDI : {BC2BD6A2-420B-4721-9EED-D5AD9CFFD885}
# 	2:5:Class:CImpactCtrl
# 	2:10:HeaderFile:impact.h
# 	2:8:ImplFile:impact.cpp
# End Section
# Section DiscoverMDI : {FC470417-2BF8-11D2-AF03-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:spreadsheet.h
# 	2:16:DefaultSinkClass:CSpreadsheet
# End Section
# Section DiscoverMDI : {B888ED83-0911-11D2-AEE6-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:projecttree.h
# 	2:16:DefaultSinkClass:CProjectTree
# End Section
# Section DiscoverMDI : {20DD1B9E-87C4-11D1-8BE3-0000F8754DA1}
# 	2:21:DefaultSinkHeaderFile:dtpicker.h
# 	2:16:DefaultSinkClass:CDTPicker
# End Section
# Section DiscoverMDI : {B0A92BA4-41AC-11D2-AF18-00A0C9B71DC4}
# 	2:5:Class:CReportTemplate
# 	2:10:HeaderFile:reporttemplate.h
# 	2:8:ImplFile:reporttemplate.cpp
# End Section
# Section DiscoverMDI : {26C03861-0D33-11D2-AEE9-00A0C9B71DC4}
# 	2:5:Class:CQueryResults
# 	2:10:HeaderFile:queryresults.h
# 	2:8:ImplFile:queryresults.cpp
# End Section
# Section DiscoverMDI : {6E672C25-1543-11D2-AEED-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:queriescombo1.h
# 	2:16:DefaultSinkClass:CQueriesCombo
# End Section
# Section DiscoverMDI : {7CA41F77-091C-11D2-AEE6-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:datasource.h
# 	2:16:DefaultSinkClass:CDataSource
# End Section
# Section DiscoverMDI : {2451D66C-1203-11D2-AEED-00A0C9B71DC4}
# 	2:21:DefaultSinkHeaderFile:categoriescombo.h
# 	2:16:DefaultSinkClass:CCategoriesCombo
# End Section
# Section DiscoverMDI : {AA00DC06-36DE-11D2-AF0F-00A0C9B71DC4}
# 	2:5:Class:CMSDEVIntegrator
# 	2:10:HeaderFile:msdevintegrator.h
# 	2:8:ImplFile:msdevintegrator.cpp
# End Section
