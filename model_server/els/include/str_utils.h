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
#ifndef _str_utils_h
#define _str_utils_h

#ifndef _cLibraryFunctions_h
#include <cLibraryFunctions.h>
#endif

#define MAXBUF 256

/* extract string of the form <ch1>str<ch2>
   char* to should have enough room for resulting string
   if fail, return NULL, else return to */
char* extract_str(char* from, char ch1, char ch2, char* to);

/* extract num from a string a return pointer to the next char after num
   in from or NULL if fail */
char* extract_num(char* from, int& num);

/* return true if buf  has [END] as a substring */
bool is_end(char* buf);

/* strip leading and trailing spaces */
char* strip_spaces(char* from);

/* allocate enough memory for the new string and copy */
char* my_strcpy(char*& to, const char* from);

/* copy strings in stead of just copying pointers */
void copy_gen_arr(genArrCharPtr& to, const genArrCharPtr& from);

/* delete all strings in the array */
void clear_gen_arr(genArrCharPtr& arr);

/* get index of the string in the array. return -1 if not found */
int get_arr_index(const genArrCharPtr& arr, const char* str);

class genString;

bool get_next_line(FILE* file, char* line, int& LineNum);

class objArr;

void ParseAttributeList(const char* list, objArr& attributes,
			const char* FileName, int LineNum);

#endif
