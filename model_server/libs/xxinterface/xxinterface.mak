include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/xxinterface.lib

OBJS= \
	$(INTDIR)/xxinterface.obj \
	$(INTDIR)/license_stubs.obj \
	$(INTDIR)/xxsn.obj

VPATH=../../../xxinterface/src;\
      $(PATH2BS)/xxinterface/src

CPP_INCLUDES=\
/I ../../interface/include \
/I ../../ui/include \
/I ../../../FLEXlm/flexlm/current/machind \
/I ../../../gen/include \
/I ../../../stream_message/include \
/I ../../../xxinterface/include \
/I $(PATH2BS)/FLEXlm/flexlm/current/machind \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/xxinterface/include \

include $(ADMMAKE)/targets.mak
