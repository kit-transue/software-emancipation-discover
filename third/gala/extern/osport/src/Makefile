##############################################
#
# Makefile for libadsosport.so
#
##############################################

##############################################
# Get the OS that we are building on
##############################################

ifeq "$(ARCH)" "sun5-g"
  SRCROOT    = /paraset/src/extern
  LIBROOT    = /paraset/src/extern/lib
  OBJROOT    = /paraset/src/extern/obj
  OSTYPE     = sun5
  CCC        = /net/oxen/vol01/share/sun5/opt/SUNWspro/bin/CC
  CFLAGS     = -pic 
  CCFLAGS    = -pic 
  CDEFINES   = -Dsun5
  CCDEFINES  = -Dsun5
  CINCLUDES  = 
  CCINCLUDES = -I$(SRCROOT)/osport/include -I$(SRCROOT)/ads/include
  LINKER     = ld
  LINKOPTS   = -r
  MOVE       = mv 
  MOVEOPTS   = -f 
  MKDIR      = mkdir
  MKDIROPTS  = -p
  LIBSFX     = so
  OPTOPT     = -O
  DEBOPT     = -g
  CSFX       = c
  CPPSFX     = C
  OBJSFX     = o
endif

ifeq "$(ARCH)" "sun4-g"
  SRCROOT    = /paraset/src/extern
  LIBROOT    = /paraset/src/extern/lib
  OBJROOT    = /paraset/src/extern/obj
  OSTYPE     = sun4
  CCC        = /share/opt-sparc/SWorks3.0.1/CC
  CFLAGS     = 
  CCFLAGS    = -pic 
  CDEFINES   = -Dsun4
  CCDEFINES  = -Dsun4
  CINCLUDES  = 
  CCINCLUDES = -I$(SRCROOT)/osport/include -I$(SRCROOT)/ads/include
  LINKER     = ld
  LINKOPTS   = -r
  MOVE       = mv
  MOVEOPTS   = -f 
  MKDIR      = mkdir
  MKDIROPTS  = -p
  LIBSFX     = so
  OPTOPT     = -O
  DEBOPT     = -g
  CSFX       = c
  CPPSFX     = C
  OBJSFX     = o
endif

ifeq "$(ARCH)" "hp700-g"
  SRCROOT    = /paraset/src/extern
  LIBROOT    = /paraset/src/extern/lib
  OBJROOT    = /paraset/src/extern/obj
  OSTYPE     = hp700
  CCC        = /net/oxen/vol01/share/hp700/opt/galaCC/bin/CC
  CFLAGS     = -n +z
  CCFLAGS    = -n +z
  CDEFINES   = -Dhp700 
  CCDEFINES  = -Dhp700 -DCANT_FWD_DECL_NESTED
  CINCLUDES  = 
  CCINCLUDES = -I$(SRCROOT)/osport/include -I$(SRCROOT)/ads/include -I/net/oxen/vol03/share/hp700/opt/galaCC/include/CC
  LINKER     = ld
  LINKOPTS   = -b
  MOVE       = mv
  MOVEOPTS   = -f 
  MKDIR      = mkdir
  MKDIROPTS  = -p
  LIBSFX     = sl
  OPTOPT     = -O
  DEBOPT     = -g
  CSFX       = c
  CPPSFX     = C
  OBJSFX     = o
endif

ifeq "$(ARCH)" "irix6-g"
  SRCROOT    = /paraset/src/extern
  LIBROOT    = /paraset/src/extern/lib
  OBJROOT    = /paraset/src/extern/obj
  OSTYPE     = irix6
  CCC        = /usr/bin/CC
  CFLAGS     = 
  CCFLAGS    = -KPIC 
  CDEFINES   = 
  CCDEFINES  = -Dirix6 -DCANT_FWD_DECL_NESTED -DSGIxUX
  CINCLUDES  = 
  CCINCLUDES = -I$(SRCROOT)/osport/include -I$(SRCROOT)/ads/include
  LINKER     = ld
  LINKOPTS   = -r
  MOVE       = mv 
  MOVEOPTS   = -f 
  MKDIR      = mkdir
  MKDIROPTS  = -p
  LIBSFX     = so
  OPTOPT     = -O
  DEBOPT     = -g
  CSFX       = c
  CPPSFX     = C
  OBJSFX     = o
endif

ifeq "$(ARCH)" "NT-g"
  SRCROOT    = n:/paraset/src/extern
  LIBROOT    = n:/paraset/src/extern/lib
  OBJROOT    = n:/paraset/src/extern/obj
  OSTYPE     = NT
  CCC        = cl
  CFLAGS     = 
  CCFLAGS    = 
  CDEFINES   = /D_WIN32 /DWIN32 /DPCxOS
  CCDEFINES  = /D_WIN32 /DWIN32 /DPCxOS
  LINKER     = link
  CINCLUDES  = 
  CCINCLUDES = /I$(SRCROOT)/osport/include /I$(SRCROOT)/ads/include
  LINKOPTS   = /dll
  MOVE       = mv
  MOVEOPTS   = -f 
  MKDIR      = mkdir
  MKDIROPTS  = -p
  LIBSFX     = dll
  OPTOPT     = /O
  DEBOPT     = /g
  CSFX       = c
  CPPSFX     = cxx
  OBJSFX     = obj
endif

##############################################
# Determine some debug-related stuff
##############################################

ifeq "X$(DEBUG)" "X"
  OBJDIR = $(OBJROOT)/$(OSTYPE)-O/libadsosport
  LIBDIR = $(LIBROOT)/$(OSTYPE)-O
  OPTLVL = $(OPTOPT)
else
  OBJDIR = $(OBJROOT)/$(OSTYPE)-g/libadsosport
  LIBDIR = $(LIBROOT)/$(OSTYPE)-g
  OPTLVL = $(DEBOPT)
endif

##############################################
# Common to all platforms
##############################################

SRCDIR1 = $(SRCROOT)/ads/source
SRCDIR2 = $(SRCROOT)/osport/source

VPATH  = $(SRCDIR1):$(SRCDIR2):$(OBJDIR):$(LIBDIR)

OBJS = \
	bitMEM.$(OBJSFX) \
	bufFile.$(OBJSFX) \
	dirFSet.$(OBJSFX) \
	endian.$(OBJSFX) \
	pdudir.$(OBJSFX) \
	pdufile.$(OBJSFX) \
	pduio.$(OBJSFX) \
	pdumem.$(OBJSFX) \
	pdupath.$(OBJSFX) \
	pduproc.$(OBJSFX) \
	pdustring.$(OBJSFX) \
	pdutime.$(OBJSFX) \
	pdutype.$(OBJSFX) \
	perfStat.$(OBJSFX) \
	raFile.$(OBJSFX) \
	argParam.$(OBJSFX) \
	argPrmAssign.$(OBJSFX) \
	cstring.$(OBJSFX) \
	dataSort.$(OBJSFX) \
	heapSort.$(OBJSFX) \
	indAccess.$(OBJSFX) \
	indFSet.$(OBJSFX) \
	intrCont.$(OBJSFX) \
	listitem.$(OBJSFX) \
	mergeSort.$(OBJSFX) \
	parray.$(OBJSFX) \
	pqueue.$(OBJSFX) \
	pstack.$(OBJSFX)

%.$(OBJSFX):	$(SRCDIR1)/%.$(CPPSFX)
	cd $(SRCDIR1); $(CCC) $(OPTLVL) $(CCFLAGS) $(CCINCLUDES) $(CCDEFINES) -c $<
	cd $(SRCDIR1); $(MOVE) $@ $(OBJDIR)

%.$(OBJSFX):	$(SRCDIR2)/%.$(CPPSFX)
	cd $(SRCDIR2); $(CCC) $(OPTLVL) $(CCFLAGS) $(CCINCLUDES) $(CCDEFINES) -c $<
	cd $(SRCDIR2); $(MOVE) $@ $(OBJDIR)

libadsosport.$(LIBSFX): $(OBJDIR) $(LIBDIR) $(OBJS)
	cd $(OBJDIR) ; $(LINKER) $(LINKOPTS) -o $@ $(OBJS)
	cd $(OBJDIR) ; $(MOVE) $(MOVEOPTS) $@ $(LIBDIR)

$(OBJDIR): 
	$(MKDIR) $(MKDIROPTS) $(OBJDIR)

$(LIBDIR): 
	$(MKDIR) $(MKDIROPTS) $(LIBDIR)
