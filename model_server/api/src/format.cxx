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
#include <machdep.h>
#include <format.h>
#include <symbolArr.h>
#include <charstream.h>
#include <genString.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <externAttribute.h>

struct offsetIndex {int order;int offset;};
genArr(offsetIndex);

symbolFormat::symbolFormat(const char* name)
: objDictionary(name)
{
    format     = "";
    header     = "";
    sort_order = "";
    do_padding = 0;
}

symbolFormat::symbolFormat(const char* name, objArr& attributes)
: objDictionary(name)
{
    setAttributes(attributes);
    format     = "";
    header     = "";
    sort_order = "";
    do_padding = 0;
}

symbolFormat::symbolFormat(const char* name, objArr& attributes, char* form)
: objDictionary(name)
{
    setFormat(form);
    setAttributes(attributes);
    header     = "";
    sort_order = "";
    do_padding = 0;
}

symbolFormat::symbolFormat(const char* name, objArr& attributes, char* form, char* head)
: objDictionary(name)
{
    setFormat(form);
    setAttributes(attributes);
    setHeader(header);
    sort_order = "";
    do_padding = 0;
}

void
symbolFormat::setSortOrder(char* order)
  // The sort order should be a string of 'a's and 'd's specifying if the sort on the 
  //    nth attribute should be acsending or decsending.
  //    Ascending is the default.
{
    if (order) {
	sort_order = order;
    };
}

void  
symbolFormat::setAttributes(objArr& attributes)
{
    Initialize(symbolFormat::setAttributes);
    format_array = attributes;
    for (int i= attributes.size()-1; i>=0; i--) {
	if (!format_array[i]) {
	    format_array.remove(i);
	};
    };
    
    format_size = format_array.size();
}

void  
symbolFormat::setHeader(char* head)
{
    if (head && *head) 
	header = head;
}

void  
symbolFormat::setFormat(const char* form)
{
  if(form && *form){
    format = form;
    int len = strlen(form);
    if(form[len-1] == '\n'){
      char *form2 = format;
      form2[len-1] = '\0';
    }
  }
}

void symbolFormat::print(ostream& str, int level) const
{
    //char spaces[STRING_SIZE];
    //int i = 0;
    //for (i=0;i<level;i++){
    
    str << "CLASS: symbolFormat" << endl;
    //str << "  Name = " << name <<"" << endl;
    str << "  Header =  " << (char *)header << " " << endl;
    str << "  Format = " << (char *)format << " " << endl;
    str << endl << "  Attribute Array:" << endl;
    int size = format_array.size();
    for (int i=0; i<size; i++) {
        format_array[i]->print(str, level+1);
    };
}




static char* string_array_pointer = NULL;


static int string_array_compare(const void* i, const void* j) {
    Initialize(string_array_compare);
    if (!string_array_pointer) {return 0;};
    int a = ((offsetIndex*)i)->offset;
    int b = ((offsetIndex*)j)->offset;
    int v = strcmp(string_array_pointer+a, string_array_pointer+b);
    return v ? v : (a<b ? -1 : a>b);
};


struct symbolArraySorter {
    symbolArraySorter(symbolArr& arr, genArrOf(offsetIndex)& result, objArr attributeArray, genString sortOrder,
		      int do_padding);
    ocharstream chstr;
    ocharstream tempStr;
    char* sort_order;
};  

symbolArraySorter::symbolArraySorter(symbolArr& arr, genArrOf(offsetIndex)& result, objArr attributeArray, 
				     genString sortOrder, int do_padding)
: sort_order(sortOrder)
{
    Initialize(symbolArraySorter::symbolArraySorter);
    int size = arr.size();
    int attribSize = attributeArray.size();

    int orderSize = strlen(sort_order);
    int i;
    for (i=0;i<size;i++) {	
	for (int ii=0; ii<attribSize; ii++) {
	    if ((orderSize > ii) && ((sort_order[ii] == 'd') || (sort_order[ii] == 'D'))) {
		tempStr.reset();
		symbolAttribute *att = checked_cast(symbolAttribute,attributeArray[ii]);
		if(do_padding && (is_intAttribute(att) || is_extIntAttribute(att)) ){
		    char buf[15]; 
		    sprintf(buf, "%010d", OSapi_atoi(att->value(arr[i])));
		    tempStr << buf << '\x01' << '\0';
		} else
		    tempStr << att->value(arr[i]) << '\x01' << '\0';
		char* tempChar = tempStr.ptr();
		int size = strlen(tempChar);
		for (int k = 0; k<size-1; k++) {
		    tempChar[k] = (char)(127-(int)tempChar[k]);
		};
		tempChar[size-1] = '\x7f';
		chstr << tempChar;
	    } else {
		symbolAttribute *att = checked_cast(symbolAttribute,attributeArray[ii]);
		if(do_padding && (is_intAttribute(att) || is_extIntAttribute(att)) ){
		    char buf[15]; 
		    sprintf(buf, "%010d", OSapi_atoi(att->value(arr[i])));
		    chstr << buf << '\x01';
		} else {
		  const char* val = att->value(arr[i]);
		  chstr << val << '\x01';
		}
	    };
	    
	};
	chstr << '\0';
    };
    
    char* allStrings = chstr.ptr();
    char* nextString = allStrings;
    offsetIndex current;
    int prev;
    prev = current.offset = current.order = 0;
    result.reset();
    result.append(&current);
    for (i=1;i<size;i++) {
	int next = strlen(nextString);
	current.offset = prev + next + 1;
	prev = current.offset;
	current.order = i;
	result.append(&current);
	nextString = nextString + next + 1;
    };

    string_array_pointer = allStrings;
    OSapi_qsort((char*)result[0], size, sizeof(offsetIndex), string_array_compare);
    string_array_pointer = NULL;

}

void symbolFormat::sort(symbolArr& arr, symbolArr& result)
{
    Initialize(symbolFormat::sort);

    genArrOf(offsetIndex) indexArray;
    symbolArraySorter sorter(arr, indexArray, format_array, sort_order, do_padding);
    int size = arr.size();

    if (&result == &arr) {
	symbolArr temp;
	for (int i=0; i<size; i++) {
	    temp.insert_last(arr[indexArray[i]->order]);
	};
	arr.removeAll();
	arr = temp;
    } else {
	result.removeAll();
	for (int i=0; i<size; i++) {
	    result.insert_last(arr[indexArray[i]->order]);
	};
    };
}

static symbolAttribute* get_expression_attr(const char* exp)
{
  // first see if this is a standard attribute

  symbolAttribute* attr =  symbolAttribute::get_by_name(exp);
  if(attr)
    return attr;

  // parse expression

  expr* expression = api_parse_expression(exp);
    
  if(expression)
    attr = new exprAttribute(NULL, expression);

  return attr;
}

void free_attribute_expression(symbolAttribute *attr)
{
    Initialize(free_attribute_expression);
    
    const char* name = attr->get_name();
    if(! (name&&name[0])){
	exprAttribute *eattr = (exprAttribute*) attr;
	delete eattr->op;
	delete eattr;
    }
}
int api_print_attribute(symbolArr& arr, const char*exp, ostream& os)
{
  int result = 0;
  int size = arr.size();
  if(size == 0) {
    return result;
  }

  symbolAttribute* attr = get_expression_attr(exp);
  if(!attr)
    return 1;

  for(int ii=0; ii<size; ++ii){
    const char *val = attr->value(arr[ii]);
    os << val << '\n';  
  }
  os << flush;

  free_attribute_expression(attr);
  return result;
}

int api_sort_apply(symbolArr& arr, const char*exp, bool once, setTraverseFunction f, Interpreter*inter, void* data)
{
  Initialize(api_sort_apply);

  symbolAttribute* attr = get_expression_attr(exp);
  if(!attr)
    return 1;

  int result = 0;
  int size = arr.size();
  if(size == 0) {
      free_attribute_expression(attr);
      return result;
  } else if(size ==1){
      const char *val = attr->value(arr[0]);
      result          = f(arr, 0, 0, size, (char*)val, inter, data);
      free_attribute_expression(attr);
      return result;
  }

  objArr atts;
  atts.insert_last(attr);
  
  genArrOf(offsetIndex) indexArray;
  symbolArraySorter sorter(arr,indexArray,atts,genString(""), 0);
	
  char* str  = sorter.chstr.ptr();
  int i      = 0;
  while (i<size) {
    int j = i+1;
    while (j<size && (strcmp(str + indexArray[i]->offset, str + indexArray[j]->offset) == 0)) {
      j++;
    };
	    
    char* val =str + indexArray[i]->offset;
    val[strlen(val)-1] = '\0'; // remove extra '\x7f' at end;
	    
    if ((result = f(arr, indexArray[i]->order, 0, j-i, val, inter, data)) != 0){
      /* goto DONE; */
      break;
    }
    if (!once) {
      for (int x = i + 1; x < j; x++) {
	if((result = f(arr, indexArray[x]->order, x-i, j-i, val, inter, data)) != 0){
	  /* goto DONE; */
	  i = j = size;
	  break;
	}
      };
    };
    i=j;
    
  };
  /* DONE:; */
  free_attribute_expression(attr);
  return result;
}

static bool bool_value(const char*val)
{
  if(!(val&&*val))  //  empty
    return false;
  if(val[1] == 0)   // one character
    return val[0] != '0';
  if(strcmp(val, "no")==0)
    return false;
  if(strcmp(val, "off")==0)
    return false;
  return true;
}

void symbolFormat::printOut(ostream& outstr, symbolArr& arr, int print_indexes, const char* yes, const char* no)
{
  Initialize(symbolFormat::printOut);

  if (header.length()){
    outstr << (char *)header;
  }
  int size = arr.size();
  
  for (int i=0; i<size; i++) {
    if(print_indexes)
      outstr << i + 1 << "  ";

    symbolPtr&sym =  arr[i];
    printOut(outstr, sym, yes, no);
    outstr << '\n';
  }
}

void symbolFormat::printOut(ostream& outstr, const symbolPtr&sym, const char* yes, const char* no)
{

  int n = 0;
  char ch;
  char last_ch = '\0';
  static char width_str[8];
  int width;

  int no_attr = format_array.size();
  for(char * frmt = (char*)format; (ch=*frmt); ++frmt) {
    if (ch != '%'){
      outstr << ch;
      last_ch = ch;
      continue;
    }
    ++frmt;
    width = 0;

    // Read next characters as digits in field width, if they are digits.
    int i;
    for (i=0; i<8 && isdigit(frmt[i]); i++) {
        width_str[i] = frmt[i];
    }
    if (i>0 && i<8) {
        frmt += i;
        width = atoi (width_str);
    }

    ch = *frmt;
    bool padding = true;
    bool strtype, capitalize;
    switch(ch){
    case 'B':
      strtype = false;
      capitalize = true;
      break;
    case 'b':
      strtype=false;
      capitalize = false;
      break;
    case 's':
      strtype = true;
      capitalize = false;
      break;
    case 'S':
      strtype = true;
      capitalize = true;
      break;
    default:
      outstr << ch;
      last_ch = ch;
      continue;  // loop
    }

    if(n >= no_attr)
      continue;
    
    symbolAttribute*attr = (symbolAttribute*)format_array[n];
    const char* val = attr->value((symbolPtr&)sym);
    if(!strtype){
      if(bool_value(val))
	val = attr->get_name();
      else
	val = NULL;
    }
    if (val && *val) {
      if (padding && isalnum(last_ch))
	outstr << ' ';

      int len = strlen(val);
      if (width && width<len) len = width;

      if (capitalize) {
        outstr << (char)toupper(val[0]);
        outstr.write (val+1, len-1);
      } else {
        outstr.write (val, len);
      }
      last_ch = val[len-1];
    }
    n++;
  }
}

