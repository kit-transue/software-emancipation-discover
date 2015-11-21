include $(MAKELIB)/Make.general

EXECS= \
	cm_normalize_date

all: $(EXECS)

clean:
	rm -f $(EXECS)

cm_normalize_date: cm_normalize_date.c
	$(CC) -o $(@) $(CARCHFLAGS) $(?)
