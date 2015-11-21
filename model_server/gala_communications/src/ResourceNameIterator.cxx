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
* ResourceNameIterator.C - Class to interate through tokens in a resource name.
*
***** Resource Name Format
*
* Resources names consist of 0 or more dictionaries followed by the
* name. The valid seperators are given in the Seperator_List
* list below.
*
**********/

#include <vport.h>
#include vstdioHEADER
#include vstrHEADER
#include vnumHEADER
#include "ResourceNameIterator.h"


// Class static list of valid seperators.
const vchar * ResourceNameIterator::Seperator_List = (const vchar*) ".:,";


// ResourceNameIterator Class Constructor (const vchar*).
ResourceNameIterator::ResourceNameIterator(const vchar* resourceName) {
    resourceName_ = vstrClone(resourceName);
    state_ = ResourceNameIterator::FIRST;
}


// ResourceNameIterator Class Constructor (const char*).
ResourceNameIterator::ResourceNameIterator(const char* resourceName) {
    resourceName_ = vstrClone(vnameInternGlobalLiteral(resourceName) );
    state_ = ResourceNameIterator::FIRST;
}

// ResourceNameIterator Class Destructor.
ResourceNameIterator::~ResourceNameIterator() {
    if (resourceName_) vstrDestroy(resourceName_);
}

// ResourceNameIterator method to get the next value.
const vname* ResourceNameIterator::GetNext() {
    return(next_);
}

// ResourceNameIterator method to determine if the interation is complete.
// Also updates token_ for GetNext.
vbool ResourceNameIterator::Next() {
    vbool status = vTRUE;

    switch (state_) {
        case ResourceNameIterator::FIRST	:
            token_ = vcharGetFirstToken(resourceName_,
                                        ResourceNameIterator::Seperator_List);
            break;
        case ResourceNameIterator::NEXT:
            token_ = vcharGetNextToken(ResourceNameIterator::Seperator_List);
            break;
        case ResourceNameIterator::DONE	:
            break;
    }

    if (token_) {
        next_ = vnameInternGlobal(token_);
        state_ = ResourceNameIterator::NEXT;
    } else {
        state_ = ResourceNameIterator::DONE;
        status = vFALSE;
    }

    return(status);
}



///////////     No ResourceNameIterator class Private methods.     //////////







//////////     End of ResourceNameIterator.cxx     //////////
