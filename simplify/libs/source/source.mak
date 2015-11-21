PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/source.lib

OBJS= \
	$(INTDIR)/cleanup.obj \
	$(INTDIR)/InclusionNode.obj \
	$(INTDIR)/InclusionTree.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/ModifySrc.obj \
	$(INTDIR)/ParserCallBacks.obj \
	$(INTDIR)/simplify.obj \
	$(INTDIR)/simplify_CBS.obj \
	$(INTDIR)/simplify_globals.obj

VPATH=../../src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/gala/extern/osport/include \
	/I $(PATHTOROOT)/gala/extern/ads/include \
	/I $(PATHTOROOT)/els/if_parser/include \
 	/I $(PATHTOROOT)/machine/include \
	/I $(PATH2BS)/gala/extern/osport/include \
	/I $(PATH2BS)/gala/extern/ads/include \
	/I $(PATH2BS)/els/if_parser/include \
 	/I $(PATH2BS)/machine/include \
	/I ../../include


CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

include $(MAKESCRIPTSPATH)/targets.mak
