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
* SelectionIterator.C - Class to interate through an RTL selection.
*
***** Selection Format
*
* Selections have comma seperated tokens which can either be single
* values or ranges: "1,3,5" or 1, 2-7,14".
*
**********/

#include <vport.h>
#include vstdioHEADER
#include vstrHEADER
#include vnumHEADER
#include "SelectionIterator.h"


// SelectionIterator Class Constructor.
SelectionIterator::SelectionIterator(vchar* selection) {
    selection_ = vstrClone(selection);
    state_ = SelectionIterator::FIRST;
}
SelectionIterator::SelectionIterator(char* selection) {
    selection_ = vstrClone((vchar*)selection);
    state_ = SelectionIterator::FIRST;
}

// SelectionIterator Class Destructor.
SelectionIterator::~SelectionIterator() {
    if (selection_) vstrDestroy(selection_);
}

// SelectionIterator method to get the next value.
int SelectionIterator::GetNext() {
    return(next_);
}

// SelectionIterator method to determine if the interation is complete.
vbool SelectionIterator::Next() {
    vbool status = vTRUE;
    switch (state_) {
        case SelectionIterator::FIRST	:
            token_ = vcharGetFirstToken(selection_, ",");
            state_ = (SelectionIterator::IteratorState) handleToken(token_);
            break;
        case SelectionIterator::SIMPLE_TOKEN:
            token_ = vcharGetNextToken(",");
            state_ = (SelectionIterator::IteratorState) handleToken(token_);
            break;
        case SelectionIterator::RANGE_TOKEN	:
            state_ = (SelectionIterator::IteratorState) getNextFromRange();
            break;
        case SelectionIterator::DONE	:
            break;
    }

    if (state_ == SelectionIterator::DONE) status = vFALSE;

    return(status);
}



///////////     SelectionIterator class Private methods.     //////////


int SelectionIterator::handleToken(vchar* token)
{
    IteratorState state = SelectionIterator::SIMPLE_TOKEN;  

    if (token == NULL) {
        state = SelectionIterator::DONE;
    } else {
        vchar* remStr;
        vexWITH_HANDLING {
            next_ = vnum::ParseInt(token, &remStr);
            if ((char) remStr[0] == '-') {    // Is a Range token.
                state = SelectionIterator::RANGE_TOKEN;  
                rangeFirst_ = next_;
                rangeIndex_ = rangeFirst_ + 1;
                // Note - the hyphen will make this look negative.
                rangeLast_ = -(vnum::ParseInt(remStr, NULL) ); 
                if (rangeIndex_ > rangeLast_)
                    state = SelectionIterator::SIMPLE_TOKEN;
            }
        }
        vexON_EXCEPTION {
            if (vexGetMathException()) {    // Error parsing for a number.
                state = SelectionIterator::DONE;
                printf("Error parsing Selection Iterator!!!\n");
            } else    // Handle the expection.
                vexPropagate(vexGetRootClass(), vexNULL);
        } vexEND_HANDLING;
    }

    return(state);
}


int SelectionIterator::getNextFromRange()
{
    IteratorState state = SelectionIterator::RANGE_TOKEN;
    next_ = rangeIndex_++;
    if (rangeIndex_ > rangeLast_)
        state = SelectionIterator::SIMPLE_TOKEN;

    return(state);
}





//////////     End of SelectionIterator.cxx     //////////
