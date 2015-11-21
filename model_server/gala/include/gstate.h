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
/**********
 *
 * gstate.h - Watcher and StateGroup Classes.
 *
 **********/

#ifndef _gstate_h
#define _gstate_h

#include <vport.h>
 
#ifndef vmemINCLUDED
   #include vmemHEADER
#endif
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vstrINCLUDED
   #include vstrHEADER
#endif
#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif

#include "gArray.h"

#ifndef _TCL
   #include "tcl.h"
#endif

#define NoState 0

typedef int State;

enum Transition {
    From,
    To,
    Flux,
    Change
};

//----------------------------------------------------------------------

typedef const vstr* StateName;
    
//----------------------------------------------------------------------

class Watcher {
friend class StateGroup;

public:
    Watcher(StateGroup*, const vchar* from, const vchar* to, 
        const vchar* cmdstr, const vchar* fcmdstr = 0);
    ~Watcher();

private:
    Transition transition;
    int fromCount;
    int toCount;
    const vstr* toNameList;
    const vstr* fromNameList;
    const vstr* cmd;
    const vstr* fcmd;
    int fluxstate;
    StateGroup* stateGroup;
};

typedef Watcher* WatcherPtr;

gArray(StateName);
gArray(State);
gArray(WatcherPtr);


//----------------------------------------------------------------------


class StateGroup {
  public:

    StateGroup(Tcl_Interp*);
    ~StateGroup();

    int defineState(const vchar*);
    
    void transitionTo	 (const vchar* toList, const vchar* cmd); 
    void transitionFrom	 (const vchar* fromList, const vchar* cmd);
    void transitionFlux  (const vchar* toList, const vchar* fluxcmd,
                          const vchar* flatlinecmd);
    void transitionChange(const vchar* fromList,const char* toList,
                          const vchar* cmd); 
 
    void changeState	 (const vchar* namelist);
    int checkState	     (const vchar* namelist);

    void trigger         ();
    
    Tcl_Interp* interp   ();

  private:
    Tcl_Interp* interp_;
    gArrayOf(StateName) names_;
    gArrayOf(State) states_;
    gArrayOf(State) oldstates_;
    gArrayOf(WatcherPtr) watchers_;
};

class NameListIterator {
public:
    NameListIterator(StateGroup* stateGroup, const vchar* namelist);
    ~NameListIterator();

    next (int& stateNum, State& state);

private:
    vstr* nameList;
    char* ptr;
    StateGroup* stateGroup;
};

#endif

