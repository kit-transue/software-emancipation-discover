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
#include <string.h>

#include "CmXmlNode.h"
#include "CmXmlCollection.h"

// Number of new slots when array are expanded
const int CmXmlCollection::increment = 4;


CmXmlCollection::CmXmlCollection(int initial_size) {

	int n = (initial_size > 0) ? initial_size
							   : CmXmlCollection::increment;
	
	this->arr = new CmXmlNode* [n];
	this->slots = n;
	this->amount = 0;

	for(int i = 0; i < n; i++) {
		this->arr[i] = NULL;
	}

} //constructor


CmXmlCollection::~CmXmlCollection() {

	clear();
	delete [] this->arr;

} //destructor


int CmXmlCollection::add(CmXmlNode *node) {

	// Allocate additional slots
	if(this->amount == this->slots) {
		// Allocate new array
		int new_slots = this->slots + CmXmlCollection::increment;
		CmXmlNode** new_arr = new CmXmlNode* [new_slots];

		// Copy contents of the curent array to the new array
		for(int i = 0; i < this->slots; i++) {
			new_arr[i] = this->arr[i];
		}

		// Nulling all empty slots
		for(int j = this->slots; j < new_slots; j++) {
			new_arr[j] = NULL;
		}

		// Free old array
		delete [] this->arr;

		this->arr = new_arr;
		this->slots = new_slots;
	}

	// Add this node to the array
	int n = this->amount;
	this->arr[n] = node;
	this->amount++;

	// Return index of this node
	return n;

} //add


void CmXmlCollection::clear() {

	// Free all elements of the collection
	for(int i = 0; i< this->amount; i++) {
		delete this->arr[i];
		this->arr[i] = NULL;
	}

	// There are no elements in the collection, but still a lot of free slots.
	this->amount = 0;

} //clear


CmXmlNode* CmXmlCollection::get(int index) {

	// Get element with the given index
	if(index >= 0 && index < this->amount) {
		return this->arr[index];
	}

	// Nothing was found
	return NULL;

} //get


CmXmlNode* CmXmlCollection::get(char *key) {

    if(key == NULL) {
        return NULL;
    }

	// Searching for element with the given key
	for(int i = 0; i < this->amount; i++) {
		CmXmlNode *element = this->arr[i];
		if(strcmp(element->getKey(), key) == 0) {
			return element;
		}
	}

	// Nothing was found in the collection
	return NULL;

} //get


int CmXmlCollection::size() {

	return this->amount;

} //size


// END OF FILE
