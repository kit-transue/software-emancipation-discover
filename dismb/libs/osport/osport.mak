PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/osport.lib

OBJS= \
	$(INTDIR)/bitMEM.obj \
	$(INTDIR)/dirFSet.obj \
	$(INTDIR)/pdudir.obj \
	$(INTDIR)/pdufile.obj \
	$(INTDIR)/pduio.obj \
	$(INTDIR)/pdumem.obj \
	$(INTDIR)/pdupath.obj \
	$(INTDIR)/pduproc.obj \
	$(INTDIR)/pdustring.obj \
	$(INTDIR)/pdutime.obj \
	$(INTDIR)/pdutype.obj \
	$(INTDIR)/endian.obj \
	$(INTDIR)/perfStat.obj \
	$(INTDIR)/raFile.obj

VPATH=$(PATHTOROOT)/gala/extern/osport/src;$(PATH2BS)/gala/extern/osport/src

CPP_WINDOWS=/D "WIN32" /D "_WIN32" /D "PCxOS"
CPP_INCLUDES=\
	/I $(PATHTOROOT)/gala/extern/osport/include\
	/I $(PATHTOROOT)/gala/extern/ads/include \
	/I $(PATH2BS)/gala/extern/osport/include\
	/I $(PATH2BS)/gala/extern/ads/include

CPP_PROJ=/MT \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak

