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
%a 10000
%e  2000
%o 10000
%k  2000
%n  2000
%p 25000
%{

#define YYLMAX 4096

#include <stdlib.h>

long atop(buf)
char*buf;
{
  return strtoul (buf, 0, 16);
}

static int setup(char *str);

static YY_BUFFER_STATE buf = 0;
static cur_arg = 0;
static char ** cur_argv = 0;
static cur_argc = 0;
static int eof = 0;


#define yywrap() next_string()

int next_string()
{
  if(cur_arg == (cur_argc - 1)){
    if (!eof) {
      eof = 1;
      setup("\n");
      return 0;
    }
    return 1;
  }
  ++cur_arg;
  setup(cur_argv[cur_arg]);
  return 0;
}


int    use_whole_set       = 0;
int    DI_parse_error_code = 0;
char   selection_parse_error[255];

struct lexState {
 int   cur_arg ;
 char** cur_argv ;
 int   cur_argc ;
 int   use_whole_set       ;
 int   DI_parse_error_code ;
};

lex_state_save(s)
  struct lexState *s;
{
  s->cur_arg = cur_arg;
  s->cur_argv = cur_argv;
  s->cur_argc = cur_argc;
  s->use_whole_set = use_whole_set;
  s->DI_parse_error_code = DI_parse_error_code;
}

lex_state_restore(s)
  struct lexState *s;
{
  cur_arg = s->cur_arg;
  cur_argv = s->cur_argv;
  cur_argc = s->cur_argc;
  use_whole_set = s->use_whole_set;
  DI_parse_error_code = s->DI_parse_error_code;
}

void output_filename();
void output_reference();
void output_NT_filename();
void output_reference_loff();
void output_ast();
void output_cell();
void output_scope();
void output_symbol();
void output_template();
void output_member_ptr();
void output_field();
void output_semtype();
void output_function();
void output_obj();
void output_set_element();
void output_ind();
void output_range();
void output_roots();
void output_array();
void append_array();
void output_ddKind();
static void setup_var();
void output_itag();
void output_error_handler();
void output_xrefSymbol();
void output_dataCell();
void output_pointer();
void output_symbolPtr();
void output_local();
void output_string();
void parse_selection(argc, argv)
int argc; char**argv;
{
    setup_var(argc, argv);
    yylex();
}

void output_array()
{
  use_whole_set = 0;
  append_array();
}

void output_range(start, end)
  int start, end;
{
  int ind;
  if(start > end){
    int tmp = start;
    start = end;
    end = tmp;
  }
  for(ind = start > 0 ? start : 1; ind <= end; ++ind)
     output_ind(ind);
}

/* This RANGE_STANDALONE code should be removed.
 * Sturner not doing so 020503 because a release is imminent.
 */
#ifdef RANGE_STANDALONE
void output_group(sym)
  char*sym;
{
  printf("{%s}", sym);
}
void output_filename(sym)
  char*sym;
{
  printf("{%s}", sym);
}
void output_NT_filename(sym)
  char*sym;
{
  printf("{%s}", sym);
}
void output_reference(off,sym)
  int off; char*sym;
{
  printf("{ref:%d:%s}", off, sym);
}
void output_reference_loff(line, line_offset, sym)
  int line, line_offset; char *sym;
{
  printf("{ref:%d,%d:%s}", line, line_offset, sym);
}
void output_ast(off,sym)
  int off; char*sym;
{
  printf("{ast:%d!%s}", off, sym);
}

void output_cell(off,sym)
  int off; char*sym;
{
  printf("{cell:%d?%s}", off, sym);
}

void output_dataCell(offset,pmod,off)
  int offset; int pmod; int off;
{
  printf("{cell:%d?%d/%d}", offset, pmod, off);
}

void output_scope(sym)
  char*sym;
{
  printf("{{%s}}", sym);
}
void output_symbol(sym)
  char*sym;
{
  printf("[%s]", sym);
}
void output_template(sym)
  char*sym;
{
  printf("template [%s]", sym);
}
void output_member_ptr(sym)
  char*sym;
{
  printf("memptr [%s]", sym);
}
void output_semtype(sym)
  char*sym;
{
  printf("[/t/%s]", sym);
}
void output_field(sym)
  char*sym;
{
  printf("[/f/%s]", sym);
}

void output_function(sym)
  char*sym;
{
  printf("[/f/%s]",sym);
}

void output_obj(ind)
  int ind;
{
  printf("(%d)", ind);
}

void output_set_element(ind, el)
  int ind, el;
{
  printf("(%d/%d)", ind, el);
}
void output_itag(ind)
  int ind;
{
  printf("@%d", ind);
}
void output_error_handler(ind)
  int ind;
{
  printf("error(%d) ", ind);
}
void output_local(str)
  char* str;
{
  printf("loc@'%s'", str);
}
void output_string(str)
  char* str;
{
  printf("'%s'", str);
}
void output_xrefSymbol(i,j)
  int i,j;
{
  printf("%d/%d", i,j);
}
void output_pointer(ind)
  int ind;
{
  printf("@%x", ind);
}
void output_symbolPtr(ind)
  int ind;
{
  printf("S%x", ind);
}
void append_array()
{
}
void output_ind(ind)
  int ind;
{
  printf("*%d", ind);
}
void output_ddKind(text, len, glob)
  char * text; int glob;
{
  printf("%s%d%d",text,len,glob);
}
void output_roots()
{
  printf("RR");
}

main(argc, argv)
  int argc; char**argv;
{
  if(argc >1){
    parse_selection(argc-1, argv+1);
    printf("\n");
  } else {
    char buf[5000];
    char*p;
    for(;;) {
      printf("range>");
      p = gets(buf);
      if(!((p && *p)))
	break;
      parse_selection(1, &p);
      printf("\n");
    } 
  }
  return 0;
}
#endif

#include <memory.h>
#define MBUFLEN 4100
static char fun_buf[MBUFLEN+9];
static int fun_len;
static int fun_level;

void fun_start(t,l)
char* t; int l;
{
  fun_len = l;
  memcpy(fun_buf,t,l);
}
void fun_append();

void fun_finish(t,l)
char* t; int l;
{
  if(l>0){
     memcpy(fun_buf+fun_len,t,l);
     fun_len += l;
     fun_buf[fun_len] = '\0';
  }
  if(use_whole_set)
    output_array();
  output_function(fun_buf); 
}

%}

I ([0-9][0-9]*)
C ([_#@a-zA-Z\$][_a-zA-Z0-9]*)
NON_NL ([^\n\r])
S ("::"|".")
SPATH (\/[^/ \t\n\r@\\]+([^/\t\n\r@\\:]+[^/ \t\n\r@\\:])?)+
PATH  {SPATH}/[ \t\n\r]

%s NESTED_FUNCTION FUNCTION DEFAULT ETAG

%%
  BEGIN DEFAULT;

<DEFAULT>(loc|loca|local)@[^\t\n\r/]+"/"{C}		{ 
	char *nm = yytext+3;
	while(*nm != '@') nm++;
	if(use_whole_set)
		output_array();
	use_whole_set = 0; 
	output_local(nm+1);
}

<DEFAULT>(funct|functi|functio|function)@{NON_NL}*	{
	char *nm = yytext+5;
	while(*nm != '@') nm++;
	if(use_whole_set)
		output_array();
	output_function(nm+1); 
}

<DEFAULT>field@{NON_NL}*				{
	if(use_whole_set)
		output_array();
	output_field(yytext+6); 
}

<ETAG>{NON_NL}*						{
	if(use_whole_set)
		output_array();
	output_symbol(yytext);
	BEGIN DEFAULT;
}

<DEFAULT>"0x"[0-9a-f]+					{
	long p = atop(yytext);
	output_pointer((int)p);
}

<DEFAULT>"0X"[0-9A-F]+					{
	long p = atop(yytext);
	output_symbolPtr((int)p);
}

<DEFAULT>"-i"(gnore)?					{
	output_error_handler(1);
}

<DEFAULT>"-e"(rror)?					{
	output_error_handler(0);
}

<DEFAULT>"-w"(arn)?(ing)?				{
	output_error_handler(-1);
}

<DEFAULT>([a-z]+)"@"/" "				{
	output_ddKind(yytext, yyleng, 1);
	BEGIN ETAG;
}

<DEFAULT>([a-z]+)"@"					{
	output_ddKind(yytext, yyleng, 0);
	BEGIN ETAG;
}

<DEFAULT>"("([a-z]+)")"					{
	output_ddKind(yytext, yyleng, 0);
}

<DEFAULT>itag@"-"?{I}					{
	output_itag(atoi(yytext+5));
}

<DEFAULT>{I}"?"{I}"/"{I}				{
	 int i,j,k;
	 sscanf(yytext, "%d?%d/%d", &i, &j, &k); output_dataCell(i,j,k);
}

<DEFAULT>{I}"/"{I}					{
	int i,j;
	sscanf(yytext, "%d/%d", &i, &j); output_xrefSymbol(i,j);
}

<DEFAULT>group"@"[^ \t\n\r!()\[\]/]*			{
	if(use_whole_set)
		output_array();
	use_whole_set = 1; 
	output_group(yytext+6,1);
}

<DEFAULT>\"([^\t\n\r"]|(\\.))*\"			{
	if(use_whole_set)
		output_array();
	use_whole_set = 0; 
	output_string(yytext);
}

<DEFAULT>{SPATH}"@"					{
	output_scope(yytext);
}

<DEFAULT>{I}						{
	use_whole_set = 0; output_ind(atoi(yytext));
}

<DEFAULT>{I}"-"{I}					{
	int start, end; 
	use_whole_set = 0;
	sscanf(yytext,"%d-%d",&start, &end);
	output_range(start, end);
}

<DEFAULT>{I}".."{I}					{
	int start, end; 
	use_whole_set = 0;
	sscanf(yytext,"%d..%d",&start, &end);
	output_range(start, end);
}

<DEFAULT>_DI_{I}					{
	if(use_whole_set)
		output_array();
	use_whole_set = 1;
	output_obj(atoi(yytext+4)); 
}

<DEFAULT>_DI_{I}"/"{I}					{
	int i1,i2;
	sscanf(yytext, "_DI_%d/%d", &i1, &i2);
	if(use_whole_set)
		output_array();
	use_whole_set = 0; 
	output_set_element(i1, i2);
}

<DEFAULT>"/"						{
	if(use_whole_set)
		output_array();
	output_roots(); 
}

<DEFAULT>{PATH}						{
	if(use_whole_set)
		output_array();
	output_filename(yytext); 
}

<DEFAULT>[a-zA-Z]":"{PATH}				{
	if(use_whole_set)
		output_array();
	output_NT_filename(yytext); 
}

<DEFAULT>{I}":"{PATH}					{
	char *ptr;
	int off = strtol(yytext, &ptr, 10);
	if(use_whole_set)
		output_array();
	output_reference(off, ptr+1); 
}

<DEFAULT>{I}","{I}":"{PATH}				{
	char *p1, *p2; 
	int line = strtol(yytext, &p1, 10);
	int line_offset = strtol(p1 + 1, &p2, 10);
	if (use_whole_set)
		output_array();
	output_reference_loff(line, line_offset, p2 + 1);
}

<DEFAULT>{I}"!"{PATH}					{ 
	char *ptr;
	int off = strtol(yytext, &ptr, 10);
	if(use_whole_set)
		output_array();
	output_ast(off, ptr+1); 
}

<DEFAULT>{I}"?"{PATH}					{
	char *ptr;
	int off = strtol(yytext, &ptr, 10);
	if(use_whole_set)
		output_array();
	output_cell(off, ptr + 1); 
}

<DEFAULT>"#"[^ \t\n\r!()\[\]/]*				{
	if(use_whole_set)
		output_array();
	use_whole_set = 1;
	output_group(yytext+1); /* remove `#` */ 
}

<NESTED_FUNCTION,FUNCTION>"("				{
	fun_level++;
	fun_append(yytext,1);
	BEGIN NESTED_FUNCTION;
}

<NESTED_FUNCTION>")"					{
	fun_level--;
	fun_append(yytext,1);
	if(fun_level==1)
		BEGIN FUNCTION;
}

<FUNCTION>")"						{
	fun_finish(yytext,yyleng);
	BEGIN DEFAULT;
}

<FUNCTION>")"/[ ]*"const"[_a-zAA-Z0-1]			{
	fun_finish(yytext,yyleng);
	BEGIN DEFAULT;
}

<FUNCTION>")"[ ]*"const"       				{
	fun_finish(yytext,yyleng);
	BEGIN DEFAULT;
}

<NESTED_FUNCTION,FUNCTION>[\t\n\r]      		{
	fun_finish("",0);
	BEGIN DEFAULT;
}

<NESTED_FUNCTION,FUNCTION>[^()\n\r\t]+       		{
	fun_append(yytext, yyleng);
}

<DEFAULT>{C}"("            				{
	fun_start(yytext,yyleng); fun_level = 1;
	BEGIN FUNCTION;
}

<DEFAULT>({C}{S})+"~"?{C}"("   				{
	fun_start(yytext,yyleng); fun_level = 1;
	BEGIN FUNCTION;
}

<DEFAULT>({C}{S})+"operator()("				{
	fun_start(yytext,yyleng); fun_level = 1;
	BEGIN FUNCTION;
}

<DEFAULT>({C}{S})*"operator "[^()]+"("			{
	fun_start(yytext,yyleng); fun_level = 1;
	BEGIN FUNCTION;
}

<DEFAULT>({C}{S})*operator[^()_a-zA-Z0-9][^()]*"("	{
	fun_start(yytext,yyleng); fun_level = 1;
	BEGIN FUNCTION;
}

<DEFAULT>{C}						{
	if(use_whole_set)
		output_array();
	output_symbol(yytext); 
}

<DEFAULT>{C}({S}{C})*[*&]+				{
	if(use_whole_set)
		output_array();
	output_semtype(yytext); 
}

<DEFAULT>{C}({S}{C})+					{
	if(use_whole_set)
		output_array();
	output_field(yytext); 
}

[ \t\n\r,;\[\]{}]+					{}

.							{
	if(strlen(yytext) > 250)
		yytext[250] = 0;
	sprintf(selection_parse_error, "Unrecognized token: '%s'\n", yytext); DI_parse_error_code = -1;
	BEGIN DEFAULT;
}

%%

static int setup(str)
  char*str;
{
  /* XXX: should not have to make third extra copy to add newline;
     required presence of newline probably bug in pattern specification */
  int len = strlen(str);
  char buffer[1024];

  char *tmp_str = buffer;

  if ((len + 2) > sizeof(buffer)) {
    tmp_str = malloc(len + 2);
  }
  strcpy(tmp_str, str);
  tmp_str[len] = '\n';
  tmp_str[len + 1] = 0;

  yy_delete_buffer(buf);
  buf = yy_scan_string(tmp_str);

  if (tmp_str != &buffer) {
    free(tmp_str);
  }
  return 0;
}

static void setup_var(argc, argv)
  int argc; char**argv;
{
  cur_arg = 0;
  eof = 0;
  cur_argc = argc;
  cur_argv = argv;
  setup(argv[0]);
}

void fun_append(t,l)
char*t; int l;
{
 if(l==1)
  fun_buf[fun_len++] = *t;
 else {
  if((fun_len+l)> MBUFLEN){
    BEGIN DEFAULT;
    return;
  }
  memcpy(fun_buf+fun_len,t,l);
  fun_len += l;
 }
}

