BUILDMSG=message
LIB32_FLAGS=/nologo /out:$(LIBRARY)

ALL : $(BUILDMSG) $(LIBRARY)

$(BUILDMSG):
	-@echo "Building all..."

CLEAN : 
	-@echo "Cleaning..."
	-@rm -f $(OBJS)
	-@rm -f $(LIBRARY)

$(INTDIR) :
	$(if $(shell test -e $(INTDIR)),,-@echo "Creating $(INTDIR) directory...";mkdir $(INTDIR))

$(LIBRARY) : $(INTDIR) $(OBJS)
	-@echo "Creating library $(LIBRARY)..."
	$(LIB32) $(LIB32_FLAGS) $(OBJS)

include $(ADMMAKE)/suffixes.mak

