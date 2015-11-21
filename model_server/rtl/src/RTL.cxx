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
#include <RTL.h>

#include <BooleanTree.h>
#include <RTL_externs.h>


// Called from search expression to perform inclusion test
int rtl_obj_includes (Obj *obj, RTL *rtl, int dep_index)
{
   Initialize (rtl_obj_includes);

   int        result = 0;
   RTLNodePtr rtl_node;
   RTLPtr     dependent_rtl;
   RTLNodePtr dependent_rtl_node;

   rtl_node = checked_cast(RTLNode, rtl->get_root());

   if (rtl_node) {

      // get dependents array
      objArr& dependents = rtl_node->get_dependency ();

      // get dependent rtl header
      dependent_rtl = checked_cast(RTL, dependents [dep_index]);

      if (dependent_rtl) {

         // get dependent rtl node
         dependent_rtl_node = checked_cast(RTLNode, dependent_rtl->get_root());

         // check for containment
         if (dependent_rtl_node)
            result = dependent_rtl_node->rtl_includes (obj);
      }

   }
   return result;
}
