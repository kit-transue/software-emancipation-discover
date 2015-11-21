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
#ifndef _finiteStateMachine

#define FSM_ALPHABET_COMPLEMENT  -1
#define FSM_END_OF_DEFINITION    -2

typedef struct {
    int next_state;
    int action;
} FSMtableElement;

typedef int (* FSM_actionFunction)(int, int, void *);

class finiteStateMachine {
  public:
    finiteStateMachine(int * *);
    finiteStateMachine(int [][4], int);
    ~finiteStateMachine();

    inline void next(int);
    inline void next(int, int &, int &);
    inline int getAction();
    inline int getState();

    int setState(int);

    int initActionFunctions();
    inline void associateFunction(FSM_actionFunction, int);
    int doFunction(int, int, void *);
    int doFunction(int, int, int, void *);

    inline int getStatus();
    inline void setStatus(int);

  private:
    inline void setTableIndex(int);

    static int retrieveBounds(int * *, int, int, int *);
    static int retrieveIndex(int * *, int, int, int *, int);
    static int allocateAndInitializeIndex(int * *, int, int);
    static void allocateAndInitializeTable(int * *, finiteStateMachine *);
    static void constructMachine(int * *, finiteStateMachine *);

    int * alphabet;
    int alphabet_max;
    int alphabet_size;
    int alpha;

    int * states;
    int states_max;
    int state;
    int num_of_states;

    int * actions;
    int actions_max;
    int action;
    int num_of_actions;
    FSM_actionFunction *actionFunctions;

    int table_index;
    FSMtableElement * Table;

    int error;
};

void finiteStateMachine::setTableIndex(int input)
{
    if (input>alphabet_max || input<0 || state<0 || state>states_max)
	error = 1;
    else {
	alpha = alphabet[input];
	table_index = states[state]*alphabet_size+alpha;
    }
}

void finiteStateMachine::next(int input)
{
    setTableIndex(input);
    if (!error) {
	state = Table[table_index].next_state;
	action = Table[table_index].action;
    }
}

void finiteStateMachine::next(int input, int & next_state, int & next_action)
{
    setTableIndex(input);
    if (!error) {
	next_state = state = Table[table_index].next_state;
	next_action = action = Table[table_index].action;
    }
}

int finiteStateMachine::getAction()
{
    return action;
}

int finiteStateMachine::getState()
{
    return state;
}

void finiteStateMachine::associateFunction(FSM_actionFunction fp, int associated_action)
{
    if (associated_action<0 || associated_action>actions_max ||
	!actionFunctions)
	error = -1;
    else
	actionFunctions[actions[associated_action]] = fp;
}

int finiteStateMachine::getStatus()
{
    return error;
}

void finiteStateMachine::setStatus(int error_code)
{
    error = error_code;
}

#endif
/*
    START-LOG-------------------------------

    $Log: finiteStateMachine.h  $
    Revision 1.1 1994/01/26 10:25:28EST builder 
    made from unix file
 * Revision 1.1  1994/01/26  14:57:20  pero
 * Initial revision
 *

*/
