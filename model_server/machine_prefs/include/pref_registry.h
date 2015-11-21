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
#ifndef _pref_registry_h
#define _pref_registry_h

#ifndef _genArr_h
#include <genArr.h>
#endif
#ifndef _genString_h
#include <genString.h>
#endif

#include <avl_tree.h>

class prefKey : public avl_key {
  genString key;
  prefKey(prefKey const& pk);// { key = pk.get_key(); }
 public:
  prefKey(const char *k) { key = k; }
  //  prefKey() { key = NULL;}
  ~prefKey() {}
  char * get_key() const { return key; }
};

class prefNode : public avl_node {
  prefKey *pref_key;
  int index;
 public:
  prefNode(avl_treeptr, const char *, const int);
  ~prefNode() { delete pref_key; }
  virtual int compare(const avl_key&) const;
  int get_index() const { return index; }
  char *key() const { return pref_key->get_key(); }
};




//////////     Pref Table     //////////
struct Pref {
  genString key;
  bool persistent;
  int  prefType;  // 1=int, 2=genString.
  int       intPref;
  genString strPref;
  genString unexpandedStrPref;
  int       expandType; // See prefExpansion.
};

genArr(Pref);

  
class prefFile;
class userPrefFile;

class prefRegistry{
  public:
    prefRegistry();
    ~prefRegistry();

    // Expansion is a from-to description.
    // Note - Colons assumes meta also.
    enum prefExpansion { EXPAND_NONE,
                         EXPAND_NONE_TO_META,
                         EXPAND_NONE_TO_COLONS,
                         EXPAND_COLONS_TO_NONE
    };
    bool             getBoolPref(const char* keyname);
    const int&       getIntPref(const char* keyname);
    const genString& getStrPref(const char* keyname,
                       const int expansion = EXPAND_NONE_TO_META);

    bool putPref(const char* keyname,
                 const int   value,
                 const bool  persistent = true);
    bool putPref(const char*     keyname,
                 const genString value,
                 const int       expanded = EXPAND_NONE,
                 const bool      persistent = true);

    static const genString UNINITIALIZED_STRING_PREF;
    static const int       UNINITIALIZED_INT_PREF;
    int searchPrefValue(const char* keyname, genString&value, bool none_ok=false);
  private:
    genArrOf(Pref) prefTable;
    bool searchTable(const char* key, int**       handle);
    bool searchTable(const char* key, genString** handle,
                     const int expansion=EXPAND_NONE_TO_META);

    bool insertTable(const char* key, const int value, const bool persistent=true);
    bool insertTable(const char* key, const genString value,
                     const int expansion=EXPAND_NONE_TO_META,
                     const bool persistent=true);

    bool expandPref(int index, const genString value);
    bool expandMeta(int index);
    bool expandColons(const genString unexpanded, genString& expanded);
    bool unexpandColons(genString& unexpanded, const genString expanded);

    bool isPersistent(const char* key);
 
    bool getFromDict(const char* keyname, int&   value);
    bool getFromDict(const char* keyname, genString& value);

    bool putToDict(const char* keyname, const int   value);
    bool putToDict(const char* keyname, const char* value);

    static bool getTagFromKey(const char*, char*);
    static bool getTopKey(const char*, char*);

    enum Pref_Types { Int_Pref, Str_Pref};

     static const char* DebugLevelEnvVar;
     int debugLevel;

    // Methods and data to support the System preference file.
    bool initSystemPreferences();
    char* SysPrefFileName_;
    prefFile* SysPrefFile_;

    // Methods and data to support the System preference file.
    bool initUserPreferences();
    char* UserPrefFileName_;
    userPrefFile* UserPrefFile_;

    bool hasUserPrefs();  // True unless -noUserPrefs specified.
    bool HasUserPrefs_;

    // Utility method to check for environment variable prefs.
    const char* getPrefFromEnv(const char* prefName);

    // Methods and data needed to convert the key into an array index.
    avl_treeptr prefBtree;
    bool getTableIndex(const char* key, int& index);
    bool putTableIndex(const char* key, const int index);
};



// Inline methods.
inline bool prefRegistry::hasUserPrefs() { return HasUserPrefs_; }


#endif    // _pref_registry_h
