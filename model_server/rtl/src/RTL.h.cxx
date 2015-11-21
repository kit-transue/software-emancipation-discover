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
// Member functions for RTL class
// 
//---------------------------------------------
// synopsis:
//
// Member functions:
//	RTL
//	~RTL
//	set_selection
//	get_preview
//	set_preview
//	insert
//	remove
//	operator&
//	operator|
//	operator-
//	clear
//      sort
//---------------------------------------------
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <genError.h>
#include <RTL.h>
#include <RTL_externs.h>
#include <xrefSymbol.h>
#include <objOperate.h>
//#include <ldrList.h>
//#include <viewList.h>

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <charstream.h>

static ocharstream ostr;

init_relational(RTL, app);

void gen_print_indent(ostream&, int);

implement_copy_member(RTL);
implement_copy_member(RTLNode);

void RTL::print(ostream& os, int level) const
{
    Initialize(RTL::print);

    RTLNode * node =  (RTLNode*) get_root();
    int nn = node ? node->rtl_contents().size() : 0;
    gen_print_indent(os, level);
    os << "RTL " << get_name() << " contains " << nn << " elements" << endl;
}


void  RTLNode::print(ostream& os, int level) const
{
    contents.print(os, level);
}


void RTL::obj_remove (RelationalPtr node)
{
    Initialize(RTL::obj_remove );

    objRemover ro(checked_cast(objTree,node));

    obj_modify(*this, &ro);

    RTLNodePtr root = checked_cast (RTLNode, get_root());
    root->rtl_remove (node);
}


void RTL::obj_insert(
    objOperType op, commonTreePtr src, commonTreePtr targ, void *data)
//
// This method is written to get around typecasting problem.
// Real solution should be support of insertion of Relational object
// through obj_insert.
//
{
    ::obj_insert (this, op, appTreePtr (src), appTreePtr(targ), data);
}


RTL::RTL(const objString& nm)
: app(nm)
//
// Constructor
//
{
   Initialize (RTL::RTL);

   set_type (App_RTL);

   put_root(new RTLNode(nm));
   xref_notifier_add (this);
}


RTL::RTL(char const *nm)
: app(nm)
//
// Constructor
//
{
    Initialize (RTL::RTL);

    set_type (App_RTL);

    put_root(new RTLNode(nm));
    xref_notifier_add (this);
}


RTL::RTL(const RTL&)
//
// Copy constructor
//
{
   xref_notifier_add (this);
}


RTL::~RTL()
//
// Destructor
//
{
    Initialize (RTL::~RTL);

    xref_notifier_rem(this);
}


void RTL::insert_obj (objInserter *oi, objInserter *ni)
{
    Initialize (RTL::insert_obj);

    ObjPtr obj;
    RTLNodePtr rtl_node;
    int op; // 0->nop; 1->insert; 2->remove
    int included;

    if(oi->type==REPLACE && oi->src_obj==oi->targ_obj)
	// regenerate view
	return;

    ni->type = NULLOP;

    rtl_node = checked_cast(RTLNode, this->get_root());

    obj = (ObjPtr)oi->src_obj;

    // Make sure that we do not have loop here
    if (obj && rtl_node && obj != rtl_node) {

	switch (oi->type) {
	  case FIRST :
	  case AFTER :

	    op = 1;		// default
	    included = rtl_node->rtl_includes (obj);

	    // if op is to insert this object; check filter
	    if (op == 1) {
		  /* filter_expression dead? */
	    }

	    // if op == 1 insert this
	    if ( (op == 1) && (!included) ) {

		int rtl_size;
		Obj *last_obj = NULL;

		// insert
		symbolArr& array = rtl_node->rtl_contents();
		rtl_size = array.size ();

                if (rtl_size)   
		    for (int ii = rtl_size - 1; ii >= 0; --ii)
		    if (!array[ii].is_xrefSymbol ()) { // eliminate xrefSymbols from propagation
			last_obj = array[ii];
			break;
		    }

		rtl_node->rtl_insert (obj);

		// modify new inserter to propagate further
		if (last_obj) { // insert after
		    ni->type = AFTER;
		    ni->src_obj = oi->src_obj;
		    ni->targ_obj = objTreePtr(last_obj);
		    ni->data = NULL;
		} else {
		    ni->type = FIRST;
		    ni->src_obj = oi->src_obj;
		    ni->targ_obj = rtl_node;
		    ni->data = NULL;
		}
	    } else if ( (op == 2) && included ) {
		// remove
		obj_remove ((RelationalPtr) oi->src_obj);
	    }
	    break;

	  case REPLACE:
	    ni->type = REPLACE;
	    ni->src_obj = oi->src_obj;
	    ni->targ_obj = oi->targ_obj;
	    break;
	}
    }
}

void RTL::remove_obj(objRemover *ro, objRemover *nr)
{
    Initialize (RTL::remove_obj);

    ObjPtr obj;
    RTLNodePtr rtl_node;
    int op = 0; // 0 -> nop; 1-> insert 2-> remove
    int included;

    nr->src_obj = NULL;

    obj = (ObjPtr)ro->src_obj;

    rtl_node = checked_cast(RTLNode, this->get_root());

    if (obj && rtl_node) {

	// Make sure that we don't have loop here
	if (obj != rtl_node) {

	    op = 2;		// default
	    included = rtl_node->rtl_includes (obj);

	    // if op is to insert this object; check filter
	    if (op == 1) {
		/* filter_expression dead? */
	    }

	    // Do the right thing here.
	    if ( (op == 1) && (!included) ) {
		// insert
		obj_insert (FIRST, checked_cast(commonTree, ro->src_obj),
			    rtl_node, NULL);
	    } else if ( (op == 2) && included ) {
		// remove
		rtl_node->rtl_remove (obj);
		nr->src_obj = ro->src_obj;
	    }
	}
    }
}

void RTL::xref_notify(
    const symbolArr& modified, const symbolArr& added, const symbolArr& deleted
    )
{
    Initialize(RTL::xref_notify);

    DBG { 
        obj_prt_obj(this);
        msg("  mad\n") << eom;
	modified.print ();
	added.print ();
	deleted.print ();
    }
}

void RTL::notify(
    int flag, Relational *target, objPropagator*, RelType*)
{
    Initialize (RTL::notify);

    RTLNodePtr rtl_node = checked_cast(RTLNode, get_root());

    if (rtl_node && rtl_node->rtl_includes (target) ) {

	switch (flag) {

          case relationMode::D:
	    obj_remove((RelationalPtr) target);

	    if ((rtl_node->get_dependency()).includes(target)) {

		// Make it udl
		rtl_node->clear_dependency ();
		rtl_node->set_preview (NULL);
	    }

	    break;

          case relationMode::M:		// obj_remove(this, (appTreePtr) target);
	    if (!is_RTL(target) && !is_RTLNode(target))
		obj_insert(REPLACE, checked_cast(commonTree, target),
			   checked_cast(commonTree, target), NULL);
	    break;
	}
    }
    
}

void RTL::send_string(ostream& stream) const
{
    Initialize (RTL::send_string);

    RTLNodePtr rtl_node = checked_cast(RTLNode, this->get_root());
    if(rtl_node)
	rtl_node->send_string(stream);
}


// -------- class RTLNode methods -----------------

init_relational(RTLNode, appTree);

// Constructor
RTLNode::RTLNode(const objString&)
: preview(NULL),
  format_spec(NULL), sort_spec(NULL)
{}

// Constructor
RTLNode::RTLNode(char const *)
: preview(NULL),
  format_spec(NULL), sort_spec(NULL)
{}

// Copy constructor
RTLNode::RTLNode(const RTLNode&)
{}

// Destructor
RTLNode::~RTLNode()
{
    Initialize (RTLNode::~RTLNode);
}

// Sets selection set
void RTLNode::set_selection(symbolArr& new_selection)
{
   Initialize (RTLNode::set_selection);

   selection = new_selection;
}

// Sets format_spec string
void RTLNode::set_format_spec(char const *new_format_spec)
{
   Initialize (RTLNode::set_format_spec);

   format_spec = new_format_spec;
}

// returns format_spec string
char const *RTLNode::get_format_spec()
{
   Initialize (RTLNode::get_format_spec);

   return format_spec;
}

// Sets sort_spec string
void RTLNode::set_sort_spec(char const *new_sort_spec)
{
   Initialize (RTLNode::set_sort_spec);

   sort_spec = new_sort_spec;
}

// returns sort_spec string
char const *RTLNode::get_sort_spec()
{
   Initialize (RTLNode::get_sort_spec);

   return sort_spec;
}

// Sets preview string
void RTLNode::set_preview (char const *new_preview)
{
   Initialize (RTLNode::set_preview);

   preview = new_preview;
}

// returns preview string
char const *RTLNode::get_preview()
{
   Initialize (RTLNode::get_preview);

   return preview;
}

void RTLNode::clear_dependency()
{
   Initialize (RTLNode::clear_dependency);

   Obj *el;
   ForEach (el, dependency)
      rem_relation (boolean_of_rtl, get_header(), RelationalPtr(el)); 

   dependency.removeAll();
}

// Test for inclusion
int RTLNode::rtl_includes(symbolPtr symbol)
{
//   Initialize (RTLNode::rtl_includes);

   return contents.includes (symbol);
}

symbolArr& RTLNode::rtl_contents ()
{
//    Initialize(RTLNode::rtl_contents);

    return contents;
}

// Test for inclusion in selection
int RTLNode::selection_includes (symbolPtr obj)
{
//    Initialize (RTLNode::selection_includes);

    return selection.includes(obj);
}

// Inserts an object in the RTL
void RTLNode::rtl_insert(symbolPtr symbol)
{
    rtl_insert(symbol,0);
}

void RTLNode::rtl_insert(symbolPtr symbol, int check_include)
{
    Initialize (RTLNode::rtl_insert);

    if ( symbol.isnotnull() ){
         // check for the match
            if(!check_include || !rtl_includes(symbol)) {
	        contents.insert_last (symbol);

        	if(!symbol.is_xrefSymbol ())
	            put_relation(app_of_rtl, get_header(), RelationalPtr(symbol));
            }
    }
}

// Inserts an object in RTL selection
void RTLNode::selection_insert (symbolPtr obj)
{
    Initialize (RTLNode::selection_insert);

    if(rtl_includes(obj))
	selection.insert_last (obj);
}

// Remove an object from the RTL.
void RTLNode::rtl_remove (symbolPtr symbol)
{
    Initialize (RTLNode::rtl_remove);

    contents.remove (symbol);
    selection_remove (symbol);
}

// Removes an object from RTL selection
void RTLNode::selection_remove (symbolPtr obj)
{
    Initialize (RTLNode::selection_remove);

    selection.remove (obj);
}


// clears contents and selection of RTL. Do we need to delete preview
// and search criteria??
void RTLNode::clear()
{
    Initialize (RTLNode::clear);

    RTLPtr head = RTLPtr (this->get_header());
    rem_relation(app_of_rtl, head, NULL);

    this->contents.removeAll ();
    // Free selection
    this->selection.removeAll();

    obj_insert(head, REPLACE, this, this, NULL);
}

void RTLNode::send_string(ostream& stream) const
{
   Initialize(RTLNode::send_string);

   char const *name = get_name();
   if(name)
       stream << name;
}

int RTLNode::match (ObjPtr obj)
{
    Initialize (RTLNode::match);

    int result = 1;
    RTLPtr rtl;

    if (!obj)
	return (0);

    // get header
    rtl = checked_cast(RTL, this->get_header ());

    return (result);
}

static void rtl_format_gen(int field_size, char const *str, ostream& stream)
{
    Initialize(rtl_format_gen);

    int size;
    int i;
    char *buff;

    if (str == 0)
        str = "";

    size = strlen(str);

    if (field_size <= 0)
        field_size = size;

    if (size <= field_size) {
        stream << str;
        for (i=size; i<field_size; i++)
            stream << ' ';
    } else {
        buff = new char[field_size+1];
        strncpy (buff, str, field_size);
        buff[field_size-1] = 0;
        if (field_size == 1)
            buff[0] = '.';
        else if (field_size == 2) {
            buff[0] = '.';
            buff[1] = '.';
        } else
            strcpy( &buff[field_size - 3], "...");
        stream << buff;
    }
}


void RTLNode::rtl_format_string (
    commonTreePtr obj, char const *format, ostream& stream)
{
    Initialize(RTLNode::rtl_format_string );

    HierarchicalPtr header;
    char const *ptr;
    int field_size;
    char const *str;

    header = checked_cast(Hierarchical, obj->get_header());

    for (ptr=format; *ptr; ptr++) {
        if (*ptr == '%') {
            ptr++;
            field_size = 0;
            while (isdigit(*ptr)) {
                field_size = field_size * 10 + (*ptr - '0');
                ptr++;
            }
            if (*ptr == 'd') {
                ptr++;
                if(header && is_app (header)) 
		    switch (*ptr) {
		      case 't': // doc type
			switch (header->get_type()) {
			  case App_STE:
			    rtl_format_gen(field_size, "T", stream);
			    break;
			  case App_RTL:
			    rtl_format_gen(field_size, "R", stream);
			    break;
			  case App_SMT:
			    rtl_format_gen(field_size, "C", stream);
			    break;
			  case App_DD:
			    rtl_format_gen(field_size, "D", stream);
			    break;
			  case App_SUBSYS:
			    rtl_format_gen(field_size, "S", stream);
			    break;
			  case App_SCRAPBOOK:
			    rtl_format_gen(field_size, "X", stream);
			    break;
			  case App_PROJECT:
			    rtl_format_gen(field_size, "P", stream);
			    break;
			  case App_UFILE:
			    rtl_format_gen(field_size, "F", stream);
			    break;
			  default:
			    rtl_format_gen(field_size, "U", stream);
			    break;
			}
			break;
		      case 'n': // doc name
			str = header->get_name();
			if (str)
			    rtl_format_gen (field_size, str, stream);
			else
			    rtl_format_gen (field_size, "<noname>", stream);
			break;

		      case 'a': // doc attributes
			rtl_format_gen (field_size, "rwom", stream);
			break;

		      case 'd': // doc date
			rtl_format_gen (field_size, "jan-01-92", stream);
			break;
		    }
            }
            else if (*ptr == 'o') {
                ptr++;
                switch (*ptr) {

		  case 't':	// obj title

                    rtl_format_gen(field_size, "", stream);
                    break;

		  case 's':	// obj string
		    ostr.reset();
                    obj->send_string(ostr);
                    ostr << ends;

                    rtl_format_gen (field_size, ostr.str(), stream);
                    break;
		
		  case 'n':	// obj name
		    rtl_format_gen(field_size, obj->get_name(), stream);
		    break;

		  case 'l':
		      {
		      int linenum = 0;
		      if (is_appTree(obj))
			  linenum = checked_cast(appTree,obj)->get_linenum();
		      if (linenum != 0) {
			  char line_no[8];
			  OSapi_sprintf(line_no,"<line %d>",linenum);
			  rtl_format_gen(field_size, line_no, stream);
		      }
		  }
		    break;
		
		  case 'f':	// obj filename (stuff after last /)
		    {
		    str = obj->get_name();
		    char const *temp = strrchr(str, '/');
	  	    if(temp)
			str = ++temp;
                    rtl_format_gen(field_size, str, stream);
		    }
                    break;
                }
            }
            else {
                stream << *ptr;
            }
        } else {
            stream << *ptr;
        }
    }
}

void RTLNode::rtl_format_string (symbolPtr symbol, char const *format,
				 ostream& stream)
{
    Initialize(RTLNode::rtl_format_string );

    if (!symbol.is_xrefSymbol ()) {
	commonTree* obj = symbol;
	rtl_format_string (obj, format, stream);
	return;
    }

    fsymbolPtr xref = symbol.get_xrefSymbol();
    char const *ptr;
    int field_size;
    char const *str = 0;

    for (ptr=format; *ptr; ptr++) {
        if (*ptr == '%') {
            ptr++;
            field_size = 0;
            while (isdigit(*ptr)) {
                field_size = field_size * 10 + (*ptr - '0');
                ptr++;
            }
            if (*ptr == 'd') {
                ptr++;
		switch (*ptr) {
		  case 't': // doc type
		    rtl_format_gen(field_size, "U", stream);
		    break;

		  case 'n': // doc name
		   {
		    symbolPtr file = xref->get_def_file ();
		    if (file->xrisnotnull()) 
			str = file->get_name ();
		    if (str)
			rtl_format_gen (field_size, str, stream);
		    else
			rtl_format_gen (field_size, "<noname>", stream);
		    break;
		   }

		  case 'a': // doc attributes
		      rtl_format_gen (field_size, "rwom", stream);
		    break;

		  case 'd': // doc date
		      rtl_format_gen (field_size, "jan-01-92", stream);
		    break;
		}
            }		
            else if (*ptr == 'o') {
                ptr++;
                switch (*ptr) {
		  case 't':	// obj title
                    rtl_format_gen (field_size, "", stream);
                    break;

		  case 's':	// obj string
                    str = xref->get_name ();
                    rtl_format_gen (field_size, str, stream);
                    break;
		
		  case 'n':	// obj name
		    str = xref->get_name();
		    rtl_format_gen(field_size,str,stream);
		    break;
		    
		  case 'l':
		    rtl_format_gen(field_size,"",stream);
		    break;
		
		  case 'f':	// obj filename (stuff after last /)
		   {
		    fsymbolPtr xfile = xref->get_def_file ();
		    if (xfile->xrisnotnull())
			str = xfile->get_name();
		    else
			str = "<noname>";
		    char const *temp = strrchr(str,'/');
	  	    if(temp) str = ++temp;
                    rtl_format_gen(field_size,str,stream);
                    break;
		   }
                }
            }
            else {
                stream << *ptr;
            }
        } else {
            stream << *ptr;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                          RTL Sort Utilties                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

typedef int (*rtl_compare_func)(symbolPtr, symbolPtr);

static const int MAX_COMPARE = 6;
static rtl_compare_func rtl_compare_vector[MAX_COMPARE + 1];

static char const *sort_format_spec;		// set int rtl_init_sort,
static RTLNodePtr sort_rtl_node; 	// used in rtl_compare_OF?

//static objArr* rtl_sort_array;		// Set in rtl_init_sort, and
					// Used in rtl_compare_indices
static symbolArr* rtl_sort_array;	// Set in rtl_init_sort, and
					// Used in rtl_compare_indices
//static char sort_type;

inline int compare_names(symbolPtr one, symbolPtr two)
{
    char const *one_name = one.get_name();
    if(one_name == NULL) one_name = "";
    char const *two_name = two.get_name();
    if(two_name == NULL) two_name = "";

    return strcmp(one_name, two_name);
}


inline int get_headers(symbolPtr& one, symbolPtr& two)
{
    if (one.is_xrefSymbol () || two.is_xrefSymbol ()) {
	one = one.get_xrefSymbol ()->get_def_file ();
	two = two.get_xrefSymbol ()->get_def_file ();
    } else {
	one = (Obj *)((commonTree*)one)->get_header();
	two = (Obj *)((commonTree*)two)->get_header();
    }
    if((int)one && (int)two)
	return 0;
    else
    {
	if((int)one) return 1;
	else if((int)two) return 3;
	else return 2;
    }
}

inline int compare_ext(symbolPtr one, symbolPtr two)
{
    char const *one_name = one.get_name();
    if(one_name == NULL) one_name = "";
    char const *two_name = two.get_name();
    if(two_name == NULL) two_name = "";

    char const *one_dot = strrchr(one_name, '.');
    if(one_dot == NULL) one_dot = "";
    char const *two_dot = strrchr(two_name, '.');
    if(two_dot == NULL) two_dot = "";

    return strcmp(one_dot, two_dot);
}

// we need to call rtl_init_sort before calling this to set
// sort_format_spec correctly

static ocharstream s1;		// These should be inside compare_formats,
static ocharstream s2;		// but it will not work that way due to a
				// cfront bug with function-static variables.

int compare_formats(symbolPtr one, symbolPtr two)
{
    s1.reset();

// please leave the ostream casts below intact. SC3.0.1 c++(solaris needs it)
    sort_rtl_node->rtl_format_string(one, sort_format_spec, (ostream &)s1);
    s1 << ends;

    s2.reset();

    sort_rtl_node->rtl_format_string(two, sort_format_spec, (ostream &)s2);
    s2 << ends;

    return strcmp(s1.str(), s2.str());
}


extern "C" void view_icons(symbolPtr, char const *);
int compare_types(symbolPtr one, symbolPtr two)
{
    Initialize(compare_types);

    char icon1[4];
    char icon2[4];
    OS_dependent::bzero(icon1,sizeof(icon1));
    OS_dependent::bzero(icon2,sizeof(icon2));
    
    view_icons(one, icon1);
    view_icons(two, icon2);

    return strcmp(icon1,icon2);
}
    
static int rtl_compare_ONF(symbolPtr one, symbolPtr two)
{
    return compare_names(one, two);
}

static int rtl_compare_ONR(symbolPtr one, symbolPtr two)
{
    return compare_names(two, one);
}

static int rtl_compare_OLF(symbolPtr one, symbolPtr two)
{ 
   return compare_names(one, two);
}

static int rtl_compare_OLR(symbolPtr one, symbolPtr two)
{
    return compare_names(two, one);
}

static int rtl_compare_OTF(symbolPtr one, symbolPtr two)
{
    return compare_types(one, two);
}

static int rtl_compare_OTR(symbolPtr one, symbolPtr two)
{
    return compare_types(two, one);
}

static int rtl_compare_DNF(symbolPtr one, symbolPtr two)
{
    Initialize(rtl_compare_DNF);

    int null_header = get_headers(one, two);
    if(null_header) return null_header - 2;

    return compare_names(one, two);
}

static int rtl_compare_DNR(symbolPtr one, symbolPtr two)
{
    int null_header = get_headers(one, two);
    if(null_header) return null_header - 2;

    return compare_names(two, one);
}

static int rtl_compare_DTF(symbolPtr oneS, symbolPtr twoS)
{
    int null_header = get_headers(oneS, twoS);
    if(null_header) return null_header - 2;

    if (oneS.is_xrefSymbol () || twoS.is_xrefSymbol ())
	return 0;

    commonTree* one = oneS;
    commonTree* two = twoS;
    
    return int(one->get_node_type()) - int(two->get_node_type());
}

static int rtl_compare_DTR(symbolPtr oneS, symbolPtr twoS)
{
    int null_header = get_headers(oneS, twoS);
    if(null_header) return null_header - 2;

    if (oneS.is_xrefSymbol () || twoS.is_xrefSymbol ())
	return 0;

    commonTree* one = oneS;
    commonTree* two = twoS;
    
    return int(two->get_node_type()) - int(one->get_node_type());
}

static int rtl_compare_DXF(symbolPtr one, symbolPtr two)
{
    int null_header = get_headers(one, two);
    if(null_header) return null_header - 2;

    return compare_ext(one, two);
}

static int rtl_compare_DXR (symbolPtr one, symbolPtr two)
{
    int null_header = get_headers(one, two);
    if(null_header) return null_header - 2;

    return compare_ext(two, one);
}

static int rtl_compare_OFF(symbolPtr one, symbolPtr two)
{
    return compare_formats(one,two);
}

static int rtl_compare_OFR(symbolPtr one, symbolPtr two)
{
    return compare_formats(two,one);
}

static int rtl_compare_OXF(symbolPtr one, symbolPtr two)
{
    return compare_ext(one,two);
}

static int rtl_compare_OXR(symbolPtr one, symbolPtr two)
{
    return compare_ext(two, one);
}

static void rtl_compile_criteria(char const *criteria)
{
    static genString last_criteria;

    if(criteria == NULL)	// Default criteria
    {
	rtl_compare_vector[0] = rtl_compare_ONF;
	rtl_compare_vector[1] = NULL;
	last_criteria = NULL;
	return;
    }

    if(criteria  &&  last_criteria.length()  &&  last_criteria == criteria)
	return;			// Same as last time.

    last_criteria = criteria;

    for(int i = 0; i < MAX_COMPARE + 1; ++i)
	rtl_compare_vector[i] = NULL;

    char c;
    rtl_compare_func* vector = rtl_compare_vector;
    while((c = *criteria++)  &&  (vector - rtl_compare_vector < MAX_COMPARE))
    {
	if(isspace(c))
	    continue;

	if(c == 'o')			// OBJECT
	{
	    if(!(c = *criteria++)) break;

	    if(c == 't')		// OBJECT TYPE
	    {
		if(!(c = *criteria++)) break;

		if(c == 'f')		*vector++ = rtl_compare_OTF;
		else if(c == 'r')	*vector++ = rtl_compare_OTR;
	    }
	    else if(c == 'n')		// OBJECT NAME
	    {
		if(!(c = *criteria++)) break;

		if(c == 'f')		*vector++ = rtl_compare_ONF;
		else if(c == 'r')	*vector++ = rtl_compare_ONR;
	    }
            else if(c == 'x')           // OBJECT EXTENSION
            {
                if(!(c = *criteria++)) break;

                if(c == 'f')            *vector++ = rtl_compare_OXF;
                else if(c == 'r')       *vector++ = rtl_compare_OXR;
            }
	    else if(c == 'l')
	    {
		if(!(c = *criteria++)) break;
		
		if(c == 'l')            *vector++ = rtl_compare_OLF;
		else if(c == 'r')       *vector++ = rtl_compare_OLR;
	    }
	    else if(c == 'f')		// OBJECT FORMAT STRING
	    {
		if(!(c = *criteria++)) break;

		if(c == 'f')		*vector++ = rtl_compare_OFF;
                else if(c == 'r')       *vector++ = rtl_compare_OFR;
	    }
	}
	else if(c == 'd')		// DOCUMENT
	{
	    if(!(c = *criteria++)) break;

	    if(c == 't')		// DOCUMENT TYPE
	    {
		if(!(c = *criteria++)) break;

		if(c == 'f')		*vector++ = rtl_compare_DTF;
		else if(c == 'r')	*vector++ = rtl_compare_DTR;
	    }
	    else if(c == 'n')		// DOCUMENT NAME
	    {
		if(!(c = *criteria++)) break;

		if(c == 'f')		*vector++ = rtl_compare_DNF;
		else if(c == 'r')	*vector++ = rtl_compare_DNR;
	    }
	    else if(c == 'x')		// DOCUMENT EXT
	    {
		if(!(c = *criteria++)) break;

		if(c == 'f')		*vector++ = rtl_compare_DXF;
		else if(c == 'r')	*vector++ = rtl_compare_DXR;
	    }
	}
    }
}

static int rtl_compare_ptrs(symbolPtr one, symbolPtr two)
{
    int result = 0;

    if(one.isnotnull() && two.isnotnull())
    {
	for(int i = 0; i <= MAX_COMPARE; ++i)
	{
	    rtl_compare_func func = rtl_compare_vector[i];
	    if(func == NULL  ||  (result = (*func)(one, two)))
		break;
	}
    }
    else if(one.isnotnull())
	result = -1;
    else if(two.isnotnull())
	result = 1;

    return result;
}

int rtl_compare_indices(const void* one_index, const void* two_index)
{
    Initialize(rtl_compare_indices);

    symbolPtr one, two;
    one = (*rtl_sort_array)[*(const int*)one_index];
    two = (*rtl_sort_array)[*(const int*)two_index];
    return rtl_compare_ptrs (one, two);
}

int rtl_compare_elems(symbolPtr one, symbolPtr two, char const *)
{
    return rtl_compare_ptrs(one, two);
}

// call this routine before using rtl_compare_elems to sort the list
int rtl_init_sort(RTLNodePtr rtl_node)
{
    rtl_compile_criteria(rtl_node->get_sort_spec());
    sort_format_spec = rtl_node->get_format_spec();
    sort_rtl_node = rtl_node;
    rtl_sort_array = &rtl_node->rtl_contents ();
    return rtl_sort_array->size ();
}

// return an array of integers that sort the given RTL.
// Caller is responsible for deleting the array.

int* rtl_sort_indices(RTLNodePtr rtl_node)
{
    rtl_init_sort(rtl_node);

    const int count = rtl_sort_array->size();

    if(count == 0)
	return NULL;

    int* indices = new int[count];
    for(int i = 0; i < count; ++i)
	indices[i] = i;

    if(count > 1)
	OSapi_qsort(indices, count, sizeof(int), rtl_compare_indices);

    return indices;
}
