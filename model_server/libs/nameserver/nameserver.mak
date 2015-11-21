include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/nameserver.lib

OBJS= \
	$(INTDIR)/nameServCalls.obj 

VPATH=../../../nameserver/src; \
      $(PATH2BS)/nameserver/src

CPP_PROJ=$(CPP_BASE)\
	/I "../../../nameserver/include"\
	/I "$(PATH2BS)/nameserver/include"\
	/D "WIN32" /D "_WINDOWS"\
	/D vdebugDEBUG=0\
	/Fo"$(INTDIR)/" /c

include $(ADMMAKE)/targets.mak
