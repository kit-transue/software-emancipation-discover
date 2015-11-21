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

#include <gstate.h>

//----------------------------------------------------------------------

static int uniqueId=0;

NameListIterator::NameListIterator (StateGroup* sg, const vchar* nl)
{
    if (!nl) nl = (vchar*)"";
    nameList = vstrCloneScribed(vcharScribeSystem(nl));
    ptr = (char*)nameList;
    stateGroup = sg;
}

NameListIterator::~NameListIterator()
{
    if (nameList) vstrDestroy(nameList);
}

NameListIterator::next (int& stateNum, State& state)
{
    if (!*ptr) return 0;
    char* ptr1;

    char* comma = strchr (ptr, ',');
    if (comma) {
        *comma = 0;
        ptr1 = ptr;
        ptr = comma+1;
    } else {
        ptr1 = ptr;
        ptr = "";
    }
    char* equal = strchr(ptr1,'=');
    if (equal) {
        *equal = 0;
        state = (State) *(equal+1);
        stateNum = stateGroup->defineState ((vchar*) ptr1);
        return 1;
    }

    return 0;
}

Watcher::Watcher(StateGroup* sg,
                 const vchar* from, const vchar* to, const vchar* cmdstr,
                 const vchar* fcmdstr)
{
    int stateNum;
    State state;

    stateGroup = sg;

    if (!from) from = (vchar*) "";
    if (!to) to = (vchar*) "";

    fromCount = 0;
    NameListIterator iter1 (stateGroup, from);
    while (iter1.next (stateNum, state)) fromCount++;
    if (!fromCount) fromCount = -1;

    toCount = 0;
    NameListIterator iter2 (stateGroup, to);
    while (iter2.next (stateNum, state)) toCount++;
    if (!toCount) toCount = -1;


    fromNameList = vstrCloneScribed(vcharScribeSystem(from));
    toNameList = vstrCloneScribed(vcharScribeSystem(to));

    cmd = vstrCloneScribed(vcharScribeSystem(cmdstr));
    fcmd = vstrCloneScribed(vcharScribeSystem(fcmdstr));

    fluxstate = 0;
}

Watcher::~Watcher()
{
    if (fromNameList) vstrDestroy((vstr*)fromNameList);
    if (toNameList) vstrDestroy((vstr*)toNameList);
    if (cmd) vstrDestroy((vstr*)cmd);
    if (fcmd) vstrDestroy((vstr*)fcmd);
}

StateGroup::StateGroup (Tcl_Interp* interp)
{
    interp_ = interp;
}

StateGroup::~StateGroup ()
{

}

int StateGroup::defineState (const vchar* name)
{
    for (int i=0; i<names_.size(); i++) 
		if (vcharCompare (names_[i], name) == 0)
            return i;				
    oldstates_.append (NoState); 
    states_.append (NoState); 
    names_.append (vstrCloneScribed(vcharScribeSystem(name)));
    return names_.size()-1;
}

void StateGroup::changeState (const vchar* namelist)
{
    NameListIterator iter(this, namelist);
    int stateNum;
    State state;

    while (iter.next(stateNum,state)) {
        oldstates_[stateNum] = states_[stateNum];
        states_[stateNum] = state;
    }

    trigger();
}   


void StateGroup::trigger()
{
    int stateNum;
    State state;
    int triggerCount = 0;
  
    for (int i=0; i<watchers_.size(); i++) {
        int fromMatch = 0;
        int toMatch = 0;
        int fromCount = 0;
        int toCount = 0;
        int toChanged = 0;
        int fromChanged = 0;
 
        NameListIterator iter1(this, watchers_[i]->fromNameList);
        while (iter1.next (stateNum, state)) {
           int changeCount = 0;
           if (states_[stateNum] != oldstates_[stateNum]) {
               if (oldstates_[stateNum] == state) {
                    fromCount++;
                    fromMatch++;     
               }
           } else if (states_[stateNum] == state) fromMatch++;
        }

        NameListIterator iter2(this, watchers_[i]->toNameList);
        while (iter2.next (stateNum, state)) {
           int changeCount = 0;
           if (states_[stateNum] != oldstates_[stateNum]) {
               toChanged++;
               if (states_[stateNum] == state) {
                    toCount++;
                    toMatch++;
               }
           } else if (states_[stateNum] == state) toMatch++;
        }

        int eval = 0;
        switch (watchers_[i]->transition) {
        case From:
            if (fromCount && (watchers_[i]->fromCount == fromCount))
               eval = 1;
            break;
        case To:
            if (toCount && (watchers_[i]->toCount == toMatch))
               eval = 1;
            break;
        case Flux:
            if ((watchers_[i]->fluxstate == 0) && toCount){
                watchers_[i]->fluxstate = 1;
                eval = 1;
            } else if ((watchers_[i]->fluxstate != 0) && !toCount) {
                watchers_[i]->fluxstate = 0;
                eval = 2;
            }   
            break;
        case Change:
            if ((watchers_[i]->fromCount == fromMatch) &&
                (watchers_[i]->toCount == toCount))
               eval = 1;
            break;
        }

        if (eval == 1)
            Tcl_Eval (interp_, (char*)watchers_[i]->cmd);
        else if (eval == 2)
            Tcl_Eval (interp_, (char*)watchers_[i]->fcmd);
  }
}

void StateGroup::transitionTo	 (const vchar* toList, const vchar* cmd)
{
    Watcher* watcher = new Watcher(this, 0, toList, cmd);
    watcher->transition = To;
    watchers_.append (watcher);
}

void StateGroup::transitionFrom	 (const vchar* fromList, const vchar* cmd)
{
    Watcher* watcher = new Watcher(this, fromList, 0, cmd);
    watcher->transition = From;
    watchers_.append (watcher);
}

void StateGroup::transitionFlux	 (const vchar* toList, const vchar* tcmd,
                                  const vchar* fcmd)
{
    Watcher* watcher = new Watcher(this, 0, toList, tcmd, fcmd);
    watcher->transition = Flux;
    watchers_.append (watcher);
}

 

