CCC	= gcc
LDFLAGS	= 

CFLAGS	= 
LIBS	= 

SRCDIR	= ../src
INCLUDES=-I../../../private/ttt/include -I../../../private/sysinclude \
	-I../../../src/ttt/include -I../../../src/sysinclude \
	-I$$PSETHOME/include/cp-include -I$$PSETHOME/include

OBJ =  color.o \
	location.o \
	move.o \
	TicTacToeMove.o \
	square.o \
	application.o \
	display.o \
	board.o \
	gameBoard.o \
	player.o \
	humanPlayer.o \
	myComputerPlayer.o \
	displaySquare.o \
	displayBoard.o \
	displayStatus.o \
	displayButton.o \
	displayControl.o \
	displayApp.o \
	history.o \
	game.o \
	TicTacToeGame.o \
	main.o 

.SUFFIXES: .C .o
.IGNORE:
.DEFAULT:
	@if [ -f $(SRCDIR)/$(@:.o=.C) ] ;\
	then \
	  $(MAKE) OBJ=$(@) $(@) ;\
	else \
	  echo "make: Fatal error: Don't know how to make target \`$(@)'" ;\
	fi

.C.o:	$(CCC) $(CFLAGS) $(INCLUDES) -c $<

executable: $(OBJ)
	$(CCC) $(CFLAGS) -o executable $(OBJ) $(LDFLAGS) $(LIBS)
	@rm -f *.dir *.pag
	`discover -where`/bin/symtab $@

$(OBJ): $(SRCDIR)/$$(@:.o=.C)
	$(CCC) $(CFLAGS) $(INCLUDES) -c $(SRCDIR)/$(@:.o=.C) -o $@

makemf:
	@echo "Dummy Rule: .DEFAULT will accept new src files"
 
clean:
	@rm -f core
	@rm -f executable
	@rm -f executable.pag executable.dir
	@rm -f ../bin/executable
	@rm -f $(OBJ)
	@rm -f junk*
