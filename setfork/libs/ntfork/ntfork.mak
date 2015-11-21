PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/ntfork.lib

OBJS= \
	$(INTDIR)/fork.obj

VPATH=$(PATHTOROOT)/gala/extern/osport/ntfork;$(PATH2BS)/gala/extern/osport/ntfork

CPP_INCLUDES=\
	/I $(PATHTOROOT)/gala/extern/osport/include \
	/I $(PATH2BS)/gala/extern/osport/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak


