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
#include "mrg_update.h"
#include <genString.h>

void get_delta(char **p, genString& insert_str)
{
	char *index = *p;
	int c;

	while((c=*index) == ' ' || c == '\t' || c == '\n' || c == ',')
		index++;

	int delimit = c;

	index++;
	char *pEnd = strchr(index,delimit);  

	char *pNewLine;
	genString temp_str;
	int len;
	while( pNewLine = strchr(index,'\n')) {
		if( pNewLine > pEnd ) break;
		len = pNewLine + 1 - index;
		temp_str.put_value(index, len);
		insert_str += temp_str;
		index = pNewLine + 3;
	}
	
	len = pEnd - index;
	if(len) {
		temp_str.put_value(index, len);
		insert_str += temp_str;
	}

	*p = pEnd + 2; //skip both the delimit and the newline
}

void print_ifdef(char *def, genString& old_buf, genString& new_buf, FILE* out_file)
{
	fprintf(out_file, "#ifdef	%s\n", def);
	fprintf(out_file, "%s#else\n", old_buf.str());
	fprintf(out_file, "%s#endif\n", new_buf.str());
}

int check(char *buf) /* buf always starts from beginning of a line and ends in "\n\0" */  
{
	char *index1 = buf;
	char *index2;
	
	while(index2 = strchr(index1, '\n')) {
		// check if #ifdef #if #ifndef #elif #endif #else #... exist or not
		if(*index1 == '#' && (*(index1+1) == 'e' || !strncmp(index1+1, "if", 2)))
			return 1;
		index1 = index2 + 1;
	}

	return 0;
}


void do_merge(char* src, char* delta, FILE* out_file, int flag_comment, char *comment,int flag_ifdef, char *def)
{	
	int offset, length;
	char *index = delta;

	if((flag_comment == 0) && (flag_ifdef == 0))  { 
		char *pStart, *pEnd;
		pStart = src;
		
		while(1) {
			offset = GetOffset(&index);
			if (offset == -1) break; 
			assert(offset >= 0);
			pEnd = src+offset;
			print_string(pStart, pEnd, out_file);
			length = GetLength(&index);
			assert( length >= 0);
			pStart = pEnd + length;
			print_delta(&index, out_file);
		}
		print_string(pStart, NULL, out_file);
	} else {
		int state = 0;
		genString insert_str, comment_str;
		genString old_buf, new_buf;
		char *ptr = src;

		if(flag_comment) { /* append a newline in comment */
			comment_str = comment;
			comment_str += '\n';
		} else 
			comment_str = "/* DISCOVER merge change. */\n";

		offset = GetOffset(&index);
		if (offset == -1) state = 3;
		if(state < 3) {
			assert(offset >= 0);
			length = GetLength(&index);
			get_delta(&index, insert_str);
		}

		while(state < 3) {
			switch(state) {
			case 0:
				state = 1;
				while(ptr < src+offset) { 
					old_buf += *ptr;
					if(*ptr == '\n' && *(ptr-1) != '\\') {
						fputs(old_buf, out_file);
						old_buf = NULL;
						ptr++;
						state = 0;
						break;
					} 

					ptr++;
				}

				if(state == 1) {
					new_buf = old_buf;
					new_buf += insert_str;
					insert_str = NULL;
				}

				break;

			case 1:
				while(ptr < src+offset+length) {
					old_buf += *ptr;
					ptr++;
				} 

				offset = GetOffset(&index);
				if (offset == -1) state = 3;
				if(state < 3) {
					assert(offset >= 0);
					length = GetLength(&index);
					get_delta(&index, insert_str);
					state = 2;
				} else {
					while(*ptr != NULL) {
						old_buf += *ptr;
						new_buf += *ptr;
						
						if(*ptr == '\n' && *(ptr-1) != '\\') {
							ptr++;
							break;
						}
						ptr++;
					}

					if(flag_comment)
						fputs(comment_str, out_file);
					
					if(flag_ifdef) {
						if(check(old_buf) || check(new_buf)) {
							if(!flag_comment)
								fputs(comment_str, out_file);
							fputs(new_buf, out_file);
							if(!flag_comment)
								fputs(comment_str, out_file);
						} else
							print_ifdef(def, old_buf, new_buf, out_file);
					} else
						fputs(new_buf, out_file);

					if(flag_comment)
						fputs(comment_str, out_file);
					
					old_buf = NULL;
					new_buf = NULL;
				}
					
				break;

			case 2:
				state = 1;
				while(ptr < src+offset) {
					old_buf += *ptr;
					new_buf += *ptr;

					if(*ptr == '\n' && *(ptr-1) != '\\') {	
						if(flag_comment)
							fputs(comment_str, out_file);
						
						if(flag_ifdef) 
							if(check(old_buf) || check(new_buf)) {
								if(!flag_comment)
									fputs(comment_str, out_file);
								fputs(new_buf, out_file);
								if(!flag_comment)
									fputs(comment_str, out_file);
							} else
								print_ifdef(def, old_buf, new_buf, out_file);
						else
							fputs(new_buf, out_file);

						if(flag_comment)
							fputs(comment_str, out_file);
						
						old_buf = NULL;
						new_buf = NULL;
						ptr++;

						state = 0;
						break;
					} 
					ptr++;
				}

				if(state == 1) {
					new_buf += insert_str;
					insert_str = NULL;
				}

				break;

			default:
				break;
			}
		}

		if(ptr)
				fputs(ptr, out_file);
	}
}

/*void do_merge(char* src, char* delta, FILE* out_file)
{
	char *pStart, *pEnd;
	char *index;
	int offset, length;

	pStart = src;
	index = delta;
	while(1) {
		offset = GetOffset(&index);
		if (offset == -1) break; 
		assert(offset >= 0);
		pEnd = src+offset;
		print_string(pStart, pEnd, out_file);
		length = GetLength(&index);
		assert( length >= 0);
		pStart = pEnd + length;
		print_delta(&index, out_file);
	}

	print_string(pStart, NULL, out_file);
}
*/

//pSatrt point to the first char, pEnd point to the char AFTER the last one to be printed
void print_string(char* pStart, char* pEnd, FILE* out_file)
{
	if(pEnd == NULL)
		fputs(pStart, out_file);
	else {
		int length = pEnd - pStart;
		if(length > 0) {
			char *temp = new char[length+1];

			strncpy(temp, pStart, length);
			temp[length] = 0;

			fputs(temp, out_file);
			delete[] temp;
		}
	}
}

void print_delta(char **p, FILE* out_file)
{
	char *index = *p;
	int c;

	while((c=*index) == ' ' || c == '\t' || c == '\n' || c == ',')
		index++;

	int delimit = c;

	index++;
	char *pEnd = strchr(index,delimit);  

	char *pNewLine;
	while( pNewLine = strchr(index,'\n')) {
		if( pNewLine > pEnd ) break;
		print_string(index, pNewLine+1, out_file);
		index = pNewLine + 3;
	}
	
	print_string(index, pEnd, out_file);

	*p = pEnd + 2; //skip both the delimit and the newline
}

int GetOffset(char **p)
{
	char token[MAXTOKEN];
	char *index = *p;
	int c;

	while((c=*index) == ' ' || c == '\t' || c == '\n' || c == ',')
			index++;

	// the comment # must start at the beginning of the line
	while((c=*index) == '#') {
		index = strchr(index+1, '\n');
		index++;
		while((c=*index) == ' ' || c == '\t' || c == '\n')
			index++;
	}

	if(c == '\0') return -1;

	if(c == '[') {
		index = strchr(index+1,']');
		index++;
		while((c=*index) == ' ' || c == '\t' || c == '\n')
			index++;
	}

	assert(isdigit(c));

	char *pToken = token;

	while(isdigit(c)) {
		*pToken++ = c;
		c= *++index;
	}
	
	*p = index;
	*pToken = 0;
	return atoi(token);
}

int GetLength(char **p)
{
	char token[MAXTOKEN];
	char *index = *p;
	int c;

	while((c=*index) == ' ' || c == '\t' || c == ',')
		index++;

	assert(isdigit(c));

	char *pToken = token;

	while(isdigit(c)) {
		*pToken++ = c;
		c= *++index;
	}
	
	*p = index;
	*pToken = 0;
	return atoi(token);
}

int GetNewPosition(int pos, char **p /*, int& num , int &offset*/)
{
        /* the follwoing two lines should be erased if we put num and offset in arg list*/
	int num = 0 ;
	int offset =0 ;
	int last_offset=0;
	int last_del=0;

	char *index = *p;
	offset  = GetOffset(&index); /* should be erased if ...*/
	while(offset != -1 && offset <= pos ) {
	  last_offset = offset;
	  num -= last_del = GetLength(&index);
	  num += GetInsertStringSize(&index);
	  offset = GetOffset(&index);
	}

//	*p = index;
	if(pos < last_offset+last_del) 
	  return -1;
	else 
	  return pos+num;
}

void GetNewPositionArray(position* pPosition, int size, char* index)
{
  int num = 0;
  int last_offset = 0;
  int last_del = 0;

  int offset = GetOffset(&index);
  for(int i=0; i<size; i++) {
    while(offset != -1 && offset <= pPosition[i].pos ) { //assume insert before
      last_offset = offset;
      num -= last_del = GetLength(&index);
      num += GetInsertStringSize(&index);
      offset = GetOffset(&index);
    }
    if(pPosition[i].pos < last_offset+last_del) 
      pPosition[i].pos = -1;
    else 
      pPosition[i].pos += num;
    last_offset = last_del = 0;
  }
}

int GetInsertStringSize(char **p) 
{
	int c;
	int line_num = 0;
	char *index = *p;
	char *pNewLine;
	
	while((c=*index) == ' ' || c == '\t' || c == '\n' || c == ',')
		index++;

	int delimit = c;

	index++;
	char *pEnd = strchr(index,delimit); 
	int length = pEnd - index;
	
	while( pNewLine = strchr(index,'\n')) {
		if( pNewLine > pEnd ) break;
		line_num++;
		index = pNewLine + 3;
	}

	*p = pEnd + 2;
	return length - 2*line_num;
}
