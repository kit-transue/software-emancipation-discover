#
# This file was the makefile used to build xxinterface.o and license_stubs.o.
# Now, these files will be built during the regular software build.  To
# generate xxinterface.cxx and xxinterface.h from xxinterface.pseudo, use
# genxxinterface.pl > xxinterface.cxx, as described in the target below.
#
# All edits should be made to xxinterface.pseudo.  DO NOT make edits directly
# to xxinterface.cxx.
#

ifeq "$(Arch)" "sun4"
  LMGRARCH = sun4_u4
  CCC = ${CC_ROOTDIR}/CC -DUSE_SPARCWORKS
endif

ifeq "$(Arch)" "sun5"
  LMGRARCH = sun4_u5
  NETLIB = -lsocket -lnsl
  CCC = ${CC_ROOTDIR}/bin/CC
endif

ifeq "$(Arch)" "hp700"
  LMGRARCH = hp700_u9
  CFLAGS = +a1
  CCC = /net/oxen/vol01/share/hp700/opt/galaCC/bin/CC -I/net/oxen/vol01/share/hp700/opt/galaCC/include/CC
endif

ifeq "$(Arch)" "hp10"
  LMGRARCH = hp700_u9
  CFLAGS = +a1
  CCC = ${CC_ROOTDIR}/bin/aCC
endif

ifeq "$(Arch)" "irix6"
  LMGRARCH = sgi32_u6
  CCC = ${CC_ROOTDIR}/bin/CC
endif

LMGRHOME	= ../../FLEXlm/flexlm/current
LMGRINCLUDE	= $(LMGRHOME)/machind
LMGRLIB		= -L$(LMGRHOME)/$(LMGRARCH) -llmgr

INCLUDES	= $(LMGRINCLUDE)
LIBS		= $(LMGRLIB) $(NETLIB)
OPTFLAGS	= -O
DBGFLAGS	= -g
DEFINES		= -D$(Arch)

RSH_CMD		= "/usr/atria/bin/cleartool setview -exec \"cd /paraset/src/xxinterface/unix ; build.csh\" $(VIEW)"

default : $(Arch)/xxinterface.o $(Arch)/xxaccess.o $(Arch)/license_stubs.o

all : FORCE
	rsh oxen $(RSH_CMD)
	rsh pragma $(RSH_CMD)
	rsh drift $(RSH_CMD)
	rsh loner $(RSH_CMD)

xxinterface.cxx : xxinterface.pseudo genxxinterface.pl
	genxxinterface.pl > xxinterface.cxx

$(Arch)/xxinterface.o : xxinterface.cxx xxinterface.h
	$(CCC) $(OPTFLAGS) $(CFLAGS) $(DEFINES) -c -o $@ -I$(LMGRINCLUDE) -I/paraset/include xxinterface.cxx

$(Arch)/xxinterface_dbg.o : xxinterface.cxx xxinterface.h
	$(CCC) $(DBGFLAGS) $(CFLAGS) $(DEFINES) -c -o $@ -I$(LMGRINCLUDE) -I/paraset/include xxinterface.cxx

$(Arch)/license_stubs.o : license_stubs.cxx
	$(CCC) $(OPTFLAGS) $(CFLAGS) $(DEFINES) -c -o $@ license_stubs.cxx

FORCE : 
