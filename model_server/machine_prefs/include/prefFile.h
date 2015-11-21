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
//**********
//
// prefFile.h - Preference File class decls.
//
//**********

#ifndef _prefFile_h
#define _prefFile_h

#include <general.h>  // for "bool"
#include <genString.h>

// Preference File Class
class prefFile {

public:
    prefFile();
    virtual ~prefFile();

    virtual bool Open(const char* name);
    virtual bool Close();

    virtual bool Read(const char* key, int& value);
    virtual bool Read(const char* key, genString& value);

protected:
    static const genString DEFAULT_PREFERENCE_FILE;

    static const genString PREF_KEY_PREFIX;
    static const genString PREF_KEY_SUFFIX;

    virtual const char*  GetDefaultFileName();
    virtual bool InitFile(const char* name);
    virtual bool InitPath(void);
    virtual bool InitBaseName(const char* prefFile);
    virtual bool CheckAccess(void);

    bool   InitText(void);

    //call this function when ever prefText_ is modified, this will keep
    // prefText_ and prefText_l consistent
    void   prefText_is_changed(void); 

    char*  SelectPref(const char*, long&);
    long  searchPref(char*, long off = 0);

    long PrefLineGetEnd(const char *);
    int  PrefLineIsComment(const char *);
    
    genString   prefText_;
    genString   prefText_l;
    genString   baseName_;
    genString   fullName_;
    genString   prefPath_;
    int         prefFD_;       // Preference file I/O.
    int         openMode_;     // Preference file open mode, usually Read|Write

    bool fileExists(const char* pFilename);
};


// Inline methods.
inline const char* prefFile::GetDefaultFileName() { return DEFAULT_PREFERENCE_FILE.str(); }




#endif    // _prefFile_h
