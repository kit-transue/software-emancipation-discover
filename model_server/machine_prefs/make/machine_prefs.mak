include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/machine_prefs.lib

OBJS= \
	$(INTDIR)/customize.h.obj \
	$(INTDIR)/evalMetaChars.h.obj \
	$(INTDIR)/prefFile.obj \
	$(INTDIR)/pref_registry.obj \
	$(INTDIR)/userPrefFile.obj 

VPATH=../src

CPP_INCLUDES=\
/I ../include \
/I ../../api/include \
/I ../../dd/include \
/I ../../driver_aset/include \
/I ../../els/include \
/I ../../interface/include \
/I ../../ldr/include \
/I ../../obj/include \
/I ../../project/include \
/I ../../project_pdf/include \
/I ../../rtl/include \
/I ../../search/include \
/I ../../smt/include \
/I ../../smt_metric/include \
/I ../../ste/include \
/I ../../style/include \
/I ../../ui/include \
/I ../../view/include \
/I ../../view_rtl/include \
/I ../../xref/include \
/I ../../../gala/extern/ads/include \
/I ../../../gala/extern/osport/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I ../../../nihcl/include \
/I ../../../stream_message/include \
/I ../../../dis_tcl8.3/generic \
/I ../../../dis_tk8.3/generic \
/I $(PATH2BS)/gala/extern/ads/include \
/I $(PATH2BS)/gala/extern/osport/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \
/I $(PATH2BS)/nihcl/include \
/I $(PATH2BS)/stream_message/include \
/I $(PATH2BS)/dis_tcl8.3/generic \
/I $(PATH2BS)/dis_tk8.3/generic \

include $(ADMMAKE)/targets.mak
