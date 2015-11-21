# Solaris 2.4

CCC	=/net/mothra/opt/SUNWspro/bin/CC

DEFINES	=-Dsun54

INCLUDE	=-I../include -I./

HEADERS	=

PRIMARY	=flexScanner.o \
	lexToken.o \
	regexp.o

.SUFFIXES: .cxx

.cxx.o:
	$(CCC) $(INCLUDE) $(DEFINES) -c $<

parse:	$(PRIMARY)
	$(CCC) -o parse $(PRIMARY)

clean:
	rm -f *.o

# header file dependencies
$(PRIMARY): $(HEADERS)
