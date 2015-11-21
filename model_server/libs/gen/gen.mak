include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/gen.lib

OBJS= \
	$(INTDIR)/Args.obj \
	$(INTDIR)/RegLex.obj \
	$(INTDIR)/charstream.obj \
	$(INTDIR)/error.obj \
	$(INTDIR)/exec_path.obj \
	$(INTDIR)/genEnum.obj \
	$(INTDIR)/genError.obj \
	$(INTDIR)/genMask.obj \
	$(INTDIR)/genString.obj \
	$(INTDIR)/genStringPlus.obj \
	$(INTDIR)/genTable.obj \
	$(INTDIR)/genTimeStamp.obj \
	$(INTDIR)/genTmpfile.obj \
	$(INTDIR)/genWild.obj \
	$(INTDIR)/globals.obj \
	$(INTDIR)/msgDict.obj \
	$(INTDIR)/options.obj \
	$(INTDIR)/psetmem.obj \
	$(INTDIR)/psetmem2.obj \
	$(INTDIR)/regexp.obj \
	$(INTDIR)/resolvepath.obj \
	$(INTDIR)/time_log.obj \
	$(INTDIR)/trace.obj \
	$(INTDIR)/vpopen.obj

VPATH=../../../gen/src;\
      $(PATH2BS)/gen/src

CPP_INCLUDES=\
/I ../../dd/include \
/I ../../debug/include \
/I ../../driver_aset/include \
/I ../../interface/include \
/I ../../machine_prefs/include \
/I ../../obj/include \
/I ../../process/include \
/I ../../search/include \
/I ../../ui/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \

include $(ADMMAKE)/targets.mak
