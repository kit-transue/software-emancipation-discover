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
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <genArr.h>
#include <Hash.h>

#ifdef _WIN32
#include <io.h>
#define STAT _stat 
#else
#include <unistd.h>
#define STAT stat
#define _MAX_PATH 260
#endif

#define MAXTOKEN 128

struct position {
	int pos;
	int line_num;	/* line_num stores line number of instance in the .grp or .attr file, start from 1 */ 
	int attr; //only used in ATTR
};

genArr(position);

class mrg_object : public namedObject {
public:
	char *tga_buf;		//tga_buf will point to the memory, not the file name 
	genArrOf(position) pos_arr;
	mrg_object(char* name): namedObject(name) {	tga_buf = 0; }
	~mrg_object()	{	if(tga_buf)	delete tga_buf;	}
};

typedef mrg_object *pObject;
genArr(pObject);

class mrgHash : public nameHash
{
	virtual const char* name(const Object* oo) const;	
};

int GetOffset(char **p);
int GetLength(char **p);
void print_string(char* pStart, char* pEnd, FILE* out_file); 
void print_delta(char **p, FILE* out_file);
int GetInsertStringSize(char **p);
int GetNewPosition(int pos, char **p/*, int& num, int& offset*/); 
//void do_merge(char* src, char* delta, FILE* out_file);
void do_merge(char* src, char* delta, FILE* out_file, int flag_comment, char *comment,int flag_ifdef, char *def);
//int apply_merge(char* src_name, char* delta_name);
int apply_merge(char* src_name, char* delta_name, FILE* out_file, int flag_comment, char *comment,int flag_ifdef, char *def);
int apply_mapP_to_P(int n, char** p, char *delta_name);
int apply_mapL_to_P(int n, int* pLine, int *pPos, char *src_name, char *delta_name);
int apply_mapL_to_L(int n, int* pOldLine, int* pLine, char *src_name, char *delta_name, char *target_name, int instances_flag);
int sort_pos( const void *arg1, const void *arg2 );
int sort_linenum( const void *arg1, const void *arg2 );
int sort_line( const void *arg1, const void *arg2 );
void GetNewPositionArray(position* , int size, char* index);
int hash_file(char* fname, genArrOf(pObject)  &objPtr_arr, FILE* flog);
char* gen_array_provide(char*, int len, int& capacity, int objsize);
int update_group(char *fgroup, genArrOf(pObject) &objPtr_arr, FILE* flog);
int update_attribute(char *fattr, genArrOf(pObject) &objPtr_arr, FILE* flog);
int GAT_list_update(char* fname, genArrOf(pObject) &objPtr_arr, FILE* flog);   
