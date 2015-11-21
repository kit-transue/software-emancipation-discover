INCLUDEFLAGS=-I. "-IC:/Program Files/DevStudio/VC/include"
CFLAGS=-c -GX -Fd -Z7 -MTd -GR $(OPTIMIZE)
.SUFFIXES : .obj .cxx .l .y

ALL: message.exe message.lib gui_message.lib

OBJECTS=\
	message.obj \
	simple_msgstream.obj

.cxx.obj:
	$(CC) $(CFLAGS) $< /Fo$*.obj $(INCLUDEFLAGS)
	
message.lib: $(OBJECTS) message_manager.obj
	link -lib $(OBJECTS) message_manager.obj /out:$@

gui_message.lib: $(OBJECTS) gui_message_manager.obj
	link -lib $(OBJECTS) gui_message_manager.obj /out:$@

message.exe: message.lib main.obj
	link main.obj message.lib /map /fixed:no /debug /libpath:"C:/Program Files/DevStudio/VC/lib" advapi32.lib /out:message.exe
