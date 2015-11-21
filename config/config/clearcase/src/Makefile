###########################################################
# DEFINED IN ALL MAKEFILES:
###########################################################
 
include $(MAKELIB)/make_options
 
BUILD_DATE = echo 'char build_date[] = "'"`date`"'";'
 
 
###########################################################
# MACRO DEFINITIONS
###########################################################

EXECUTABLES= \
	atria_revisions \
	atria_revision_list \
	atria_revisions_only \
	atria_keepfile \
	atria_filter

###########################################################
# TARGETS
###########################################################

all : $(EXECUTABLES)

clean :
	rm -f $(EXECUTABLES)

### SPECIFIC TARGETS

atria_filter:
	-cp $(@) .

atria_revisions: atria_revisions.c 

atria_revision_list: atria_revision_list.c

atria_revisions_only: atria_revisions_only.c

atria_keepfile: atria_keepfile.c

###########################################################
# SUFFIX RULES
###########################################################
 
.SUFFIXES :
.SUFFIXES : .l .c .o
 
.l.c :
	$(LEX) -t $< > $@

.c.o :
	$(CC) -c $< -o $@
 
.c :
	$(CC) -o $@ $<
