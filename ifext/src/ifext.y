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
#include "iffYacc.h"
int yyerror(char*, ...);
/* #define YYMAXDEPTH 10000 */
int ast_level = 0;
int ifp_sym_offset = 0;
%}

%union {
  struct iffPosition position;
  struct iffRefId     ref;
  struct { struct iffAstNode* first; struct iffAstNode*last; } list;
  struct iffAstNode*  node;
  struct iffSymAtr*  symbol;
  struct iffRegion*   region;
  struct iffRel* relation;
  struct iffKeyword*  key;
  unsigned int   integer;
  char*  string;
  void*  dummy;
}

%token AST MAP ERR 
%token <integer> ATR REL SYM SMT
%token SRC_FILE LANGUAGE KEYWORD
%token IMPLICIT AUTO  CONSTRUCTOR  DESTRUCTOR   INLINE  LINKC  ELLIPSIS
%token PRIVAT PROTECTED PUBLIC VIRTUAL CONST POINTER REFERENCE
%token DEF CG map_COPY map_FIXED map_LINES
%token <integer> ID INTEGER  
%token <string> TOKEN ATTRIBUTE
%token  <string> STRING
%token <position> POSITION

%type <node>   ast_node
%type <symbol> sym_stmt
%type <region> region
%type <list>   ast_list
%type <string> attribute
%type <string> mapostring
%type <symbol> atr_stmt
%type <relation> rel_stmt
%type  <dummy> stmts statement
%type <ref> ref_stmt
%type <key> key_stmt

%start stmts

%%

stmts        : statement | stmts statement
                        ;

statement    : sym_stmt {}
             | atr_stmt {}
             | rel_stmt {}
             | err_stmt {}
             | ast_stmt {}
             | ref_stmt {}
             | file_stmt {}
             | key_stmt {}
             | lang_stmt {}
             | map_stmt {}
             ;
    
region      :   POSITION    {$$ = reg_make(&$1);}
            |   POSITION  POSITION
                 {$$ = reg_make(&$1); if($$) $$ = reg_end($$, &$2);}
            |  POSITION  INTEGER  
                 {$$ = reg_make(&$1); if($$) {
		      $$->en.line = $$->st.line;
                      $$->en.column = $$->st.column + $2 -1;
                 }}
            ;
sym_stmt       : SYM TOKEN
                      {$$ = sym_make($1, $2);}
               | sym_stmt DEF 
                      {$1->def = 1;}
               | sym_stmt CG 
                      {$1->cg = 1;}
               | sym_stmt STRING 
                      {sym_add_name($1, $2);}
               | sym_stmt region 
                      {sym_add_reg($1,$2);}
               ;

attribute      : TOKEN | ATTRIBUTE ;

atr_stmt       : ATR
                 { $$ = sym_get_atr($1);}
               | atr_stmt attribute  
                 {sym_add_atr($1, $2);}
               | atr_stmt AUTO {$1->au = 1;}
               | atr_stmt VIRTUAL {$1->virt = 1;}
               | atr_stmt CONSTRUCTOR {$1->ctor = 1;}
               | atr_stmt DESTRUCTOR  {$1->dtor = 1;}
               | atr_stmt INLINE      {$1->inln = 1;}
               | atr_stmt LINKC       {$1->linkc = 1;}
               | atr_stmt ELLIPSIS    {$1->ellipsis = 1;}
               ;

rel_stmt       :  REL STRING {$2 = ifp_rel_get_code($2);} ID
                    { $$ = sym_add_rel($1, $2, $4);}
               |  rel_stmt attribute
                    { rel_add_atr($1, $2);}
               |  rel_stmt PRIVAT   {$1->ppp = 1;}
               |  rel_stmt PROTECTED {$1->ppp = 2;}
               |  rel_stmt PUBLIC    {$1->ppp = 3;}
               |  rel_stmt VIRTUAL   {$1->virt = 1;}
               |  rel_stmt CONST     {$1->constant = 1;}
               |  rel_stmt POINTER   {$1->pointer = 1;}
               |  rel_stmt REFERENCE {$1->reference = 1;}
               ;

err_stmt       :  ERR POSITION STRING       
                    {err_make($2.line, $3);}
               ;     

ast_stmt       :  AST
                    { ast_level = 1;}
               |  ast_stmt STRING
		    { file_make($2);}
               |  ast_stmt  '{' ast_list  '}'
		    { ast_level = 0; if($3.first)ast_add_roots($3.first);}
               ;

ast_list       : ast_node
                    { $$.first = $$.last = $1;}
               | STRING
                    {$$.first = $$.last = ast_make_string($1);}
               | ast_list ast_node
                    { if($2) {
                        if($1.last) {
                            $$.last->next = $2; $$.last = $2;
                        } else {
                            $$.first = $$.last = $2;
                        }
                      }
		    }
	       ;

ast_node       : TOKEN
                    {$$ = ast_make_node($1);}   
               | ast_node DEF 
                    {$1->def = 1;}
               | ast_node CG 
                    {$1->cg = 1;}
               | ast_node ID
                    {$1->ind = $2;}   
               | ast_node region
                    {if($2) $1->region = $2; else if(ast_level==1)$$ = 0;}   
               | ast_node STRING region
                    {$$ = ast_add_extern_region($1, $2, $3);}
               | ast_node '{' '}'
               | ast_node  '{' {ast_level++;} ast_list {ast_level--;} '}'
                    { if($1) $1->first =  $4.first;}
               ;                  

ref_stmt       : SMT
		   {$$.id = $1; $$.implicit = 0;}
               | ref_stmt IMPLICIT
		   { $1.implicit = 1;}
               | ref_stmt region
		   { if($2){ $1.region = $2; ref_make(&$1);}}
               ;

key_stmt       : KEYWORD STRING
                  {$$ = key_make($2);}
               | key_stmt region
                  {key_add_region($1, $2);}
               ;
file_stmt      : SRC_FILE STRING
                   { file_make($2);}      
               ;

lang_stmt      : LANGUAGE STRING
                   { language($2); }
               ;

mapostring       : {}
               | STRING 
               | STRING STRING 
                   {ifp_set_output($2);} 
               ;

map_stmt  : map_COPY mapostring mapcopy '}'
          | map_FIXED mapostring mapfixed '}'
          | map_LINES mapostring maplines '}'
          ;

mapcopy   : '{' 
          | mapcopy POSITION  POSITION POSITION POSITION
               { map_copy_pppp(&$2, &$3, &$4, &$5);}
          | mapcopy POSITION  INTEGER  POSITION
               { map_copy_pcp(&$2, $4.column + $3 - 1, &$4);}
          ;

mapfixed  : '{' 
          | mapfixed POSITION  POSITION  POSITION POSITION
               { map_fixed_pppp(&$2, &$3, &$4, &$5);}
          ;

maplines  : '{'
          | maplines INTEGER INTEGER  INTEGER
               { map_lines($2, $3, $4);}
          ;

%%

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>

static  FILE*yyin ;
static  int yych;

#if 0

#define GETC getc
#define YYEOF EOF
#define init_buffer(f)
#define reset_buffer()

#else 
#define GETC(f) (*(buf_ptr++))
#define YYEOF 0
#define init_buffer(f) (ifp_use_mmap ? init_buffer_mmap(f) : init_buffer_read(f)) 
#define reset_buffer() (ifp_use_mmap ? reset_buffer_mmap() : reset_buffer_read()) 

static char*buf_ptr, *buf_start;
int ifp_use_mmap;

#ifndef _WIN32
#include <sys/mman.h>

#ifndef SEEK_END
#define SEEK_END 2
#endif
static void init_buffer_mmap(FILE *f)
{
  int len, ii;
  int unit = fileno(f);

  if (unit > 0) {
    len = lseek(unit,0,SEEK_END);
    buf_ptr = buf_start = mmap(0, len+10, PROT_READ, MAP_PRIVATE, unit, 0);
  }
}

static void reset_buffer_mmap()
{
  buf_ptr = buf_start = NULL;
  yyin = NULL;
}
# else

#define init_buffer_mmap init_buffer_read
#define reset_buffer_mmap reset_buffer_read

#endif

static void init_buffer_read(FILE *f)
{
  int len, ii;
  int unit = fileno(f);

  struct stat st;
  int size;
  fstat(unit, &st);
  size = st.st_size;
  buf_ptr = buf_start = malloc(size+10);
  len = read(fileno(f), buf_ptr, size);
  for(ii=0; ii<10; ++ii)
    buf_ptr[len+ii] = '\0';
}
static void reset_buffer_read()
{
  free(buf_start);
  buf_ptr = buf_start = NULL;
  yyin = NULL;
}

#endif


#define YYMAXBUF 36000
static char yytext[YYMAXBUF+1];

#if 0

#define input() (yych=GETC(yyin))
#define settext() 
#define setint(v)  yylval.integer = v
#define ret(x) {state=x; state=0;goto try;}

#else

#define input() (yych=yytext[++yyleng]=GETC(yyin))
#define settext() yytext[yyleng] = '\0', yylval.string = yytext
#define setint(v)  yylval.integer = v

#if YYDEBUG
#define ret(x) return(printf("token %s\n", yytext),x)
#else
#define ret(x) return x
#endif

#endif

int ifp_skip_flag = 0;
int ifp_line = 1;

#define yyassert(x)

struct _t {char* tag; int tok; int len;} tokens[] = {
  {"cg",             CG}, 
  {"def",            DEF},
  {"auto",           AUTO},
  {"const",          CONST},
  {"inline",         INLINE},
  {"public",         PUBLIC},
  {"pointer",        POINTER},
  {"private",        PRIVAT},
  {"virtual",        VIRTUAL},
  {"implicit",       IMPLICIT},
  {"ellipsis",       ELLIPSIS},
  {"protected",      PROTECTED},
  {"reference",      REFERENCE},
  {"destructor",     DESTRUCTOR},
  {"constructor",    CONSTRUCTOR},
};

int toksize = sizeof(tokens) / sizeof(struct _t);

static int setlen()
{
  int ii;
  int sz = toksize;

  for(ii=0;ii<sz; ++ii){
    struct _t *p = &tokens[ii];
    p->len = strlen(p->tag);
  }
}

void ifp_init_input ( FILE * f)
{
    yyin = f;
    yych = ' ';
    setlen();
    ifp_line = 1;
    init_buffer(f);
}
void ifp_reset_input() 
{ 
  reset_buffer();
}

static int yymatch_token(char*text, int len)
{
  int ii;
  int sz = toksize;
  int ch = text[0];


  for(ii=0;ii<sz; ++ii){
    struct _t *p = &tokens[ii];
    if(p->len < len) continue;
    if(p->len > len) break;
    if((ch==*p->tag)&& strcmp(p->tag, text)==0){
/*      printf("yymatch %s\n", text); */
      return p->tok;
    }
  }
  return TOKEN;
}

int yylex() {
  int val;
  int yyleng;
  int state = 0;
 try:
  while(isspace(yych)){
    if (yych=='\n') ++ifp_line;
    yych =  GETC(yyin);
  }

  switch(yych) {
  case YYEOF: 
    return 0;

  case ',': case ':':
    yych =  GETC(yyin);
    goto try;

  case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
  case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
  case 'K':  case 'L':  case 'M':  case 'N':  case 'O':
  case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
  case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
  case 'Z':
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
  case 'z':
    yyleng = 0;
    yytext[0] = yych;
    while(isalnum(input()) || (yych=='_'));
    if(yych=='('){
      while(input() != ')'){
	if(yych=='"') {
	  while((input()) != '"') {
	    if(yych == '\\') input();
	  }
	}
      }
      input();  /* now yytext has extra character */
      if(yyleng==10 && yytext[0]=='l' && (strncmp(yytext,"linkage(c)",10)==0)){
	{ret(LINKC);}
      } else {
	settext();
	ret(ATTRIBUTE);
      }
    } else if (state==MAP){
      switch (yytext[0]){
      case 'c' : ret(map_COPY); 
      case 'f' : ret(map_FIXED); 
      case 'l' : ret(map_LINES); 
      default  : yyerror("wrong MAP token: %s", yytext);
      }
        
    } else if(state == SMT){
      switch (yytext[0]){
      case 'f' : ret(SRC_FILE); 
      case 'k' : if(ifp_skip_flag)
	           {ifp_skip_rest_of_line(); state = 0; goto try;}
                 else 
		   {ret(KEYWORD);}
      case 'l' : ret(LANGUAGE); 
      default  : yyerror("wrong SMT token: %s", yytext);
      }
    }
    if(yyleng==3){
      switch (yytext[0]){
      case 'A':
	if(yytext[1] == 'S' && yytext[2]=='T'){
	  if(ifp_skip_flag) 
	    { ifp_skip_block(0); state = 0; goto try; }
	  ret(AST);
	} else if(yytext[1] == 'T' && yytext[2]=='R') {
	  state = ATR; goto try;
	}
	else break;
      case 'E':
	if(yytext[1] == 'R' && yytext[2]=='R')
	  {ret(ERR);}
        else
	  break;
      case 'M':
	if(yytext[1] == 'A' && yytext[2]=='P'){
	  state =  MAP; 
	  goto try;
        } else
	  break;
      case 'R':
	if(yytext[1] == 'E' && yytext[2]=='L')
	  {state =  REL; goto try;}
        else
	  break;
	
      case 'S':
	if(yytext[1] == 'Y' && yytext[2]=='M')
	  {state =  SYM; goto try;}
	else if(yytext[1] == 'M' && yytext[2]=='T')
	  {state = SMT; goto try;}
	else
	  break;
      defalt:
	break;
      }
    }  
  
    settext();
    ret(yymatch_token(yytext, yyleng));

  case '0':  case '1':  case '2':  case '3':  case '4':
  case '5':  case '6':  case '7':  case '8':  case '9':
    val = yych - '0';
    while(isdigit(yych=GETC(yyin)))
      val = val * 10 + (yych - '0');
    if(yych=='/'){
      yylval.position.line = val;
      yych=GETC(yyin);
      val = yych - '0';
      while(isdigit(yych=GETC(yyin)))
	val = val * 10 + (yych - '0');
      yylval.position.column = val;
      ret(POSITION);
    } else {
      setint(val);
      ret(state ? state : INTEGER);
    }

  case '[':
    if(state==SMT && ifp_skip_flag)
      {ifp_skip_rest_of_line(); state = 0; goto try;}
    yych=GETC(yyin);
    val = yych - '0';
    while((yych=GETC(yyin)) !=']')
      val = val * 10 + (yych - '0');
    yych=GETC(yyin);
    setint(val+ifp_sym_offset);
    ret(state ? state : ID);

  case '"':
    yyleng = 0;
    yytext[0] = yych;
    while((input()) != '"') {
      if(yych=='\n'){
	yyerror("unterminating string");
      } else if(yyleng >= YYMAXBUF){
	yyerror("string too long");
      }
      if(yych == '\\') input();
    }
    input();

    if(!state)
      state = STRING;

    settext();
    ret(state);

  case '{':
    yych=GETC(yyin);
    if(yych=='}'){
      yych=GETC(yyin);
      goto try;
    } else {
      ret('{');
    }

  default: {
    int ch = yych;
    yych = GETC(yyin);
    ret(ch);
   }
  }
}

void ifp_skip_block (cur_level)
  int cur_level;
{
  int ch;
  int linecount = 0;
  FILE*yyin_loc = yyin;
  /* printf("ifp_skip_block(%d)\n", cur_level); */
  while((ch=GETC(yyin_loc))!=YYEOF){
    if(ch=='{'){
      ++cur_level;
    } else if (ch=='}') {
      --cur_level;
      if(cur_level==0)
       break;
    } else if (ch == '"') {
       while(ch = GETC(yyin_loc)) {
          if(ch == '\\') 
            GETC(yyin_loc);
          else if(ch == '"')
            break;
       }
    } else if(ch=='\n') {
       ++linecount;
    }
  }
  ifp_line +=linecount;
  yych=' ';
}
void ifp_skip_rest_of_line ()
{
  int ch;
 /*   printf("ifp_skip_rest_of_line\n"); */
  while((ch=GETC(yyin)) && (ch != '\n'));
  ifp_line++;
  yych=' ';
}

int yyerror(char* fmt, ...)
{
  int column;
  char*start;
  int ch;
  void *estringp, *lstringp, *fstringp;
  char *echo, *locator, *formatted;
  va_list ap1, ap2;

  fstringp = start_estring();
  va_start(ap1, fmt);
  va_start(ap2, fmt);
  vsprintf_estring(fstringp, fmt, ap1, ap2);
  va_end(ap2);
  va_end(ap1);
  formatted = finish_estring(fstringp);

/* print current line */
  if(ifp_line==1){
    start = buf_start;
    column = buf_ptr - buf_start;
  } else {
    column = 1;
    for(start=buf_ptr; start[-1] != '\n'; --start)
      ++column;
  }
  estringp = start_estring();
  while((ch=*start )){
    if(ch == '\n')
      break;
    append_estring(estringp, ch);
    start++;
  }
  echo = finish_estring(estringp);
  lstringp = start_estring();
  while(column-- > 2)
    append_estring(lstringp, ' ');
  append_estring(lstringp, '^');
  locator = finish_estring(lstringp);

  parser_error(ifp_line, formatted, echo, locator);

  free_estring(fstringp);
  free_estring(estringp);
  free_estring(lstringp);

  exit(77);
  return 0;
}
