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
// convertPrefFile.h - Preference File class decls.
//
//**********

#ifndef _convertPrefFile_h
#define _convertPrefFile_h

#include <userPrefFile.h>


// Structure to be used when transalating to Galaxy format.
typedef struct {
    char* oldKey;
    char* newKey;
    int   type;
} TranslateInfo;


// Preference File Class
class convertPrefFile : public userPrefFile {

public:

    convertPrefFile();
    virtual ~convertPrefFile();

protected:
    static const vname* DEFAULT_PREFERENCE_FILE;
    virtual const vchar* GetDefaultFile();

    virtual vbool InitBaseName(const vchar* convertPrefFile);

    // methods and data for conversion from text to Galaxy format.
    // [Was used for 6 months, not currently used.]
    virtual vbool needToRename(vstr* pFilename);
    virtual vbool renameOldPrefFile(vstr* pFilename);
    virtual vbool loadOldPrefFile (const char* pFilename);
    char* UnconvertedPrefFileName_;

    // table with old and new key names.
    static TranslateInfo keyTable[];
    static const int KEY_TABLE_SIZE;
    // method to find old key in the key table.
    virtual int findOldKey(const char* oldKey);
};


// Inline methods.
inline const vchar* convertPrefFile::GetDefaultFileName() {
    return DEFAULT_PREFERENCE_FILE;
}




#endif    // _convertPrefFile_h
