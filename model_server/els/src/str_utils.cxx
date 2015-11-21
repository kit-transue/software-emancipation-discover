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
#include <msg.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <genArrCharPtr.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#include <cctype>
#endif /* ISO_CPP_HEADERS */
#include <str_utils.h>
#include <messages.h>
#include <genString.h>
#include <genError.h>
#include <attribute.h>

/*************** utility functions for working with strings ************/
/* extract <name> from the following string: <anything>ch1<name>ch2<anything>
   and return pointer to a char after ch2 or NULL if an error occured */
char* extract_str(char* from, char ch1, char ch2, char* to)
{
    char* start = strchr(from, ch1);
    if (start == NULL) return NULL;

    start += 1;
    char* end = strchr(start, ch2);
    if (end == NULL) return NULL;

    *end = '\0';
    strcpy(to, start);
    *end = ch2;
    return end + 1;
}

/* extract the first num from char* from and return pointer to a next char
   after num or NULL if an error occured */
char* extract_num(char* from, int& num)
{
    char* ptr[1];
    num = (int)strtol(from, ptr, 10);
    /* if failed, ptr[0] == from, otherwise it points to the char on 
    which it stopped */
    
    if (ptr[0] == from)
	return NULL;
    else
	return ptr[0];
}

/* return true if buf contains [END] as a substring */
bool is_end(char* buf)
{
    char str[MAXBUF];
    if (extract_str(buf, '[', ']', str))
	return strcmp(str, "END") == 0;
    else 
	return false;
}

/* strip leading and trailing spaces. Return a pointer to the first not 
   space char or NULL if from is NULL */
char* strip_spaces(char* from)
{
    if (!from) return NULL;

    while (isspace(*from))
	++from;
    if(*from) {
      char* end = from + strlen(from) - 1;
      while (isspace(*end))
	--end;
	
      *(end + 1) = '\0';
    }
    return from;
}


/************ Some ohter helper functions ************************/



/* allocate enough space for the new string and copy it */
char* my_strcpy(char*& to, const char* from)
{
    if (from)
    {
	to = new char [strlen(from) + 1];
	if (to == NULL) return NULL;
	strcpy(to, from);
	return to;
    }
    else
    {
	to = NULL;
	return NULL;
    }
}

/* copy genArrCharPtr and allocate storage for all strings that need to be copied */
void copy_gen_arr(genArrCharPtr& to, const genArrCharPtr& from)
{
    to.reset();
    to.provide(from.size());
    for (int i = 0; i < from.size(); i++)
    {
	char* temp = NULL;
	my_strcpy(temp, *from[i]);
	to.append(&temp);
    }
}

/* delete all strings in the array and reset the array */
void clear_gen_arr(genArrCharPtr& arr)
{
    for (int i = 0; i < arr.size(); i++)
	delete [] *arr[i];
    arr.reset();
}

int get_arr_index(const genArrCharPtr& arr, const char* str)
{
    int size = arr.size();
    for (int i = 0; i < size; i++)
	if (strcmp(str, *arr[i]) == 0)
	    return i;
    
    return -1;
}

//get the next not empty line which is not comment. If the end of file or
//[END] is reached, return false, else return true;
bool get_next_line(FILE* file, char* line, int& LineNum)
{
  char buf[MAXBUF];
  while (OSapi_fgets(buf, MAXBUF, file))
    {
      ++LineNum;

      if (buf[0] == '#') //comment
	continue;

      char* ptr = strip_spaces(buf);
      if (strlen(ptr) == 0) //empty line
	continue;

      if (is_end(buf))
	return false;

      strcpy(line, buf);
      return true;
    }
      
  return false;
}
	  

/*****************************************************************/
/* parse a list of the form <ws>AttName<ws>AttName... 
   and return result in attributes array */
void ParseAttributeList(const char* list, objArr& attributes,
			const char* FileName, int LineNum)
{
  Initialize(ParseAttributeList);
  char* next = (char*) list;

  while (*next)
    {
      while (*next && isspace(*next))
	++next;
      if (*next == '\0')
	break;

      char* start = next;
      while (!isspace(*next) && *next != '\0')
	++next;
      char old = *next;
      *next = '\0';
      symbolAttribute* att = symbolAttribute::get_by_name(start);
      if (att)
	{
	  attributes.insert_last(att);
	}
      else
	{
	  msg("ERROR: $1 Line $2: Attribute $3 is not defined") << FileName << eoarg << LineNum << eoarg << start << eom;
	}
      
      *next = old;
    }
}


