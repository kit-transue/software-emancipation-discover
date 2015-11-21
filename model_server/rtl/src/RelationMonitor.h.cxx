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
// RelationMonitor.h.C
//------------------------------------------
// synopsis:
// Implementation of class RelationMonitor
//------------------------------------------

// include files

#include <objArr.h>
#include <RTL.h>
#include <RelationMonitor.h>

// relational stuff

init_relational(RelationMonitor,RTL);

// function definitions
typedef RelType* RelTypePtr;

static void monitor_insert(
    RelationMonitorPtr rm, RelTypePtr rt, appTreePtr app_node)
{
    Initialize(monitor_insert);

    RTLNodePtr rm_root = checked_cast(RTLNode,rm->get_root());
    if(rm->monitors(rt))
	rm->obj_insert(FIRST, app_node, rm_root, NULL);
}

static void monitor_hook(
    int op, Relational* target, Relational* arrow, RelTypePtr rel_type)
{
    Initialize(monitor_hook);

    if(op != 1)		// We only care about put_relation operations.
	return;

    Obj* elem;
    Obj& contents = *get_relation(browselist_of_object,target);
    ForEach(elem, contents)
	monitor_insert(checked_cast(RelationMonitor,elem), rel_type, checked_cast(appTree,arrow));
}

static void set_hook(RelTypePtr rt)
{
    static objArr monitored_set;

    if(!monitored_set.includes(rt))
    {
	monitored_set.insert_last(rt);
	obj_rel_put_hook(rt, monitor_hook);
    }
}

RelationMonitor::RelationMonitor(
    RelationalPtr obj, RelTypeTable monitors, const int num)
: RTL((char*)0), rel_type_table(monitors), num_rel_types(num)
{
    Initialize (RelationMonitor::RelationMonitor);
    init(obj);
}

RelationMonitor::RelationMonitor(const RelationMonitor& that)
: RTL(that), rel_type_table(that.rel_type_table),
  num_rel_types(that.num_rel_types)
{
    Initialize (RelationMonitor::RelationMonitor);
    init(that.obj_of_this());
}

void RelationMonitor::init(RelationalPtr obj)
{
    Initialize(RelationMonitor::init);

    put_relation(browselist_of_object,obj,this);

    for(int i = 0; i < num_rel_types; ++i)
    {
	RelTypePtr rel_type = *rel_type_table[i];

	set_hook(rel_type);

	// Incorporate the members of the new relation.

	RTLNodePtr rtl_node = checked_cast(RTLNode,get_root());

	Obj* elem;
	Obj& contents = *get_relation(rel_type,obj);
	ForEach(elem,contents)
	    obj_insert(FIRST, checked_cast(commonTree,elem), rtl_node, NULL);
    }
}

RelationMonitor::~RelationMonitor()
{}

RelationalPtr RelationMonitor::obj_of_this() const
{
    Initialize(RelationMonitor::obj_of_this);
    return(RelationalPtr(get_relation(owner_of_browselist,this)));
}

bool RelationMonitor::monitors(const RelTypePtr rel_type) const
{
    Initialize(RelationMonitor::monitors);

    for(int i = 0; i < num_rel_types; ++i)
	if(*rel_type_table[i] == rel_type)
	    ReturnValue(1);

    return(0);
}
/*
   START-LOG-------------------------------------------

   $Log: RelationMonitor.h.cxx  $
   Revision 1.1 1993/06/25 10:14:04EDT builder 
   made from unix file
Revision 1.2.1.5  1993/06/25  14:13:55  bakshi
c++3.0.1 port

Revision 1.2.1.4  1992/11/24  20:16:10  smit
Fix casting problems of many individuals.

Revision 1.2.1.3  1992/11/21  23:51:37  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:24:31  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
