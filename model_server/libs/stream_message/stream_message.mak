include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/stream_message.lib

OBJS= \
	$(INTDIR)/msg_comms.obj \
	$(INTDIR)/msg_format.obj \
	$(INTDIR)/minidom.obj \
	$(INTDIR)/minixml.obj \
	$(INTDIR)/minisax.obj \
	$(INTDIR)/aconnectn.obj \
	$(INTDIR)/mbmsgser.obj \
	$(INTDIR)/mbuildmsg.obj \
	$(INTDIR)/service.obj \
	$(INTDIR)/startprocess.obj \
	$(INTDIR)/transport.obj 

VPATH=../../../stream_message/src; \
      $(PATH2BS)/stream_message/src

CPP_PROJ=$(CPP_BASE)\
	/I "../../../stream_message/include"\
	/I "../../../nameserver/include"\
	/I "$(PATH2BS)/stream_message/include"\
	/I "$(PATH2BS)/nameserver/include"\
	/D WIN32 /D "_WINDOWS"\
	/D vdebugDEBUG=0\
	/Fo"$(INTDIR)/" /c

include $(ADMMAKE)/targets.mak
