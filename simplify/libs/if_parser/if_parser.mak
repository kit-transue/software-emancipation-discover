PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/if_parser.lib

OBJS= \
	$(INTDIR)/iff_parser.lex.obj \
	$(INTDIR)/iff_parser.yacc.obj \
	$(INTDIR)/astbodyStruct.obj \
	$(INTDIR)/astnodeList.obj \
	$(INTDIR)/astnodeStruct.obj \
	$(INTDIR)/attributeList.obj \
	$(INTDIR)/attributeStruct.obj \
	$(INTDIR)/discover-interface.obj \
	$(INTDIR)/ifFileCache.obj \
	$(INTDIR)/locationList.obj \
	$(INTDIR)/locationStruct.obj \
	$(INTDIR)/maplineList.obj \
	$(INTDIR)/maplineStruct.obj \
	$(INTDIR)/nodeinfoStruct.obj \
	$(INTDIR)/positionStruct.obj \
	$(INTDIR)/relationStruct.obj \
	$(INTDIR)/stringList.obj \
	$(INTDIR)/symbolStruct.obj \
	$(INTDIR)/transformationList.obj \
	$(INTDIR)/transformationStruct.obj \
	$(INTDIR)/uintList.obj

VPATH=$(PATHTOROOT)/els/if_parser/src;$(PATH2BS)/els/if_parser/src

CPP_INCLUDES=\
	/I $(PATHTOROOT)/gala/extern/osport/include \
	/I $(PATHTOROOT)/gala/extern/ads/include \
	/I $(PATHTOROOT)/els/if_parser/include \
	/I $(PATHTOROOT)/els/mapper/include \
	/I $(PATHTOROOT)/gen/include \
	/I $(PATHTOROOT)/machine/include \
	/I $(PATHTOROOT)/model_server/debug/include \
	/I $(PATH2BS)/gala/extern/osport/include \
	/I $(PATH2BS)/gala/extern/ads/include \
	/I $(PATH2BS)/els/if_parser/include \
	/I $(PATH2BS)/els/mapper/include \
	/I $(PATH2BS)/gen/include \
	/I $(PATH2BS)/machine/include \
	/I $(PATH2BS)/model_server/debug/include

CPP_PROJ=/MD \
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

################################################################################

$(INTDIR)/iff_parser.yacc.h	$(INTDIR)/iff_parser.yacc.obj	: $(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I $(PATHTOROOT)/els/if_parser/include \
			/I $(PATHTOROOT)/machine/include\
			/I $(PATH2BS)/els/if_parser/include \
			/I $(PATH2BS)/machine/include"\
		YACC="$(ADMMAKE)/BISON.EXE -d -y"\
		VPATH=$(PATHTOROOT)/els/if_parser/src:$(PATH2BS)/els/if_parser/src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=iff_parser yacch


################################################################################
# Begin Source File

$(INTDIR)/iff_parser.lex.obj : $(INTDIR) $(INTDIR)/iff_parser.yacc.obj
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I $(PATHTOROOT)/els/if_parser/include \
			/I $(PATHTOROOT)/machine/include\
			/I $(PATH2BS)/els/if_parser/include \
			/I $(PATH2BS)/machine/include"\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE\
			 -S$(ADMMAKE)/FLEX_2.5.4.SKL -i"\
		CPP_PROJ="/MD /D YY_SKIP_YYWRAP"\
		VPATH=$(PATHTOROOT)/els/if_parser/src:$(PATH2BS)/els/if_parser/src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=iff_parser lex_targ

################################################################################

include $(MAKESCRIPTSPATH)/targets.mak


