#
# make discrypt
#

include $(MAKELIB)/Make.general

CFLAGS = -O
INCLUDES = -I/usr/include

discrypt: discrypt.c
	$(CC) $(CARCHFLAGS) $(CFLAGS) $(INCLUDES) -o discrypt $<

clean:
	rm -f discrypt
