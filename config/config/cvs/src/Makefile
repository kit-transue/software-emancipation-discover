include $(MAKELIB)/Make.general

EXECS= \
	cm_normalize_date \
	cm_version_string \
	rcs_rem_log_from_src \
	rcs_revisions \
	cm_quote \
	rcs_srcver

all: $(EXECS)

clean:
	rm -f $(EXECS)

cm_quote: cm_quote.c
	${CC} $(CARCHFLAGS) -o cm_quote ${?}

cm_normalize_date: cm_normalize_date.c
	${CC} $(CARCHFLAGS) -o cm_normalize_date ${?}

cm_version_string: cm_version_string.l
	$(LEX) ${?}
	mv lex.yy.c cm_version_string.c
	${CC} $(CARCHFLAGS) -o cm_version_string cm_version_string.c
	rm -f cm_version_string.c

rcs_rem_log_from_src: rcs_rem_log_from_src.l
	${LEX} ${?}
	mv lex.yy.c rcs_rem_log_from_src.c
	${CC} $(CARCHFLAGS) -o rcs_rem_log_from_src rcs_rem_log_from_src.c
	rm -f rcs_rem_log_from_src.c

rcs_revisions: rcs_revisions.l
	${LEX} ${?}
	mv lex.yy.c rcs_revisions.c
	${CC} $(CARCHFLAGS) -o rcs_revisions rcs_revisions.c
	rm -f rcs_revisions.c

rcs_srcver: rcs_srcver.l
	${LEX} ${?}
	mv lex.yy.c rcs_srcver.c
	${CC} $(CARCHFLAGS) -o rcs_srcver rcs_srcver.c
	rm -f rcs_srcver.c

