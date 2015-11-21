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
#include <stdlib.h>
#include <string.h>

#include "CmXmlStringTokenizer.h"


const int CmXmlStringTokenizer::ONE_SIDE_TOKEN      = 0;
const int CmXmlStringTokenizer::BOTH_SIDES_TOKEN    = 1;


CmXmlStringTokenizer::CmXmlStringTokenizer(const char *str, const char *delim, int mode) {

    this->str = strdup(str);
    this->delim = strdup(delim);
    this->mode = mode;
    this->curPtr = this->str;

} //constructor


CmXmlStringTokenizer::~CmXmlStringTokenizer() {

    free(this->str);
    free(this->delim);

} //destructor


char* CmXmlStringTokenizer::nextToken(int *begNdx) {

    char *token = NULL;

    if(this->mode == CmXmlStringTokenizer::ONE_SIDE_TOKEN) {
        token = nextToken1();
    } else if(this->mode == CmXmlStringTokenizer::BOTH_SIDES_TOKEN) {
        token = nextToken2();
    }

    // Calculate index of the beginning of the token
    if(begNdx != NULL) {
        *begNdx = (token != NULL) ? (int)(token - this->str) : -1;
    }

    return token;

} //nextToken


int CmXmlStringTokenizer::howManyTokensLeft() {

    int amount = 0;

    if(this->mode == CmXmlStringTokenizer::ONE_SIDE_TOKEN) {
        amount = howManyTokensLeft1();
    } else if(this->mode == CmXmlStringTokenizer::BOTH_SIDES_TOKEN) {
        amount = howManyTokensLeft2();
    }

    return amount;

} //howManyTokensLeft


char* CmXmlStringTokenizer::nextToken1() {

	// Check arguments
	if(this->curPtr == NULL || this->delim == NULL) {
		return NULL;
	}

    // Token starts from current position
    char *token = this->curPtr;

	// Find first occurence of one of the delimeters
	this->curPtr = strpbrk(this->curPtr, this->delim);

	// If found -> place '\0' instead of delimeter and 
	//    go to the beginning of the next token.
	if(this->curPtr != NULL) {
		this->curPtr[0] = 0;
		this->curPtr++;
	}

	// Return pointer to the beginning of the current token
	return token;

} //nextToken1


char* CmXmlStringTokenizer::nextToken2() {

	// Check arguments
	if(this->curPtr == NULL || this->delim == NULL) {
		return NULL;
	}

	// Try to find the beginning of the next token
	char *token = strpbrk(this->curPtr, this->delim);

    // There are no more tokens
    if(token == NULL) {
        this->curPtr = NULL;
        return NULL;
    }

    // This character is used as boundary for this token
    char bound = token[0];
    token++;

    // This is expected to be a right boundary of this token
    char *endPtr = strchr(token, bound);
    if(endPtr == NULL) {
        this->curPtr = NULL;
        return NULL;
    }

    // Move further behind the current token
    endPtr[0] = 0;
    this->curPtr = endPtr + 1;

    return token;

} //nextToken2


int CmXmlStringTokenizer::howManyTokensLeft1() {

	// Check arguments
	if(this->curPtr == NULL || this->delim == NULL) {
		return 0;
	}

    // Number of tokens equals to number of delimeters plus 1
    int amount = 0;
    for(char *ptr = this->curPtr; ptr != NULL; ptr = strpbrk(ptr + 1, this->delim)) {
        amount++;
    }

    return amount;

} //howManyTokensLeft1


int CmXmlStringTokenizer::howManyTokensLeft2() {

	// Check arguments
	if(this->curPtr == NULL || this->delim == NULL) {
		return 0;
	}

    // Number of tokens equals number of pair of delimiters
    int amount = 0;
    for(char *ptr = this->curPtr; ptr != NULL; ptr = strpbrk(ptr, this->delim)) {
        char bound = *ptr++;
        char *endPtr = strchr(ptr, bound);
        if(endPtr == NULL) {
            break;
        } else {
            amount++;
            ptr = endPtr + 1;
        }
    }

    return amount;

} //howManyTokensLeft2


// END OF FILE
