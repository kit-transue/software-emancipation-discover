/*
 * S Q L _ K E Y W O R D . H
 *
 * Copyright 1998 Software Emancipation Technology, Inc.
 *
 * Created Feb. 1998 Mark B. Kaminsky
 *
 * Include file for handling SQL keyword (reserved words) recognition.
 * This is done with a table of keywords containing the text of a keyword,
 * its yacc token value, and a series of bit mask fields indicating when
 * a keyword is active.  These values can be ORed together to combine them.
 * If a keyword is not active, then it is treated as an identifier.
 */

#ifndef SQL_KEYWORD_H
#define SQL_KEYWORD_H

#include "sql_all.h"

/* Entry for each keyword -- see bit mask lists below
 * and masks for key_vendor in sql_symbol.h
 */
typedef struct sql_keyentry {
   const char* 	key_text;		/* token text */
   int		key_token;		/* yacc token value */
   int		key_type;		/* type of sql reserved word */
   int		key_command;		/* only in certain sql commands */
   int		key_vendor;		/* database vendor */
} SQL_KEY;

/* SQL Reserved words type mask for key_type */
#define SQLKEY_NONE		0x00	/* not a reserved word */
#define SQLKEY_SQL		0x01	/* standalone SQL */
#define SQLKEY_PLSQL		0x02	/* PL/SQL */
#define SQLKEY_START		0x04	/* only at statement start */
#define SQLKEY_SPECIAL		0x08	/* special handling */
#define SQLKEY_COMMAND		0x10	/* only in certain command */
#define SQLKEY_ESQL		0x20	/* Embedded SQL */

/* accept all sql basic keyword types */
#define SQLKEY_ALL		(SQLKEY_SQL | SQLKEY_PLSQL)

/* accept all sql keyword types including esql */
#define SQLKEY_ALL_ESQL		(SQLKEY_SQL | SQLKEY_PLSQL | SQLKEY_ESQL)

/* keyword mask for specific SQL commands (key_command)
 * when (key_type & SQLKEY_COMMAND) is true.
 *
 * TBD: Currently this is a bit mask, but as of this comment (2/19/1999)
 * there are no examples of any of these ORed together, just single
 * values, so this can be changed to a simple enum list if it gets too
 * big.  If later a case of a keyword only valid in two commands arises
 * then that could be handled with SQLKEY_SPECIAL logic in sql_keyword.C.
 * See also sql_keyword::add_command().
 */
#define SQLKEYC_NONE		0x0000	/* no command */
#define SQLKEYC_CT		0x0001	/* create table */
#define SQLKEYC_TYPE		0x0002	/* type (table, record, cursor ref) */
#define SQLKEYC_COMMIT		0x0004	/* commit/rollback */
#define SQLKEYC_CONNECT		0x0008	/* connect */
#define SQLKEYC_CONTEXT		0x0010	/* esql context */
#define SQLKEYC_DESCRIBE	0x0020	/* esql describe */
#define SQLKEYC_ENABLE		0x0040	/* esql enable threads */
#define SQLKEYC_VARCHAR		0x0080	/* esql varchar stmt */
#define SQLKEYC_IAF		0x0100	/* esql exec iaf stmt */
#define SQLKEYC_DATABASE	0x0200	/* database stmt */
#define SQLKEYC_DATETIME	0x0400	/* Informix datetime/interval */

/* Compare strings ignoring case */
EXTERN_C int compcaseless(const char* s1, const char* s2);

/* C wrappers to C++ functions in sql_keyword.C */
EXTERN_C int sql_keyword_lookup (const char* text);

EXTERN_C void sql_keyword_start_of_statement (int flag);

EXTERN_C void sql_set_keyword_mode (int mode);

EXTERN_C int sql_get_keyword_mode (void);

EXTERN_C void sql_push_keyword_mode (int mode);

EXTERN_C void sql_pop_keyword_mode (void);

EXTERN_C void sql_keyword_command (int command);

#endif /* SQL_KEYWORD_H */
