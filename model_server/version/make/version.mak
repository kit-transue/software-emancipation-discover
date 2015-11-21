include $(ADMMAKE)/defs.mak

DEFAULT : ALL

LIBRARY=$(INTDIR)/version.lib

OBJS= \
	$(INTDIR)/date.obj

SOURCE=$(INTDIR)/date.cxx

$(INTDIR)/date.obj : $(INTDIR)
	cp ../src/timestamp.cxx $(INTDIR)/timestamp.cxx
	$(CPP) /o $(INTDIR)/timestamp.exe /Fo"$(INTDIR)/" $(INTDIR)/timestamp.cxx
	$(INTDIR)/timestamp.exe > $(SOURCE)
	$(CPP) $(CPP_PROJ) $(SOURCE)

include $(ADMMAKE)/targets.mak
