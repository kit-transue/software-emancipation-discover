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
   
%{
#ifdef _WIN32
#include <malloc.h>
#endif

#define BITS_PER_UNIT 8
#define BITS_PER_WORD 32
#define HOST_BITS_PER_INT 32
#define HOST_BITS_PER_LONG 64
#define DEFAULT_SIGNED_CHAR 1

#define TARGET_BELL 007
#define TARGET_BS 010
#define TARGET_TAB 011
#define TARGET_NEWLINE 012
#define TARGET_VT 013
#define TARGET_FF 014
#define TARGET_CR 015

#define pedwarn printf

void cli_error_msg();

#define error(msg)  cli_error_msg(msg)

/* #define warning(msg)  printf ("warning: %s\n", msg) */
#define warning(msg)  cli_error_msg("Warning: "); cli_error_msg(msg)


#define lookup (DEFAULT_SIGNED_CHAR ? 0 : -1)

#include <setjmp.h>
/* #define YYDEBUG 1 */

#ifdef MULTIBYTE_CHARS
#include <stdlib.h>
#include <locale.h>
#endif

#include <stdio.h>

typedef unsigned char U_CHAR;

/* Define a generic NULL if one hasn't already been defined.  */

#ifndef NULL
#define NULL 0
#endif

#ifndef GENERIC_PTR
#if defined (USE_PROTOTYPES) ? USE_PROTOTYPES : defined (__STDC__)
#define GENERIC_PTR void *
#else
#define GENERIC_PTR char *
#endif
#endif

#ifndef NULL_PTR
#define NULL_PTR ((GENERIC_PTR)0)
#endif

/*extern char expr_error[1024];*/

int yylex ();
static void yyerror ();

static jmp_buf parse_return_error;

/* Nonzero means count most punctuation as part of a name.  */
static int keyword_parsing = 0;

/* some external tables of character types */
/*extern*/ unsigned char is_idstart[], is_idchar[];

/*extern char *xmalloc ();*/

#ifndef CHAR_TYPE_SIZE
#define CHAR_TYPE_SIZE BITS_PER_UNIT
#endif

#ifndef INT_TYPE_SIZE
#define INT_TYPE_SIZE BITS_PER_WORD
#endif

#ifndef LONG_TYPE_SIZE
#define LONG_TYPE_SIZE BITS_PER_WORD
#endif

#ifndef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE INT_TYPE_SIZE
#endif

#ifndef MAX_CHAR_TYPE_SIZE
#define MAX_CHAR_TYPE_SIZE CHAR_TYPE_SIZE
#endif

#ifndef MAX_INT_TYPE_SIZE
#define MAX_INT_TYPE_SIZE INT_TYPE_SIZE
#endif

#ifndef MAX_LONG_TYPE_SIZE
#define MAX_LONG_TYPE_SIZE LONG_TYPE_SIZE
#endif

#ifndef MAX_WCHAR_TYPE_SIZE
#define MAX_WCHAR_TYPE_SIZE WCHAR_TYPE_SIZE
#endif


/* Yield nonzero if adding two numbers with A's and B's signs can yield a
   number with SUM's sign, where A, B, and SUM are all C integers.  */
#define possible_sum_sign(a, b, sum) ((((a) ^ (b)) | ~ ((a) ^ (sum))) < 0)

static void integer_overflow ();
static long left_shift ();
static long right_shift ();

typedef char *nodePtr;

static nodePtr expression_value;

nodePtr unary_expr_NOT();
nodePtr unary_expr_MINUS();
nodePtr unary_expr_COMP();
nodePtr unary_expr();

nodePtr binary_expr();
nodePtr binary_expr_EQ();
nodePtr binary_expr_NE();
nodePtr binary_expr_LE();
nodePtr binary_expr_GE();
nodePtr binary_expr_LT();
nodePtr binary_expr_GT();
nodePtr binary_expr_AND();
nodePtr binary_expr_OR();
nodePtr binary_expr_BIT_AND();
nodePtr binary_expr_BIT_OR();
nodePtr binary_expr_PLUS();
nodePtr binary_expr_MINUS();
nodePtr binary_expr_MULT();
nodePtr binary_expr_DIV();
nodePtr binary_expr_INT_DIV();
nodePtr binary_expr_LSH();
nodePtr binary_expr_RSH();
nodePtr binary_expr_BIT_XOR();

nodePtr if_expr();
nodePtr string_const();
nodePtr integer_const();
nodePtr name_const();
nodePtr var_ref();
nodePtr fun_call();
nodePtr arr_ref();
nodePtr arr_range();
nodePtr build_list();
nodePtr build_format();

nodePtr tree_set();
nodePtr tree_set_element();
nodePtr tree_setcur();
nodePtr tree_find();
nodePtr tree_search();
nodePtr tree_find1();
nodePtr tree_search1();
nodePtr tree_symbol();
nodePtr tree_visit();
nodePtr tree_top();
nodePtr tree_AND();
nodePtr tree_iterate();
nodePtr tree_times();
nodePtr tree_right();
nodePtr tree_left();
nodePtr tree_down();
nodePtr tree_up();
nodePtr build_command();
nodePtr tree_action();
nodePtr tree_assign();
nodePtr tree_equal();
nodePtr tree_string();
nodePtr tree_self();
nodePtr tree_nil();
nodePtr tree_true();
nodePtr tree_node();

static int filter_command_level = 0;
static int start_command = 0;

static void start_cmd(){++filter_command_level; start_command = 1;}
static void init_cmd() {filter_command_level = 0; start_command = 0;}
static void end_cmd()  {--filter_command_level; start_command = 0;}
static void prt_cmd()  { 
  printf("cmd_level %d start %d\n", filter_command_level, start_command);
}

%}

%union {
  struct constant {long value; int unsignedp;} integer;
  struct name {U_CHAR *address; int length;} name;
  int voidval;
  char *sval;
  char* node;
}

%type <node> exp iexp strexp boolexp list arglist action start term1 tree 
%type <node> subtree walk repeat tree_or_exp
%token <integer> INT CHAR SET NIL
%token <name> ANAME VAREF FNAME FORMAT STRING COMMAND
%token <integer> ERROR

%token  FIND FIND1  SEARCH SEARCH1 REPEAT
%token SETCUR SYMBOL

%right '?' ':'
%right ',' DOWN UP LEFT NULL_ACTION RANGE
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQUAL NOTEQUAL
%left '<' '>' LEQ GEQ
%left LSH RSH
%left '+' '-'
%left '*' '/' '%'
%right UNARY

/* %expect 40 */

%%

start   :	{ init_cmd(); } tree_or_exp
		{ expression_value = $2; }
        ;


tree_or_exp:   tree {$$ = tree_top($1);} | iexp;

arglist :       ANAME
                        { $$ = build_list(0,  name_const($1.address, $1.length));}
        |       arglist ',' ANAME
                        { $$ = build_list($1,  name_const($3.address, $3.length));}
        ;

list    :       COMMAND
			{ $$ = build_list(0, string_const($1.address, $1.length));}
	|	list exp
			{ $$ = build_list($1, $2);}
	|       list VAREF
	                { $$ = build_list($1, var_ref($2.address, $2.length));}
        |       list '.' 
                        {$$ =  build_list($1, tree_self());}
	;

exp     : iexp | strexp | boolexp 
        |       STRING
	                { $$ = string_const($1.address, $1.length);}
	|	INT
			{ $$ = integer_const(yylval.integer.value); }
	|	CHAR
			{ $$ = integer_const(yylval.integer.value); }
;

iexp	:	'-' exp    %prec UNARY
			{ $$ = unary_expr_MINUS($2);}
	|	'+' exp    %prec UNARY
			{ $$ = $2; }
	;

/* Binary operators in order of decreasing precedence.  */
iexp	:       exp '*' exp
			{ $$ = binary_expr_MULT( $1, $3);}
	|	exp '/' exp
			{ $$ = binary_expr_DIV( $1, $3);}
	|	exp '%' exp
			{ $$ = binary_expr_INT_DIV( $1, $3);}
	|	exp '+' exp
			{ $$ = binary_expr_PLUS( $1, $3);}
	|	exp '-' exp
			{ $$ = binary_expr_MINUS( $1, $3);}
	|       strexp '[' exp ']'
                        { $$ = arr_ref($1, $3);}
	|	strexp '[' exp ',' exp ']'
                        { $$ = arr_range($1, $3, $5);}
;

boolexp : 	'!' exp    %prec UNARY
			{ $$ = unary_expr_NOT($2);}
	|       exp EQUAL exp
			{ $$ = binary_expr_EQ($1, $3);}
	|	exp NOTEQUAL exp
			{ $$ = binary_expr_NE( $1, $3);}
	|	exp LEQ exp
			{ $$ = binary_expr_LE( $1, $3);}
	|	exp GEQ exp
			{ $$ = binary_expr_GE( $1, $3);}
	|	exp '<' exp
			{ $$ = binary_expr_LT( $1, $3);}
	|	exp '>' exp
			{ $$ = binary_expr_GT($1, $3);}
	|	exp AND exp
			{ $$ = binary_expr_AND( $1, $3);}
	|	exp OR exp
			{ $$ = binary_expr_OR($1, $3);}
	|	'(' iexp ')'
			{ $$ = $2; }
	|	'[' {start_cmd();}  list ']'
			{ $$ = build_command($3); end_cmd();}
;

strexp  :	ANAME
	                { $$ = name_const($1.address, $1.length);}
	|	FNAME '(' ')'
                 { $$ = fun_call($1.address, $1.length, 0, 0, 0, 0, 0, 0);}
	|	FNAME '(' exp ')'
                 { $$ = fun_call($1.address, $1.length, 1, $3, 0, 0, 0, 0);}
	|	FNAME '(' exp ',' exp ')'
                 { $$ = fun_call($1.address, $1.length, 2, $3, $5, 0, 0, 0);}
	|	FNAME '(' exp ',' exp ',' exp ')'
                 { $$ = fun_call($1.address, $1.length, 3, $3, $5, $7, 0, 0);}
	|	FNAME '(' exp ',' exp ',' exp ',' exp ')'
                 { $$ = fun_call($1.address, $1.length, 4, $3, $5, $7, $9, 0);}
	|	FNAME '(' exp ',' exp ',' exp ',' exp ',' exp ')'
                 { $$ = fun_call($1.address, $1.length, 5, $3, $5, $7, $9, $11);}
	|	exp '?' exp ':' exp
		 { $$ = if_expr($1, $3, $5);}
        |       FORMAT '(' STRING ',' arglist ')'
                 { $$ = build_format(string_const($3.address, $3.length), $5);}
	;

walk    :   ',' 
             { $$ = tree_right();}
        |   LEFT
             { $$ = tree_left();}
        |   DOWN 
             { $$ = tree_down();}
        |   UP
             { $$ = tree_up();}
;
 
tree    
        :   term1
        |   walk 
        |   repeat
        |   walk tree
             { $$ = tree_AND($1, $2);}
        |   term1 tree
             { $$ = tree_AND( $1, $2);}
;

subtree :   walk
        |   '(' tree ')' 
                {$$ = $2;}
;

repeat  :   REPEAT subtree tree
             { $$ = tree_iterate($2, $3); }
        |    INT RANGE  subtree tree
             { if($1.value == 0)
                $$=tree_iterate($3, $4);
               else if($1.value > 0)
                $$=tree_AND(tree_times($3, $1.value, 0), tree_iterate($3, $4));
               else
	        yyerror("negative number of iterations");
	     }
;

term1   :   strexp | boolexp 
        |   STRING
             { $$ = tree_string($1.address, $1.length);}
	|   VAREF
	     { $$ = tree_equal(var_ref($1.address, $1.length));}
	|   SET
	     { $$ = tree_set($1.value);}
	|   SET '/' INT
	     { $$ = tree_set_element($1.value, $3.value);}
        |   ANAME ':'
 	     { $$ = tree_assign($1.address, $1.length);}
        |  INT subtree
             { $$ = tree_times($2, $1.value, 1);}
        |  INT '!' subtree
             { $$ = tree_AND(tree_times($3, $1.value, 0), 
                             unary_expr_NOT(tree_visit($3)));}
        |  LSH tree RSH {$$=$2;}
;

action  : '{' 	{start_cmd();} list '}'	
              {$$ = tree_action($3); end_cmd();}
        | SETCUR '(' VAREF ')' 
              {$$ = tree_setcur(var_ref($3.address, $3.length));}
;

boolexp     :  '(' tree ')'
              { $$ = tree_visit($2); }
            |  FIND '(' tree ')'
              { $$ = tree_find($3); }
            |  SEARCH '(' tree ')'
              { $$ = tree_search($3); }
            |  FIND1 '(' tree ')'
              { $$ = tree_find1($3); }
            |  SEARCH1 '(' tree ')'
              { $$ = tree_search1($3); }
     |  NULL_ACTION
             {$$ = tree_true();}

        |  '.'
              {$$ = tree_node();}
        |  NIL 
              {$$ = tree_nil();}
        | action
;

strexp  :     SYMBOL '.' exp
              { $$ = tree_symbol($3);}
;  
%%

/* During parsing of a C expression, the pointer to the next character
   is in this variable.  */

static char *lexptr;

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/* maybe needs to actually deal with floating point numbers */

int
parse_number (olen)
     int olen;
{
  register char *p = lexptr;
  register int c;
  register unsigned long n = 0, nd, ULONG_MAX_over_base;
  register int base = 10;
  register int len = olen;
  register int overflow = 0;
  register int digit, largest_digit = 0;
  int spec_long = 0;

  yylval.integer.unsignedp = 0;

  if (len >= 3 && (!strncmp (p, "0x", 2) || !strncmp (p, "0X", 2))) {
    p += 2;
    base = 16;
    len -= 2;
  }
  else if (*p == '0')
    base = 8;

  ULONG_MAX_over_base = (unsigned long) -1 / base;

  for (; len > 0; len--) {
    c = *p++;

    if (c >= '0' && c <= '9')
      digit = c - '0';
    else if (base == 16 && c >= 'a' && c <= 'f')
      digit = c - 'a' + 10;
    else if (base == 16 && c >= 'A' && c <= 'F')
      digit = c - 'A' + 10;
    else {
      /* `l' means long, and `u' means unsigned.  */
      while (1) {
	if (c == 'l' || c == 'L')
	  {
	    if (spec_long)
	      yyerror ("two `l's in integer constant");
	    spec_long = 1;
	  }
	else if (c == 'u' || c == 'U')
	  {
	    if (yylval.integer.unsignedp)
	      yyerror ("two `u's in integer constant");
	    yylval.integer.unsignedp = 1;
	  }
	else
	  break;

	if (--len == 0)
	  break;
	c = *p++;
      }
      /* Don't look for any more digits after the suffixes.  */
      break;
    }
    if (largest_digit < digit)
      largest_digit = digit;
    nd = n * base + digit;
    overflow |= ULONG_MAX_over_base < n | nd < n;
    n = nd;
  }

  if (len != 0) {
    yyerror ("Invalid number in attribute expression\n");
    return ERROR;
  }

  if (base <= largest_digit)
    {warning ("integer constant contains digits beyond the radix7\n");};

  if (overflow)
    {warning ("integer constant out of range7\n");};

  /* If too big to be signed, consider it unsigned.  */
  if ((long) n < 0 && ! yylval.integer.unsignedp)
    {
      if (base == 10) 
	{warning ("integer constant is so large that it is unsigned\n");};
      yylval.integer.unsignedp = 1;
    }

  lexptr = p;
  yylval.integer.value = n;
  return INT;
}

struct token {
  char *operator;
  int token;
};

static struct token tokentab2[] = {
  {"&&", AND},
  {"||", OR},
  {"<<", LSH},
  {">>", RSH},
  {"==", EQUAL},
  {"!=", NOTEQUAL},
  {"<=", LEQ},
  {">=", GEQ},
  {"->", DOWN},
  {"<-", UP},
  {"{}",  NULL_ACTION},
  {"++", ERROR},
  {"--", LEFT},
  {"..", RANGE},
  {NULL, ERROR}
};

/* Read one token, getting characters through lexptr.  */

int
yylex ()
{
  register int c;
  register int namelen;
  register unsigned char *tokstart;
  register struct token *toktab;
  int wide_flag;

 retry:

  tokstart = (unsigned char *) lexptr;
  c = *tokstart;
  /* See if it is a special token of length 2.  */
  if (! keyword_parsing)
    for (toktab = tokentab2; toktab->operator != NULL; toktab++)
      if (c == *toktab->operator && tokstart[1] == toktab->operator[1]) {
	lexptr += 2;
	if (toktab->token == ERROR)
	  {
	    char *buf = (char *) alloca (40);
	    sprintf (buf, "`%s' not allowed in operand of expression\n", toktab->operator);
	    yyerror (buf);
	  }
	return toktab->token;
      }

  switch (c) {
  case 0:
    return 0;
    
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    lexptr++;
    goto retry;
    
  case 'L':
    /* Capital L may start a wide-string or wide-character constant.  */
    if (lexptr[1] == '\'')
      {
	lexptr++;
	wide_flag = 1;
	goto char_constant;
      }
    if (lexptr[1] == '"')
      {
	lexptr++;
	wide_flag = 1;
	goto string_constant;
      }
    break;

  case '\'':
    wide_flag = 0;
  char_constant:
    lexptr++;
    /* This code for reading a character constant
       handles multicharacter constants and wide characters.
       It is mostly copied from c-lex.c.  */
    {
      register int result = 0;
      register num_chars = 0;
      unsigned width = MAX_CHAR_TYPE_SIZE;
      int max_chars;
      char *token_buffer;

      if (wide_flag)
	{
	  width = MAX_WCHAR_TYPE_SIZE;
#ifdef MULTIBYTE_CHARS
	  max_chars = MB_CUR_MAX;
#else
	  max_chars = 1;
#endif
	}
      else
	max_chars = MAX_LONG_TYPE_SIZE / width;

      token_buffer = (char *) alloca (max_chars + 1);

      while (1)
	{
	  c = *lexptr++;

	  if (c == '\'' || c == EOF)
	    break;

	  if (c == '\\')
	    {
	      c = parse_escape (&lexptr);
	      if (width < HOST_BITS_PER_INT
		  && (unsigned) c >= (1 << width))
		pedwarn ("escape sequence out of range for character\n");
	    }

	  num_chars++;

	  /* Merge character into result; ignore excess chars.  */
	  if (num_chars < max_chars + 1)
	    {
	      if (width < HOST_BITS_PER_INT)
		result = (result << width) | (c & ((1 << width) - 1));
	      else
		result = c;
	      token_buffer[num_chars - 1] = c;
	    }
	}

      token_buffer[num_chars] = 0;

      if (c != '\'')
	error ("malformatted character constant\n");
      else if (num_chars == 0)
	error ("empty character constant\n");
      else if (num_chars > max_chars)
	{
	  num_chars = max_chars;
	  error ("character constant too long\n");
	}
      else if (num_chars != 1)
	{warning ("multi-character character constant\n");};

      /* If char type is signed, sign-extend the constant.  */
      if (! wide_flag)
	{
	  int num_bits = num_chars * width;

	  if (lookup || ((result >> (num_bits - 1)) & 1) == 0)
	    yylval.integer.value
	      = result & ((unsigned long) ~0 >> (HOST_BITS_PER_LONG - num_bits));
	  else
	    yylval.integer.value
	      = result | ~((unsigned long) ~0 >> (HOST_BITS_PER_LONG - num_bits));
	}
      else
	{
#ifdef MULTIBYTE_CHARS
	  /* Set the initial shift state and convert the next sequence.  */
	  result = 0;
	  /* In all locales L'\0' is zero and mbtowc will return zero,
	     so don't use it.  */
	  if (num_chars > 1
	      || (num_chars == 1 && token_buffer[0] != '\0'))
	    {
	      wchar_t wc;
	      (void) mbtowc (NULL_PTR, NULL_PTR, 0);
	      if (mbtowc (& wc, token_buffer, num_chars) == num_chars)
		result = wc;
	      else
		{warning ("Ignoring invalid multibyte character\n");};
	    }
#endif
	  yylval.integer.value = result;
	}
    }

    /* This is always a signed type.  */
    yylval.integer.unsignedp = 0;
    
    return CHAR;

    /* some of these chars are invalid in constant expressions;
       maybe do something about them later */
  case '/':
  case '+':
  case '-':
  case '*':
  case '%':
  case '|':
  case '&':
  case '^':
  case '~':
  case '!':
/*  case '@': */
  case '<':
  case '>':
  case '[':
  case ']':
  case '.':
  case '?':
  case ':':
  case '=':
  case '{':
  case '}':
  case ',':
  case '(':
  case ')':
    lexptr++;
    return c;

  case '@':
    {int fl = 0; int par=0;
    for (namelen = 1; (c=tokstart[namelen]);  ++namelen){
       switch (c) {
       case '(': ++par; break;
       case ')': --par; if(par < 0) fl =1; break;
       case ',':
       case ' ':
       case '\t':
       case '\n':
       case '\r':
        if(par==0) fl = 1;
     }
     if(fl) break;
    }
    lexptr += (namelen);
    yylval.name.address = tokstart+1;
    yylval.name.length = namelen - 1;

    return STRING;
   }
  case '`':
    for (namelen = 1; tokstart[namelen] != '`'&&(tokstart[namelen] != '\0'); ++namelen);

    if (tokstart[namelen] == '\0') {
	warning("String not terminated with single-quote.\n");
	namelen--;
    };

    lexptr += (namelen+1);
    yylval.name.address = tokstart+1;
    yylval.name.length = namelen - 1;

    return STRING;

  case '"':
  string_constant:
    for (namelen = 1; (tokstart[namelen] != '"')&&(tokstart[namelen] != '\0'); ++namelen);

    if (tokstart[namelen] == '\0') {
	warning("String not terminated with double-quote.\n");
	namelen--;
    };

    lexptr += (namelen+1);
    yylval.name.address = tokstart+1;
    yylval.name.length = namelen - 1;

    return STRING;
  }

  if (c >= '0' && c <= '9' && !keyword_parsing) {
    /* It's a number */
    for (namelen = 0;
	 c = tokstart[namelen], is_idchar[c]; 
	 namelen++)
      ;
    return parse_number (namelen);
  }

  /* It is a name.  See how long it is.  */

  {
    if (!is_idstart[c]) {
      yyerror ("Invalid token in expression\n");
      return ERROR;
    }
    
    for (namelen = 1; is_idchar[tokstart[namelen]]; namelen++)
      ;
  }
  
  lexptr += namelen;
  if(tokstart[0]== '$'){
    yylval.name.address = tokstart+1;
    yylval.name.length = namelen-1;
    return VAREF;
  }
  yylval.name.address = tokstart;
  yylval.name.length = namelen;
 
  if(namelen==3 && tokstart[0]=='n' && tokstart[1]=='i' && tokstart[2] == 'l')
    return NIL;

  if(filter_command_level && start_command) {
     start_command = 0;
     return COMMAND;
  } 

  if((namelen>4) && (tokstart[0]=='_')) {
    if(strncmp(tokstart, "_DI_", 4)==0){
      int ind = atoi(tokstart+4);
      yylval.integer.value = ind;
      return SET;
    }
  }
  switch (namelen) {
  case 1:
    {
      int ch = tokstart[0];
      if(ch == 'r')
	return REPEAT;
      else if(ch == 's')
	return SYMBOL;
    }
    break;
  case 4:
    if(strncmp(tokstart, "find", namelen)==0)
      return FIND;
    break;
  case 5:
    if(strncmp(tokstart, "find1", namelen)==0)
      return FIND1;
    break;
  case 6:
    if(strncmp(tokstart, "setcur", namelen)==0)
      return SETCUR;
    else if(strncmp(tokstart, "search", namelen)==0)
      return SEARCH;
    else if(strncmp(tokstart, "repeat", namelen)==0)
      return REPEAT;
    else if(strncmp(tokstart, "format", namelen)==0)
      return FORMAT;
    break;
  case 7:
    if(strncmp(tokstart, "search1", namelen)==0)
      return SEARCH1;
    break;
  }
  { 

    int ntype = cli_name_type(tokstart, namelen);
    int tp = STRING;
    if(ntype == 0 && filter_command_level == 0)
      tp = ANAME; /* to generate 'non-existing attribute error' */
    else if(ntype==1)
      tp = FNAME;
    else if(ntype==2)
      tp = ANAME;

    return tp;
 }
}


/* Parse a C escape sequence.  STRING_PTR points to a variable
   containing a pointer to the string to parse.  That pointer
   is updated past the characters we use.  The value of the
   escape sequence is returned.

   A negative value means the sequence \ newline was seen,
   which is supposed to be equivalent to nothing at all.

   If \ is followed by a null character, we return a negative
   value and leave the string pointer pointing at the null character.

   If \ is followed by 000, we return 0 and leave the string pointer
   after the zeros.  A value of 0 does not mean end of string.  */

int
parse_escape (string_ptr)
     char **string_ptr;
{
  register int c = *(*string_ptr)++;
  switch (c)
    {
    case 'a':
      return TARGET_BELL;
    case 'b':
      return TARGET_BS;
    case 'e':
    case 'E':
      return 033;
    case 'f':
      return TARGET_FF;
    case 'n':
      return TARGET_NEWLINE;
    case 'r':
      return TARGET_CR;
    case 't':
      return TARGET_TAB;
    case 'v':
      return TARGET_VT;
    case '\n':
      return -2;
    case 0:
      (*string_ptr)--;
      return 0;
      
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
	register int i = c - '0';
	register int count = 0;
	while (++count < 3)
	  {
	    c = *(*string_ptr)++;
	    if (c >= '0' && c <= '7')
	      i = (i << 3) + c - '0';
	    else
	      {
		(*string_ptr)--;
		break;
	      }
	  }
	if ((i & ~((1 << MAX_CHAR_TYPE_SIZE) - 1)) != 0)
	  {
	    i &= (1 << MAX_CHAR_TYPE_SIZE) - 1;
	    warning ("octal character constant does not fit in a byte\n");
	  }
	return i;
      }
    case 'x':
      {
	register unsigned i = 0, overflow = 0, digits_found = 0, digit;
	for (;;)
	  {
	    c = *(*string_ptr)++;
	    if (c >= '0' && c <= '9')
	      digit = c - '0';
	    else if (c >= 'a' && c <= 'f')
	      digit = c - 'a' + 10;
	    else if (c >= 'A' && c <= 'F')
	      digit = c - 'A' + 10;
	    else
	      {
		(*string_ptr)--;
		break;
	      }
	    overflow |= i ^ (i << 4 >> 4);
	    i = (i << 4) + digit;
	    digits_found = 1;
	  }
	if (!digits_found)
	  yyerror ("\\x used with no following hex digits");
	if (overflow | (i & ~((1 << BITS_PER_UNIT) - 1)))
	  {
	    i &= (1 << BITS_PER_UNIT) - 1;
	    warning ("hex character constant does not fit in a byte\n");
	  }
	return i;
      }
    default:
      return c;
    }
}

static void
yyerror (s)
     char *s;
{
  error (s);
  longjmp (parse_return_error, 1);
}

static void
integer_overflow ()
{
  pedwarn ("integer overflow in attribute expression\n");
}

static long
left_shift (a, b)
     struct constant *a;
     unsigned long b;
{
  if (b >= HOST_BITS_PER_LONG)
    {
      if (! a->unsignedp && a->value != 0)
	integer_overflow ();
      return 0;
    }
  else if (a->unsignedp)
    return (unsigned long) a->value << b;
  else
    {
      long l = a->value << b;
      if (l >> b != a->value)
	integer_overflow ();
      return l;
    }
}

static long
right_shift (a, b)
     struct constant *a;
     unsigned long b;
{
  if (b >= HOST_BITS_PER_LONG)
    return a->unsignedp ? 0 : a->value >> (HOST_BITS_PER_LONG - 1);
  else if (a->unsignedp)
    return (unsigned long) a->value >> b;
  else
    return a->value >> b;
}

/* This page contains the entry point to this file.  */

/* Parse STRING as an expression, and complain if this fails
   to use up all of the contents of STRING.  */
/* We do not support C comments.  They should be removed before
   this function is called.  */

nodePtr
api_parse_expression_intern (string)
     char *string;
{
/*  expr_error[0] = '\0'; */
  
  lexptr = string;
  
  if (lexptr == 0 || *lexptr == 0) {
    error ("empty attribute expression\n");
    return 0;			/* don't include the #if group */
  }

  /* if there is some sort of scanning error, just return 0 and assume
     the parsing routine has printed an error message somewhere.
     there is surely a better thing to do than this.     */
  if (setjmp (parse_return_error))
    return 0;

  if (yyparse ()) {
      return 0;			/* actually this is never reached
				   the way things stand. */
  };

  if (*lexptr)
    error ("Junk after end of expression.\n");

  return expression_value;	/* set by yyparse () */
}


/* table to tell if char can be part of a C identifier. */
unsigned char is_idchar[256];
/* table to tell if char can be first char of a c identifier. */
unsigned char is_idstart[256];
/* table to tell if c is horizontal space.  isspace () thinks that
   newline is space; this is not a good idea for this program. */
char is_hor_space[256];

/*
 * initialize random junk in the hash table and maybe other places
 */
initialize_random_junk ()
{
  register int i;

  /*
   * Set up is_idchar and is_idstart tables.  These should be
   * faster than saying (is_alpha (c) || c == '_'), etc.
   * Must do set up these things before calling any routines tthat
   * refer to them.
   */
  for (i = 'a'; i <= 'z'; i++) {
    ++is_idchar[i - 'a' + 'A'];
    ++is_idchar[i];
    ++is_idstart[i - 'a' + 'A'];
    ++is_idstart[i];
  }
  for (i = '0'; i <= '9'; i++)
    ++is_idchar[i];
  ++is_idchar['_'];
  ++is_idstart['_'];
#if DOLLARS_IN_IDENTIFIERS
  ++is_idchar['$'];
#endif
  ++is_idstart['$'];

}

#ifdef TEST_EXP_READER
#define quote(x) #x
#include <string.h>

char* tree_nil() { return "nil";}
char* tree_true() { return "{}";}
char* tree_node() { return ".";}

char* con2(a1,a2)
  char* a1, *a2;
{
  char buf[1024];
  buf[0] ='(';
  if(a1)
    strcpy(buf+1, a1);
  else
    buf[1] = '\0';
  if(a2){
    strcat(buf, "^");
    strcat(buf, a2);
  }
  strcat(buf, ")");
  return strdup(buf);
}
nodePtr niy2(n,a1,a2)
  char*n;
  nodePtr a1,a2;
{
  char* res = con2(a1,a2);
  printf("%s %s \n", n, res);
  return res;
}
#define NIY(x) nodePtr x (){printf("%s\n", quote(x)); return 0;}
#define NIY1(x) nodePtr x (a1) nodePtr a1;{printf("%s %s\n", quote(x), a1); return a1;}
#define NIY2(x) nodePtr x (a1,a2) nodePtr a1,a2;{return niy2(quote(x), a1, a2);}

NIY(unary_expr);
NIY(unary_expr_MINUS);
NIY(unary_expr_NOT);
NIY(if_expr);

NIY1(build_command);
NIY1(tree_action);
NIY2(build_list);
NIY2(build_format);
NIY2(tree_walk);
NIY1(tree_setcur);
NIY1(tree_find);
NIY1(tree_search);
NIY1(tree_find1);
NIY1(tree_search1);
NIY1(tree_symbol);
NIY1(tree_visit);
NIY1(tree_top);
NIY1(tree_equal);

nodePtr tree_right()
{
  return ",";
}

nodePtr tree_left()
{
  return ";";
}

nodePtr tree_down()
{
  return "->";
}
nodePtr tree_up()
{
  return "<-";
}
nodePtr tree_self()
{
  return ".";
}

int cli_name_type(tokstart, namelen)
  unsigned char *tokstart;
  int namelen;
{ 
#if 0
    return 0;  /* string */
#else
    return 2;  /* aname */
#endif
}

/*char expr_error[1024];*/


void cli_error_msg(msg)
char* msg;
{
  printf("%s", msg);
}

   NIY2(binary_expr_DIV );
   NIY2(binary_expr_BIT_XOR );
   NIY2(binary_expr_MULT);
   NIY2(binary_expr_PLUS);
   NIY(unary_expr_COMP);
   NIY2(binary_expr_INT_DIV);
   NIY2(binary_expr_MINUS);
   NIY2(binary_expr_BIT_AND); 
   NIY2(binary_expr_BIT_OR);

NIY2(tree_AND);
NIY2(tree_iterate);
NIY2(binary_expr_AND);
NIY2(binary_expr_NOT);
NIY2(binary_expr_OR);

nodePtr integer_const(val)
  int val;
{
  char buf[6];
  sprintf(buf, "%d", val);
  printf ("int %d\n", val);
  return strdup(buf);
}
nodePtr tree_times(tree, val, del)
  int val; nodePtr tree;  int del;
{
  char buf[60];
  sprintf(buf, "%d(%s %d)", val, tree, del);
  printf ("times %d(%s %d)\n", val, tree, del);
  return strdup(buf);
}

nodePtr handle_string(title, ptr, len)
  char* title; int len; char* ptr;
{ 
  char buf[100];
  printf("%s:", title);
  fwrite(ptr,1,len,stdout); 
  printf("\n");
  strncpy(buf, ptr, len);
  buf[len] = '\0';
  return strdup(buf);
}

nodePtr name_const( ptr, len)
  int len; char* ptr;
{ 
  return handle_string("name", ptr, len);
}

nodePtr var_ref( ptr, len)
  int len; char* ptr;
{ 
  return handle_string("var_ref", ptr, len);
}

nodePtr tree_assign( ptr, len)
  int len; char* ptr;
{ 
  return handle_string("assign", ptr, len);
}

nodePtr string_const( ptr, len)
  int len; char* ptr;
{
  return handle_string("string", ptr, len);
}

nodePtr tree_string( ptr, len)
  int len; char* ptr;
{
  return handle_string("tree_string", ptr, len);
}

nodePtr fun_call( ptr, len, no, arg1, arg2)
  int len; char* ptr; int no; nodePtr arg1, arg2;
{ 
  printf("fun_call %d %s \b", no, ptr);
  printf("\n");
  return 0;
}

NIY2(arr_ref);
NIY(arr_range);

NIY2(binary_expr);
NIY2(binary_expr_GE);
NIY2(binary_expr_LE);
NIY2(binary_expr_EQ);
NIY2(binary_expr_NE);
NIY2(binary_expr_GT);
NIY2(binary_expr_LT);

extern int yydebug;

/* Main program for testing purposes.  */
int
main (argc, argv)
 int argc; char** argv;
{
  int n, c;
  char buf[1024];

/*
  yydebug = 1;
*/
  initialize_random_junk ();

  if (argc < 2) for (;;) {
    printf ("enter expression: ");
    n = 0;
    while ((buf[n] = getchar ()) != '\n' && buf[n] != EOF)
      n++;
    if (buf[n] == EOF)
      break;
    buf[n] = '\0';
    printf ("parser returned %d\n", api_parse_expression_intern (buf));
  } else {
    printf ("parser returned %d\n", api_parse_expression_intern (argv[1]));
  }
  return 0;
}
#endif 
