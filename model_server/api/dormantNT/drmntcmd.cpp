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
#include "autosubsys.h"
#include "tcl.h"
#include "Interpreter.h"
#include "projList.h"
#include "globdef.h"

// --------------------- Dormant Analysis for NT -----------------------------
#ifdef _WIN32
#define DEFWT(a,b,c,d,e)                \
   defaultweights[a].outgoing = b;              \
   defaultweights[a+NWEIGHTS/2].outgoing=c;     \
   defaultweights[a].count = e;         \
   defaultweights[a+NWEIGHTS/2].count=e;\
   defaultweights[a].m_weight=d;

static void setupdefaults() {
  static int isinitialized;
  if (isinitialized) return;
  DEFWT(AUS_FCALL,      true,   false,  10, -1);
  DEFWT(AUS_DATAREF,    true,   false,  10, -1);
  DEFWT(AUS_INSTANCE,   true,   false,  10, -1);
  DEFWT(AUS_ARGTYPE,    true,   false,  10, -1);
  DEFWT(AUS_RETTYPE,    true,   false,  10, -1);
  DEFWT(AUS_ELEMENT,    false,  true,   10, -1);
  DEFWT(AUS_FRIEND,     false,  false,  15, -1);
  DEFWT(AUS_SUBCLASS,   true,   false,  10, -1);
  DEFWT(AUS_MEMBER,     true,   false,  15, -1);
  DEFWT(AUS_FILENAME,   false,  false,  10, 0);
  DEFWT(AUS_LOGICNAME,  false,  false,  10, 0);
  isinitialized=1;
}
#endif

static int RunDormantCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#ifdef _WIN32
    Interpreter *i = (Interpreter *)cd;
    symbolArr roots;
    symbolArr scope;

        if(i->ParseArgString((char *)argv[1], roots)!=0)  return TCL_ERROR;
        if(i->ParseArgString((char *)argv[2], scope)!=0)  return TCL_ERROR;


    // Find all projnodes in input and set them up as domain
    for(int ii = roots.size() - 1; ii >= 0; ii--){
            symbolPtr sym = roots[ii];
            if(sym.get_kind() == DD_PROJECT){
                Relational *rp = (Relational *)sym;
                projNode *pn   = (projNode *)rp;
                if(pn){
                        projList::domain_add(pn->get_name());
                        }
                roots.remove(sym);
                }
    }
    setupdefaults();
    decomposer *d = new decomposer;
    d->go_deadcode_only(roots, defaultweights, NWEIGHTS, TRUE, &scope);
    Tcl_SetResult(i->interp, "", TCL_VOLATILE);
#endif
    return TCL_OK;
}
// ----------------------------------------------------------------------------

static int add_dormant_cmd() 
{
  //--------------- Dormant for NT -------------------
  new cliCommandInfo("dis_run_dormant", RunDormantCmd);
  //--------------------------------------------------
  return 0;
}

static volatile int var_to_add_dormant_command_to_the_list_of_commands = add_dormant_cmd();


