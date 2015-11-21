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
//////////////////////////  FILE steCategory_h.C   ///////////////////////////
//
// -- Contains  related steCategory Class Description                  
#include "steCategory.h"
#include "steStyle.h"
#include "steSlot.h"
#include "genError.h"
#include "ste-report.h"

#define Delete(x) if (x) delete(x)

init_relation(slotstyle_of_category,1,NULL,category_of_slotstyle,MANY,NULL);
init_relation(system_category_of_app,1,relationMode::D,app_of_system_category,1,NULL);
init_relation(user_category_of_app,1,relationMode::D,app_of_user_category,1,NULL);
init_relation(node_of_category,MANY,NULL,category_of_node,MANY,relationMode::C);

init_relational(steCategory,Relational);

/*
 *    Constructor
 */

steCategory::steCategory (char *name, steSlotPtr sl)
{
   category_name = new char [ strlen(name) + 1 ];
   strcpy ( category_name, name);
   put_relation(slotstyle_of_category, this,sl);
}

/*
 *    Constructor
 */

steCategory::steCategory (char *name, steCategory_type c_type)
{
   category_name = new char [ strlen(name) + 1 ];
   strcpy ( category_name, name);
   category_type = c_type;

/*
   if (c_type == USER_DEFINED_CATEGORY)
      ste_report_user_category_add (this);
   else
      ste_report_system_category_add (this);
*/
}
  
char *steCategory::get_name() const {
#if 0
   char *p;
   if (category_name == NULL)
      return NULL;

   p = new char [ strlen(category_name) + 1 ];
   strcpy (p, category_name);
  return p;
#endif
   return (char *)name();
}

const char *steCategory::name() const { 
  return category_name;
}


steStylePtr steCategory::get_category_style()  {
    Initialize(steCategory::get_category_style);

    steStylePtr st = NULL;
    steSlotPtr sl = get_category_styleslot();
    if ( sl ) st = checked_cast(steStyle,sl->get_slot_obj());
    return st;
}

steSlotPtr steCategory::get_category_styleslot()  {
    Initialize(steCategory::get_category_styleslot);

    steSlotPtr obj_sl = checked_cast(steSlot,get_relation(slotstyle_of_category, this));
    return obj_sl;
}



// static member definitions

char *steCategory::equate_categ_name = "Equates";
