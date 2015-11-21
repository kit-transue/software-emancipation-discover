EXECS= \
	cm_normalize_date \
	cm_version_string \
	rcs_rem_log_from_src \
	rcs_revisions \
	rcs_srcver

all: $(EXECS)

install: all
	mv $(EXECS) ../util
	- (cd ../util; strip $(EXECS))

clean:
	rm -f $(EXECS)

cm_normalize_date: cm_normalize_date.c
	cc -o cm_normalize_date cm_normalize_date.c

cm_version_string: cm_version_string.l
	lex cm_version_string.l
	mv lex.yy.c cm_version_string.c
	cc -o cm_version_string cm_version_string.c
	rm -f cm_version_string.c

rcs_rem_log_from_src: rcs_rem_log_from_src.l
	lex rcs_rem_log_from_src.l
	mv lex.yy.c rcs_rem_log_from_src.c
	cc -o rcs_rem_log_from_src rcs_rem_log_from_src.c
	rm -f rcs_rem_log_from_src.c

rcs_revisions: rcs_revisions.l
	lex rcs_revisions.l
	mv lex.yy.c rcs_revisions.c
	cc -o rcs_revisions rcs_revisions.c
	rm -f rcs_revisions.c

rcs_srcver: rcs_srcver.l
	lex rcs_srcver.l
	mv lex.yy.c rcs_srcver.c
	cc -o rcs_srcver rcs_srcver.c
	rm -f rcs_srcver.c

