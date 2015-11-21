########
#
# targets.mk - Common targets for integration.
#
##### History
#
# 11/10/95 tlw - Split off from Make.clients.components.
#
##### Notes
#
#	This file should work with all components.
#
##########

COBJ:=$(foreach f, $(CROOTS), $(f).o)
CPOBJ:=$(foreach f, $(CPROOTS), $(f).o)
CPPOBJ:=$(foreach f, $(CPPROOTS), $(f).o)
CXXOBJ:=$(foreach f, $(CXXROOTS), $(f).o)
LEXOBJ:=$(foreach f, $(LEXROOTS), $(f).lex.o)
LEXPPOBJ:=$(foreach f, $(LEXPPROOTS), $(f).lex.o)
YOBJ:=$(foreach f, $(YLROOTS), $(f).yacc.o) $(foreach f, $(YROOTS), $(f).yacc.o)
YPPOBJ:=$(foreach f, $(YLPPROOTS), $(f).yacc.o) $(foreach f, $(YPPROOTS), $(f).yacc.o)
YLOBJ:=$(foreach f, $(YLROOTS), $(f).lex.o)
YLPPOBJ:=$(foreach f, $(YLPPROOTS), $(f).lex.o)
UIMXOBJ:=$(foreach f, $(UIMXROOTS), $(f).o)
LTOBJ:=$(foreach f, $(LTROOTS), $(f).o)
OBJ:=$(YOBJ) $(YPPOBJ) $(YLOBJ) $(YLPPOBJ) $(LEXOBJ) $(LEXPPOBJ) $(CXXOBJ) $(UIMXOBJ) $(LTOBJ) $(COBJ) $(CPOBJ) $(CPPOBJ)

ifdef COBJ
$(COBJ): %.o: $(SRC_DIR)/%.c
	-@ rm -f $(*).o;
	$(CC) -c $(CFLAGS) $(SRC_DIR)/$*.c || (rm -f $(*).o; false)
endif

ifdef CPOBJ
$(CPOBJ): %.o: $(SRC_DIR)/%.C
	-@ rm -f $(*).o
	$(CCC) -c $(CCFLAGS) $(SRC_DIR)/$(*).C || (rm -f $(*).o; false)
endif

ifdef CPPOBJ
$(CPPOBJ): %.o: $(SRC_DIR)/%.cpp
	-@ rm -f $(*).o
	$(CCC) -c $(CCFLAGS) $(SRC_DIR)/$(*).cpp || (rm -f $(*).o; false)
endif

ifdef CXXOBJ
$(CXXOBJ): %.o: $(SRC_DIR)/%.cxx
	-@ rm -f $(*).o
	$(CCC) -c $(CCFLAGS) $(SRC_DIR)/$(*).cxx || (rm -f $(*).o; false)
endif

ifdef LEXOBJ
$(LEXOBJ): %.lex.o: $(SRC_DIR)/%.l
	rm -f $(*).lex.o $(*).lex.c $(*).sed lex.yy.c; \
	$(LEX) $(SRC_DIR)/$(*).l && \
	sed s/replace/$(*)/g $(TOOLS)/yy-lsed > $(*).sed && \
	sed -f $(*).sed lex.yy.c > $(*).lex.c && \
	rm -f lex.yy.c $(*).sed
	$(CC) -c $(CFLAGS) $(*).lex.c || (rm -f $(*).lex.o; false)
endif

ifdef LEXPPOBJ
$(LEXPPOBJ): %.lex.o: $(SRC_DIR)/%.l
	rm -f $(*).lex.o $(*).lex.C $(*).sed lex.yy.c; \
	$(FLEX) $(SRC_DIR)/$(*).l && \
	sed s/replace/$(*)/g $(TOOLS)/yy-lsed > $(*).sed && \
	sed -f $(*).sed lex.yy.c > $(*).lex.C && \
	rm -f lex.yy.c $(*).sed
	$(CCC) -c $(CCFLAGS) $(YLPPINC) $(*).lex.C || (rm -f $(*).lex.o; false)
endif

ifdef YLOBJ
$(YLOBJ): %.lex.o: %.yacc.o $(SRC_DIR)/%.l
	rm -f $(*).lex.o $(*).lex.c $(*).sed lex.yy.c; \
	$(LEX) $(SRC_DIR)/$(*).l && \
	sed s/replace/$(*)/g $(TOOLS)/yy-lsed > $(*).sed && \
	sed -f $(*).sed lex.yy.c > $(*).lex.c && \
	rm -f lex.yy.c $(*).sed
	$(CC) -c $(CFLAGS) $(*).lex.c || (rm -f $(*).lex.o; false)
endif

ifdef YLPPOBJ
$(YLPPOBJ): %.lex.o: %.yacc.o $(SRC_DIR)/%.l
	rm -f $(*).lex.o $(*).lex.C $(*).sed lex.yy.c; \
	$(FLEX) $(SRC_DIR)/$(*).l && \
	sed s/replace/$(*)/g $(TOOLS)/yy-lsed > $(*).sed && \
	sed -f $(*).sed lex.yy.c > $(*).lex.C && \
	rm -f lex.yy.c $(*).sed
	$(CCC) -c $(CCFLAGS) $(YLPPINC) $(*).lex.C || (rm -f $(*).lex.o; false)
endif

ifdef YOBJ
$(YOBJ): %.yacc.o: $(SRC_DIR)/%.y
	rm -f $(*).yacc.o $(*).yacc.c $(*).yacc.h $(*).sed y.tab.c y.tab.h; \
	$(YACC) -d $(SRC_DIR)/$(*).y && \
	sed s/replace/$(*)/g $(TOOLS)/yy-lsed > $(*).sed && \
	sed -f $(*).sed y.tab.c > $(*).yacc.c && \
	sed -f $(*).sed y.tab.h > $(*).yacc.h && \
	rm -f y.tab.c y.tab.h $(*).sed
	$(CC) -c $(CFLAGS) $(*).yacc.c || (rm -f $(*).yacc.o; false)
endif

ifdef YPPOBJ
$(YPPOBJ): %.yacc.o: $(SRC_DIR)/%.y
	rm -f $(*).yacc.o $(*).yacc.C $(*).yacc.h $(*).sed y.tab.c y.tab.h; \
	$(YACC) -d $(SRC_DIR)/$(*).y && \
	sed s/replace/$(*)/g $(TOOLS)/yy-lsed > $(*).sed && \
	sed -f $(*).sed y.tab.c > $(*).yacc.C && \
	sed -f $(*).sed y.tab.h > $(*).yacc.h && \
	rm -f y.tab.c y.tab.h $(*).sed
	$(CCC) -c $(CCFLAGS) $(YLPPINC) $(*).yacc.C || (rm -f $(*).yacc.o; false)
endif

ifdef UIMXOBJ
$(UIMXOBJ): %.o: $(SRC_DIR)/%.if
	rm -f $(*).o $(*).c; \
	$(UXCGEN) $(SRC_DIR)/$(*).if
	$(CC) -c $(CFLAGS) $(*).c || (rm -f $(*).o; false)
endif

ifdef LTOBJ
$(LTOBJ): %.o: $(SRC_DIR)/%.lt $(SRC_DIR)/%.inc
	rm -f $(*).o $(*).C $(*).lex; \
	$(SUBLEX) $(SRC_DIR)/$(*).inc < $(SRC_DIR)/$(*).lt > $(*).lex && \
	$(OLDFLEX) -S$(SRC_DIR)/flex.skel.cpp -8 -t $(*).lex > $(*).C
	$(CCC) -c $(CCFLAGS) $(*).C || (rm -f $(*).o; false)
endif

lib_$(LIB_NAME).o: $(FLIST) $(OBJ)
	rm -f lib_$(LIB_NAME).o
	$(LD) lib_$(LIB_NAME).o $(shell set -- $^;shift;echo $$*) \
		|| (rm -f lib_$(LIB_NAME).o; false)

### Target to make .code file from .spec file.
### Used for client/server galgen stuff.
###%.code: %.spec
###	galgen < $(<) > $(@) || (rm -f $(@) ; false)










