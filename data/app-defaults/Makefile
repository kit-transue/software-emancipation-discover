all: ParaSET-color ParaSET-bw

ParaSET-color: ParaSET.source
	/usr/lib/cpp -P -DCOLOR ParaSET.source | cat -s > $@

ParaSET-bw: ParaSET.source
	/usr/lib/cpp -P -UCOLOR ParaSET.source | cat -s > $@

makemf:
	@ echo '### No automatic Makefile for this diretory.'
