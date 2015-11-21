PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/source.lib

OBJS= \
	$(INTDIR)/disperl.obj

VPATH=../../src

CPLUSPLUS_FLAGS=

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

