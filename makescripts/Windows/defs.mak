
# TARGTYPE "Win32 (x86) Static Library" 0x0104

ifeq "$(CFG)" ""
CFG=Debug
$(warning No configuration specified.  Defaulting to Debug.)
endif

ifneq "$(CFG)" "Release"
 ifneq "$(CFG)" "Debug"
  $(warning Invalid configuration "$(CFG)" specified.
  $(warning You can specify a configuration when running NMAKE on this makefile)
  $(warning by defining the macro CFG on the command line.  For example:)
  $(warning)
  $(warning NMAKE /f <Makefile> CFG="Debug")
  $(warning)
  $(warning Possible choices for configuration are:)
  $(warning)
  $(warning "Release" \(based on "Win32 \(x86\) Static Library"\) )
  $(warning "Debug" \(based on "Win32 \(x86\) Static Library"\) )
  $(warning)
  $(error An invalid configuration is specified.)
 endif
endif

CPP=cl.exe

ifeq "$(OS)" "Windows_NT"
NULL=
else
NULL=nul
endif
ifeq "$(CFG)" "Release"

INTDIR=Release
CPP_OPTS=/nologo /W3 /GX /O2 /D "NDEBUG"
	
else # debug

INTDIR=Debug
CPP_OPTS=/nologo /W3 /GX /Z7 /Od /Ob1 /D "_DEBUG"
	
endif 

CPP_RUNTIME=/MT

CPP_BASE=$(CPP_RUNTIME) $(CPP_OPTS)

CPP_WINDOWS=/D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_WIN32" /D "PCxOS"

CPP_FINISH=/FI"$(ADMMAKE)/warn_disable.h" /Fo"$(INTDIR)/" /c 

# Be wary of adding __STDC__ or _POSIX_ here.  I don't think MS has
# their posix compliance quite straightened out, and the __STDC__ flag
# has gotten wrapped up.
# (actually, -D__STDC__=0 might be OK)

# these get applied to both C and C++ files:
CPP_DEFINES=\
 /D "GCC2" /D "NEW_UI" /D "NEWCPP"\
 /D "iv2_6_compatible" /D vdebugDEBUG=0

CPP_PROJ=$(CPP_BASE)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_INCLUDES_FILE)\
 $(CPP_FINISH)

# these get applied to C++ files only:
CPLUSPLUS_FLAGS=\
 /D "ISO_CPP_HEADERS"

LIB32=link.exe -lib

YL_MAKE=$(ADMMAKE)/makefile.lex
