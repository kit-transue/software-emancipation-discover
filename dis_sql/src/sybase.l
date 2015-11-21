/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
/* This is the Flex lexer for the Sybase SQL dialect.  It works in
 * conjunction with the Bison parser in Sybase.y.  They are used
 * instead of the parser/lexer in sql.[yl] when the command line
 * option "-database Sybase" is specified.
 */

%{
#include "sql_scanner.h"
#include "sybase.yacc.h"
#include <string.h>
#include "sql_iff.h"

static token* add_token(SQL_SYM*);
static token* add_token_from(const char*, SQL_SYM*);
static int id_or_kwd(int kwd, int only_after_at);
static void handle_newline();
static int discard_comment();
static int begin_or_label_name();
static int collect_string(int terminator);
int map_to_allowed_token(int from, int to);
static int pending_at = 0;	/* if last token was "@" */
static int pending_between = 0; /* if we saw BETWEEN and no AND yet */
static size_t get_start_file_offset();
static size_t get_end_file_offset();
static int sybase_get_icolumn();
static unsigned icolumn_last = 1;
static int start_string_line;
static int start_string_column;
extern int yy_sybase_debug;

/*
 * Track the file offset of the current token (yytext).
 *
 * YY_INPUT is the flex input macro which is called in
 * yy_get_next_buffer() to read in more characters.
 * This macro differs from the standard macro in two
 * respects:
 * 1) it assumes that the yy_current_buffer->yy_is_interactive
 *    flag is false (getc until newline vs. fread)
 * 2) it tracks the current offset within the input file.
 *
 * current_offset -- the file offset at the start of the current buffer.
 * next_offset -- will be the file offset for the next read.
 * number_to_move -- a local in yy_get_next_buffer() which is
 * the count of bytes left in the buffer from the last read.
 *
 * newline_offset -- updated for every newline.
 */

static size_t current_offset = 0;
static size_t next_offset = 0;
static size_t newline_offset = 0;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
   if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
	  && ferror( yyin ) ) \
       YY_FATAL_ERROR( "input in flex scanner failed" ); \
   else { \
      current_offset = next_offset - number_to_move; \
      next_offset += result; \
   }

%}

%%

	/* Literal keyword tokens.  Because keywords are not (very)
	 * reserved in Sybase SQL, most of these keywords can be simple
	 * identifiers in certain contexts.  A preceding "@" always
	 * marks an identifier instead of a keyword.  The routine
	 * map_to_allowed_token interrogates the Bison state and, if
	 * the keyword is not syntactically allowed but an identifier
	 * is, an identifier is needed.  The routine id_or_kwd queries
	 * these contextual parameters and returns either the supplied
	 * token id or "identifier".  (The second parameter, only_after_at,
	 * disables the map_to_allowed_token check, to accommodate
	 * otherwise ambiguous resolutions in the grammar.)
	 */

ADD                 { return id_or_kwd(Y_ADD, 1); }
ALL                 { return id_or_kwd(Y_ALL, 1); }
ALLOW_DUP_ROW       { return id_or_kwd(Y_ALLOW_DUP_ROW, 0); }
ALTER               { return id_or_kwd(Y_ALTER, 1); }
AND                 { if (pending_between) {
                        pending_between = 0;
			iff_keyword(add_token(NULL));
                        return Y_AND_AFTER_BETWEEN;
		      }
                      return id_or_kwd(Y_AND, 1);
                    }
ANSINULL            { return id_or_kwd(Y_ANSINULL, 0); }
ANSI_PERMISSIONS    { return id_or_kwd(Y_ANSI_PERMISSIONS, 0); }
ANY                 { return id_or_kwd(Y_ANY, 1); }
ARITHABORT          { return id_or_kwd(Y_ARITHABORT, 0); }
ARITHIGNORE         { return id_or_kwd(Y_ARITHIGNORE, 0); }
ARITH_OVERFLOW      { return id_or_kwd(Y_ARITH_OVERFLOW, 1); }
AS                  { return id_or_kwd(Y_AS, 1); }
ASC                 { return id_or_kwd(Y_ASC, 1); }
AT                  { return id_or_kwd(Y_AT, 1); }
AUTHORIZATION       { return id_or_kwd(Y_AUTHORIZATION, 1); }
AVG                 { return id_or_kwd(Y_AVG, 1); }
BEGIN               { return id_or_kwd(Y_BEGIN, 1); }
BETWEEN             { if (id_or_kwd(Y_BETWEEN, 1) == Y_BETWEEN) {
                        pending_between = 1;
                        return Y_BETWEEN;
		      }
                      else return identifier;
                    }
BINARY              { return id_or_kwd(Y_BINARY, 0); }
BIT                 { return id_or_kwd(Y_BIT, 0); }
BLOCKSIZE           { return id_or_kwd(Y_BLOCKSIZE, 0); }
BREAK               { return id_or_kwd(Y_BREAK, 1); }
BROWSE              { return id_or_kwd(Y_BROWSE, 1); }
BY                  { return id_or_kwd(Y_BY, 0); }
BYTES               { return id_or_kwd(Y_BYTES, 0); }
CAPACITY            { return id_or_kwd(Y_CAPACITY, 0); }
CASCADE             { return id_or_kwd(Y_CASCADE, 1); }
CASE                { return id_or_kwd(Y_CASE, 0); }
CHAINED             { return id_or_kwd(Y_CHAINED, 0); }
CHAR                { return id_or_kwd(Y_CHAR, 0); }
CHARS               { return id_or_kwd(Y_CHARACTERS, 0); }
CHARACTERS          { return id_or_kwd(Y_CHARACTERS, 0); }
CHAR_CONVERT        { return id_or_kwd(Y_CHAR_CONVERT, 1); }
CHECK               { return id_or_kwd(Y_CHECK, 1); }
CHECKALLOC          { return id_or_kwd(Y_CHECKALLOC, 0); }
CHECKCATALOG        { return id_or_kwd(Y_CHECKCATALOG, 0); }
CHECKDB             { return id_or_kwd(Y_CHECKDB, 0); }
CHECKPOINT          { return id_or_kwd(Y_CHECKPOINT, 1); }
CHECKTABLE          { return id_or_kwd(Y_CHECKTABLE, 0); }
CLIENT              { return id_or_kwd(Y_CLIENT, 0); }
CLOSE               { return id_or_kwd(Y_CLOSE, 1); }
CLUSTERED           { return id_or_kwd(Y_CLUSTERED, 1); }
CNTRLTYPE           { return id_or_kwd(Y_CNTRLTYPE, 0); }
COMMIT              { return id_or_kwd(Y_COMMIT, 1); }
COMMITTED           { return id_or_kwd(Y_COMMITTED, 0); }
COMPUTE             { return id_or_kwd(Y_COMPUTE, 1); }
CONSTRAINT          { return id_or_kwd(Y_CONSTRAINT, 1); }
CONTIGUOUS          { return id_or_kwd(Y_CONTIGUOUS, 0); }
CONTINUE            { return id_or_kwd(Y_CONTINUE, 1); }
CONVERT             { return id_or_kwd(Y_CONVERT, 1); }
COUNT               { return id_or_kwd(Y_COUNT, 1); }
CREATE              { return id_or_kwd(Y_CREATE, 1); }
CROSS               { return id_or_kwd(Y_CROSS, 0); }
CURRENT             { return id_or_kwd(Y_CURRENT, 1); }
CURSOR              { return id_or_kwd(Y_CURSOR, 1); }
DATABASE            { return id_or_kwd(Y_DATABASE, 1); }
DATEADD             { return id_or_kwd(Y_DATEADD, 0); }
DATEDIFF            { return id_or_kwd(Y_DATEDIFF, 0); }
DATEFIRST           { return id_or_kwd(Y_DATEFIRST, 0); }
DATEFORMAT          { return id_or_kwd(Y_DATEFORMAT, 0); }
DATENAME            { return id_or_kwd(Y_DATENAME, 0); }
DATEPART            { return id_or_kwd(Y_DATEPART, 0); }
DATETIME            { return id_or_kwd(Y_DATETIME, 0); }
DAY                 { return id_or_kwd(Y_DAY, 0); }
DAYOFYEAR           { return id_or_kwd(Y_DAYOFYEAR, 0); }
DBCC                { return id_or_kwd(Y_DBCC, 1); }
DBREPAIR            { return id_or_kwd(Y_DBREPAIR, 0); }
DD                  { return id_or_kwd(Y_DAY, 0); }
DEALLOCATE          { return id_or_kwd(Y_DEALLOCATE, 1); }
DEC                 { return id_or_kwd(Y_DECIMAL, 0); }
DECIMAL             { return id_or_kwd(Y_DECIMAL, 0); }
DECLARE             { return id_or_kwd(Y_DECLARE, 1); }
DEFAULT             { return id_or_kwd(Y_DEFAULT, 1); }
DELAY               { return id_or_kwd(Y_DELAY, 0); }
DELETE              { return id_or_kwd(Y_DELETE, 1); }
DENSITY             { return id_or_kwd(Y_DENSITY, 0); }
DESC                { return id_or_kwd(Y_DESC, 1); }
DISK                { return id_or_kwd(Y_DISK, 1); }
DISMOUNT            { return id_or_kwd(Y_DISMOUNT, 0); }
DISTINCT            { return id_or_kwd(Y_DISTINCT, 1); }
DOUBLE              { return id_or_kwd(Y_DOUBLE, 1); }
DROP                { return id_or_kwd(Y_DROP, 1); }
DROPDB              { return id_or_kwd(Y_DROPDB, 0); }
DUMP                { return id_or_kwd(Y_DUMP, 1); }
DUMPVOLUME          { return id_or_kwd(Y_DUMPVOLUME, 0); }
DW                  { return id_or_kwd(Y_WEEKDAY, 0); }
DY                  { return id_or_kwd(Y_DAYOFYEAR, 0); }
ELSE                { return id_or_kwd(Y_ELSE, 1); }
END                 { return id_or_kwd(Y_END, 1); }
ENDTRAN             { return id_or_kwd(Y_ENDTRAN, 1); }
ERROR               { return id_or_kwd(Y_ERROR, 0); }
ERRORDATA           { return id_or_kwd(Y_ERRORDATA, 1); }
ERROREXIT           { return id_or_kwd(Y_ERROREXIT, 1); }
ESCAPE              { return id_or_kwd(Y_ESCAPE, 1); }
EXEC                { return id_or_kwd(Y_EXECUTE, 1); }
EXECUTE             { return id_or_kwd(Y_EXECUTE, 1); }
EXISTS              { return id_or_kwd(Y_EXISTS, 1); }
FAST                { return id_or_kwd(Y_FAST, 0); }
FETCH               { return id_or_kwd(Y_FETCH, 1); }
FILE                { return id_or_kwd(Y_FILE, 0); }
FILLFACTOR          { return id_or_kwd(Y_FILLFACTOR, 1); }
FIPSFLAGGER         { return id_or_kwd(Y_FIPSFLAGGER, 0); }
FIX                 { return id_or_kwd(Y_FIX, 0); }
FIX_TEXT            { return id_or_kwd(Y_FIX_TEXT, 0); }
FLOAT               { return id_or_kwd(Y_FLOAT, 0); }
FLUSHMESSAGE        { return id_or_kwd(Y_FLUSHMESSAGE, 0); }
FOR                 { return id_or_kwd(Y_FOR, 1); }
FOREIGN             { return id_or_kwd(Y_FOREIGN, 1); }
FROM                { return id_or_kwd(Y_FROM, 1); }
FULL                { return id_or_kwd(Y_FULL, 0); }
GO                  { return id_or_kwd(Y_GO, 1); }
GOTO                { return id_or_kwd(Y_GOTO, 1); }
GRANT               { return id_or_kwd(Y_GRANT, 1); }
GROUP               { return id_or_kwd(Y_GROUP, 1); }
HAVING              { return id_or_kwd(Y_HAVING, 1); }
HEADERONLY          { return id_or_kwd(Y_HEADERONLY, 0); }
HH                  { return id_or_kwd(Y_HOUR, 0); }
HOLDLOCK            { return id_or_kwd(Y_HOLDLOCK, 1); }
HOUR                { return id_or_kwd(Y_HOUR, 0); }
IDENTITY            { return id_or_kwd(Y_IDENTITY, 0); }
IDENTITY_INSERT     { return id_or_kwd(Y_IDENTITY_INSERT, 1); }
IF                  { return id_or_kwd(Y_IF, 1); }
IGNORE_DUP_KEY      { return id_or_kwd(Y_IGNORE_DUP_KEY, 0); }
IGNORE_DUP_ROW      { return id_or_kwd(Y_IGNORE_DUP_ROW, 0); }
IMAGE               { return id_or_kwd(Y_IMAGE, 0); }
IN                  { return id_or_kwd(Y_IN, 1); }
INDEX               { return id_or_kwd(Y_INDEX, 1); }
INDEXALLOC          { return id_or_kwd(Y_INDEXALLOC, 0); }
INIT                { return id_or_kwd(Y_INIT, 0); }
INNER               { return id_or_kwd(Y_INNER, 0); }
INSERT              { return id_or_kwd(Y_INSERT, 1); }
INT                 { return id_or_kwd(Y_INTEGER, 0); }
INTEGER             { return id_or_kwd(Y_INTEGER, 0); }
INTO                { return id_or_kwd(Y_INTO, 1); }
IO                  { return id_or_kwd(Y_IO, 0); }
IS                  { return id_or_kwd(Y_IS, 1); }
ISOLATION           { return id_or_kwd(Y_ISOLATION, 1); }
JOIN                { return id_or_kwd(Y_JOIN, 0); }
KEY                 { return id_or_kwd(Y_KEY, 1); }
KILL                { return id_or_kwd(Y_KILL, 1); }
LANGUAGE            { return id_or_kwd(Y_LANGUAGE, 0); }
LEFT                { return id_or_kwd(Y_LEFT, 0); }
LEVEL               { return id_or_kwd(Y_LEVEL, 1); }
LIKE                { return id_or_kwd(Y_LIKE, 1); }
LISTONLY            { return id_or_kwd(Y_LISTONLY, 0); }
LOAD                { return id_or_kwd(Y_LOAD, 1); }
LOG                 { return id_or_kwd(Y_LOG, 0); }
LRU                 { return id_or_kwd(Y_LRU, 0); }
MAX                 { return id_or_kwd(Y_MAX, 1); }
MAX_ROWS_PER_PAGE   { return id_or_kwd(Y_MAX_ROWS_PER_PAGE, 1); }
MI                  { return id_or_kwd(Y_MINUTE, 0); }
MILLISECOND         { return id_or_kwd(Y_MILLISECOND, 0); }
MIN                 { return id_or_kwd(Y_MIN, 1); }
MINUTE              { return id_or_kwd(Y_MINUTE, 0); }
MIRROR              { return id_or_kwd(Y_MIRROR, 1); }
MIRROREXIT          { return id_or_kwd(Y_MIRROREXIT, 1); }
MM                  { return id_or_kwd(Y_MONTH, 0); }
MODE                { return id_or_kwd(Y_MODE, 0); }
MONEY               { return id_or_kwd(Y_MONEY, 0); }
MONTH               { return id_or_kwd(Y_MONTH, 0); }
MRU                 { return id_or_kwd(Y_MRU, 0); }
MS                  { return id_or_kwd(Y_MILLISECOND, 0); }
NAME                { return id_or_kwd(Y_NAME, 0); }
NATURAL             { return id_or_kwd(Y_NATURAL, 0); }
NCHAR               { return id_or_kwd(Y_NCHAR, 0); }
NOCOUNT             { return id_or_kwd(Y_NOCOUNT, 0); }
NODISMOUNT          { return id_or_kwd(Y_NODISMOUNT, 0); }
NOEXEC              { return id_or_kwd(Y_NOEXEC, 0); }
NOFIX               { return id_or_kwd(Y_NOFIX, 0); }
NOHOLDLOCK          { return id_or_kwd(Y_NOHOLDLOCK, 1); }
NOINIT              { return id_or_kwd(Y_NOINIT, 0); }
NONCLUSTERED        { return id_or_kwd(Y_NONCLUSTERED, 1); }
NOSERIAL            { return id_or_kwd(Y_NOSERIAL, 0); }
NOT                 { return id_or_kwd(Y_NOT, 1); }
NOTIFY              { return id_or_kwd(Y_NOTIFY, 0); }
NOUNLOAD            { return id_or_kwd(Y_NOUNLOAD, 0); }
NOWAIT              { return id_or_kwd(Y_NOWAIT, 0); }
NO_ERROR            { return id_or_kwd(Y_NO_ERROR, 0); }
NO_LOG              { return id_or_kwd(Y_NO_LOG, 0); }
NO_TRUNCATE         { return id_or_kwd(Y_NO_TRUNCATE, 0); }
NULL                { return id_or_kwd(Y_NULL, 1); }
NUMERIC             { return id_or_kwd(Y_NUMERIC, 0); }
NUMERIC_TRUNCATION  { return id_or_kwd(Y_NUMERIC_TRUNCATION, 0); }
NVARCHAR            { return id_or_kwd(Y_NVARCHAR, 0); }
OF                  { return id_or_kwd(Y_OF, 1); }
OFF                 { return id_or_kwd(Y_OFF, 1); }
OFFSETS             { return id_or_kwd(Y_OFFSETS, 1); }
ON                  { return id_or_kwd(Y_ON, 1); }
ONLINE              { return id_or_kwd(Y_ONLINE, 1); }
ONLY                { return id_or_kwd(Y_ONLY, 1); }
OPEN                { return id_or_kwd(Y_OPEN, 1); }
OPERATOR_CONSOLE    { return id_or_kwd(Y_OPERATOR_CONSOLE, 0); }
OPTIMIZED           { return id_or_kwd(Y_OPTIMIZED, 0); }
OPTION              { return id_or_kwd(Y_OPTION, 1); }
OR                  { return id_or_kwd(Y_OR, 1); }
ORDER               { return id_or_kwd(Y_ORDER, 1); }
OUTER               { return id_or_kwd(Y_OUTER, 0); }
OUTPUT              { return id_or_kwd(Y_OUTPUT, 0); }
OVERRIDE            { return id_or_kwd(Y_OVERRIDE, 0); }
PARAM               { return id_or_kwd(Y_PARAM, 0); }
PARSEONLY           { return id_or_kwd(Y_PARSEONLY, 0); }
PARTITION           { return id_or_kwd(Y_PARTITION, 1); }
PATINDEX            { return id_or_kwd(Y_PATINDEX, 0); }
PHYSNAME            { return id_or_kwd(Y_PHYSNAME, 0); }
PRECISION           { return id_or_kwd(Y_PRECISION, 1); }
PREFETCH            { return id_or_kwd(Y_PREFETCH, 0); }
PREPARE             { return id_or_kwd(Y_PREPARE, 1); }
PRIMARY             { return id_or_kwd(Y_PRIMARY, 1); }
PRINT               { return id_or_kwd(Y_PRINT, 1); }
PRIVILEGES          { return id_or_kwd(Y_PRIVILEGES, 1); }
PROC                { return id_or_kwd(Y_PROCEDURE, 1); }
PROCEDURE           { return id_or_kwd(Y_PROCEDURE, 1); }
PROCESSEXIT         { return id_or_kwd(Y_PROCESSEXIT, 1); }
PROCID              { return id_or_kwd(Y_PROCID, 0); }
PUBLIC              { return id_or_kwd(Y_PUBLIC, 1); }
QQ                  { return id_or_kwd(Y_QUARTER, 0); }
QUARTER             { return id_or_kwd(Y_QUARTER, 0); }
QUOTED_IDENTIFIER   { return id_or_kwd(Y_QUOTED_IDENTIFIER, 0); }
RAISERROR           { return id_or_kwd(Y_RAISERROR, 1); }
READ                { return id_or_kwd(Y_READ, 1); }
READTEXT            { return id_or_kwd(Y_READTEXT, 1); }
REAL                { return id_or_kwd(Y_REAL, 0); }
RECOMPILE           { return id_or_kwd(Y_RECOMPILE, 0); }
RECONFIGURE         { return id_or_kwd(Y_RECONFIGURE, 1); }
REFERENCES          { return id_or_kwd(Y_REFERENCES, 1); }
REFIT               { return id_or_kwd(Y_REFIT, 0); }
REINDEX             { return id_or_kwd(Y_REINDEX, 0); }
REINIT              { return id_or_kwd(Y_REINIT, 0); }
REMIRROR            { return id_or_kwd(Y_REMIRROR, 0); }
REMOVE              { return id_or_kwd(Y_REMOVE, 0); }
REPLACE             { return id_or_kwd(Y_REPLACE, 1); }
RETAIN              { return id_or_kwd(Y_RETAIN, 0); }
RETAINDAYS          { return id_or_kwd(Y_RETAINDAYS, 0); }
RETURN              { return id_or_kwd(Y_RETURN, 1); }
REVOKE              { return id_or_kwd(Y_REVOKE, 1); }
RIGHT               { return id_or_kwd(Y_RIGHT, 0); }
ROLE                { return id_or_kwd(Y_ROLE, 1); }
ROLLBACK            { return id_or_kwd(Y_ROLLBACK, 1); }
ROWCOUNT            { return id_or_kwd(Y_ROWCOUNT, 1); }
ROWS                { return id_or_kwd(Y_ROWS, 1); }
RULE                { return id_or_kwd(Y_RULE, 1); }
SAVE                { return id_or_kwd(Y_SAVE, 1); }
SCHEMA              { return id_or_kwd(Y_SCHEMA, 1); }
SECOND              { return id_or_kwd(Y_SECOND, 0); }
SECONDARY           { return id_or_kwd(Y_SECONDARY, 0); }
SELECT              { return id_or_kwd(Y_SELECT, 1); }
SELF_RECURSION      { return id_or_kwd(Y_SELF_RECURSION, 0); }
SERIAL              { return id_or_kwd(Y_SERIAL, 0); }
SERIALIZABLE        { return id_or_kwd(Y_SERIALIZABLE, 0); }
SET                 { return id_or_kwd(Y_SET, 1); }
SETUSER             { return id_or_kwd(Y_SETUSER, 1); }
SHARED              { return id_or_kwd(Y_SHARED, 1); }
SHOWPLAN            { return id_or_kwd(Y_SHOWPLAN, 0); }
SHUTDOWN            { return id_or_kwd(Y_SHUTDOWN, 1); }
SIZE                { return id_or_kwd(Y_SIZE, 0); }
SKIP_NCINDEX        { return id_or_kwd(Y_SKIP_NCINDEX, 0); }
SMALLDATETIME       { return id_or_kwd(Y_SMALLDATETIME, 0); }
SMALLINT            { return id_or_kwd(Y_SMALLINT, 0); }
SMALLMONEY          { return id_or_kwd(Y_SMALLMONEY, 0); }
SORTED_DATA         { return id_or_kwd(Y_SORTED_DATA, 0); }
SS                  { return id_or_kwd(Y_SECOND, 0); }
STATEMENT           { return id_or_kwd(Y_STATEMENT, 0); }
STATISTICS          { return id_or_kwd(Y_STATISTICS, 1); }
STRING_RTRUNCATION  { return id_or_kwd(Y_STRING_RTRUNCATION, 0); }
STRIPE              { return id_or_kwd(Y_STRIPE, 1); }
SUBQUERYCACHE       { return id_or_kwd(Y_SUBQUERYCACHE, 0); }
SUM                 { return id_or_kwd(Y_SUM, 1); }
SYSNAME             { return id_or_kwd(Y_SYSNAME, 0); }
TABLE               { return id_or_kwd(Y_TABLE, 1); }
TABLEALLOC          { return id_or_kwd(Y_TABLEALLOC, 0); }
TEXT                { return id_or_kwd(Y_TEXT, 0); }
TEXTSIZE            { return id_or_kwd(Y_TEXTSIZE, 1); }
THEN                { return id_or_kwd(Y_THEN, 0); }
TIME                { return id_or_kwd(Y_TIME, 0); }
TIMESTAMP           { return id_or_kwd(Y_TIMESTAMP, 0); }
TINYINT             { return id_or_kwd(Y_TINYINT, 0); }
TO                  { return id_or_kwd(Y_TO, 1); }
TRAN                { return id_or_kwd(Y_TRANSACTION, 1); }
TRANSACTION         { return id_or_kwd(Y_TRANSACTION, 1); }
TRIGGER             { return id_or_kwd(Y_TRIGGER, 1); }
TRUNCATE            { return id_or_kwd(Y_TRUNCATE, 1); }
TRUNCATE_ONLY       { return id_or_kwd(Y_TRUNCATE_ONLY, 0); }
UNCOMMITTED         { return id_or_kwd(Y_UNCOMMITTED, 0); }
UNION               { return id_or_kwd(Y_UNION, 1); }
UNIQUE              { return id_or_kwd(Y_UNIQUE, 1); }
UNLOAD              { return id_or_kwd(Y_UNLOAD, 0); }
UNMIRROR            { return id_or_kwd(Y_UNMIRROR, 0); }
UNPARTITION         { return id_or_kwd(Y_UNPARTITION, 1); }
UPDATE              { return id_or_kwd(Y_UPDATE, 1); }
USE                 { return id_or_kwd(Y_USE, 1); }
USER                { return id_or_kwd(Y_USER, 1); }
USING               { return id_or_kwd(Y_USING, 1); }
VALUES              { return id_or_kwd(Y_VALUES, 1); }
VARBINARY           { return id_or_kwd(Y_VARBINARY, 0); }
VARCHAR             { return id_or_kwd(Y_VARCHAR, 0); }
VDEVNO              { return id_or_kwd(Y_VDEVNO, 0); }
VIEW                { return id_or_kwd(Y_VIEW, 1); }
VSTART              { return id_or_kwd(Y_VSTART, 0); }
WAIT                { return id_or_kwd(Y_WAIT, 0); }
WAITFOR             { return id_or_kwd(Y_WAITFOR, 1); }
WEEK                { return id_or_kwd(Y_WEEK, 0); }
WEEKDAY             { return id_or_kwd(Y_WEEKDAY, 0); }
WHEN                { return id_or_kwd(Y_WHEN, 0); }
WHERE               { return id_or_kwd(Y_WHERE, 1); }
WHILE               { return id_or_kwd(Y_WHILE, 1); }
WITH                { return id_or_kwd(Y_WITH, 1); }
WK                  { return id_or_kwd(Y_WEEK, 0); }
WORK                { return id_or_kwd(Y_WORK, 1); }
WRITES              { return id_or_kwd(Y_WRITES, 0); }
WRITETEXT           { return id_or_kwd(Y_WRITETEXT, 1); }
YEAR                { return id_or_kwd(Y_YEAR, 0); }
YY                  { return id_or_kwd(Y_YEAR, 0); }

	/* Punctuation.  Note that Sybase appears to allow a space between
	 * multiple characters of at least some tokens (urk!).
	 */

">=" |
"> =" |
"! <" |
"!<"                { add_token(NULL); return OP_GE; }

"<=" |
"< =" |
"! >" |
"!>"                { add_token(NULL); return OP_LE; }

"<>" |
"< >" |
"! =" |
"!="                { add_token(NULL); return OP_NE; }

"*=" |
"* ="		{ add_token(NULL); return OP_LEFT_JOIN; }

"=*" |
"= *"		{ add_token(NULL); return OP_RIGHT_JOIN; }

[-=><+*/:(),.;&|%]	{ add_token(NULL); return yytext[0]; }

	/* Keep track of having seen an "@", since that triggers treating
	 * keywords as identifiers.
	 */
@		{ pending_at = 1; add_token(NULL); return '@'; }

	/* Names.  If control gets here, it's not one of the keywords listed
	 * above.
	 */
[A-Za-z#_][A-Za-z0-9#@\$_]*	{ pending_at = 0;
				  add_token(sql_symbol_lookup(yytext));
				  return identifier; }

	/* We use the trailing ":" as context to allow a separate token id
	 * for label names (which avoids *lots* of conflicts in the Bison
	 * grammar that would result if labels were treated as identifiers).
	 * The code from Interact has a couple of instances of "BEGIN:",
	 * hence the special treatment of that case here (blech!).
	 */
[A-Za-z#_][A-Za-z0-9#@\$_]*/[ \t]*:	{ int tok = begin_or_label_name();
					  SQL_SYM* sym = NULL;
					  pending_at = 0;
					  if (tok != Y_BEGIN) {
					      sym = sql_symbol_lookup(yytext);
					  }
					  add_token(sym);
					  return tok; }

	/* numbers */

[0-9]+	|
[0-9]+"."[0-9]* |
"."[0-9]*	{ add_token(NULL); return number; }

[0-9]+[eE][+-]?[0-9]+	|
[0-9]+"."[0-9]*[eE][+-]?[0-9]+ |
"."[0-9]*[eE][+-]?[0-9]+	{ add_token(NULL); return number; }

	/* strings */

'	{ if (collect_string('\'') == EOF) { yyterminate(); }
	  return string;
	}

\"	{ if (collect_string('\"') == EOF) { yyterminate(); }
	  return string;
	}

\n		{ handle_newline(); }

[ \t\r]+	;	/* white space */

"--".*$		;	/* comment */

\/\*    {
        if (discard_comment() == EOF) {
            yyterminate();
        }
    }

%%

/* Many of these routines are copied from sql.l, simply because they
 * have to access static data like yytext.
 */
static token* add_token(SQL_SYM* sym) {
    return add_token_from(strdup(yytext), sym);
}

static token* add_token_from(const char* txt, SQL_SYM * sym) {
 unsigned int token_no;
 token *t = new_token (txt, &token_no);
 t->olineno = ilineno;
 t->lineno = ilineno;
 t->file_offset = get_start_file_offset ();
 t->column = sybase_get_icolumn ();

 yylval.start_token = yylval.end_token = token_no;
 yylval.sym = sym;
 yylval.ast = 0;
 return (t);
}

int id_or_kwd(int kwd, int only_after_at) {
    int tok = kwd;
    SQL_SYM* sym = NULL;
    token* t;
    if (pending_at) {
	pending_at = 0;
	tok = identifier;
    }
    else if (!only_after_at) {
	tok = map_to_allowed_token(kwd, identifier);
    }
    if (tok == identifier) {
	sym = sql_symbol_lookup(yytext);
    }
    t = add_token(sym);
    if (tok != identifier) {
	iff_keyword(t);
    }
    return tok;
}

void yyerror(char *s) {
    printf("%d: %s at %s\n", ilineno, s, yytext);
}

static void handle_newline() {
    size_t position = get_end_file_offset();
    icolumn_last = position - newline_offset;
    if (icolumn_last < 1) {
	icolumn_last = 1;
    }
    ilineno++;
    newline_offset = position + 1;
}

static int discard_comment() {
    int ch;
    for (;;) {
        while ((ch = input()) != '*' && ch != EOF) {
            if (ch == '\n') {
                handle_newline();
            }
        }
        if (ch == '*') {
            while ((ch = input()) == '*')
                { }
            if (ch == '/' || ch == EOF) {
                break;
            }
            if (ch == '\n') {
                handle_newline();
            }
        }
    }
    return ch;
}

/* collect_string accumulates a '...' or "..." string (depending on
 * the character passed in as "terminator").  It uses a local buffer
 * to store the string, which is then strduped into the token.
 */
static int collect_string(int terminator) {
    int ch;
    static char* string_buffer;
    static size_t string_buffer_len;
    int string_len = 1;
    int first_line = ilineno;
    int first_char = sybase_get_icolumn();
    unsigned token_no;
    token* first_token = new_token(NULL, &token_no);
    token* last_token;

#define INIT_STRING_BUFFER_LEN 4000
    if (!string_buffer) {
	string_buffer = (char*) malloc(INIT_STRING_BUFFER_LEN);
	string_buffer_len = INIT_STRING_BUFFER_LEN;
    }
#undef INIT_STRING_BUFFER_LEN

    *string_buffer = terminator;
    
    while ((ch = input()) != EOF) {
	if (string_len >= string_buffer_len - 1) {
	    string_buffer_len *= 2;
	    string_buffer = (char*) realloc(string_buffer, string_buffer_len);
	}
	if (ch == terminator) {
	    int ch2 = input();
	    if (ch2 != terminator) {
	        unput(ch2);
		string_buffer[string_len++] = ch;
		break;
	    }
	}
	else if (ch == '\n') {
	    handle_newline();
	}
	string_buffer[string_len++] = ch;
    }
    string_buffer[string_len] = 0;
    first_token->text = strdup(string_buffer);
    first_token->lineno = first_line;
    first_token->column = first_char;
    yylval.start_token = token_no;
    yylval.sym = NULL;
    yylval.ast = NULL;
    if (ilineno != first_line) {
        /* Tokens only indicate the starting point, with the length
         * implicit in the strlen of the text.  A multi-line string
         * therefore is represented by a token pair, with the text
         * in the first and the ending point and an empty string in
         * in the second.  (yylval represents a token range.)
         */
	last_token = new_token(strdup(""), &token_no);
	last_token->lineno = ilineno;
	last_token->column = get_end_file_offset() - newline_offset + 1;
    }
    yylval.end_token = token_no;
    return ch;
}

int yywrap() { return 1; }

/* Support the Interact (erroneous?) code that places a ":" after the
 * BEGIN keyword in some places.
 */
int begin_or_label_name() {
    if (yyleng == 5 &&
	toupper(yytext[0]) == 'B' &&
	toupper(yytext[1]) == 'E' &&
	toupper(yytext[2]) == 'G' &&
	toupper(yytext[3]) == 'I' &&
	toupper(yytext[4]) == 'N') {
	return Y_BEGIN;
    }
    return label_name;
}

int yydebug = 1;

/* Reinitialize all local variables and do the parse. */
int sybase_process(FILE* inp) {
    int retval;
    if (!yy_init) {
	yy_init = 1;
	ilineno = 1;
	current_offset = 0;
	next_offset = 0;
	newline_offset = 0;
	ast_root = 0;
	sql_errcount = 0;
	free_token_list();
	yyrestart(inp);
    }
    yyin = inp;
    get_icolumn = &sybase_get_icolumn;
    yydebug = 0;
    retval = yyparse();
    if (!retval && sql_errcount) {
        retval = 1;
    }
    return retval;
}

/*
 * Calculate file offset of yytext.
 *
 * current_offset -- the file offset at the start of the current buffer.
 * yy_ch_buf -- the flex input buffer.
 * yytext_ptr -- current position in buffer (same as yytext).
 */
static size_t get_start_file_offset (void)
{
   size_t offset = current_offset + (yytext - yy_current_buffer->yy_ch_buf);
   return (offset);
}

/*
 * Calculate file offset of end of yytext.
 *
 * current_offset -- the file offset at the start of the current buffer.
 * yy_ch_buf -- the flex input buffer.
 * yy_c_buf_p -- next position in buffer (one after end of yytext).
 */
static size_t get_end_file_offset (void)
{
   size_t offset = current_offset +
	 (yy_c_buf_p - yy_current_buffer->yy_ch_buf) - 1;
   return (offset);
}

/*
 * Calculate column on the current line of yytext.
 *
 * newline_offset -- the file offset of the first character of the
 * current line (one past the newline char).  Add one to make column
 * a one-relative number.
 */
static int sybase_get_icolumn (void)
{
   size_t column = get_start_file_offset () - newline_offset + 1;
   return (column);
}

