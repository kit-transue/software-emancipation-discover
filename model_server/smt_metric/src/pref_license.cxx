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
#include <customize.h>
#include <metric.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <cLibraryFunctions.h>
#include <msg.h>
#include <genError.h>
#include <xxinterface.h>

static char const *debug_prefs = OSapi_getenv("DIS_DEBUG_MODEL_PREFS");
typedef unsigned int (*FLAG_FUNC)();

static void
print_flag(FLAG_FUNC f,
	   const char *nm)
{
  unsigned int i = f();
  msg("flag: $1; val: $2", normal_sev) << nm << eoarg << i << eom ; 
}

static void
debug_print(const char *key,
	    int val)
{
  msg("pref: $1; type: int ; val: $2", normal_sev) << key << eoarg << val << eom;
}

static void
debug_print(const char *key,
	    const char *val)
{
  msg("pref: $1; type: char ; val: $2", normal_sev) << key << eoarg << val << eom;
}

static int
get_int_pref(const char *key)
{
  int i = customize::getIntPref(key);
  return i;
}

static const char *
get_str_pref(const char *key)
{
  const char *i = customize::getStrPref(key);
  return i;
}

static bool
get_bool_pref(const char *key)
{
  bool i = customize::getBoolPref(key);
  return i;
}

static const char *dfa_key       = "doBuildDFA";

static bool
get_pref_dfa()
{
  static bool pref_dfa = get_bool_pref(dfa_key);
  return pref_dfa;
}

unsigned int
dfa_enabled()
{
  return
    get_pref_dfa();
}

extern bool is_batch();

void
debug_get_print_flags()
{
  static int init = 0;

  if (init)
    return;

  ++init;

  if (debug_prefs == 0)
    return;
    
  print_flag(&dfa_enabled, "dfa_enabled");
}

