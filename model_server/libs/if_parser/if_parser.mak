include $(ADMMAKE)/defs.mak

LIBRARY=$(INTDIR)/if_parser.lib

OBJS= \
	$(INTDIR)/astbodyStruct.obj \
	$(INTDIR)/astnodeList.obj \
	$(INTDIR)/astnodeStruct.obj \
	$(INTDIR)/attributeList.obj \
	$(INTDIR)/attributeStruct.obj \
	$(INTDIR)/dirFSet.obj \
	$(INTDIR)/discover-interface.obj \
	$(INTDIR)/ifFileCache.obj \
	$(INTDIR)/iff_parser.yacc.obj \
	$(INTDIR)/iff_parser.lex.obj \
	$(INTDIR)/symbolStruct.obj \
	$(INTDIR)/locationList.obj \
	$(INTDIR)/locationStruct.obj \
	$(INTDIR)/maplineList.obj \
	$(INTDIR)/maplineStruct.obj \
	$(INTDIR)/nodeinfoStruct.obj \
	$(INTDIR)/positionStruct.obj \
	$(INTDIR)/relationStruct.obj \
	$(INTDIR)/stringList.obj \
	$(INTDIR)/transformationList.obj \
	$(INTDIR)/transformationStruct.obj \
	$(INTDIR)/uintList.obj

VPATH=../../../els/if_parser/src;\
      ../../debug/src;\
      $(PATH2BS)/els/if_parser/src;\
      $(PATH2BS)/model_server/debug/src

CPP_INCLUDES=\
/I ../../debug/include \
/I ../../../els/if_parser/include \
/I ../../../els/mapper/include \
/I ../../../gala/extern/ads/include \
/I ../../../gala/extern/osport/include \
/I ../../../gen/include \
/I ../../../machine/include \
/I $(PATH2BS)/els/if_parser/include \
/I $(PATH2BS)/els/mapper/include \
/I $(PATH2BS)/gala/extern/ads/include \
/I $(PATH2BS)/gala/extern/osport/include \
/I $(PATH2BS)/gen/include \
/I $(PATH2BS)/machine/include \

include $(ADMMAKE)/targets.mak

################################################################################

SOURCE=iff_parser.y

$(INTDIR)/iff_parser.yacc.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH="$(VPATH)"\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		CPP_PROJ="/D YYSSIZE=10000"\
		WORK_BASE=iff_parser yacch

################################################################################

SOURCE=iff_parser.l

$(INTDIR)/iff_parser.lex.obj : $(INTDIR) $(SOURCE)
	$(MAKE) -f $(ADMMAKE)/makefile.lex\
		CPP=$(CPP)\
		VPATH="$(VPATH)"\
		INTDIR=$(INTDIR)\
		CPP_INCLUDES="$(CPP_INCLUDES)"\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE \
			-S$(ADMMAKE)/FLEX_2.5.4.SKL"\
                LEX_OUTPUT_FILE="lex.yy.c"\
		WORK_BASE=iff_parser lex_targ

################################################################################
