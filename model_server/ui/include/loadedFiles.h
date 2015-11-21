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
#ifndef _loadedFiles_h
#define _loadedFiles_h

// loadedFiles.h
//------------------------------------------
// synopsis:
// 
// Loaded Files implementation
//------------------------------------------
#include <RTL.h>
class smtHeader;
typedef unsigned int uint;

class loadedFiles : public RTL
{
public:
friend class projNode;                 // so that projNode::proj_init() can see our instances
    define_relational(loadedFiles,RTL);
    loadedFiles(char*nm);
    ~loadedFiles();
//    void loadedFiles::notify(int, Relational*, objPropagator*, RelType*);

// proj_add_to_loaded  called from set_actual_mapping, from the steDocument
//   constructor, and anywhere else
//   that might know of a new loaded-file for us
    static void  proj_add_to_loaded(appPtr module);
    
// proj_remove_from_loaded used by app destructor, to remove files
// from both loaded-files and unparsed-files rtls
    static void  proj_remove_from_loaded(appPtr module);

// This call is used by anyone who might be changing the reparse state of
// one of the loaded files.  Call it with smtHeader of a loaded file.
    static void  change_reparse_flag(smtHeader*, uint);

#if 0
//  tell how many files are in the unparsed-files rtl
    static uint count_unparsed_files();
#endif
//  fetch the symbols from the unparsed-files rtl
    static void get_unparsed_files(symbolArr&);
    static void get_loaded_files(symbolArr&);

};


/*
   START-LOG-------------------------------------------
 
   $Log: loadedFiles.h  $
   Revision 1.5 2000/01/05 15:06:17EST sschmidt 
   Pruning more OODT stuff
 * Revision 1.4  1994/07/11  17:17:23  so
 * Bug track: n/a
 * project for re-load pdf
 *
 * Revision 1.3  1994/05/15  15:44:48  davea
 * Bug track: 7289
 * When starting a change propagation (from several places)
 * if there are any unparsed files, ask the user whether to
 * continue or cancel. Previously, it always gave a message
 * and canceled.
 *
 * Revision 1.2  1994/04/15  16:19:03  davea
 * bug 6649
 * total rewrite of loadedFiles class
 *
 * Revision 1.1  1993/07/12  15:58:04  dmitry
 * Initial revision
 *
   END-LOG---------------------------------------------
*/

#endif // _loadedFiles_h
