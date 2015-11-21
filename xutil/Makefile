#
# Makefile: 
#
#	xscan
#	xopen
#

include $(MAKELIB)/Make.general

CFLAGS = -O -D$(Arch)
INCLUDES = -I/usr/include ${X_FLAG}
XLIB = -lX11

ifeq "$(Arch)" "hp10"
  CFLAGS := $(CFLAGS) -D_HPUX_SOURCE
  XLIB := -L/usr/lib/X11R5 $(XLIB)
endif

ifeq "$(Arch)" "linux2"
  XLIB := -L/usr/X11R6/lib $(XLIB)
endif

ifeq "$(Arch)" "sun5"
  SOCKETLIB = -lsocket
endif

all: xscan xopen

xscan: xscan.c
	$(CC) $(CARCHFLAGS) $(CFLAGS) $(INCLUDES) -o xscan $< $(SOCKETLIB)

xopen: xopen.c
	$(CC) $(CARCHFLAGS) $(CFLAGS) $(INCLUDES) -o xopen $< $(XLIB)

clean:
	rm -f xscan xopen
