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
// Message Dictionary
// 
// This class implements a dictionary of strings. 
// The key/value pairs may be loaded from a text file 
// formatted like so:
//  
//   key:val
// 
// A typical entry:
// 
//   M_NO_W_PERM:E:No Write Permissions for file: %s
// 
// When constructing a new dictionary, you must specify 
// whether or not to load the database with one of
// the 'msgDictCode' directives below. You may add
// key/value pairs explicitly if necessary. 

#include <Object.h>
#include <objSet.h>

#define SAFE_STR(s) ((s) ? (s) : "(nil)")           // So printf() doesn't crash.

extern unsigned string_hash(const char *);

class msgDictEntry : public Object {
  public:
    msgDictEntry(const char *k, const char *v) : key(k), val(v) {}
 
    bool isEqual(const Object &oo) const;
    unsigned hash() const {return string_hash(key);}
    const char *get_key() const {return key;}
    const char *get_val() const {return val;}
  
  private:
    const char *key;
    const char *val;
};

typedef enum {
  msgDictDONT_LOAD,              // Pass this to the constructor if you want to create an empty dictionary.
  msgDictLOAD                    // Tell the contructor to load the database from a default location.
} msgDictCode;

class msgDict : public objSet {
  public:
    msgDict(const msgDictCode);
 
    msgDictEntry *add(const char *, const char *);    // Add an entry to the dictionary: "add(key, val)".
    const char *find(const char *) const;             // Returns the value associated w/a particular key.
    msgDictEntry *find(const msgDictEntry &) const;   // Alternate method for finding keys/values.
    bool load();                                      // Determine what file should be loaded, and load it.
    bool load(const char *);                          // Load the database from a file.
    bool dbLoaded() const;                            // Return non-zero if a db is loaded.
    int prt() const;                                  // Print the database.

  private:
    static bool dbBeingLoaded_;
    bool dbLoaded_;
    int debugLevel;
};


// Inline methods.
inline bool msgDict::dbLoaded() const { return dbLoaded_;} ;
