SOURCES:=\
alloca.c \
cLibraryFunctions.cxx \
fd_fstream.cxx \
fd_streambuf.cxx \
machdep.cxx \
map_manager.cxx \
shell_calls.cxx \
test.cxx \

ifeq ($(mc_platform),mingw)
SOURCES+= \
DisFName.cxx \
DisRegistry.cxx
endif

# EOF