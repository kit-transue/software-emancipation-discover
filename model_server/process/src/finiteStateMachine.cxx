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
#include <finiteStateMachine.h>
#include <psetmem.h>

finiteStateMachine::finiteStateMachine(int * * FSM_definition)
{
    constructMachine(FSM_definition, this);
}

finiteStateMachine::finiteStateMachine(int FSM_definition [][4], int size)
{
    size /= (4*sizeof(int));
    int * * mappedAddrs = (int * *)psetmalloc(size*sizeof(int *));
    if (mappedAddrs && size>1) {
	for (int i=0; i<size; i++)
	    mappedAddrs[i] = &FSM_definition[i][0];
	constructMachine(mappedAddrs, this);
	free((char *)mappedAddrs);
    } else
	error = -1;
}

finiteStateMachine::~finiteStateMachine()
{
    free((char *)Table);
    free((char *)actions);
    free((char *)alphabet);
    free((char *)states);
    if (actionFunctions)
	free((char *)actionFunctions);
}

int finiteStateMachine::doFunction(int A, int B, void *C)
{
    int ret_val;

    if (actionFunctions)
	ret_val = (*(actionFunctions[actions[action]]))(A, B, C);
    else
	ret_val = error = -1;

    return ret_val;
}

int finiteStateMachine::doFunction(int associated_action, int A, int B, void *C)
{
    int ret_val;

    if (actionFunctions && associated_action>=0 &&
	associated_action<=actions_max)
	ret_val = (*(actionFunctions[actions[associated_action]]))(A, B, C);
    else
	ret_val = error = -1;

    return ret_val;
}

int finiteStateMachine::setState(int state_to_set)
{
    int ret_val;

    if (state_to_set<0 || state_to_set>=states_max) 
	ret_val = error = -1;
    else
	ret_val = state = state_to_set;

    return ret_val;
}

int finiteStateMachine::initActionFunctions()
{
    int ret_val;

    if (actionFunctions)
        free((char *)actionFunctions);

    if (actionFunctions = (FSM_actionFunction *)
	psetmalloc( sizeof(FSM_actionFunction)*num_of_actions ))
	ret_val = 0;
    else
	ret_val = error = -1;

    return ret_val;
}

int finiteStateMachine::retrieveBounds(int * * definition, int columns, int delimiter,
				       int * maximum)
{
    int ret_val = -1;

    if (definition && maximum && columns>0) {
	int counter;
	for (counter = 0; counter<columns; counter++)
	    maximum[counter] = definition[0][counter];

	for (int done=0, row=0; !done; row++)
	    for (counter=0; counter<columns && !done; counter++) {
		done = done || definition[row][counter]==delimiter;
		if (maximum[counter]<definition[row][counter] && !done)
		    maximum[counter]=definition[row][counter];
	    }
	ret_val = 0;
    }

    return ret_val;
}

int finiteStateMachine::retrieveIndex(int * * definition, int delimiter, int column, 
				      int * target, int target_size)
{
    int ret_val = -1;

    if (definition && target && column>=0) {
	int index;
	int i;
	for (i=0; (index = definition[i][column]) != delimiter; i++)
	    if (index>=0)
		target[index] = index;

	int map = 0;
	for (i = 0; i < target_size; i++)
	    if (target[i] != delimiter) {
		target[i] = map;
		map++;
	    }

	for (i=0; i<target_size; i++)
	    if (target[i] == delimiter)
		target[i] = map;
	ret_val = map+1;
    }

    return ret_val;
}

int finiteStateMachine::allocateAndInitializeIndex(int * * target, int size, int init)
{
    int ret_val = -1;

    if (target) {
	*target = (int *)psetmalloc( size*sizeof(int) );
	ret_val = *target ? 0 : -1;
	for (int i=0; !ret_val && i<size; i++)
	    (*target)[i] = init;
    }

    return ret_val;
}

void finiteStateMachine::allocateAndInitializeTable( int * * definition,
						     finiteStateMachine * instance)
{
    int size = instance->num_of_states * instance->alphabet_size;
    FSMtableElement * table = (FSMtableElement *)psetmalloc( size*sizeof(FSMtableElement) );

    int i;
    for (i = 0; i<size; i++)
	table[i].next_state = table[i].action = FSM_END_OF_DEFINITION;

    int ndx, on, next_state, cur_action, seg; 
    for (i=0; definition[i][0] != FSM_END_OF_DEFINITION && table; i++) {
	on  = definition[i][1];
	seg = instance->states[definition[i][0]]*instance->alphabet_size;

	next_state = definition[i][2];
	cur_action = definition[i][3];

	if (on == FSM_ALPHABET_COMPLEMENT) {
	    int next_seg = seg+instance->alphabet_size;
	    for (int j=seg; j<next_seg; j++)
		if (table[j].next_state == FSM_END_OF_DEFINITION) {
		    table[j].next_state = next_state;
		    table[j].action = cur_action;
		}
	} else {
	    ndx = (on<0 || on>instance->alphabet_max) ? 
		(seg+instance->alphabet_size-1) : (seg+instance->alphabet[on]);
	    table[ndx].next_state = next_state;
	    table[ndx].action = cur_action;
	}
    }

    instance->Table = table;
}

void finiteStateMachine::constructMachine(int * * FSM_definition, finiteStateMachine * instance)
{
    int maximum[4];
    int ret_bound = retrieveBounds( FSM_definition, 
				    4, 
				    FSM_END_OF_DEFINITION, 
				    maximum );

    if (ret_bound == 0 && maximum[3] <= maximum[1]) {
	instance->states_max = maximum[0];
	instance->alphabet_max = maximum[1];
	instance->actions_max = maximum[3];
	
	allocateAndInitializeIndex( &instance->alphabet, 
				    instance->alphabet_max+1,
				    FSM_END_OF_DEFINITION );
	allocateAndInitializeIndex( &instance->states, 
				    instance->states_max+1, 
				    FSM_END_OF_DEFINITION );
	allocateAndInitializeIndex( &instance->actions, 
				    instance->actions_max+1, 
				    FSM_END_OF_DEFINITION );

	instance->num_of_states = retrieveIndex( FSM_definition, 
						 FSM_END_OF_DEFINITION, 
						 0, 
						 instance->states, 
						 instance->states_max+1 );
	instance->num_of_actions = retrieveIndex( FSM_definition, 
						  FSM_END_OF_DEFINITION, 
						  3, 
						  instance->actions, 
						  instance->actions_max+1 );
        instance->alphabet_size = retrieveIndex( FSM_definition, 
						 FSM_END_OF_DEFINITION, 
						 1, 
						 instance->alphabet, 
						 instance->alphabet_max+1 );

	allocateAndInitializeTable( FSM_definition, instance );

	instance->error = instance->action = instance->state = instance->alpha = 0;
	instance->actionFunctions = (FSM_actionFunction *)0;
    } else
	instance->error = -1;
}

/*
    START-LOG-------------------------------

    $Log: finiteStateMachine.cxx  $
    Revision 1.3 2000/07/07 08:12:34EDT sschmidt 
    Port to SUNpro 5 compiler
 * Revision 1.3  1994/03/28  14:20:18  kws
 * psetmalloc - compliance
 *
 * Revision 1.2  1994/02/04  05:20:52  pero
 * Problem: Negative array index (FSM_ALPHABET_COMPLEMENT)
 *
 * Revision 1.1  1994/01/26  14:52:53  pero
 * Initial revision
 *

*/
