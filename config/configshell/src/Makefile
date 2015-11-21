###########################################################
# DEFINED IN ALL MAKEFILES:
###########################################################
 
include $(MAKELIB)/make_options

ifndef $(MODE)
MODE=O
endif

CCDEFINES:=-D$(ARCH) $(CCDEFINES)
 
BUILD_DATE = echo 'char build_date[] = "'"`date`"'";'

###########################################################
# MACRO DEFINITIONS
###########################################################

EXECUTABLES = configshell

CCINCLUDES=-I$(LOCAL_SRC_ROOT)/machine/include -I$(BUILD_SRC_ROOT)/machine/include

OBJ = \
	configshell.o\
	totop.o

CCFILES = \
	configshell.C \
	totop.C \
	
###########################################################
# TARGETS
###########################################################
### GENERIC TARGETS

# Default make target
all: $(EXECUTABLES)

# Date creation automation
DYNAMIC_DATE_FILE.o :
	rm -f DYNAMIC_DATE_FILE.c DYNAMIC_DATE_FILE.o
	$(BUILD_DATE) > DYNAMIC_DATE_FILE.c
	$(CCC) -c DYNAMIC_DATE_FILE.c -o DYNAMIC_DATE_FILE.o
 
# Delete junk files
clean :
	rm -f *.o *~ \#*\# DYNAMIC_DATE_FILE.c DYNAMIC_DATE_FILE.o $(OBJ) $(EXECUTABLES)

### SPECIFIC TARGETS

configshell : $(OBJ) DYNAMIC_DATE_FILE.o
	echo "Building target now."
	$(CCC) DYNAMIC_DATE_FILE.o $(OBJ) -o $@ $(LFLAGS) $(X11LIB)

###########################################################
# SUFFIX RULES
###########################################################
 
.SUFFIXES :
.SUFFIXES : .C .o
 
.C.o :
	#$(CCC) -c $(CCDEFINES) $(ASETINCLUDES) $(CCINCLUDES) $(XINCLUDES) $(DEFINES) $< -o $@
	$(CCC) -c $(CCFLAGS) $< -o $@
