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
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <msgDict.h>
#include <psetmem.h>
#include <Object.h>
#include <objSet.h>
#include <genString.h>
#include <customize.h>
#include <systemMessages.h>

extern "C" int dos2unix(char*, int);

// Class static to protect constructor from recursion.
bool msgDict::dbBeingLoaded_ = false;

bool msgDictEntry::isEqual(const Object &oo) const
{
  const char *okey = ((msgDictEntry &)oo).key;

  if ((!key) && (!okey)) return 1;
  if ((!key) || (!okey)) return 0;
  return (OSapi_strcmp((char *)key, (char *)okey) == 0);
}

// Add a pair of strings to the dictionary
msgDictEntry *msgDict::add(const char *key, const char *val)
{
  if (debugLevel > 0) {
    if (!key)
      msg("Trying to create message entry with NULL key.", warning_sev) << eom;
    if (!val)
      msg("Message key points to NULL value: \"$1\".", warning_sev) << (SAFE_STR((char *)key)) << eom;
  }

  int len1 = key ? OSapi_strlen(key) + 1 : 0;
  int len2 = val ? OSapi_strlen(val) + 1 : 0;
 
  char *st1 = len1 ? (char *)psetmalloc(len1) : NULL;
  char *st2 = len2 ? (char *)psetmalloc(len2) : NULL;

  if (st1) OSapi_bcopy(key, st1, len1);
  if (st2) OSapi_bcopy(val, st2, len2);
  
  msgDictEntry *entry = new msgDictEntry(st1, st2);
  os_list.add(*entry);
  
  return entry;
}

const char *msgDict::find(const char *key) const
{
  const char* value = NULL;

  if (dbLoaded() ) {
      msgDictEntry tmp(key, (const char *)NULL);
      msgDictEntry* entry = find(tmp);
      if (entry) value = entry->get_val();
  }
  return(value);
}

msgDictEntry *msgDict::find(const msgDictEntry &entry) const
{
  Object *oo = os_list.findObjectWithKey(entry);

  if (debugLevel > 0) {
    if (!((char *)entry.get_key()))
      msg("Trying to find NULL key.", warning_sev) << eom;
    else if (!(msgDictEntry *)oo)
      msg("Message key not found: \"$1\".", warning_sev) << (SAFE_STR((char *)entry.get_key())) << eom;
    else if (!((char *)(((msgDictEntry *)oo)->get_val())))
      msg("Message key points to NULL value: \"$1\".", warning_sev) << (SAFE_STR((char *)entry.get_key())) << eom;
  }

  return (msgDictEntry *)oo;
}

msgDict::msgDict(const msgDictCode code)
{
  // No database loaded yet.
  dbLoaded_ = false;

  // Set debug level.
  if (customize::is_init() )
    debugLevel = customize::getIntPref("DIS_internal.MsgDict_DebugLevel");
  else 
    debugLevel = -1;

  if (!dbBeingLoaded_) {
      dbBeingLoaded_ = true;

      switch (code) {
        case msgDictDONT_LOAD: break;
        case msgDictLOAD: load(); break;
        default: break;
      }
      dbBeingLoaded_ = false;
  }
}

// btok(p, tok)                                          
// 
// p - pointer to buffer
// tok - delimiter
//
// Returns a null-terminated string that matches 
// all the characters from *p to tok (inclusive).
// Additionally the value of p is incremented to 
// the position of the first char after tok. 

static char *btok(char **p, char tok)
{
  char *s = NULL;
  char *head = *p;
  char *tail = OSapi_index(head, tok);
  if (tail) {
    int len = tail - head;
    s = new char[len + 1];
    OSapi_strncpy(s, head, len);
    s[len] = '\0';
    *p = tail + 1;
  }

  return s;  
}

// esc_to_ctrl(esc)
//
// esc - string containing escape sequences (e.g. \n)
//
// Converts escape sequences into control codes; leaves 
// other characters intact. Returns a copy of esc with 
// the escape sequences converted into control codes (from 
// newbugwin.C).

static char *esc_to_ctrl(char *esc)
{
  if (!esc) return NULL;

  char *s = new char[strlen(esc) + 1];

  char *ctrl;
  for (ctrl = s; *esc; ctrl++, esc++) {
    if (*esc == '\\') 
      switch (*(++esc)) {
        case 'a': *ctrl = '\a'; break;
	case 'b': *ctrl = '\b'; break;
	case 'f': *ctrl = '\f'; break;
	case 'n': *ctrl = '\n'; break;
        case 'r': *ctrl = '\r'; break;
        case 't': *ctrl = '\t'; break;
	case 'v': *ctrl = '\v'; break;
        case '\\':
	case '\?':
        case '\'':
        case '\"': 
	default: *ctrl = *esc; break;
      }
    else
      *ctrl = *esc;
  }
  *ctrl = '\0';

  return s;
}

// msgDict::load(fname)
//
// fname - file to load into the dictionary
//
// Loads a file into the dictionary. 
// Returns true on success.
bool msgDict::load(const char *fname)
{
    bool status = true;
    FILE *fl = NULL;

    if (fname) 
        fl = OSapi_fopen(fname, "r");
    else {
        msg("Empty message data file name given.", error_sev) << eom;
        status = false;
    }

    if (status && !fl) {
        msg("Unable to open messages file: \'$1\'.", error_sev) << (SAFE_STR(fname)) << eom;
        status = false;
    }

    if (status) {
        struct OStype_stat st;
        OSapi_stat((char*)fname, &st);
        int fl_sz = (int)st.st_size;
        char *buf = new char[(fl_sz / sizeof(char)) + 2];
        int read_sz = OSapi_fread(buf, sizeof(char), fl_sz / sizeof(char), fl);
        read_sz = dos2unix(buf, read_sz);
        buf[read_sz] = '\n';
        buf[read_sz + 1] = '\0';

        for (char *p = buf; p - buf < read_sz + 1;) {
          char *key;
          while (isspace(*p)) if (p - buf < read_sz + 1) p++; else break;
          if ((key = btok(&p, ':')) != NULL) {
	    char* temp = btok(&p, '\n');
            char * val = esc_to_ctrl(temp);
            add(key, val);
	    delete val;
	    delete temp;
	    delete key;
          }
          else {
            if (p == buf)
              msg("Empty messages file: \"$1\".", warning_sev) << (SAFE_STR(fname)) << eom;
            break;
          }
        }

        delete buf;
        OSapi_fclose(fl);

        if (debugLevel > 0) prt();

        // Mark dictionary as having loaded a database.
        dbLoaded_ = true;
    }

  return (status);
}  

// Loads standard message database.
// Returns true on success.
bool msgDict::load()
{
  char const *tmp;
  genString fname;
  bool status = true;

  if ((tmp = OSapi_getenv("PSET_MSG_FILE")) != NULL)
    fname = tmp;
  else if (customize::is_init()) {
    fname = customize::getStrPref("DIS_msg.MsgLogFile", true);
  } else {
    status = false;
  }

  if (status) 
    status = load((char*) fname);

  if (status && (debugLevel > 0) ) 
      msg("Loaded messages database from: \"$1\".", normal_sev) << (char *)fname << eom;

 return(status);
}

int msgDict::prt() const
{
  int ii = 0;
  Obj *el;
 
  ForEach(el, *this) {
    msgDictEntry *entry = (msgDictEntry *)el;
    msg("$1:$2", normal_sev) << (SAFE_STR(entry->get_key())) << eoarg << (SAFE_STR(entry->get_val())) << eom;
    ++ii;
  }

  return ii;
}

msgDict *msg_dict;
