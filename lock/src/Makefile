#
# Makefile: 
#
#	dis_lock
#

include $(MAKELIB)/make_options

CFLAGS = -O -D$(Arch) $(DEFINES)
INCLUDES = -I/usr/include

all: dis_lock

dis_lock: dis_lock.c
	$(CC) $(CFLAGS) $(INCLUDES) -o dis_lock $<

clean:
	rm -f dis_lock
