PATHTOROOT=../../..
MAKESCRIPTSPATH=$(PATHTOROOT)/makescripts/Windows
include $(MAKESCRIPTSPATH)/defs.mak

LIBRARY=$(INTDIR)/source.lib
MAKE=$(MAKEEXE) $(MAKEOPT)

OBJS= \
	$(INTDIR)/charOpts.obj \
	$(INTDIR)/chartImage.obj \
	$(INTDIR)/reportTemplateParser.yacc.obj \
	$(INTDIR)/reportTemplateParser.lex.obj \
	$(INTDIR)/reportDataParser.yacc.obj \
	$(INTDIR)/reportDataParser.lex.obj \
	$(INTDIR)/descParser.yacc.obj \
	$(INTDIR)/descParser.lex.obj \
	$(INTDIR)/descParser.obj \
	$(INTDIR)/inputItem.obj \
	$(INTDIR)/inputs.obj \
	$(INTDIR)/inventory.obj \
	$(INTDIR)/inventoryItem.obj \
	$(INTDIR)/metric.obj \
	$(INTDIR)/metricFolder.obj \
	$(INTDIR)/qualityItem.obj \
	$(INTDIR)/query.obj \
	$(INTDIR)/queryFolder.obj \
	$(INTDIR)/reportDataParser.obj \
	$(INTDIR)/reporterArgs.obj \
	$(INTDIR)/reportItem.obj \
	$(INTDIR)/reportItemList.obj \
	$(INTDIR)/reportStream.obj \
	$(INTDIR)/reportStreamHTML.obj \
	$(INTDIR)/reportStreamText.obj \
	$(INTDIR)/reportTemplate.obj \
	$(INTDIR)/rootFolder.obj \
	$(INTDIR)/sevreporter.obj

VPATH=../../src

CPP_INCLUDES=\
	/I $(INTDIR) \
	/I ../../include \
 	/I ../../gd/include \
	/I ../../gdchart/include \
	/I $(PATHTOROOT)/gala/extern/osport/include \
	/I $(PATHTOROOT)/machine/include \
	/I $(PATHTOROOT)/gala/extern/ads/include \
	/I $(PATH2BS)/gala/extern/osport/include \
	/I $(PATH2BS)/machine/include \
	/I $(PATH2BS)/gala/extern/ads/include

CPLUSPLUS_FLAGS=

CPP_PROJ=/MT\
 $(CPP_OPTS)\
 $(CPP_WINDOWS)\
 $(CPP_DEFINES)\
 $(CPP_INCLUDES)\
 $(CPP_FINISH)

$(INTDIR)/reportTemplateParser.lex.obj	:	$(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I ../../include \
			/I $(INTDIR) \
			/I $(PATHTOROOT)/gala/extern/ads/include \
			/I $(PATHTOROOT)/gala/extern/osport/include \
			/I $(PATH2BS)/gala/extern/ads/include \
			/I $(PATH2BS)/gala/extern/osport/include \
			/I ."\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE\
			 -S$(ADMMAKE)/FLEX_2.5.4.SKL -i"\
		CPP_PROJ="/D YY_SKIP_YYWRAP"\
		VPATH=../../src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=reportTemplateParser lex_targ

$(INTDIR)/reportTemplateParser.yacc.h	$(INTDIR)/reportTemplateParser.yacc.obj	: $(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I ../../include \
			/I $(INTDIR) \
			/I $(PATHTOROOT)/gala/extern/ads/include \
			/I $(PATHTOROOT)/gala/extern/osport/include \
			/I $(PATH2BS)/gala/extern/ads/include \
			/I $(PATH2BS)/gala/extern/osport/include \
			/I ."\
		YACC="$(ADMMAKE)/bison.exe -d -y"\
		VPATH=../../src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=reportTemplateParser yacch

$(INTDIR)/reportDataParser.lex.obj	 : $(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I ../../include \
			/I $(INTDIR) \
			/I $(PATHTOROOT)/gala/extern/ads/include \
			/I $(PATHTOROOT)/gala/extern/osport/include \
			/I $(PATH2BS)/gala/extern/ads/include \
			/I $(PATH2BS)/gala/extern/osport/include \
			/I ."\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE\
			 -S$(ADMMAKE)/FLEX_2.5.4.SKL -i"\
		CPP_PROJ="/D YY_SKIP_YYWRAP"\
		VPATH=../../src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=reportDataParser lex_targ

$(INTDIR)/reportDataParser.yacc.h	$(INTDIR)/reportDataParser.yacc.obj	 : $(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I ../../include \
			/I $(INTDIR) \
			/I $(PATHTOROOT)/gala/extern/ads/include \
			/I $(PATHTOROOT)/gala/extern/osport/include \
			/I $(PATH2BS)/gala/extern/ads/include \
			/I $(PATH2BS)/gala/extern/osport/include \
			/I ."\
		YACC="$(ADMMAKE)/bison.exe -d -y"\
		VPATH=../../src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=reportDataParser yacch

$(INTDIR)/descParser.yacc.h	$(INTDIR)/descParser.yacc.obj : $(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I ../../include \
			/I $(INTDIR) \
			/I $(PATHTOROOT)/gala/extern/ads/include \
			/I $(PATHTOROOT)/gala/extern/osport/include \
			/I $(PATH2BS)/gala/extern/ads/include \
			/I $(PATH2BS)/gala/extern/osport/include \
			/I ."\
		YACC="$(ADMMAKE)/bison.exe -d -y"\
		VPATH=../../src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=descParser yacch

$(INTDIR)/descParser.lex.obj : $(INTDIR)
	$(MAKE) -f $(YL_MAKE)\
		EXTRN_INCLUDE="$(EXTRN_INCLUDE) \
			/I ../../include \
			/I $(INTDIR) \
			/I $(PATHTOROOT)/gala/extern/ads/include \
			/I $(PATHTOROOT)/gala/extern/osport/include \
			/I $(PATH2BS)/gala/extern/ads/include \
			/I $(PATH2BS)/gala/extern/osport/include \
			/I ."\
		LEX="$(ADMMAKE)/FLEX_2.5.4.EXE\
			 -S$(ADMMAKE)/FLEX_2.5.4.SKL -i"\
		CPP_PROJ="/D YY_SKIP_YYWRAP"\
		VPATH=../../src\
		INTDIR=$(INTDIR)\
		CPP="$(CPP)" \
		WORK_BASE=descParser lex_targ

include $(MAKESCRIPTSPATH)/targets.mak

