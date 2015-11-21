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
#ifndef _elsLanguage_h
#define _elsLanguage_h

#include <objDictionary.h>

class objArr;

class elsLanguage : public objDictionary {
 public:
  elsLanguage(const char*nm) : objDictionary(nm), fileIconCode(0) {}
  genString flags;
  genString suffixes;
  int fileIconCode;
  void  print(ostream&, int=0) const;
  static const objArr& list();
  static const elsLanguage* get_els_language(symbolPtr& sym);
  static const elsLanguage* get_els_language_for_cpp();
  static const elsLanguage* get_els_language_for_c();
};

class elsSuffix : public objDictionary {
 public:
  elsSuffix(const char*nm) : objDictionary(nm) {}
  int len;
  elsLanguage*els;
  void  print(ostream&, int=0) const;
  static const objArr& list();
};

const char* els_flags(const char* lan);
const char* els_guess_language(const char*fn);
int els_parse_file(const symbolPtr& mod);
int els_parse_file(const Relational* mod);
const elsLanguage* els_get_language(const char*fn);

#endif
