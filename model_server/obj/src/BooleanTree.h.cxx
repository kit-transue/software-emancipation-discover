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
#include "BooleanTree.h"

init_relational (BooleanNode, objTree);
init_relational (BooleanOperatorNode, BooleanNode);
init_relational (BooleanOperandNode, BooleanNode);


 void BooleanNode:: put_dependency (objArr *) {}
 BooleanNodeTypes BooleanNode:: BooleanNodeIsA() const 
   {return BooleanNodeType;}
 void BooleanNode::generate_function (ostream&, objArr&) {}

BooleanOperatorNode::BooleanOperatorNode (RTLBooleanOperators new_operator)
{
   boolean_operator = new_operator;
}

BooleanOperatorNode::~BooleanOperatorNode()
{
}

void BooleanOperatorNode::put_dependency (objArr *dependency)
{
   BooleanNodePtr node;

   Initialize (BooleanOperatorNode::put_dependency);

   if (node = (BooleanNodePtr) this->get_first()) {
      node->put_dependency (dependency);
   }

   return;
}

BooleanNodeTypes BooleanOperatorNode::BooleanNodeIsA () const
{
   Initialize (BooleanOperatorNode::BooleanNodeIsA);

   ReturnValue (BooleanOperatorNodeType);
}

void BooleanOperatorNode::generate_function
		(ostream& ostr, objArr& dependency)
{
    BooleanNodePtr node;
    int one_operand_in = 0;

    Initialize (BooleanOperatorNode::generate_function);

    node = (BooleanNodePtr) this->get_first();

    while (node) {
       switch (boolean_operator) {

           case RTL_UNION: 

		// make (a) || (b)
		if (!one_operand_in) {

                   // insert begin paren
                   ostr << "( ";
                   one_operand_in = 1;

                } else {

                   // insert logical or
                   ostr << " || ";
                }

		node->generate_function(ostr, dependency);
		break;

           case RTL_INTERSECTION:

		// make (a) && (b)
                if (!one_operand_in) {

                   // insert begin paren
                   ostr << "( ";
                   one_operand_in = 1;

                } else {

                   // insert logical and
                   ostr << " && ";
                }

                node->generate_function(ostr, dependency);
                break;
  
           case RTL_SUBTRACTION:

		// make (a) && (!b)
                if (!one_operand_in) {

                   // insert begin paren
                   ostr << "( ";
                   one_operand_in = 1;

		   node->generate_function(ostr, dependency);

                } else {

                   // insert logical and
                   ostr << " && ";

		   // insert left paren and negation
		   ostr << "( !";

		   node->generate_function(ostr, dependency);

		   // insert right paren
		   ostr << " )";
                }
		break;

           case RTL_NOP:
           default: break;
       }

       node = (BooleanNodePtr) node->get_next ();
    }

    // insert end paren
    if (one_operand_in)
	ostr << " )";

    return;
}

BooleanOperandNode::BooleanOperandNode (RTLPtr rtl)
{
   this->rtl_header = rtl;
}

BooleanOperandNode::~BooleanOperandNode() {};

void BooleanOperandNode::put_dependency (objArr *array)
{
   Initialize (BooleanOperandNode::put_dependency);

   if (!array->includes (this->rtl_header))
      array->insert_last (this->rtl_header);

   return;
}

BooleanNodeTypes BooleanOperandNode::BooleanNodeIsA () const
{
   Initialize (BooleanOperandNode::BooleanNodeIsA);

   ReturnValue(BooleanOperandNodeType);
}

void BooleanOperandNode::generate_function (ostream& ostr, objArr& dependency)
{
   RTLNodePtr node;
   int        dep_index;

   Initialize (BooleanOperandNode::generate_function);

   node = checked_cast(RTLNode, rtl_header->get_root());

   if (node) {

      // insert in dependency list
      put_dependency (&dependency);

      // get index in the dependency array
      for (dep_index = 0; dep_index < dependency.size(); dep_index++)
          if (dependency[dep_index] == rtl_header)  break;

      ostr << "(rtl_obj_includes(" << "$OBJ,$RTL,"  << dep_index << "))";
   } else {
      ostr << "(0)";
   }

   return;
}
/*
   START-LOG-------------------------------------------

   $Log: BooleanTree.h.cxx  $
   Revision 1.1 1993/07/12 22:18:07EDT builder 
   made from unix file
Revision 1.2.1.4  1993/07/13  02:15:20  mg
uninlined virtuals

Revision 1.2.1.3  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:52:57  boris
*** empty log message ***


   END-LOG---------------------------------------------

 */
