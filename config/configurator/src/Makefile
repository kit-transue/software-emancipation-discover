###########################################################
# DEFINED IN ALL MAKEFILES:
###########################################################
 
include $(MAKELIB)/make_options

ifndef $(MODE)
MODE=O
endif
 
BUILD_DATE = echo 'char build_date[] = "'"`date`"'";'

###########################################################
# MACRO DEFINITIONS
###########################################################

EXECUTABLES = configurator

OBJ = \
	configurator.o

CFILES = \
	configurator.c 

###########################################################
# TARGETS
###########################################################
### GENERIC TARGETS

# Default make target
all : $(EXECUTABLES)

# Date creation automation
DYNAMIC_DATE_FILE.o :
	rm -f DYNAMIC_DATE_FILE.c DYNAMIC_DATE_FILE.o
	$(BUILD_DATE) > DYNAMIC_DATE_FILE.c
	$(CC) $(CFLAGS) -c DYNAMIC_DATE_FILE.c -o DYNAMIC_DATE_FILE.o

# Delete junk files
clean :
	rm -f *.o $(EXECUTABLES) $(OBJ) DYNAMIC_DATE_FILE.c

### SPECIFIC TARGETS

configurator : $(OBJ) DYNAMIC_DATE_FILE.o
	$(CC) DYNAMIC_DATE_FILE.o $(OBJ) $(LFLAGS) -o $(@)

###########################################################
# SUFFIX RULES
###########################################################
 
.SUFFIXES :
.SUFFIXES : .c .o
 
.c.o :
	#$(CC) $(DEFINES) $(ASETINCLUDES) $(CINCLUDES) $(XINCLUDES) $(FRAMEINCLUDES) -c $< -o $@
	$(CC) $(CFLAGS) $(XINCLUDES) -c $< -o $@
