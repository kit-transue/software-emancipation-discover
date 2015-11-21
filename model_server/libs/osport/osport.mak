include $(ADMMAKE)/defs.mak

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

VPATH=../../../gala/extern/osport/src; \
      $(PATH2BS)/gala/extern/osport/src

CPP_PROJ=$(CPP_BASE)\
	/I "../../../gala/extern/osport/include"\
	/I "../../../gala/extern/ads/include"\
	/I "$(PATH2BS)/gala/extern/osport/include"\
	/I "$(PATH2BS)/gala/extern/ads/include"\
	/I "$(GALAXYHOME)/include"\
	/D "WIN32" /D "_WINDOWS" /D "PCxOS"\
	/D vdebugDEBUG=0\
	/Fo"$(INTDIR)/" /c

include $(ADMMAKE)/targets.mak
