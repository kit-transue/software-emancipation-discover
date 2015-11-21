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
#ifndef _genError_h
#define _genError_h

#include <general.h>
#include <stdio.h>

#if defined(hp10)
typedef double *PUSH_BUF_RET;
#elif defined(irix6)
typedef __uint64_t *PUSH_BUF_RET;
#else
typedef int *PUSH_BUF_RET;
#endif

PUSH_BUF_RET push_buf();
void pop_buf();
int  max_buf();
void throw_error(int);

int  gen_if_handler(const char* name, const char* expr);
int  gen_dbg_handler(const char* name, int lev);
void gen_init_handler(const char* name);
void gen_exit_handler(const char* name);
//void gen_error_handler(const char* name, int code); // OLD
void gen_error_handler(
    const char* name, int code, const char* filename, int line);
void gen_conv_error_handler_new(
    const char* tp, const char* name, int line_no, void* obj);

int  gen_trace_get_level();
void gen_trace_set_level(int level);

//-------------------------------------------------------------------------

#define ReturnValue(x)  return(x)
#define Return		{ return; }

struct IniRet {
     const char * nnn;
     operator const char* () { return nnn;}

     IniRet(const char * n) : nnn(n) {
          gen_init_handler(nnn); 
	}
    ~IniRet()    {
          gen_exit_handler(nnn); 
	}
};

#define Initialize(name) \
        IniRet _my_name(quote(name))

#define IF(x) if((x) && gen_if_handler(_my_name, quote(x)))
#define Error(x) gen_error_handler(_my_name, x, __FILE__, __LINE__)

void PrintLanguageError(const char* fun, const char* file, int line);
#define LanguageError() PrintLanguageError(_my_name, __FILE__, __LINE__)

#ifdef _ASET_OPTIMIZE

#define _ASET_OPTIMIZE_LEVEL 0

#else

#define _ASET_OPTIMIZE_LEVEL 1

#endif

#define DBG if(gen_dbg_handler(_my_name, _ASET_OPTIMIZE_LEVEL))

//-------------------------------------------------------------------------

#define underInitialize(name) \
	static char* _my_name = quote(name);


#define Assert(x) IF(!(x)) Error(ERR_ASSERT)

enum
ERR_TYPE
{
    ERR_INPUT=101,
    ERR_FAIL,
    ERR_ALLOC,
    NOT_IMPLEMENTED_YET,
    ERR_ASSERT,
    EPOCH_TIME_OUT 
} ;

class genError
{
  public:
    int code;
    genError(int cc) : code(cc) {}
    virtual void print();
    virtual ~genError() {}
};


#endif // _genError_h
