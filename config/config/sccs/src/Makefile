include $(MAKELIB)/Make.general

EXECS= \
	sccs_version_string \
	sccs_lockfile_version \
	sccs_srcver

all: $(EXECS)

clean:
	rm -f $(EXECS)

sccs_version_string: sccs_version_string.l
	${LEX} $?
	mv lex.yy.c sccs_version_string.c
	$(CC) $(CARCHFLAGS) -o sccs_version_string sccs_version_string.c
	rm -f sccs_version_string.c

sccs_lockfile_version: sccs_lockfile_version.l
	${LEX} $?
	mv lex.yy.c sccs_lockfile_version.c
	$(CC) $(CARCHFLAGS) -o sccs_lockfile_version sccs_lockfile_version.c
	rm -f sccs_lockfile_version.c

sccs_srcver: sccs_srcver.l
	${LEX} $?
	mv lex.yy.c sccs_srcver.c
	$(CC) $(CARCHFLAGS) -o sccs_srcver sccs_srcver.c
	rm -f sccs_srcver.c
