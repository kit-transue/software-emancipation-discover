# Solaris 2.4

CCC	=/net/mothra/opt/SUNWspro/bin/CC
LTC	=/paraset/admin/int_tools/bin/sun5/sublex
LC	=/paraset/admin/int_tools/bin/sun5/flex
LCSKEL	=/paraset/src/extern/parse/cplex/flex.skel.cpp

DEFINES	=-Dsun54

INCLUDE	=-I../../include -I./

HEADERS	=

# needs flexScanner.o, lexToken.o, and regexp.o

PRIMARY	=main.o \
	example.o

.SUFFIXES: .cxx .lt

.cxx.o:
	$(CCC) $(INCLUDE) $(DEFINES) -c $<

.lt.o:
	$(LTC) $*.inc < $< > $*.lex
	$(LC) -S$(LCSKEL) -8 -t $*.lex > $*.cxx
	$(CCC) $(DEFINES) $(INCLUDE) -c $*.cxx > $@

parse:	$(PRIMARY)
	$(CCC) -o parse $(PRIMARY)

clean:
	rm -f *.o *.lex

# header file dependencies
$(PRIMARY): $(HEADERS)
