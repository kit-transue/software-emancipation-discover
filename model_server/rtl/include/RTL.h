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
#ifndef _RTL_h
#define _RTL_h

//
// Run-Time List (RTL) class declaration.
//

#include <Relational.h>
#include <objOper.h>
#include <objArr.h>
#include <symbolArr.h>
#include <objString.h>
#include <RTL_Names.h>


RelClass(RTL);

class RTL : public app
{
  public:
    RTL(const objString& name);
    RTL(const RTL&);
    RTL(char const * name);
    ~RTL();

    define_relational(RTL, app);
    declare_copy_member(RTL);

    void obj_insert(objOperType, commonTreePtr, commonTreePtr, void *);
    void obj_remove(RelationalPtr);

    virtual void insert_obj(objInserter* oi, objInserter* ni);
    virtual void remove_obj(objRemover *, objRemover* nr);

    virtual void notify(int, Relational*, objPropagator*, RelType*);

    virtual void send_string(ostream&) const;

    virtual objArr* search(commonTreePtr root, void* constraints,
			   int options, int load_flag);
    
    virtual void print(ostream& = cout, int level = 0) const;
    virtual void xref_notify (const symbolArr& modified, 
				   const symbolArr& added, 
				   const symbolArr& deleted);
  
};
generate_descriptor(RTL, app);

RelClass(RTLNode);

class RTLNode: public appTree
{
  public:
    RTLNode(const objString& name);
    RTLNode(const RTLNode&);
    RTLNode(char const *name);
    ~RTLNode();

    define_relational(RTLNode, appTree);
    declare_copy_member(RTLNode);

    void set_selection(symbolArr&);
    const symbolArr& get_selection() const;

    void  set_format_spec(char const *);
    char const * get_format_spec();

    void  set_sort_spec(char const *);
    char const * get_sort_spec();

    void  set_preview(char const *);
    char const * get_preview();

    void set_dependency(objArr& array);
    objArr& get_dependency();    // note, this really should be const !!!
    void clear_dependency();

    void rtl_insert(symbolPtr);		// Inserts an object in RTL
    virtual void rtl_insert(symbolPtr, int check_include); // Inserts an object in RTL
    virtual void rtl_remove(symbolPtr);		// Removes an object from list
    int  rtl_includes(symbolPtr);	// Test for membership of object

    // objArr& rtl_contents();		// Returns contents set
    symbolArr& rtl_contents();

    void selection_insert(symbolPtr);	// Inserts object in selection
    void selection_remove(symbolPtr);	// Removes object from selection
    int  selection_includes(symbolPtr);	// Test for inclusion

    void rtl_intersection(const RTLNode&);	// intersection
    void rtl_union(const RTLNode&);		// merge
    void rtl_subtract(const RTLNode&);		// difference

    int match(ObjPtr);		// match with search criteria

    void clear();

    virtual void send_string(ostream&) const;

    static void rtl_format_string(commonTreePtr obj, char const * format, 
				  ostream &strm);

    static void rtl_format_string(symbolPtr, char const *, ostream&);

    void sort();           // sort according to criteria in sort_spec

    int symbols() const;

    // void set_use_symbols();
    // void set_use_objs();   
	                     
    virtual void print(ostream& = cout, int level = 0) const;

  protected:
    // objArr   contents;
    symbolArr   contents;		// array of symbols

  private:

    objArr      dependency;		// dependency array.
    symbolArr 	selection;		// default selection

    objString 	preview;		// Preview Text

    objString	format_spec;		// Format specification
    objString	sort_spec;		// Sorting specification
};
generate_descriptor(RTLNode, appTree);


inline const symbolArr& RTLNode::get_selection() const
{ return selection; }



inline void    RTLNode::set_dependency(objArr& array)
{ dependency = array;}

inline  objArr& RTLNode::get_dependency() 
{ return dependency; }

inline int RTLNode::symbols() const
{ return contents.symbols(); }


define_relation(boolean_of_rtl, booleaned_by_rtl);

define_relation(styles_rtl_of_document, document_of_styles_rtl);

define_relation(rtl_of_app, app_of_rtl);

define_relation(browselist_of_object,owner_of_browselist);


#endif

