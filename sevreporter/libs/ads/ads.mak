PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/ads.lib

OBJS= \
	$(INTDIR)/argParam.obj \
	$(INTDIR)/argPrmAssign.obj \
	$(INTDIR)/cstring.obj \
	$(INTDIR)/parray.obj

VPATH=$(PATHTOROOT)/gala/extern/ads/src;$(PATH2BS)/gala/extern/ads/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/gala/extern/ads/include \
	/I $(PATHTOROOT)/gala/extern/osport/include \
	/I $(PATHTOROOT)/machine/include\
	/I $(PATH2BS)/gala/extern/ads/include \
	/I $(PATH2BS)/gala/extern/osport/include \
	/I $(PATH2BS)/machine/include

CPP_PROJ=/MT\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


