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
#ifndef _dataDefn_h
#define _dataDefn_h 

/*******************************************************************************
 *  Data definition classes
 *
 *
 *  fkb 6/20/91
 ******************************************************************************/

#ifndef _ui_struct_h
#include <ui-struct.h>
#endif
#ifndef _general_h
#include <general.h>
#endif

class DataDefn
{
 public:
    char* varName;

    DataDefn(char* var) { varName = var; }
    virtual char* GenerateCodeDefn() = 0;
};


class ElementaryDefn : public DataDefn
{
 public:
    char*         domainName;
    bool          hasInitialValue;
    referenceType refType;
    
    ElementaryDefn(char* var,
                   char* domain,
                   referenceType ref_type,
                   bool has_init_val = false)
    : DataDefn(var)
      {
         domainName = domain;
         hasInitialValue = has_init_val;
         refType = ref_type;
      }
};


class IntegerDefn : public ElementaryDefn
{
 public:
    int  initialValue;
    bool isSigned;
    bool isLong;

    IntegerDefn(char* var, char* domain, referenceType ref_type,
                bool has_init_val, int init_val, bool is_signed, bool is_long)

      : ElementaryDefn(var, domain, ref_type, has_init_val)
	{ initialValue = init_val; isSigned = is_signed; isLong = is_long; }

    virtual char* GenerateCodeDefn();
};


class CharDefn : public ElementaryDefn
{
 public:
    char initialValue;

    CharDefn(char* var, char* domain, referenceType ref_type,
             bool has_init_val, char init_val)
      : ElementaryDefn(var, domain, ref_type, has_init_val)
	{ initialValue = init_val; }

    virtual char* GenerateCodeDefn();
};


class RealDefn : public ElementaryDefn
{
 public:
    float initialValue;
    bool  isLong;

    RealDefn(char* var, char* domain, referenceType ref_type,
             bool has_init_val, float init_val, bool is_long)
      : ElementaryDefn(var, domain, ref_type, has_init_val)
	{ initialValue = init_val; isLong = is_long; }

    virtual char* GenerateCodeDefn();
};


class ArrayDefn : public DataDefn
{
 public:
    char* elementName;
    int   numberOfElements;

    ArrayDefn(char* var, char* elem_name, int num_of_elem) : DataDefn(var)
      { elementName = elem_name; numberOfElements = num_of_elem; }

    virtual char* GenerateCodeDefn();
};


/*
    START-LOG-------------------------------

    $Log: dataDefn.h  $
    Revision 1.1 1993/05/27 16:23:09EDT builder 
    made from unix file
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:42  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif
