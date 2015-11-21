include ..\..\makescripts\Windows\defs.mak

LIBRARY=".\$(INTDIR)\libs.lib"

OBJS= \
	".\gdchart\$(INTDIR)\gdchart.lib" \
	".\ads\$(INTDIR)\ads.lib" \
	".\gd\$(INTDIR)\gd.lib" \
	".\source\$(INTDIR)\source.lib" \
	".\osport\$(INTDIR)\osport.lib"

VPATH=./source\
	./osport\
	./gd\
	./gdchart\
	./ads

include ..\..\makescripts\Windows\targets.mak

MAKE_CMD=omake -k -v -W -EN -a -f $<

.mak{.\source\$(INTDIR)}.lib:
	$(MAKE_CMD)	
.mak{.\osport\$(INTDIR)}.lib:
	$(MAKE_CMD)	
.mak{.\gd\$(INTDIR)}.lib:
	$(MAKE_CMD)	
.mak{.\gdchart\$(INTDIR)}.lib:
	$(MAKE_CMD)	
.mak{.\ads\$(INTDIR)}.lib:
	$(MAKE_CMD)	

