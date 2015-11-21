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
#ifndef _idfDict_h
#define _idfDict_h

//============================================================================
// classes for the dictionaries used in IF comparison
//----------------------------------------------------------------------------
// Two types of dictionary indexes are supported: int and string.
//============================================================================

#include <idf.h>
#include <general.h>
class idfDictionary;
class idfIterator {
public:
private:
  void     *current;

  friend class idfDictionary;
};

//============================================================================
// idfIntDictionary
//----------------------------------------------------------------------------
// dictionary class indexed by int
//============================================================================
class idfIntDictionary {
public:
                  idfIntDictionary();
  virtual         ~idfIntDictionary();
  boolean         add(int, void *value);
  boolean         reassign(int, void *value);
  void           *lookup(int) const;
  boolean         contains(int) const;
  void           *lookup(const idfIterator &) const;
  int             index(const idfIterator &) const;
  idfIterator     first() const;
  boolean         next(idfIterator &) const;
  boolean         done(const idfIterator &) const;
private:
  idfDictionary  *dict;
};

//============================================================================
// idfStringDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by string
//============================================================================
class idfStringDictionary {
public:
                  idfStringDictionary();
  virtual         ~idfStringDictionary();
  boolean         add(const char *, void *value);
  boolean         reassign(const char *, void *value);
  void           *lookup(const char *) const;
  boolean         contains(const char *) const;
  void           *lookup(const idfIterator &) const;
  const char *    index(const idfIterator &) const;
  idfIterator     first() const;
  boolean         next(idfIterator &) const;
  boolean         done(const idfIterator &) const;
private:
  idfDictionary  *dict;
};

//============================================================================
// idfRelationDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by string (the relation name)
// and yielding an idfRelationshipDictionary (symbol id to idfAttributeDictionary).
//============================================================================
class idfRelationDictionary : private idfStringDictionary {
public:
  boolean         add(const char *idx, idfRelationshipDictionary *value)
   { return idfStringDictionary::add(idx, (void *)value); }
  boolean         reassign(const char *idx, idfRelationshipDictionary *value)
   { return idfStringDictionary::reassign(idx, (void *)value); }
  idfRelationshipDictionary *lookup(const char *idx) const
   { return (idfRelationshipDictionary *)idfStringDictionary::lookup(idx); }
  idfRelationshipDictionary *lookup(const idfIterator &iter) const
   { return (idfRelationshipDictionary *)idfStringDictionary::lookup(iter); }
                  idfStringDictionary::index;
                  idfStringDictionary::first;
                  idfStringDictionary::next;
                  idfStringDictionary::done;
                  idfStringDictionary::contains;
};

//============================================================================
// idfRelationshipDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by int (a symbol id) and yielding
// an attribute dictionary (the attributes of the containing relation)
//============================================================================
class idfRelationshipDictionary : private idfIntDictionary {
public:
  boolean         add(int idx, idfAttributeDictionary *value)
   { return idfIntDictionary::add(idx, (void *)value); }
  boolean         reassign(int idx, idfAttributeDictionary *value)
   { return idfIntDictionary::reassign(idx, (void *)value); }
  idfAttributeDictionary *lookup(int idx) const
   { return (idfAttributeDictionary *)idfIntDictionary::lookup(idx); }
  idfAttributeDictionary *lookup(const idfIterator &iter) const
   { return (idfAttributeDictionary *)idfIntDictionary::lookup(iter); }
                  idfIntDictionary::index;
                  idfIntDictionary::first;
                  idfIntDictionary::next;
                  idfIntDictionary::done;
                  idfIntDictionary::contains;
};

//============================================================================
// idfAttributeDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by string (the attribute name)
// and yielding a string (value of attribute).
// Note that the value strings storage is managed by the caller.
//============================================================================
class idfAttributeDictionary : private idfStringDictionary {
public:
  boolean         add(const char *idx, const char *value)
   { return idfStringDictionary::add(idx, (void *)value); }
  boolean         reassign(const char *idx, const char *value)
   { return idfStringDictionary::reassign(idx, (void *)value); }
  const char     *lookup(const char *idx) const
   { return (const char *)idfStringDictionary::lookup(idx); }
  const char     *lookup(const idfIterator &iter) const
   { return (const char *)idfStringDictionary::lookup(iter); }
                  idfStringDictionary::index;
                  idfStringDictionary::first;
                  idfStringDictionary::next;
                  idfStringDictionary::done;
                  idfStringDictionary::contains;
};

//============================================================================
// idfSymbolDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by int (a symbol id) and yielding
// a symbol object indicating the properties of the symbol
//============================================================================
class idfSymbolDictionary : private idfIntDictionary {
public:
  boolean         add(int idx, idfSymbol *value)
   { return idfIntDictionary::add(idx, (void *)value); }
  boolean         reassign(int idx, idfSymbol *value)
   { return idfIntDictionary::reassign(idx, (void *)value); }
  idfSymbol *lookup(int idx) const
   { return (idfSymbol *)idfIntDictionary::lookup(idx); }
  idfSymbol *lookup(const idfIterator &iter) const
   { return (idfSymbol *)idfIntDictionary::lookup(iter); }
                  idfIntDictionary::index;
                  idfIntDictionary::first;
                  idfIntDictionary::next;
                  idfIntDictionary::done;
                  idfIntDictionary::contains;

};

//============================================================================
// idfStringSet
//----------------------------------------------------------------------------
// dictionary class, indexed by string and yielding void
//============================================================================
class idfStringSet : private idfStringDictionary {
public:
  boolean         add(const char *idx)
   { return idfStringDictionary::add(idx, (void *)this); }
  boolean         lookup(const char *idx) const
   { return idfStringDictionary::lookup(idx) != NULL; }
  boolean         lookup(const idfIterator &iter) const
   { return idfStringDictionary::lookup(iter) != NULL; }
                  idfStringDictionary::index;
                  idfStringDictionary::first;
                  idfStringDictionary::next;
                  idfStringDictionary::done;
                  idfStringDictionary::contains;
};

//============================================================================
// idfStringSetDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by string (the name)
// and yielding a StringSet (value).
// Note that the value storage is managed by the caller.
//============================================================================
class idfStringSetDictionary : private idfStringDictionary {
public:
  boolean         add(const char *idx, idfStringSet *value)
   { return idfStringDictionary::add(idx, (void *)value); }
  boolean         reassign(const char *idx, idfStringSet *value)
   { return idfStringDictionary::reassign(idx, (void *)value); }
  idfStringSet   *lookup(const char *idx) const
   { return (idfStringSet *)idfStringDictionary::lookup(idx); }
  idfStringSet   *lookup(const idfIterator &iter) const
   { return (idfStringSet *)idfStringDictionary::lookup(iter); }
                  idfStringDictionary::index;
                  idfStringDictionary::first;
                  idfStringDictionary::next;
                  idfStringDictionary::done;
                  idfStringDictionary::contains;
};

//============================================================================
// idfSymbolNameDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by string (a symbol name) and yielding
// a symbol object indicating the properties of the symbol
//============================================================================
class idfSymbolNameDictionary : private idfStringDictionary {
public:
  boolean         add(const char *idx, idfSymbol *value)
   { return idfStringDictionary::add(idx, (void *)value); }
  boolean         reassign(const char *idx, idfSymbol *value)
   { return idfStringDictionary::reassign(idx, (void *)value); }
  idfSymbol *lookup(const char *idx) const
   { return (idfSymbol *)idfStringDictionary::lookup(idx); }
  idfSymbol *lookup(const idfIterator &iter) const
   { return (idfSymbol *)idfStringDictionary::lookup(iter); }
                  idfStringDictionary::index;
                  idfStringDictionary::first;
                  idfStringDictionary::next;
                  idfStringDictionary::done;
                  idfStringDictionary::contains;

};

//============================================================================
// idfStringStringDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by string (the name)
// and yielding a string.
// Note that the value storage is managed by the caller.
//============================================================================
class idfStringStringDictionary : private idfStringDictionary {
public:
  boolean         add(const char *idx, const char *value)
   { return idfStringDictionary::add(idx, (void *)value); }
  boolean         reassign(const char *idx, const char *value)
   { return idfStringDictionary::reassign(idx, (void *)value); }
  const char     *lookup(const char *idx) const
   { return (const char *)idfStringDictionary::lookup(idx); }
  const char     *lookup(const idfIterator &iter) const
   { return (const char *)idfStringDictionary::lookup(iter); }
                  idfStringDictionary::index;
                  idfStringDictionary::first;
                  idfStringDictionary::next;
                  idfStringDictionary::done;
                  idfStringDictionary::contains;
};
#endif
