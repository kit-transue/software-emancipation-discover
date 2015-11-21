.SUFFIXES:
.SUFFIXES: .lex.obj .lex.c .l .yacc.obj .yacc.c .y

$(INTDIR)/%.lex.obj $(INTDIR)/%.lex.c : %.l
	cp $< $(INTDIR)/$(WORK_BASE)_tmp.l

$(INTDIR)/%.yacc.obj $(INTDIR)/%.yacc.c : %.y
	cp $< $(INTDIR)/$(WORK_BASE)_tmp.y

LEX_INCLUDE=/I "$(SYSCC)/include" $(EXTRN_INCLUDE)
#ROOTDIR variable is used inside of LEX.exe to find etc/yylex.c file
ROOTDIR=$(ADMMAKE)/../bin/win32
LEX_OUTPUT_FILE=lex.yy.c
LEX=$(ROOTDIR)/LEX.EXE -o $(LEX_OUTPUT_FILE)
LEX_SED=$(PATH2BS)/tools/yy-lsed

#BISON_SIMPLE="${ADMMAKE}/bison.simple"

YACC=$(ADMMAKE)/bison.exe -d

lex_targ: $(INTDIR)/$(WORK_BASE).lex.obj $(INTDIR)/$(WORK_BASE).lex.c
	$(LEX) $(INTDIR)/$(WORK_BASE)_tmp.l
	# please don't use 'mv' --it confuses clearcase and breaks builds
	cp $(LEX_OUTPUT_FILE) $(INTDIR)/$(WORK_BASE)_lex.c
	rm $(LEX_OUTPUT_FILE)
	sed s/replace/$(WORK_BASE)/g $(LEX_SED) > $(INTDIR)/$(WORK_BASE).sed 
	sed -f $(INTDIR)/$(WORK_BASE).sed $(INTDIR)/$(WORK_BASE)_lex.c > $(INTDIR)/$(WORK_BASE).lex.c
	$(CPP) /MT /D__STDC__=0 /I. /D"WIN32" $(CPP_PROJ) $(LEX_INCLUDE) $(CPP_INCLUDES) /Fo"$(INTDIR)/" /c $(INTDIR)/$(WORK_BASE).lex.c
	rm -f $(INTDIR)/$(WORK_BASE)_tmp.l $(INTDIR)/*.c $(INTDIR)/*.sed $(INTDIR)/y.*
	
yacch:  $(INTDIR)/$(WORK_BASE).yacc.obj $(INTDIR)/$(WORK_BASE).yacc.c
	$(YACC) $(INTDIR)/$(WORK_BASE)_tmp.y -o $(INTDIR)/y.tab.c 
	sed s/replace/$(WORK_BASE)/g $(LEX_SED) > $(INTDIR)/$(WORK_BASE).sed 
	sed -f $(INTDIR)/$(WORK_BASE).sed $(INTDIR)/y.tab.c > $(INTDIR)/$(WORK_BASE).yacc.c
	sed -f $(INTDIR)/$(WORK_BASE).sed $(INTDIR)/y.tab.h > $(INTDIR)/$(WORK_BASE).yacc.h
	$(CPP) /MT /D__STDC__=0 /D"WIN32" $(CPP_PROJ) $(LEX_INCLUDE) $(CPP_INCLUDES) /Fo"$(INTDIR)/" /c $(INTDIR)/$(WORK_BASE).yacc.c
	rm -f $(INTDIR)/$(WORK_BASE)_tmp.y $(INTDIR)/*.c $(INTDIR)/*.sed $(INTDIR)/y.*


