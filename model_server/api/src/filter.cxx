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
#include "attribute.h"
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stringstream.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <sstream>
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include "genWild.h"
#include "genArr.h"
#include "objArr.h"
#include "RegLex.h"
#include "machdep.h"
#include "charstream.h"
#include "genStringPlus.h"
#include "externAttribute.h"
#include "Interpreter.h"
#include "genError.h"
#include "msg.h"

extern Interpreter*  GetActiveInterpreter();

genArr(char);
genArr(int);

extern int cli_parse_function_args(const symbolPtr& sym, const char** args, genArrOf(int)* indexArray);
static ocharstream * error_buffer;
static int err_code = 0;
#define expr_error (*error_buffer)

extern "C" void cli_error_msg(const char *msg)
{
  expr_error << msg;
  err_code++;
}

extern "C" char* cli_error_use()
{
  expr_error << '\0'; 
  return expr_error.ptr();
}

extern "C" void cli_error_reset()
{
  err_code = 0;
  if(!error_buffer)
    error_buffer = new ocharstream;
  expr_error.reset();
}

extern "C" expr*api_parse_expression_intern(char*string);

extern "C" expr*api_parse_expression(const char*string)
{
  cli_error_reset();
  genString tmp_str(string);
  expr* eee = api_parse_expression_intern ((char *)tmp_str);
  const char* err_msg = cli_error_use(); // always valid pointer
  if(!eee && !err_code){
    err_msg = "syntax error";
    err_code = 1;
  }

  if(err_msg[0]){
    Interpreter* i = GetActiveInterpreter();
    if(eee){
      delete eee;
      eee = NULL;
    }
    if(i){
      ostream&os = i->GetOutputStream();
      os << err_msg << endl;
    }
  }
  cli_error_reset();
  return eee;
}

bool expr::is_tree() const {return false;}
int expr::nil_value() const {return 0;}
enum EXP_STYLE {REGEXP, WILD};

struct attrExpr : public expr
{
  symbolAttribute * attr;

  attrExpr(symbolAttribute*a) : expr("AT"), attr(a) {setType();}
  int value(const symbolPtr& sym) 
     { return (int) attr->test((symbolPtr&)sym);}
  virtual void print(ostream& = cout, int level = 0) const;
  virtual void setType();
};

void attrExpr::print(ostream&str, int) const
{
  str << attr->get_name();
}

void attrExpr::setType()
{
  value_type = attr->is_bool() ? INT_TYPE : STRING_TYPE ;
}

struct unaryExpr : public expr
{
  expr * op;
  unaryExpr(expr*o, const char*nm) : expr(nm), op(o) {setType();}
  virtual ~unaryExpr();
  void print(ostream& = cout, int level = 0) const;
};

unaryExpr::~unaryExpr(void)
{
  delete op;
}

void unaryExpr::print(ostream&str, int) const
{
  str << '(' << (char*)ename;
  op->print(str);
  str << ')';
}

struct binaryExpr : public expr
{
  expr * op1;
  expr * op2;
  binaryExpr(expr *o1, expr*o2, const char*nm) : expr(nm), op1(o1), op2(o2) {setType();}
  virtual ~binaryExpr() {delete op1; delete op2;}
  void print(ostream& = cout, int level = 0) const;
};

void binaryExpr::print(ostream& str, int) const
{
  str << '(';
  if (op1)
  op1->print(str);
  str << ' ' << (char*)ename << ' ';
  op2->print(str);
  str << ')';
}

struct notExpr : public unaryExpr
{
  notExpr(expr*o) : unaryExpr(o, "!") {setType();}
  int value(const symbolPtr& sym);
  int nil_value() const;
  virtual void setType() {value_type = INT_TYPE;}
};

int notExpr::value(const symbolPtr& sym) 
{ 
  int res = sym.isnull() ? op->nil_value() : op->boolValue(sym);
  return ! res;
}
int notExpr::nil_value()  const
{ 
  int res = op->nil_value();
  return ! res;
}

struct uminusExpr : public unaryExpr
{
  uminusExpr(expr*o) : unaryExpr(o, "-") {setType();}
  int value(const symbolPtr& sym) { return -(op->value(sym));}
  virtual void setType() {value_type = INT_TYPE;}
};

struct compExpr : public unaryExpr
{
  compExpr(expr*o) : unaryExpr(o, "~") {setType();}
  int value(const symbolPtr& sym) { return ~(op->value(sym));}
  virtual void setType() {value_type = INT_TYPE;}
};

#define GEN_INT_EXPR(name, op) struct name : public binaryExpr \
{                                                          \
  name(expr*o1, expr*o2) : binaryExpr(o1, o2, quote(op)) {setType();}\
  int value(const symbolPtr& sym) ;                        \
  virtual void setType();                                   \
};                                                         \
int name::value(const symbolPtr& sym)                      \
{                                                          \
  return (op1->value(sym)) op (op2->value(sym));               \
}                                                          

#define GEN_BOOL_EXPR(name, op) struct name : public binaryExpr \
{                                                          \
  name(expr*o1, expr*o2) : binaryExpr(o1, o2, quote(op)) {setType();}\
  int value(const symbolPtr& sym) ;                        \
  int nil_value() const;                        \
  virtual void setType();                                   \
};                                                         \
int name::nil_value() const                                \
{                                                          \
  return (op1->nil_value()) op (op2->nil_value());        \
}                                                          \
int name::value(const symbolPtr& sym)                      \
{                                                          \
  return (op1->boolValue(sym)) op (op2->boolValue(sym));   \
}

GEN_BOOL_EXPR(andExpr, &&)
void andExpr::setType() {value_type = INT_TYPE;}

GEN_BOOL_EXPR(orExpr, ||)
void orExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(eqExpr, ==)
void eqExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(neExpr, !=)
void neExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(geExpr, >=)
void geExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(gtExpr, >)
void gtExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(leExpr, <=)
void leExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(ltExpr, <)
void ltExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(plusExpr, +)
void plusExpr::setType() 
{
    if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == INT_TYPE)) {
	value_type = STRING_TYPE;
    } else {
	if ((op1->valueType() == INT_TYPE) && (op2->valueType() == INT_TYPE)) {
	    value_type = INT_TYPE;
	} else {
	    value_type = NO_TYPE;
	}
    }
}

//GEN_EXPR(minusExpr, -)
//void minusExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(multExpr, *)
void multExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(divExpr, /)
void divExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(intDivExpr, %)
void intDivExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(lshExpr, <<)
void lshExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(rshExpr, >>)
void rshExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(bitAndExpr, &)
void bitAndExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(bitOrExpr, |)
void bitOrExpr::setType() {value_type = INT_TYPE;}

GEN_INT_EXPR(bitXorExpr, ^)
void bitXorExpr::setType() {value_type = INT_TYPE;}

struct minusExpr : public binaryExpr 
{
  minusExpr(expr*o1, expr*o2) : binaryExpr(o1, o2, "-") {setType();}
  int value(const symbolPtr& sym) ;
  void setType();
};

int minusExpr::value(const symbolPtr& sym) 
{ 
  int result = 0;
  if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == STRING_TYPE)) {
      result = ((char*)op1->value(sym) - (char*)op2->value(sym));
  } else {
      result = (op1->value(sym) - op2->value(sym));
  }
  return result;
}

void minusExpr::setType() {value_type = INT_TYPE;}


struct strEqExpr : public binaryExpr 
{
  strEqExpr(expr*o1, expr*o2) : binaryExpr(o1, o2, "==") {setType();}
  int value(const symbolPtr& sym) ;
  void setType();
};

int strEqExpr::value(const symbolPtr& sym) 
{ 
    return (strcmp((char*)op1->value(sym),(char*)op2->value(sym)) == 0);
}

void strEqExpr::setType() {value_type = INT_TYPE;}

struct strNeExpr : public binaryExpr 
{
  strNeExpr(expr*o1, expr*o2) : binaryExpr(o1, o2, "!=") {setType();}
  int value(const symbolPtr& sym) ;
  void setType();
};

int strNeExpr::value(const symbolPtr& sym) 
{ 
    return (strcmp((char*)op1->value(sym),(char*)op2->value(sym)) != 0);
}

void strNeExpr::setType() {value_type = INT_TYPE;}


struct strPlusExpr : public binaryExpr 
{
  strPlusExpr(expr*o1, expr*o2) : binaryExpr(o1, o2, "+") {setType();}
  int value(const symbolPtr& sym) ;
  genArrOf(char)  val;
  void setType();
};

int strPlusExpr::value(const symbolPtr& sym) 
{ 
    char* s1 = (char*)op1->value(sym);
    char* s2 = (char*)op2->value(sym);
    val.reset();
    val.grow( strlen(s1) + strlen(s2) );
    strcpy(val[0], s1);
    strcpy(val[strlen(s1)], s2);
    return (int)val[0];
}

void strPlusExpr::setType() {value_type = STRING_TYPE;}


struct ifExpr : public expr 
{
  ifExpr(expr*o1, expr*o2, expr*o3) : expr("IF"), op1(o1), op2(o2), op3(o3) {setType();}
  virtual ~ifExpr() {delete op1; delete op2; delete op3;}
  int value(const symbolPtr& sym);
  void print(ostream& = cout, int level = 0) const;
  void setType();
  expr* op1;
  expr* op2;
  expr* op3;  
};

void ifExpr::print(ostream& os, int level) const
{
    os << "(";
    op1->print(os);
    os << " ? ";
    op2->print(os);
    os << " : ";
    op3->print(os);
    os << ")";
}

int ifExpr::value(const symbolPtr& sym) 
{ 
  return (op1->boolValue(sym) ? op2->value(sym) : op3->value(sym));
}

void ifExpr::setType() 
{
    if (op2->valueType() == op3->valueType()) {
	value_type = op2->valueType();
    } else {
	value_type = NO_TYPE;
    }
}

struct arrRefExpr : public expr 
{
  arrRefExpr(expr*o1, expr*o2) : expr("arr_ref"), op1(o1), op2(o2) {setType();}
  virtual ~arrRefExpr() {delete op1; delete op2;}
  int value(const symbolPtr& sym);
  void print(ostream& = cout, int level = 0) const;
  void setType();
  expr* op1;
  expr* op2;
};

void arrRefExpr::print(ostream& os, int level) const
{
    op1->print(os);
    os << "[";
    op2->print(os);
    os << "]";
}

int arrRefExpr::value(const symbolPtr& sym) 
{ 
  //for now we assume op1 is a string
  int index = op2->value(sym);
  char* string = (char*)(op1->value(sym));
  int result = 0;
    if (index < 0) {
	result = (int) (string[index + strlen(string)]);
    } else {
	result = (int) (string[index]);
    }
  return result;
}

void arrRefExpr::setType() 
{
    if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == INT_TYPE)) {
	value_type = INT_TYPE;
    } else {
	value_type = NO_TYPE;
    }
}

struct arrRangeExpr : public expr 
{
  arrRangeExpr(expr*o1, expr*o2, expr*o3) : expr("arr_range"), op1(o1), op2(o2), op3(o3) {setType();}
  virtual ~arrRangeExpr() {delete op1; delete op2; delete op3;}
  int value(const symbolPtr& sym);
  void print(ostream& = cout, int level = 0) const;
  void setType();
  expr* op1;
  expr* op2;
  expr* op3;
  genArrOf(char) val;
};

void arrRangeExpr::print(ostream& os, int level) const
{
    op1->print(os);
    os << "[";
    op2->print(os);
    os << ",";
    op3->print(os);
    os << "]";
}

int arrRangeExpr::value(const symbolPtr& sym) 
{ 
  //for now we assume op1 is a string
  int index1 = op2->value(sym);
  int index2 = op3->value(sym);
  char* string = (char*)(op1->value(sym));

  if (index1 < 0) {
      index1 = index1 + strlen(string);
  }
  if (index2 < 0) {
      index2 = index2 + strlen(string);
  }

  // make sure index1 is smaller that index2
  if (index2 < index1) {
      int temp = index2;
      index2 = index1;
      index1 = temp;
  }

  int len = index2 - index1 + 1;
  val.reset();
  val.grow(len+1);
  if (len > 0) {
      strncpy(val[0], string + index1,len);
  }
  strcpy(val[len],"\0");
  int result = (int)"\0";
  if (len>0) {
      result = (int)val[0];
  }
  return result;
}

void arrRangeExpr::setType() 
{
    if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == INT_TYPE) && (op3->valueType() == INT_TYPE)) {
	value_type = STRING_TYPE;
    } else {
	value_type = NO_TYPE;
    }
}


struct stringSubExpr : public expr 
{
  stringSubExpr(expr*o1, expr*o2, expr*o3, EXP_STYLE style) : expr("string_subexpression"), op1(o1), op2(o2), op3(o3), old_exp(0), _style(style), reg(NULL), wreg(NULL)
     {setType();}
  virtual ~stringSubExpr() {delete op1; delete op2; delete op3; delete reg; delete wreg;}
  int value(const symbolPtr& sym);
  void print(ostream& = cout, int level = 0) const;
  void setType();
  char *old_exp;
  Regexp* reg;
  WildRegexp* wreg;
  expr* op1;
  expr* op2;
  expr* op3;
  genArrOf(char) val;
  EXP_STYLE _style;
};

void stringSubExpr::print(ostream& os, int level) const
{
    op1->print(os);
    os << "[";
    op2->print(os);
    if (op3) {
	os << ",";
	op3->print(os);
    }
    os << "]";
}


int stringSubExpr::value(const symbolPtr& sym) 
{ 
    char* input_pattern = (char*)op2->value(sym);
    int input_len = strlen(input_pattern);
    char* output_pattern = op3 ? (char*)op3->value(sym) : NULL;

    char* realText = (char*)op1->value(sym);

    char* text = new char[strlen(realText)+1];
    strcpy(text,realText);

    bool is_wild = (_style == WILD);
    if (!old_exp || (strcmp(old_exp, input_pattern) != 0)) {
	// Need to make a new expression, copy pattern into old_exp
	if (old_exp) delete [] old_exp;
	old_exp = new char[input_len+1];
	strcpy (old_exp, input_pattern);
	if (is_wild) {
	    if (wreg) delete wreg;
	    wreg = new WildRegexp(old_exp, COMPLICATE_LEVEL_1);
	} else {
	    if (reg) delete reg;
	    reg = new Regexp(old_exp);
	}
    }

    lexToken tokens[20];    
    int num_subs = matchRegSubs(text, strlen(text), (is_wild ? wreg:reg), tokens, 20);
    
    ostringstream os;
    
    if (output_pattern) {
	//parse output pattern
	bool in_parens = false;    
	int len = strlen(output_pattern);
	for (int index=0; index < len; index++) {
	    switch (output_pattern[index]) {
	      case '\\' :
		if (!in_parens) {
		    index ++;
		    os << output_pattern[index];
		}
		break;
	      case '(':
		if (!in_parens) {
		    in_parens = true;
		    int sub =  OSapi_atoi(&output_pattern[index+1]);
		    if ((sub > 0) && (sub <= num_subs)) {
			os << tokens[sub].getString();
		}
		}
		break;	    
	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7':
	      case '8':
	      case '9':
		if (!in_parens) {
		    os << output_pattern[index];
		}
		break;
	      case ')':
		if (!in_parens) {
		    os << ")";
		} else {
		    in_parens = false;
		}
		break;
	      default:
		if (!in_parens) {
		    os << output_pattern[index];
		}
		break;
	    }
	}	
    } else {
	if (num_subs > 1)  
	    os << tokens[1].getString();
	for (int sub = 2; sub<num_subs; sub++) {
	    os << "/" << tokens[sub].getString();
	}
    }
    os << '\0';

    delete [] text;

    string output_string = os.str();
    val.reset();
    val.grow(output_string.size() + 1);
    strcpy(val[0], output_string.c_str());

    return (int)val[0];
}

void stringSubExpr::setType() 
{
    if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == STRING_TYPE) && op3?(op3->valueType() == STRING_TYPE):1) {
	value_type = STRING_TYPE;
    } else {
	value_type = NO_TYPE;
    }
}

struct regexpExpr : public expr 
{
  regexpExpr(expr*o1, expr*o2, EXP_STYLE style) : expr("regexp"), op1(o1), op2(o2), old_exp(0), _style(style), reg(NULL), wreg(NULL)
     {setType();}
  virtual ~regexpExpr() {delete op1; delete op2; delete reg; delete wreg;}
  int value(const symbolPtr& sym);
  void print(ostream& = cout, int level = 0) const;
  void setType();
  expr* op1;
  expr* op2;
  Regexp* reg;
  WildRegexp* wreg;
  char *old_exp;
  EXP_STYLE _style;
};

void regexpExpr::print(ostream& os, int level) const
{
    if (_style == WILD) {	
	os << "wild(";
    } else {
	os << "regexp(";
    }
    op1->print(os);
    os << ",";
    op2->print(os);
    os << ")";
}

int regexpExpr::value(const symbolPtr& sym) 
{ 
    bool is_wild = (_style == WILD);
    char *input_pattern = (char *)op1->value(sym);
    int input_len = strlen(input_pattern);

    if (!old_exp || (strcmp(old_exp, input_pattern) != 0)) {
	// Need to make a new expression, copy pattern into old_exp
	if (old_exp) delete [] old_exp;
	old_exp = new char[input_len+1];
	strcpy (old_exp, input_pattern);
	if (is_wild) {
	    if (wreg) delete wreg;
	    wreg = new WildRegexp(old_exp);
	} else {
	    if (reg) delete reg;
	    reg = new Regexp(old_exp);
	    reg->Compile();  // Pre-compile the regular expression
	}
    }

    char* text = (char*)op2->value(sym);
    int len = strlen(text);
    if (is_wild)
	return (wreg->Match(text, len, 0) >= 0);
    else
	return (reg->MatchPrebuilt(text, len, 0) >= 0);
}

void regexpExpr::setType() 
{
    if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == STRING_TYPE)) {
	value_type = INT_TYPE;
    } else {
	value_type = NO_TYPE;
    }
}



struct argExpr : public expr 
{
  argExpr(expr*o1) : expr("arg"), op1(o1)  {setType();}
  virtual ~argExpr() {delete op1;}
  int value(const symbolPtr& sym);
  void print(ostream& = cout, int level = 0) const;
  void setType();
  expr* op1;
  genString val;
};

void argExpr::print(ostream& os, int level) const
{
    os << "arg(";
    op1->print(os);
    os << ")";
}

int argExpr::value(const symbolPtr& sym) 
{ 
    char* result = (char *)"";
    int argNum = op1->value(sym);
    if (argNum > 0) {
	const char* argString;
	genArrOf(int) indexArr;
	int num = cli_parse_function_args(sym, &argString, &indexArr);
	if (num >= argNum) { 
	  int i = 2*(argNum-1);
	  int offset = *(indexArr[i]);
	  int length = *(indexArr[i+1]);
	  val.put_value(argString+offset, length);
	  result = val;
	}
    }
    return (int)result;
}

void argExpr::setType() 
{
    if (op1->valueType() == INT_TYPE) {
	value_type = STRING_TYPE;
    } else {
	value_type = NO_TYPE;
    }
}



struct intExpr : public expr
{
  int val;
  intExpr(int v) : expr("int"), val(v) {setType();}
  int value(const symbolPtr& sym);
  virtual void print(ostream& = cout, int level = 0) const;
  void setType() {value_type = INT_TYPE;}
};

void intExpr::print(ostream&str, int) const
{
  str << val;
}


int intExpr::value(const symbolPtr& sym) { return val;}



struct stringExpr : public expr
{
  genString val;
  stringExpr(char*ptr) : expr(""), val(ptr) { setType();}
  stringExpr(char*ptr, int len) : expr("string") 
    {
      if(len)
	val.put_value(ptr, len);
      else if(ptr)
	val = "";
      setType();
    }
  int value(const symbolPtr& sym);
  virtual void print(ostream& = cout, int level = 0) const;
  void setType() {value_type = STRING_TYPE;}
};

void stringExpr::print(ostream&str, int) const
{
  str << '"' << (char*)val << '"';
}

int stringExpr::value(const symbolPtr& sym) 
{ 
  return (int) (char*)val;
}

const char* ATT_cname(symbolPtr&);

struct varExpr : public stringExpr
{
  varExpr(char*ptr, int len) : stringExpr(ptr,len) {}
  int value(const symbolPtr& sym);
};

int varExpr::value(const symbolPtr& sym) 
{ 
   Interpreter* i = GetActiveInterpreter();
   char const *res = Tcl_GetVar(i->interp, (char*) val, 0);
   return (int) res;
}

extern "C" {
typedef int (*FUNPTR)(int, int, int, int, int);
}

const int MAXARG = 5;

static objArr* builtin;

struct FUNC : public objDictionary
{
  FUNPTR funptr;
  int no_arg;
  exprType type;
  exprType arg_type[MAXARG];
  int get_no_arg() {return no_arg;}
  FUNC(const char* name, FUNPTR f, int n, exprType returnType, exprType* argType); 
};

FUNC::FUNC(const char* name, FUNPTR f, int n, exprType returnType, exprType* argType) : 
    objDictionary(name), funptr(f), no_arg(n), type(returnType)
{
    builtin->insert_last(this);
    for (int x = 0; x<MAXARG; x++) {
	arg_type[x]=argType[x];
    }
}

static char *mg_strlwr(char *str)
{
    static genString tmp;
    tmp = str;
    str = (char *)tmp;
    while(*str){
	*str = tolower(*str);
	str++;
    }
    return (char *)tmp;
}

static char *mg_strupr(char *str)
{
    static genString tmp;
    tmp = str;
    str = (char *)tmp;
    while(*str){
	*str = toupper(*str);
	str++;
    }
    return (char *)tmp;
}

static int mg_count(char const *str, int val)
{
  int cnt=0;
  int ch;
  while(ch=*(str++)) 
    if(ch==val)
      ++cnt;
  return cnt;
}

static size_t mg_strlen ( const char * _c ) {
    return strlen ( _c ) ;
}

static int mg_strcmp ( const char * _c , const char * _d ) {
    return strcmp ( _c , _d ) ;
}

static int mg_strncmp ( const char * _c , const char * _d, size_t _n ) {
    return strncmp ( _c , _d , _n ) ;
}

static char * mg_strchr ( char * _c , int _i ) {
    return ( char * ) strchr ( ( const char * ) _c , _i ) ;
}

static char * mg_strrchr ( char * _c , int _i ) {
    return ( char * ) strrchr ( ( const char * ) _c , _i ) ;
}

static char * mg_strpbrk ( char * _c , const char * _d ) {
    return ( char * ) strpbrk ( ( const char * ) _c , _d ) ;
}

static size_t mg_strspn ( const char * _c , const char * _d) {
    return strspn ( _c , _d );
}

static size_t mg_strcspn ( const char * _c , const char * _d) {
    return strcspn ( _c , _d );
}

static char * mg_strstr ( char * _c , const char * _d ) {
    return ( char * ) strstr ( ( const char * ) _c , _d ) ;
}

static char* mg_strnchr(char*str, int val, int ind)
{
  if(ind < 0)
    return 0;
  
  int cnt=0;

  for(int ch; (ch=*str); ++str) {
    if(ch==val){
      if(cnt==ind)
	return str;
      ++cnt;
    }
  }
  return NULL;
}
exprType typeArr[MAXARG];

#define add_func(name, func_name, args, rtype, arg1, arg2, arg3, arg4, arg5) \
typeArr[0] = arg1; \
typeArr[1] = arg2; \
typeArr[2] = arg3; \
typeArr[3] = arg4; \
typeArr[4] = arg5; \
new FUNC(quote(name), (FUNPTR)func_name, args, rtype, typeArr)

int builtinlist()
{    
  static int once = 0;
  if (!once) {
    builtin = new objArr();
    once = 1;
    add_func(strlen, mg_strlen, 1, INT_TYPE,      STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strlwr, mg_strlwr, 1, STRING_TYPE,   STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strupr, mg_strupr, 1, STRING_TYPE,   STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strcmp, mg_strcmp, 2, INT_TYPE,      STRING_TYPE, STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strncmp, mg_strncmp, 3, INT_TYPE,     STRING_TYPE, STRING_TYPE, INT_TYPE, NO_TYPE, NO_TYPE);
    add_func(strchr, mg_strchr, 2, STRING_TYPE,   STRING_TYPE, INT_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(count, mg_count, 2, INT_TYPE,  STRING_TYPE, INT_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strrchr, mg_strrchr, 2, STRING_TYPE,  STRING_TYPE, INT_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strnchr, mg_strnchr, 3, STRING_TYPE,  STRING_TYPE, INT_TYPE, INT_TYPE, NO_TYPE, NO_TYPE);
    add_func(strpbrk, mg_strpbrk, 2, STRING_TYPE,  STRING_TYPE, STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strspn, mg_strspn, 2, INT_TYPE,      STRING_TYPE, STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strcspn, mg_strcspn, 2, INT_TYPE,     STRING_TYPE, STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
    add_func(strstr, mg_strstr, 2, STRING_TYPE,   STRING_TYPE, STRING_TYPE, NO_TYPE, NO_TYPE, NO_TYPE);
  }
    return 0;
}

static int ggg = builtinlist();

typedef expr * exprPtr;

struct listExpr : public expr
{
  listExpr(void);
  ~listExpr(void);

  genArr(exprPtr) items;
  Tcl_DString  buf;

  int value(const symbolPtr& sym);
  int nil_value() const;
  virtual void print(ostream& = cout, int level = 0) const;
  void setType() 
    {
      value_type = STRING_TYPE;
    }
};

listExpr::listExpr(void)
    : expr("list")
{
  setType();
  Tcl_DStringInit(&buf);
}

listExpr::~listExpr(void)
{
  Tcl_DStringFree(&buf); 
  for(int i = 0; i < items.size(); i++)
    {
      expr **expr_pp = items[i];
      if ( expr_pp && *expr_pp )
	delete *expr_pp;
    }
}

void listExpr::print(ostream& os, int lev) const
{
   os << "list:" << endl; 
   lev++;
   int sz = items.size();
   for(int ii=0; ii<sz; ++ii)
     (*items[ii])->print(os, lev);
   os << flush;
}
int listExpr::nil_value() const
{
  symbolPtr nil_sym;
  return ((listExpr*)this) -> value(nil_sym);
}

int listExpr::value(const symbolPtr& sym)
{
  Initialize(command::value);

   Tcl_DStringSetLength(&buf, 0);

   int sz = items.size();
   genString valbuf;
   for(int ii=0; ii<sz; ++ii){
     expr* cur = *items[ii];
     int val = cur->value(sym);
     const char*pv = (const char*) val;
     if(cur->valueType() != STRING_TYPE){
       valbuf.printf("%d", val);
       pv = valbuf;
     }
     Tcl_DStringAppendElement(&buf, (char *)pv);
   }

   char* res = (char *)Tcl_DStringValue(&buf);
   return (int) res;
}

struct funcallExpr : public expr
{
  FUNC *fun;
  expr* args[MAXARG];
  funcallExpr(FUNC*ff, int no, expr*a1, expr*a2, expr*a3, expr*a4, expr*a5) : 
     expr("funcall"), fun(ff)
    {
      args[0] = a1;
      args[1] = a2;
      args[2] = a3;
      args[3] = a4;
      args[4] = a5;
      setType();
    }
  int value(const symbolPtr& sym);
  virtual void print(ostream& = cout, int level = 0) const;
  void setType() {value_type = fun?(fun->type):NO_TYPE;}
};

void funcallExpr::print(ostream&str, int) const
{
  str << fun->get_name() << '(';
  if(fun->no_arg > 0)
    args[0]->print(str);
  for(int ii=1; ii<fun->no_arg; ++ii){
    str << ',';
    args[ii]->print(str);
  }
  str << ')';
}


int funcallExpr::value(const symbolPtr& sym)
{
  int ret;
  switch(fun->no_arg){
  case 0:
    ret = (*fun->funptr)(0, 0, 0, 0, 0);
    break;
  case 1:
    ret = (*fun->funptr)(args[0]->value(sym), 0, 0, 0, 0);
    break;
  case 2:
    ret = (*fun->funptr)(args[0]->value(sym), args[1]->value(sym), 0, 0, 0);
    break;
  case 3:
    ret = (*fun->funptr)(args[0]->value(sym), args[1]->value(sym), args[2]->value(sym), 0, 0);
    break;
  case 4:
    ret = (*fun->funptr)(args[0]->value(sym), args[1]->value(sym), args[2]->value(sym), args[3]->value(sym), 0);
    break;
  case 5:
    ret = (*fun->funptr)(args[0]->value(sym), args[1]->value(sym), args[2]->value(sym), args[3]->value(sym), args[4]->value(sym));
    break;
  }
  if ((value_type == STRING_TYPE) && (ret == 0)) 
      ret = (int)"\0";
  return ret;
}

#define GEN_EXPR_FUNCT(name1, name2, symbol) extern "C" expr* name1 (expr*op1, expr*op2) \
{                                                                             \
    expr* result = NULL;                                                      \
    if (op1 && op2) {                                                         \
	if ((op1->valueType() == INT_TYPE) && (op2->valueType() == INT_TYPE)) {	 \
	    result = new name2 (op1, op2);                                    \
	} else {                                                              \
	    expr_error <<"String used in comparison/arithmatic operation: ";  \
	    op1->print(expr_error);                                           \
	    expr_error << " " << quote(symbol) << " ";                        \
	    op2->print(expr_error);                                           \
	    expr_error << '\n' ;                                              \
	}                                                                     \
    }                                                                         \
    if (result == NULL) {                                                     \
	delete op1;                                                           \
	delete op2;                                                           \
    }                                                                         \
    return result;			                                      \
}

//GEN_EXPR_FUNCT(binary_expr_AND, andExpr)
//GEN_EXPR_FUNCT(binary_expr_OR, orExpr)
//GEN_EXPR_FUNCT(binary_expr_EQ, eqExpr)
//GEN_EXPR_FUNCT(binary_expr_NE, neExpr)
GEN_EXPR_FUNCT(binary_expr_GE, geExpr,>=)
GEN_EXPR_FUNCT(binary_expr_GT, gtExpr,>)
GEN_EXPR_FUNCT(binary_expr_LE, leExpr,>=)
GEN_EXPR_FUNCT(binary_expr_LT, ltExpr,>)
//GEN_EXPR_FUNCT(binary_expr_PLUS, plusExpr)
//GEN_EXPR_FUNCT(binary_expr_MINUS, minusExpr)
GEN_EXPR_FUNCT(binary_expr_MULT, multExpr,*)
GEN_EXPR_FUNCT(binary_expr_DIV, divExpr,/)
GEN_EXPR_FUNCT(binary_expr_INT_DIV, intDivExpr,%)
GEN_EXPR_FUNCT(binary_expr_BIT_AND, bitAndExpr,&)
GEN_EXPR_FUNCT(binary_expr_BIT_OR, bitOrExpr,|)
GEN_EXPR_FUNCT(binary_expr_LSH, lshExpr, <<)
GEN_EXPR_FUNCT(binary_expr_RSH, rshExpr, >>)
GEN_EXPR_FUNCT(binary_expr_BIT_XOR, bitXorExpr, ^)

extern "C" listExpr* build_list(listExpr*list, expr*item)
{
  if(!item) {
    cli_error_msg("Bad list element\n");
    return NULL;
  }
  if(!list)
    list = new listExpr;
  list->items.append(&item);
  return list;
}

extern "C" expr* binary_expr_AND (expr*op1, expr*op2)
{                                    
  if (op1 && op2) {
    return new andExpr (op1, op2);
  } else {
    delete op1;
    delete op2;
  }
  return NULL;
}

extern "C" expr* binary_expr_OR (expr*op1, expr*op2)
{                                    
  if (op1 && op2) {
    return new orExpr (op1, op2);
  } else {
    delete op1;
    delete op2;
  }
  return NULL;
}

extern "C" expr* binary_expr_PLUS (expr*op1, expr*op2) 
{
    expr* result = NULL;
    if (op1 && op2) {
	if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == STRING_TYPE)) {
	    result = new strPlusExpr(op1, op2);
	} else
	    if ((op1->valueType() != NO_TYPE) && (op2->valueType() == INT_TYPE)) {
		result = new plusExpr(op1, op2);
	    } else {
		cli_error_msg("Cannot perform specified addition.\n");
		delete op1;
		delete op2;

	    }
	
  } else {
    delete op1;
    delete op2;
  }
  return result;
}

extern "C" expr* binary_expr_MINUS (expr*op1, expr*op2) 
{
    expr* result = NULL;
    if (op1 && op2 && (op1->valueType() == op2->valueType()) && (op2->valueType() != NO_TYPE)) {
	result = new minusExpr(op1, op2);
    } else {
	if (op1 && op2) {
	    cli_error_msg("Badly formed subtraction expression.\n");
	}
	delete op1;
	delete op2;
    }
    return result;
}

extern "C" expr* binary_expr_EQ (expr*op1, expr*op2) 
{
    expr* result = NULL;
    if (op1 && op2) {
	if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == STRING_TYPE)) {
	    result = new strEqExpr(op1, op2);
	} else
	    if ((op1->valueType() == INT_TYPE) && (op2->valueType() == INT_TYPE)) {
		result = new eqExpr(op1, op2);
	    } else {
		cli_error_msg("Cannot perform specified comparison.\n");
		delete op1;
		delete op2;

	    }
	
  } else {
    delete op1;
    delete op2;
  }
  return result;
}

extern "C" expr* binary_expr_NE (expr*op1, expr*op2) 
{
    expr* result = NULL;
    if (op1 && op2) {
	if ((op1->valueType() == STRING_TYPE) && (op2->valueType() == STRING_TYPE)) {
	    result = new strNeExpr(op1, op2);
	} else
	    if ((op1->valueType() == INT_TYPE) && (op2->valueType() == INT_TYPE)) {
		result = new neExpr(op1, op2);
	    } else {
		cli_error_msg("Cannot perform specified comparison.\n");
		delete op1;
		delete op2;

	    }
	
  } else {
    delete op1;
    delete op2;
  }
  return result;
}

extern "C" expr* unary_expr_NOT(expr*op)
{
  if (op && (op->valueType() != NO_TYPE)) {  
      return new notExpr(op);
  }
  return NULL;
}

extern "C" expr* unary_expr_MINUS(expr*op)
{
  if (op && (op->valueType() != NO_TYPE)) {  
      return new uminusExpr(op);
  }
  return NULL;
}

extern "C" expr* unary_expr_COMP(expr*op)
{
  if (op && (op->valueType() != NO_TYPE)) {  
    return new compExpr(op);
  }
  return NULL;
}

extern "C" expr* if_expr(expr*op1, expr*op2, expr*op3)
{
  if (op1 && op2 && op3 && 
      (op1->valueType() != NO_TYPE) && 
      (op2->valueType() != NO_TYPE) && 
      (op3->valueType() == op2->valueType())) {  
    return new ifExpr(op1, op2, op3);
  } else {
    expr_error << "Badly formed IF expression:  ";
    if(op1){
      op1->print(expr_error); 
      expr_error << " ? ";
      delete op1;
    }
    if(op2){
      op2->print(expr_error);
      expr_error << " : ";
      delete op2;
    }
    if(op3){
	op3->print(expr_error);
	delete op3;
    }
    expr_error << '\n' << '\0';
  }
  return NULL;
}

extern "C" expr* arr_ref(expr*op1, expr*op2)
{
    if (op1 && op2 && (op1->valueType() == STRING_TYPE)) {
	if (op2->valueType() == INT_TYPE)
	    return new arrRefExpr(op1, op2);
	if (op2->valueType() == STRING_TYPE)
	    return new stringSubExpr(op1, op2, NULL, WILD);
    } else {
      cli_error_msg("Badly formed array reference or string subexpression.\n");
      if(op1)
	delete op1;
      if(op2)
	delete op2;
    }
    return NULL;
}

extern "C" expr* arr_range(expr*op1, expr*op2, expr*op3)
{
    expr* result = NULL;

    if (op1 && op2 && op3 && (op1->valueType() == STRING_TYPE)) {
	if ((op2->valueType() == INT_TYPE) && (op3->valueType() == INT_TYPE)) {
	    result = new arrRangeExpr(op1, op2, op3);
	} else
	    if ((op2->valueType() == STRING_TYPE) && (op3->valueType() == STRING_TYPE)) {
		result = new stringSubExpr(op1, op2, op3, WILD);
	    }
    }

    if (result == NULL) {
      cli_error_msg("Badly formed array range or string subexpression.\n");
      if(op1)
	delete op1;
      if(op2)
	delete op2;
      if(op3)
	delete op3;
    }
    return result;
}

extern "C" expr* name_const(const char* name, int len)
{
  char *tempname = new char[len+1];
  for(int i=0; i<len; i++) {
    tempname[i] = name[i];
  }
  tempname[len] = 0;
  symbolAttribute* attr = symbolAttribute::get_by_name(tempname);

  expr* result = NULL;
  if(!attr) {
    //
    // Use of name rather than tempname is deliberate.
    //
    attr = externAttribute::find(name);
  }
  if (attr) {
    result = new attrExpr(attr);
  } else {
    cli_error_msg("Could not find attribute: ");
    cli_error_msg(tempname);
    cli_error_msg("\n");
  }
  delete tempname;
  return result;
}

extern "C" expr* integer_const(int val)
{
  return new intExpr(val);
}

extern "C" expr* string_const(char* ptr, int len)
{
  return new stringExpr(ptr, len);
}
extern "C" expr* var_ref(char* ptr, int len)
{
  return new varExpr(ptr, len);
}

#define NEQ(n) (strcmp(name, quote(n)) == 0)

extern "C" int cli_name_type(char*name, int len)
{
  int ntype = 0; // string
//  int ntype = 2; // attribute
  int ch = name[len];
   name[len] = '\0';

  if (NEQ(regexp))
    ntype = 1;
  else if (NEQ(match))
    ntype = 1;
  else if (NEQ(arg))
    ntype = 1;
  else if (obj_search_by_name(name, *builtin))
    ntype = 1;
  else if (symbolAttribute::get_by_name(name))
    ntype = 2;

  name[len] = ch;
  return ntype;
}
extern "C" expr* fun_call(char*name, int len, int no_arg, expr*arg1, expr* arg2, expr* arg3, expr* arg4, expr* arg5)
{
  // SPECIAL CASES:
  if ((strncmp(name,"match",len) == 0)    || (strncmp(name,"regexp",len) == 0)) {
      if (no_arg != 2 && no_arg !=1) {
	  int ch = name[len];
	  name[len] = '\0';
	  cli_error_msg("Wrong number of arguments for function: ");
	  cli_error_msg(name);
	  cli_error_msg("\n");
	  name[len] = ch;
	  return NULL;
      } else {
	  if ((no_arg == 1) && arg1 ) {
	      if ((arg1->valueType() == STRING_TYPE)) {
		  const char* nm = "name";
		  if (strncmp(name,"match",len) == 0) return new regexpExpr(arg1, name_const(nm,strlen(nm)), WILD);
		  if (strncmp(name,"regexp",len) == 0) return new regexpExpr(arg1, name_const(nm,strlen(nm)), REGEXP);
	      }
	  } else {
	      if ((no_arg == 2) && arg1 && arg2) {
		  if ((arg1->valueType() == STRING_TYPE) && (arg2->valueType() == STRING_TYPE)) {
		      if (strncmp(name,"match",len) == 0) return new regexpExpr(arg1,arg2,WILD);
		      if (strncmp(name,"regexp",len) == 0) return new regexpExpr(arg1,arg2,REGEXP);
		  }
	      }
	  }
	  if (arg1 && (arg2 || no_arg == 1)) {
	      int ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong type for argument(s) of function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	      return NULL;
	  }
      }
      return NULL;
  }
  
  if (strncmp(name,"arg",len) == 0) 
      if (no_arg != 1) {
	  cli_error_msg("Wrong number of arguments for function: arg\n");
	  return NULL;
      }
      else
	  if (arg1 && (arg1->valueType() == INT_TYPE)) {
	      return new argExpr(arg1);
	  } else {
	      if (arg1) 
		  cli_error_msg("Wrong type for argument of function: arg\n");
	      return NULL;
	  }
  

  
  // NORMAL CASES
  int ch = name[len];
  name[len] = '\0';

  FUNC * fun = (FUNC*) obj_search_by_name(name, *builtin);
  name[len] = ch;

  expr* result = NULL;

  // make sure function exists  and has right # of args
  bool test = (fun && (no_arg == fun->get_no_arg()));
  
  // make sure no args are void
  if (test) {
      switch (no_arg) {
	case 5:
	  test = test && arg5;
	  if (arg5)
	  if (arg5->valueType() != fun->arg_type[4]) {
	      test = false;
	      ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong type for argument #5 in function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	  }
	  //fallthrough
	case 4:
	  test = test && arg4;
	  if (arg4)
	  if (arg4->valueType() != fun->arg_type[3]) {
	      test = false;
	      ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong type for argument #4 in function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	  }
	  //fallthrough
	case 3:
	  test = test && arg3;
	  if (arg3)
	  if (arg3->valueType() != fun->arg_type[2]) {
	      test = false;
	      ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong type for argument #3 in function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	  }
	  //fallthrough
	case 2:
	  test = test && arg2;
	  if (arg2)
	  if (arg2->valueType() != fun->arg_type[1]) {
	      test = false;
	      ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong type for argument #2 in function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	  }
	  //fallthrough
	case 1:
	  test = test && arg1;
	  if (arg1)
	  if (arg1->valueType() != fun->arg_type[0]) {
	      test = false;
	      ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong type for argument #1 in function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	  }
	  //fallthrough
      }
  }
  if (test) {
      result = new funcallExpr(fun, no_arg, arg1, arg2, arg3, arg4, arg5);
  } else {
      if (fun==NULL) {
	  ch = name[len];
	  name[len] = '\0';
	  cli_error_msg("Could not find function: ");
	  cli_error_msg(name);
	  cli_error_msg("\n");
	  name[len] = ch;
      } else 
	  if (no_arg != fun->get_no_arg()) { 
	      ch = name[len];
	      name[len] = '\0';
	      cli_error_msg("Wrong number of arguments for function: ");
	      cli_error_msg(name);
	      cli_error_msg("\n");
	      name[len] = ch;
	  }
      delete arg1;
      delete arg2;
      delete arg3;
      delete arg4;
      delete arg5;
  }
  return result;
}


#define NIY(x) extern "C" expr*x () {printf("%s: %s\n", "not implemented", quote(x)); return 0;}

NIY(unary_expr);
//NIY(unary_expr_MINUS);
//NIY(if_expr);

NIY(binary_expr);
//NIY(arr_range);
//NIY(arr_ref);
//NIY(binary_expr_GE);
//NIY(binary_expr_LE);
//NIY(binary_expr_EQ);
//NIY(binary_expr_NE);
//NIY(binary_expr_GT);
//NIY(binary_expr_LT);


extern "C" symbolAttribute * test_expr()
{
    //const char * str = "mac | (fun & ! meth)";
    const char * str = "noregexp(\".*::.*\",name) || (strlen(real) > 23+(fun?9:20))";
    symbolAttribute*attr = 0;
    //const char*simple = "strlen(name) > 13+(fun?9:20)";
    const char*simple = "((name[-2] != 46) || ( (strlen(real) > 30) && nofunc(45 + 18 * (strlen(real + 4)) )))";
    expr* ee = api_parse_expression(simple);
    if(!ee)
      //return attr;
      msg(" could not parse $1\n") << simple << eom;
    if(ee) {
	//ee->print(cout);
	attr = new exprAttribute("bar", ee);
    }

    expr* exp = api_parse_expression(str);
    if(exp) {
      //exp->print(cout);
      attr = new exprAttribute("foo", exp);
    } else {
      msg("parse error: $1\n") << str << eom;
    }
    return attr;
} \

// tree expression for patterns

#define TREE2class(name) struct name : public binaryExpr { \
  name(expr*o1, expr*o2) : binaryExpr(o1,o2,quote(name)){value_type = INT_TYPE;}\
  int value(const symbolPtr& sym); \
  void setType() {value_type =  INT_TYPE;}\
  bool is_tree() const {return true;}\
}
#define TREE2fun(name,fun) extern "C" expr* fun (expr*op1, expr*op2) \
{  \
  struct name*res = NULL;   \
  if (op1 && op2) { \
    res =  new name (op1, op2); \
  } else { \
    delete op1; \
    delete op2; \
  } \
  return res;\
}
#define TREE2(name,fun) TREE2class(name);TREE2fun(name,fun)

#define TREE1class(name) struct name : public unaryExpr {	\
  name(expr*o1)							\
    : unaryExpr(o1,quote(name))					\
      {								\
	value_type = INT_TYPE;					\
      }								\
  virtual ~name(void)						\
    {								\
    }								\
  int value(const symbolPtr& sym);				\
  void setType()						\
    {								\
      value_type =  INT_TYPE;					\
    }								\
  bool is_tree() const						\
    {								\
      return true;						\
    }								\
}

#define TREE1fun(name,fun) extern "C" expr* fun (expr*op1)	\
{								\
  struct name*res = NULL;					\
    res =  new name (op1);					\
  return res;							\
}

#define TREE1(name,fun) TREE1class(name);TREE1fun(name,fun)


TREE2class(treeAnd1);

int treeAnd1::value(const symbolPtr& sym)
{
  if(sym.isnull())
    return op1->nil_value() && op2->nil_value();

  int ret = op1->boolValue(sym);
  if (ret){
    if(sym.isnull())
      ret = op2->nil_value();
    else
      ret = op2->boolValue(sym);
  }
  return ret;
}
struct treeAnd : public treeAnd1
{
  treeAnd(expr*e1, expr*e2) : treeAnd1(e1,e2) {}
  int nil_value() const;
};

int treeAnd::nil_value() const 
{
  return op1->nil_value() && op2->nil_value();
}

extern "C" expr*tree_AND(expr*e1, expr*e2)
{
  return new treeAnd(e1,e2);
}

TREE1class(treeAction1);

int treeAction1::value(const symbolPtr& sym)
{
  // abstract class
  return 1;   // action: side-effect only
}

struct treeAction : public treeAction1 
{
  treeAction(expr*o, bool is_cmd) 
    : treeAction1(o), command_flag(is_cmd) 
      {
      }
  virtual ~treeAction(void);
  bool command_flag;
  int value(const symbolPtr& sym);
  int nil_value() const;
};

treeAction::~treeAction(void)
{
}

int treeAction::nil_value() const
{
  symbolPtr sym;
  return ((treeAction*)this)->value(sym);
}

int treeAction::value(const symbolPtr& sym)
{
  Initialize(treeAction::value);
  
  char* buf = (char*) op->value(sym);

  Interpreter *curint = GetActiveInterpreter();
  int err = curint->EvalCmd(buf);

  genString valbuf;
  curint->GetResult(valbuf);
  
  if(err == TCL_ERROR){
    ostream&os = curint->GetOutputStream();
    os << (char*) valbuf << endl;
    //Error(ERR_FAIL);
    return 0;  // should not be here
  } else {
    buf = valbuf;
  }
  
  int res = 1;
  if(command_flag) {
    if(buf==NULL || buf[0] == '\0') // empty string
      res = 0;
    else if(buf[0]=='0' && buf[1] =='\0')   // int false
     res = 0;
  } 
  return res;
}

extern "C" expr* tree_action(expr*o) 
{
  return new treeAction(o, false);
}
extern "C" expr* build_command(expr*o)
{
  return new treeAction(o, true);
}

TREE2(treeRepeat, tree_iterate);

int treeRepeat::value(const symbolPtr& sym)
{
  int ret;
  for(;;){
    symbolPtr cur = sym;
    if(sym.isnull()){
      ret = op2->nil_value();
      break;
    } else {
      ret = op2->value(sym);
      if(ret)  // success
	break;
    }
    
    *(symbolPtr*) & sym = cur; // restore
    ret = op1->value(sym);     // next iteration
    if(ret == 0)
      break;
    if(sym == cur){            // did not advance: vector = 0
      ret = 0;
      break;
    }
  }
  return ret;
}

TREE1class(treeVisit);
extern "C" expr* tree_visit(expr*op1)
{
  expr* res = (op1 && op1->is_tree()) ? new treeVisit(op1) : op1;
  return res;
}
extern "C" expr* tree_top(expr*op1)
{
  return tree_visit(op1);
}

int treeVisit::value(const symbolPtr& sym)
{
  symbolPtr cur = sym;
  int ret = op->value(cur);
  return ret;
}
extern "C" long atop(char*);
static void tree_get_array(const char* tag, const symbolArr** parr, const symbolPtr** psym)
{
  Initialize(tree_get_array);
  if(strncmp(tag, "_DI_", 4) == 0){
    char*ptr = NULL;
    int ind = strtol(tag+4, &ptr, 10);
    symbolArr*arr = get_DI_symarr(ind);
    Assert(arr);
    if(ptr[0] == '\0'){
      *parr = arr;
    } else  {
      int el_ind = atoi(ptr+1);
      Assert(el_ind>=1 && el_ind <= arr->size());
      *psym = &(*arr)[el_ind-1];
    }
  } else if (tag[0] == '0' && tag[1] == 'X'){
    *psym = (const symbolPtr*) atop((char*)tag);
  }
}

TREE1(treeEqual,tree_equal);
bool dfa_equal(const symbolPtr&s1, const symbolPtr&s2);

inline bool sym_is_dfa(const symbolPtr& sym)
{
 return sym.is_ast() || sym.is_instance() || sym.is_dataCell();
}

inline int sym_is_dfa_equal(const symbolPtr& symbol, const symbolPtr& sym)
{
  return 
    sym_is_dfa(sym) ? dfa_equal(sym, symbol) : (sym.sym_compare(symbol) == 0);
}

static int dfa_array_contains(const symbolArr&array, const symbolPtr& sym)
{
  int ret = 0;
  int sz = array.size();
  bool is_dfa = sym_is_dfa(sym);
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& symbol = array[ii]; 
    if (is_dfa)
      ret = dfa_equal(sym, symbol);
    else
      ret = (sym.sym_compare(symbol.get_xrefSymbol()) == 0);
    if(ret)
      break;
  }
  return ret;
}

int treeEqual::value(const symbolPtr &sym)
{
  char *val  = (char *) op->value(sym);

  if (!val || val[0] == 0 || (val[1] == 0 && (val[0] == '.' || val[0] == ' ')))
    return 1;

  int ret;
  const symbolArr* parr = NULL;
  const symbolPtr* psym = NULL;
  tree_get_array(val, &parr, &psym);
  if(parr)
    ret = dfa_array_contains(*parr, sym);
  else if (psym)
    ret = sym_is_dfa_equal(*psym, sym);
  else 
    ret = (strcmp(val, ATT_cname((symbolPtr &) sym)) == 0);
  return ret;
}

struct treeNil : public expr {
  treeNil() : expr("nil") {setType();}
  int value(const symbolPtr& sym);
  int nil_value() const;
  virtual void setType(){ value_type =  INT_TYPE;}
  void print(ostream&str, int) const { str << "nil"; }
};
int treeNil::nil_value() const 
{
  return 1;
}

int treeNil::value(const symbolPtr& sym) 
{ 
  return sym.isnull();
}

struct treeNode : public treeNil {
  int nil_value() const {return 0;}
  int value(const symbolPtr& sym) { return sym.isnotnull();}
};

struct treeTrue : public treeNil {
  int nil_value() const {return 1;}
  int value(const symbolPtr& sym) { return 1;}
};

extern "C" expr* tree_nil()  { return new treeNil;}
extern "C" expr* tree_true() { return new treeTrue;}
extern "C" expr* tree_node() { return new treeNode;}

struct treeTimes : public treeNode {
  expr* op;
  int times;
  int del;
  treeTimes(expr*e, int t, int d) : op(e), times(t), del(d) {}
  ~treeTimes() { if(del) delete(op);}
  int value(const symbolPtr& sym);
};

int treeTimes::value(const symbolPtr& sym) 
{
  int ret = 1;
  for(int ii=0; ii<times; ++ii){
    ret = op->value(sym);
    if(!ret)
      break;
  }    
  return ret;
}

extern "C" expr* tree_times(expr*tree, int times, int del)
{
  if(times <= 0) {
    cli_error_msg("times: non-positive number of iterations\n");
    if(del)
      delete tree;
    return 0;
  } 
  
  expr* tm = (times == 1 && del) ?  tree : new treeTimes(tree, times, del);
  return tm;
}

int tree_get_first(const symbolPtr& cur, symbolPtr&first);
int tree_get_parent(const symbolPtr& cur, symbolPtr&first);

symbolArr  tree_cur_arr;
int        tree_cur_ind;
void tree_cur_init(symbolArr* arr)
{
  if(arr)
    tree_cur_arr = *arr;
  else
    tree_cur_arr.removeAll();
  tree_cur_ind = 0;

}
int tree_get_next(const symbolPtr& cur, symbolPtr&next);
int tree_get_previous(const symbolPtr& cur, symbolPtr&next);

struct treeWalk : public expr {
  virtual  void walk(const symbolPtr& cur, symbolPtr&next) = 0;
  treeWalk() : expr("") {value_type = INT_TYPE;}
  int value(const symbolPtr& sym);
  void setType() {value_type =  INT_TYPE;}
  bool is_tree() const {return true;}
  void print(ostream&str, int) const { str << "walk"; }

};

int treeWalk::value(const symbolPtr& sym)
{
  if(sym.isnull())
    return 0;

  symbolPtr next;
  walk(sym, next);
  *((symbolPtr*)&sym) = next;

  return 1;
}

struct treeRight : public  treeWalk {
 void walk(const symbolPtr& cur, symbolPtr&next){tree_get_next(cur,next);}
};
extern "C" expr* tree_right()
{
  return new treeRight;
}

struct treeLeft : public  treeWalk {
 void walk(const symbolPtr& cur, symbolPtr&next){tree_get_previous(cur,next);}
};
extern "C" expr* tree_left()
{
  return new treeLeft;
}

struct treeDown : public  treeWalk {
 void walk(const symbolPtr& cur, symbolPtr&next){tree_get_first(cur,next);}
};

extern "C" expr* tree_down()
{
  return new treeDown;
}

struct treeUp : public  treeWalk {
 void walk(const symbolPtr& cur, symbolPtr&next){tree_get_parent(cur,next);}
};

extern "C" expr* tree_up()
{
  return new treeUp;
}

struct treeString : public stringExpr {
  treeString(char* ptr, int len) : stringExpr(ptr,len) {value_type = INT_TYPE;}
  int value(const symbolPtr& sym);
  void setType() {value_type =  INT_TYPE;}
  bool is_tree() const {return true;}
};

int treeString::value(const symbolPtr& sym)
{
  const char*text = ATT_cname((symbolPtr&)sym);
  int res = strcmp(text, (char*)val);
  return res == 0;
} 

extern "C" expr* tree_string(char* ptr, int len)
{
  return new treeString(ptr, len);
}

struct treeSymbol : public expr {
  symbolPtr symbol;
  treeSymbol(const symbolPtr&s) : expr("symbol"), symbol(s) 
    {value_type = INT_TYPE;}
  int value(const symbolPtr& sym);
  void setType() {value_type =  INT_TYPE;}
  bool is_tree() const {return true;}
  virtual void print(ostream& = cout, int level = 0) const;
};

void treeSymbol::print(ostream&str, int) const
{
  str << symbol->get_name();
}

int treeSymbol::value(const symbolPtr& sym)
{
  return sym_is_dfa_equal(symbol, sym);
} 

struct treeSet : public expr {
  symbolSet set;
  treeSet(const symbolArr&arr) : expr("set")
    {
      value_type = INT_TYPE;
      set = arr;
    }
  int value(const symbolPtr& sym);
  void setType() {value_type =  INT_TYPE;}
  bool is_tree() const {return true;}
  virtual void print(ostream& = cout, int level = 0) const;
};

void treeSet::print(ostream&str, int) const
{
  str << "symbolSet" ;
}

int treeSet::value(const symbolPtr& sym)
{
  int ret;
  if (sym_is_dfa(sym)) {
    symbolPtr xsym = sym.get_xrefSymbol();
    if(xsym.isnull())
      ret = 0;
    else
      ret = set.includes(xsym);
  } else {
    ret = set.includes(sym);
  }
  return ret;
} 

struct treeArray : public expr {
  symbolArr array;
  treeArray(const symbolArr&arr) : expr("arr")
    {
      value_type = INT_TYPE;
      array = arr;
    }
  int value(const symbolPtr& sym);
  void setType() {value_type =  INT_TYPE;}
  bool is_tree() const {return true;}
  virtual void print(ostream& = cout, int level = 0) const;
};

void treeArray::print(ostream&str, int) const
{
  str << "symbolArr" ;
}

int treeArray::value(const symbolPtr& sym)
{
  return dfa_array_contains(array, sym);
} 

extern "C" expr* tree_set_element(int set_ind, int el_ind)
{
  expr*ret = NULL;
  symbolArr* arr =  get_DI_symarr(set_ind);
  if(!arr){
    cli_error_msg("set: invalid set\n");
  } else {
    int sz = arr->size();
    if(sz == 0){
      cli_error_msg("set: is empty\n");
    } else if (sz < el_ind){
      cli_error_msg("set: index too big\n");
    } else {
      symbolPtr&sym = (*arr)[el_ind - 1];
      ret =  new treeSymbol(sym);
    }
  }
  return ret;
}

static bool  dfa_array_is_xref(const symbolArr&array)
{
  int sz = array.size();
  for(int ii=0; ii<sz; ++ii){
    if(!array[ii].is_xrefSymbol())
      return false; 
  }
  return true;
}

extern "C" expr* tree_set(int ind)
{
  expr*ret = NULL;
  symbolArr* arr =  get_DI_symarr(ind);
  if(!arr){
    cli_error_msg("set: invalid index\n");
  } else {
    int sz = arr->size();
    if(sz == 0){
      cli_error_msg("set: is empty\n");
    } else if (sz == 1) {
      symbolPtr&sym = (*arr)[0];
      ret =  new treeSymbol(sym);
    } else if(dfa_array_is_xref(*arr)){
      ret = new treeSet(*arr);
    } else {
      ret = new treeArray(*arr);
    }
  } 
  return ret;
}

struct treeSelf : public stringExpr {
  symbolPtr symbol;
  int value(const symbolPtr& sym);
  treeSelf() : stringExpr("0X0000000000"){sprintf(((char*)val)+2, "%X", &symbol);}
  bool is_tree() const {return true;}
};

int treeSelf::value(const symbolPtr& sym)
{
  symbol = sym;
  char* buf = val;
  return (int)buf;
}
extern "C" expr* tree_self(char* ptr, int len)
{
  return new treeSelf();
}

struct selfExpr : public stringExpr
{
  treeSelf  dot;
  selfExpr(char*ptr, int len) : stringExpr(ptr,len) {}
  int value(const symbolPtr& sym);
};

int selfExpr::value(const symbolPtr& sym) 
{ 
  Interpreter* i = GetActiveInterpreter();
  char* self = (char*) dot.value(sym);
  Tcl_SetVar(i->interp, (char*)val, self, 0);
  return (int) self;
}

extern "C" expr* tree_assign(char* ptr, int len)
{
  return new selfExpr(ptr, len);
}

int tree_find_subtree(const symbolPtr&sym, expr*op)
{
  int res = 0;
  symbolPtr cur = sym;
  if(op->value(cur)){
    res += 1;
  }

  for(int val = tree_get_first(sym, cur); val; val = tree_get_next(cur, cur)){
    res += tree_find_subtree(cur, op);
  }
  return res;
}
int tree_find1_subtree(const symbolPtr&sym, expr*op)
{
  symbolPtr cur = sym;
  if(op->value(cur)){
    return 1;
  }

  for(int val = tree_get_first(sym, cur); val; val = tree_get_next(cur, cur)){
    if(tree_find1_subtree(cur, op))
      return 1;
  }
  return 0;
}

TREE1(treeSetcur,tree_setcur);
int treeSetcur::value(const symbolPtr& sym)
{
  int retval = 0;
  char * tag = (char*) op->value(sym);
  if (tag[0] == '0' && tag[1] == 'X') {
    symbolPtr* visit_sym = (symbolPtr*) atop(tag);
    if(visit_sym) {
      symbolPtr * ppp = (symbolPtr*) &sym;
      *ppp = *visit_sym;
      retval =  1;
    }
  }
  return retval;
}


TREE1(treeFind,tree_find);
int treeFind::value(const symbolPtr& sym)
{
  return tree_find_subtree(sym, op);
}
TREE1(treeSearch,tree_search);
int treeSearch::value(const symbolPtr& sym)
{
  int res = 0;
  symbolPtr cur = sym;
  do res += tree_find_subtree(cur, op);
  while (tree_get_next(cur, cur));
  return res;
}

TREE1(treeFind1,tree_find1);
int treeFind1::value(const symbolPtr& sym)
{
  return tree_find1_subtree(sym, op);
}
TREE1(treeSearch1,tree_search1);
int treeSearch1::value(const symbolPtr& sym)
{
  symbolPtr cur = sym;
  do
   if(tree_find1_subtree(cur, op))
     return 1;
  while (tree_get_next(cur, cur));
  return 0;
}
struct treeXrefSymbol : public unaryExpr {
  treeXrefSymbol(expr*o1) : unaryExpr(o1, "symbol") { value_type = o1->value_type;}
  int value(const symbolPtr& sym); 
  void setType() {value_type = op->value_type;}
  bool is_tree() const {return false;}
};

int treeXrefSymbol::value(const symbolPtr& sym)
{
  symbolPtr xsym = sym.get_xrefSymbol();
  return xsym.isnull() ? op->nil_value() : op->value(xsym);
}
extern "C" expr* tree_symbol(expr*op)
{
  expr* res = op ? new treeXrefSymbol(op) : NULL;
  return res;
}
expr* tree_strip(expr*tree)
{
  if(tree->is_tree())
    tree = ((treeVisit*)tree)->op;
  return tree;
}

// format
struct formatExpr : public expr 
{
  symbolFormat *fmt;
  ocharstream buf;

  formatExpr(char*f, objArr&a);
  ~formatExpr(){delete fmt;}

  int value(const symbolPtr& sym);
  virtual void print(ostream& = cout, int level = 0) const;
  void setType() {value_type = STRING_TYPE;}
};

formatExpr::formatExpr(char*format, objArr& arr) : expr("format"), fmt(NULL)
{
  Initialize(formatExpr::formatExpr);
  setType();
  fmt = new symbolFormat("attr", arr, format);
}

int formatExpr::value(const symbolPtr& sym){
  buf.reset();
  fmt->printOut(buf, sym);
  buf << '\0';
  return (int) buf.ptr();
}

void formatExpr::print(ostream& os, int level) const
{
  fmt->print(os, level);
}

extern "C" expr* build_format(stringExpr*f, listExpr*a)
{
 Initialize(build_format);
  expr*format = NULL;
  if(f&&a) {
    objArr arr;
    int sz = a->items.size();
    int err=0;
    for(int ii=0; ii<sz; ++ii){
      expr* e = *(a->items[ii]);
      if (!e) {
	cli_error_msg("Bad expression.\n");
	++err;
	continue;
      }
      attrExpr*ae = (attrExpr*)e;
      symbolAttribute* attr = ae->attr;
      arr.insert_last(attr);
    }
    if(!err)
      format = new formatExpr(f->val, arr);
  }
  if(f) delete f;
  if(a) delete a;
  return format;
}

