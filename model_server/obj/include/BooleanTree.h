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
#ifndef __booleanTree_h_
#define __booleanTree_h_

#ifndef _objTree_h
#include "objTree.h"
#endif

#ifndef _RTL_h
#include "RTL.h"
#endif

#ifndef _objArr_h
#include "objArr.h"
#endif

typedef enum _objStdFunctions_ {
    GetName,
    GetTitle,
    SendString
} objStdFunctions;

typedef enum _RTLBooleanOperators_ {
    RTL_NOP,
    RTL_UNION,
    RTL_INTERSECTION,
    RTL_SUBTRACTION
} RTLBooleanOperators;

typedef enum _BooleanNodeTypes_ {
    BooleanNodeType,
    BooleanOperatorNodeType,
    BooleanOperandNodeType
} BooleanNodeTypes;

RelClass(BooleanNode);

class BooleanNode : public objTree {

 public:
    ~BooleanNode() {}
    BooleanNode() {}
    define_relational (BooleanNode,objTree);
//    copy_member (BooleanNode);

 private:
    BooleanNode(const BooleanNode& rr);// {*this = rr;} // Copy Constructor
 public:

    virtual void put_dependency (objArr *);
    virtual BooleanNodeTypes BooleanNodeIsA() const;
    virtual void generate_function (ostream&, objArr&);
};

RelClass (BooleanOperatorNode);

class BooleanOperatorNode : public BooleanNode {

 private:
   RTLBooleanOperators boolean_operator;

 public:
   BooleanOperatorNode(RTLBooleanOperators new_operator = RTL_NOP);
  ~BooleanOperatorNode();

 private:
  BooleanOperatorNode(const BooleanOperatorNode& rr);// {*this = rr;}
 public:
   virtual BooleanNodeTypes BooleanNodeIsA() const;
   virtual void put_dependency (objArr *);   
   void generate_function (ostream&, objArr&);
//   copy_member (BooleanOperatorNode);
   define_relational (BooleanOperatorNode, BooleanNode);
};

RelClass (BooleanOperandNode);

class BooleanOperandNode : public BooleanNode {

 private:
   RTLPtr rtl_header;

 public:
   BooleanOperandNode (RTLPtr rtl = NULL);
   ~BooleanOperandNode();

 private:
   BooleanOperandNode(const BooleanOperandNode& rr);// {*this = rr;}
 public:
   virtual void put_dependency (objArr *);
   virtual BooleanNodeTypes BooleanNodeIsA() const;
   void generate_function (ostream&, objArr&);

//   copy_member (BooleanOperandNode);
   define_relational (BooleanOperandNode,BooleanNode);
};

#endif

/*
   START-LOG-------------------------------------------

   $Log: BooleanTree.h  $
   Revision 1.2 1995/07/27 20:23:58EDT rajan 
   Port
 * Revision 1.2.1.5  1993/10/05  13:33:41  builder
 * Reduce number of include files
 *
 * Revision 1.2.1.4  1993/07/13  02:14:30  mg
 * copy_member
 *
 * Revision 1.2.1.3  1993/07/12  18:45:39  aharlap
 * #ifdef __GNUG__ around copy-constr
 *
 * Revision 1.2.1.2  1992/10/09  18:58:04  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
