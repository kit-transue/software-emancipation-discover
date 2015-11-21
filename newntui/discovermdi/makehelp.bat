@echo off
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Copyright (c) 2015, Synopsys, Inc.                                     :
:: All rights reserved.                                                   :
::                                                                        :
:: Redistribution and use in source and binary forms, with or without     :
:: modification, are permitted provided that the following conditions are :
:: met:                                                                   :
::                                                                        :
:: 1. Redistributions of source code must retain the above copyright      :
:: notice, this list of conditions and the following disclaimer.          :
::                                                                        :
:: 2. Redistributions in binary form must reproduce the above copyright   :
:: notice, this list of conditions and the following disclaimer in the    :
:: documentation and/or other materials provided with the distribution.   :
::                                                                        :
:: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    :
:: "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      :
:: LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  :
:: A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   :
:: HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, :
:: SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       :
:: LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  :
:: DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  :
:: THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    :
:: (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  :
:: OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   :
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
REM -- First make map file from Microsoft Visual C++ generated .\include\resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by DISCOVERMDI.HPJ. >"hlp\DiscoverMDI.hm"
echo. >>"hlp\DiscoverMDI.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\DiscoverMDI.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 .\include\resource.h >>"hlp\DiscoverMDI.hm"
echo. >>"hlp\DiscoverMDI.hm"
echo // Prompts (IDP_*) >>"hlp\DiscoverMDI.hm"
makehm IDP_,HIDP_,0x30000 .\include\resource.h >>"hlp\DiscoverMDI.hm"
echo. >>"hlp\DiscoverMDI.hm"
echo // Resources (IDR_*) >>"hlp\DiscoverMDI.hm"
makehm IDR_,HIDR_,0x20000 .\include\resource.h >>"hlp\DiscoverMDI.hm"
echo. >>"hlp\DiscoverMDI.hm"
echo // Dialogs (IDD_*) >>"hlp\DiscoverMDI.hm"
makehm IDD_,HIDD_,0x20000 .\include\resource.h >>"hlp\DiscoverMDI.hm"
echo. >>"hlp\DiscoverMDI.hm"
echo // Frame Controls (IDW_*) >>"hlp\DiscoverMDI.hm"
makehm IDW_,HIDW_,0x50000 .\include\resource.h >>"hlp\DiscoverMDI.hm"
REM -- Make help for Project DISCOVERMDI

cp -r %MSVCDir%/MFC/Include/afxhelp.hm ./hlp/afxhelp.hm

echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\DiscoverMDI.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\DiscoverMDI.hlp" goto :Error
if not exist "hlp\DiscoverMDI.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\DiscoverMDI.hlp" Debug
if exist Debug\nul copy "hlp\DiscoverMDI.cnt" Debug
if exist Release\nul copy "hlp\DiscoverMDI.hlp" Release
if exist Release\nul copy "hlp\DiscoverMDI.cnt" Release
echo.
goto :done

:Error
echo hlp\DiscoverMDI.hpj(1) : error: Problem encountered creating help file

:done
echo.
