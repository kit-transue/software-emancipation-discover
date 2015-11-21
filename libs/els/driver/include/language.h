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
// This code is a -*- C++ -*- header file

#ifndef _language_h
#define _language_h

//++ADS
class parray;
//--ADS

//++TSET
class Settings;
//--TSET

class language
{
public:
  
  //
  // Constructor(s) / Destructor(s)
  //
  language(const char *filename);
  language(const language &other);
  ~language(void);

  // operators
  language &operator = (const language &other);

  //
  // Public Interface
  //

  // return the printable name of the language (i.e. "C", "C++", "Perl", "Fortran 77", etc.)
  const char *name(void) const;

  // return the name that DISCOVER uses for this language (i.e. "c", "cpp", "perl", "f77", etc.)
  const char *discoverName(void) const;

  // retrieve a value from the LDF
  const char *getPref(const char *prefKey) const;
 
protected:
  static const char * const ldfSearchPathPrefKey;
  static const char * const suffixPrefKey;
  static const char * const defaultSuffixPrefKey;
  static const char * const headerSuffixPrefKey;
  static const char * const defaultHeaderSuffixPrefKey;
  static const char * const langNamePrefKey;

  static const char * const ldfSearchPathPrefDefaultVal;
  static const char * const suffixPrefDefaultVal;
  static const char * const defaultSuffixPrefDefaultVal;
  static const char * const headerSuffixPrefDefaultVal;
  static const char * const defaultHeaderSuffixPrefDefaultVal;
  static const char * const langNamePrefDefaultVal;

protected:

  class ldfInfo
    {
    public:
      ldfInfo(const char *filename);
      virtual ~ldfInfo(void);

      int         handlesSuffix(const char *suffix) const;
      const char *getPref(const char *prefKey) const;
      const char *discoverName(void) const;

    public:
      static const char * const  ldfSuffix;

    private:
      char   *filename;         // the filename of the ldf that this came from
      char   *discName;         // DISCOVER's internal name of the language
      
      parray *suffixes;         // parray of const char *'s
      parray *headerSuffixes;   // same thing
      
      Settings *prefs;          // all the psetPrefs stored in the ldf file

    private:
      int  parseSuffixString(const char *, parray *) const;
    };

  friend class ldfInfo;

  static parray *cachedLDFs;   // an array of pointers to LDFInfo's, 1 for each language we loaded an LDF for
  int            ldfIndex;     // e.g.: I am language #3 in the parray of cached ldf's

private:

  const char *getSuffix(const char *filename) const;
  int         cacheLDF(const char *suffix);
  int         getLDFSearchPath(char **searchPath) const;
  int         getUnloadedLDFs(parray *filenames) const; 
  ldfInfo    *getCachedLDF(const char *suffix) const;
  ldfInfo    *getLDFInfo(const char *suffix);
};

#endif 
// _language_h

