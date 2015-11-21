# Microsoft Developer Studio Generated NMAKE File, Based on httpserver.dsp
!IF "$(CFG)" == ""
CFG=Debug
!MESSAGE No configuration specified. Defaulting to Debug.
!ENDIF 

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "httpserver.mak" CFG="httpserver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\httpserver.exe"

!ELSE 

ALL : "$(OUTDIR)\httpserver.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\httpwrappers.obj"
	-@erase "$(INTDIR)\nameServCalls.obj"
	-@erase "$(INTDIR)\servers.obj"
	-@erase "$(INTDIR)\SocketComm.obj"
	-@erase "$(INTDIR)\sockets.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\httpserver.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\httpserver.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\httpserver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\httpserver.pdb" /machine:I386 /out:"$(OUTDIR)\httpserver.exe" 
LINK32_OBJS= \
	"$(INTDIR)\httpwrappers.obj" \
	"$(INTDIR)\nameServCalls.obj" \
	"$(INTDIR)\servers.obj" \
	"$(INTDIR)\SocketComm.obj" \
	"$(INTDIR)\sockets.obj"

"$(OUTDIR)\httpserver.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\httpserver.exe"

!ELSE 

ALL : "$(OUTDIR)\httpserver.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\httpwrappers.obj"
	-@erase "$(INTDIR)\nameServCalls.obj"
	-@erase "$(INTDIR)\servers.obj"
	-@erase "$(INTDIR)\SocketComm.obj"
	-@erase "$(INTDIR)\sockets.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\httpserver.exe"
	-@erase "$(OUTDIR)\httpserver.ilk"
	-@erase "$(OUTDIR)\httpserver.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D\
 "_MBCS" /Fp"$(INTDIR)\httpserver.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /ZI /GZ /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\httpserver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib Ws2_32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\httpserver.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\httpserver.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\httpwrappers.obj" \
	"$(INTDIR)\nameServCalls.obj" \
	"$(INTDIR)\servers.obj" \
	"$(INTDIR)\SocketComm.obj" \
	"$(INTDIR)\sockets.obj"

"$(OUTDIR)\httpserver.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "Release" || "$(CFG)" == "Debug"
SOURCE=.\httpwrappers.cpp
DEP_CPP_HTTPW=\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\httpwrappers.obj" : $(SOURCE) $(DEP_CPP_HTTPW) "$(INTDIR)"


SOURCE=N:\paraset\src\nameserver\api\nameServCalls.cxx
DEP_CPP_NAMES=\
	"..\..\nameserver\include\nameServCalls.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\nameServCalls.obj" : $(SOURCE) $(DEP_CPP_NAMES) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\servers.cpp
DEP_CPP_SERVE=\
	"..\..\dish\include\SocketComm.h"\
	"..\..\nameserver\include\nameServCalls.h"\
	".\servers.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\servers.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"


SOURCE=N:\paraset\src\dish\src\SocketComm.cxx
DEP_CPP_SOCKE=\
	"..\..\dish\include\SocketComm.h"\
	

"$(INTDIR)\SocketComm.obj" : $(SOURCE) $(DEP_CPP_SOCKE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\sockets.cpp
DEP_CPP_SOCKET=\
	"..\..\dish\include\SocketComm.h"\
	"..\..\nameserver\include\nameServCalls.h"\
	".\servers.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\sockets.obj" : $(SOURCE) $(DEP_CPP_SOCKET) "$(INTDIR)"



!ENDIF 

