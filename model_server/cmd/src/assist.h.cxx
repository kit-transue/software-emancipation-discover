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
#include <cLibraryFunctions.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstdlib>
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#include <genError.h>
#include <genArr.h>
#include <customize.h>
#include <assist.h>

genArrOf(assist_line_ptr) assist_lines_array;

int *hash_indexes = NULL;
int db_loaded     = 0;

int assist_hash_key(char *txt){
    int hash_key = (int)txt[0]; // Take first letter
    hash_key   <<= 24;          // Move it to higest byte
    char *p      = strchr(txt, '.');
    if(p){
	p++;
	int tmp   = p[0];      // Add to hash next letter after first \'.\'
	tmp     <<= 16;
	hash_key |= tmp;
    }
    hash_key |= strlen(txt);
    return hash_key;
}

void load_assist_database(void)
{
    Initialize(load_assist_database);
   
    if(db_loaded)
	return;

    genString file_name = customize::install_root();
    file_name          += "/lib/assist_line.dat";
    FILE *fl = OSapi_fopen((char *)file_name, "r");
    if(fl == NULL)
	return;
    char *buffer = new char[1024];
    Assert(buffer);
    while(OSapi_fgets(buffer, 1024, fl) != NULL){
	int len = strlen(buffer);
	if(buffer[len - 1] == '\n'){
	    buffer[len - 1] = 0;
	    len--;
	}
	assist_line *line_entry = new assist_line;
	Assert(line_entry);
	line_entry->key         = new char[len + 1];
	Assert(line_entry->key);
	strcpy(line_entry->key, buffer);

	if(OSapi_fgets(buffer, 1024, fl) != NULL){
	    len = strlen(buffer);
	    if(buffer[len - 1] == '\n'){
		buffer[len - 1] = 0;
		len--;
	    }
	    line_entry->line = new char[len + 1];
	    Assert(line_entry->line);
	    strcpy(line_entry->line, buffer);
	}else
	    line_entry->line = NULL;
	assist_lines_array.append(&line_entry);
    }
    delete [] buffer;
    OSapi_fclose(fl);
    hash_indexes = new int[assist_lines_array.size()];
    Assert(hash_indexes);
    for(int index = assist_lines_array.size() - 1; index >= 0; index--){
	assist_line_ptr entry = *assist_lines_array[index];
	int hash = assist_hash_key(entry->key);
	hash_indexes[index] = hash;
    }
    db_loaded = 1;
}

char *search_assist_database(char *key)
{
    Initialize(search_assist_database);

    int hash_key = assist_hash_key(key);
    for(int index = assist_lines_array.size() - 1; index >= 0; index--){
	if(hash_indexes[index] = hash_key){
	    assist_line_ptr entry = *assist_lines_array[index];
	    if(!strcmp(entry->key, key)){
		return entry->line;
	    }
	}
    }
    return NULL;
}
