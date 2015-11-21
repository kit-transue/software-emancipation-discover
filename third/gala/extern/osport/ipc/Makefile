CCC	=/net/oxen/vol01/share/sun5/opt/SUNWspro/bin/CC

DEFINES	=-Dsun54

ASYNCW	=default

INCLUDE	=-I/paraset/src/extern/osport/include -I/paraset/src/extern/ads/include -I/paraset/src/extern/osport/ipc/$(ASYNCW) -I$(GALAXYHOME)/include

LIBSRCS = commChannel.C commFDIO.C commFDIO3.C commFDPipe.C \
	  commFDPipe3.C asyncHandler.C systemAsyncWatcher.C \
          commChannelAsync.C commPortServer.C commSocketIO.C


LIBOBJS = commChannel.o commFDIO.o commFDIO3.o commFDPipe.o \
          commFDPipe3.o asyncHandler.o systemAsyncWatcher.o \
          commChannelAsync.o $(ASYNCW)/asyncInputWatcher.o \
	  commPortServer.o commSocketIO.o

LIBTARGET = libipc.so

WITHDEBUG = -g

LIBAREAS = -L/paraset/src/extern/osport/ipc -L/paraset/src/extern/osport/source -L/paraset/src/extern/ads/source

PURIFY = /usr/local/bin/purify

.SUFFIXES: .C

.C.o:
	$(CCC) -G $(WITHDEBUG) $(INCLUDE) $(DEFINES) -c $<

libs:	$(LIBOBJS)
	$(CCC) -G $(WITHDEBUG) -o $(LIBTARGET) $(LIBOBJS)

clean:
	rm -rf $(LIBOBJS) $(ASYNCW)/*.o $(LIBTARGET)

$(ASYNCW)/asyncInputWatcher.o:	$(ASYNCW)/asyncInputWatcher.C
	(cd $(ASYNCW); $(CCC) -G $(WITHDEBUG) $(INCLUDE) $(DEFINES) -c asyncInputWatcher.C)

depend:	$(LIBSRCS)
	makedepend $(INCLUDE) $(LIBSRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

commChannel.o: /paraset/src/extern/osport/include/pdumem.h
commChannel.o: /paraset/src/extern/ads/include/parray.h
commChannel.o: /paraset/src/extern/osport/include/commChannel.h
commChannel.o: /paraset/src/extern/osport/include/pduio.h
commChannel.o: /paraset/src/extern/osport/include/commChannelAsync.h
commFDIO.o: /paraset/src/extern/osport/include/pdumem.h
commFDIO.o: /paraset/src/extern/osport/include/pdutime.h
commFDIO.o: /paraset/src/extern/osport/include/commFDIO.h
commFDIO.o: /paraset/src/extern/osport/include/commFDIO3.h
commFDIO.o: /paraset/src/extern/osport/include/commChannel.h
commFDIO.o: /paraset/src/extern/osport/include/pduio.h
commFDIO.o: /paraset/src/extern/osport/include/asyncHandler.h
commFDIO3.o: /paraset/src/extern/osport/include/pdumem.h
commFDIO3.o: /paraset/src/extern/osport/include/pdutime.h
commFDIO3.o: /paraset/src/extern/osport/include/commChannelAsync.h
commFDIO3.o: /paraset/src/extern/osport/include/commFDIO3.h
commFDIO3.o: /paraset/src/extern/osport/include/commChannel.h
commFDIO3.o: /paraset/src/extern/osport/include/pduio.h
commFDIO3.o: /paraset/src/extern/osport/include/asyncHandler.h
commFDIO3.o: /paraset/src/extern/osport/include/systemAsyncWatcher.h
commFDPipe.o: /paraset/src/extern/osport/include/pdustring.h
commFDPipe.o: /paraset/src/extern/ads/include/parray.h
commFDPipe.o: /paraset/src/extern/osport/include/commChannel.h
commFDPipe.o: /paraset/src/extern/osport/include/pduio.h
commFDPipe.o: /paraset/src/extern/osport/include/commFDIO.h
commFDPipe.o: /paraset/src/extern/osport/include/commFDIO3.h
commFDPipe.o: /paraset/src/extern/osport/include/asyncHandler.h
commFDPipe.o: /paraset/src/extern/osport/include/commFDPipe.h
commFDPipe.o: /paraset/src/extern/osport/include/commFDPipe3.h
commFDPipe3.o: /paraset/src/extern/ads/include/parray.h
commFDPipe3.o: /paraset/src/extern/osport/include/pdustring.h
commFDPipe3.o: /paraset/src/extern/osport/include/pdumem.h
commFDPipe3.o: /paraset/src/extern/osport/include/commFDPipe3.h
asyncInputWatcher.o: /paraset/src/extern/osport/ipc/$(ASYNCW)/asyncInputWatcher.h
asyncInputWatcher.o: /paraset/src/extern/osport/include/systemAsyncWatcher.h
asyncInputWatcher.o: /paraset/src/extern/osport/include/asyncHandler.h
systemAsyncWatcher.o: /paraset/src/extern/osport/include/systemAsyncWatcher.h
asyncHandler.o: /paraset/src/extern/osport/include/asyncHandler.h
commChannelAsync.o: /paraset/src/extern/osport/include/commChannel.h
commChannelAsync.o: /paraset/src/extern/osport/include/pduio.h
commChannelAsync.o: /paraset/src/extern/osport/include/commChannelAsync.h
