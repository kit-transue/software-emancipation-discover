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
#ifndef _TclList_h
#define _TclList_h

#include <tcl.h>
#include <machdep.h>
#include <genString.h>
#include <general.h>

/**********************************************************
 *
 * CLASS: TclList                                     
 *                                                    
 *        Manage and inspect Tcl list strings.
 *                                                  
 *        (Many of the following declarations do not  
 *        contain `const' specifiers because the      
 *        underlying Tcl routines are not declared 
 *        with `const'.)
 *                                                    
 * [abover, 09/04/97]                                 
 *
 **********************************************************/

class TclList
{

  public:

    TclList()                          { Init(); }
    TclList(char const *cp)            { Init(); Append(cp); }
    ~TclList()                         { Tcl_DStringFree(&dstr); 
                                         if (argv) Tcl_Free((char *)argv); }

    char *Append(char const *cp, int len)    { Synch(true); return Tcl_DStringAppend(&dstr, cp, len); }
    char *AppendElement(char const *cp)      { Synch(true); return Tcl_DStringAppendElement(&dstr, cp); }
    char *Value() const                { return Tcl_DStringValue(&dstr); }
    void StartSublist()                { Tcl_DStringStartSublist(&dstr); Synch(true); }
    void EndSublist()                  { Tcl_DStringEndSublist(&dstr); Synch(true); }
    int Length()                       { return Tcl_DStringLength(&dstr); }
    void SetLength(int len)            { Tcl_DStringSetLength(&dstr, len); Synch(true); }
    void Result(Tcl_Interp *interp)    { Tcl_DStringResult(interp, &dstr); }
    void GetResult(Tcl_Interp *interp) { Tcl_DStringGetResult(interp, &dstr); }

    int SplitList(int &, char const **&);    // caller must free second parameter; see below
    void Init();                       // see .C file
    bool Validate()                    { if (SynchList()) return true; else return false; }
    int Size()                         { if (SynchList()) return argc; else return -1; }
    char *Append(char const *cp)             { return Append(cp, OSapi_strlen(cp)); }

    TclList &operator+=(char const *cp)      { AppendElement(cp); Synch(true); return *this; }
    int operator==(char const *cp) const     { return (OSapi_strcmp(Value(), cp) == 0); }
    int operator!=(char const *cp) const     { return (OSapi_strcmp(Value(), cp) != 0); }
    operator char*() const             { return Value(); }
    operator const char*() const       { return Value(); }
    char const *operator[](int index)        { return Index(index); }
    TclList &operator=(char const *);        // see .C file

  private:
    // disallow copy:
    TclList(TclList const &);
    TclList &operator=(TclList const &);

    // Synchronization routines to efficiently
    // store and access the list as a vector:

    void Synch(bool flag)              { if (synch != flag) synch = flag; }
    bool Synch() const                 { return synch; }
    bool SynchList();                  // see .C file
    char const *Index(int);                  // see .C file

    Tcl_DString dstr;
    bool synch;                // SynchList() required before accessing as array.
    int argc;
    char const **argv;
};

inline int TclList::SplitList(int &_argc, char const **&_argv)
{
  return Tcl_SplitList(NULL, Value(), &_argc, &_argv);   // caller must free _argv
}

#endif /* !_TclList_h */
