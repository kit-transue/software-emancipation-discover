all : httpserver

include /paraset/admin/int_tools/Makelib/Make.general

OBJS = servers.o sockets.o httpwrappers.o nameServCalls.o SocketComm.o

httpserver : ${OBJS}
	${CCC} ${OBJS} -lnsl -lsocket -o $@

nameServCalls.o : /paraset/src/nameserver/api/nameServCalls.cxx
	${CCC} -c $? -o $@

SocketComm.o : /paraset/src/dish/src/SocketComm.cxx
	${CCC} -c $? -o $@

.SUFFIXES :
.SUFFIXES : .cpp .o

.cpp.o :
	${CCC} -c $< -o $@
