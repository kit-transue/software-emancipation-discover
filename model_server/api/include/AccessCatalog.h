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
#ifndef _AccessCatalog_h
#define _AccessCatalog_h

#include <general.h>
#include <genString.h>
#include <Interpreter.h>
#include <scopeMgr.h>
#include <genArrCharPtr.h>

//
// CLASS: AccessCatalog
//
// Generic interface to the low-level set management
// routines of certain Access "catalog" commands.
// Defines subcommands and processes command lines.
//
// e.g.: % foo new bar                 # uses AppendItemCmd
//         => bar
//       % foo info                    # uses ListAllCmd
//         => x1 x2 bar {a bo}
//       % foo exists bar1             # uses FindItemCmd
//         => 0
//

class AccessCatalog
{
  public:
    virtual ~AccessCatalog() {}

    typedef int (AccessCatalog::*HookMP)(int, char const *[], genString &);
    typedef char const * HookCmd;
    class Hook { 
      public:
        Hook(HookCmd, HookMP);
        HookCmd cmd;
        HookMP mp;
    };
    genArr(Hook);

    virtual bool InitHooks();
    virtual bool AppendHook(HookCmd, HookMP);

    virtual int ProcessCommand(ClientData, Tcl_Interp *, int, char const *[]);
    virtual bool MainUsage(genString &, char const *);

    virtual int AppendItemCmd(int, char const *[], genString &);
    virtual int FindItemCmd(int, char const *[], genString &);
    virtual int ListAllCmd(int, char const *[], genString &);
    virtual int RemoveItemCmd(int, char const *[], genString &);

    virtual bool Append(Relational *);
    virtual Relational *Find(char const *);
    virtual bool Remove(char const *);

    bool FillArr(objArr &, char const *);
    static bool FillArr(AccessCatalog *, objArr &, char const *);
    static int FillArr(AccessCatalog *, objArr &, genArrCharPtr &, genArrCharPtr &, bool = true);

  protected:

    genArrOf(Hook) hooks;
    objNameSet set;
    Interpreter *interpreter;

};

#endif /* !_AccessCatalog_h */
