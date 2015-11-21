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
#ifdef NEW_UI

#include "RTL_externs.h"
#include "genString.h"
#include "ldrList.h"
#include "proj.h"
#include "gRTListServer.h"
#include "ParaCancel.h"

#include <vport.h>
#include veventHEADER

#include "Application.h"
#include "DIS_ui/interface.h"

#undef Initialize
#include "grtlClient.h"
#define Initialize(name) IniRet _my_name(quote(name))

#undef printf
#undef sprintf

init_relational(RTListServer, RTL);
extern const unsigned char* get_prefix(symbolPtr);

defrel_one_to_many(projHeader,project,RTListServer,rtlServer);
init_rel_or_ptr(projHeader,project,0,RTListServer,rtlServer,0);

//-----------------------------------------------------------------------------

// RTL ids will start at 1, not 0.
int		RTListServer::_uniqId = 1;
RTListServer*	RTListServer::_list = 0;

static iconKind	iconTable[256];

static struct {
    unsigned char	pix;
    iconKind		icon;
} iconConv[] = {
    { PIX_FOLDER + 1,		iconFolder },
    { PIX_FOLDER,		iconHomeFolder },
    { PIX_DOC_UNKNOWN + 1,	iconDocument },
    { PIX_DOC_UNKNOWN,		iconDocument },
    { PIX_DOC_C + 1,		iconCSource },
    { PIX_DOC_C,		iconCSource },
    { PIX_DOC_CPP + 1,		iconCppSource },
    { PIX_DOC_CPP,		iconCppSource },
    { PIX_XFUNCTION,            iconFunction},
    { PIX_XMACRO,               iconMacro}, 
    { PIX_XCLASS,               iconClass},
    { PIX_XSUBSYSTEM,           iconSubsystem},
    { PIX_XUNION,               iconUnion},
    { PIX_XTYPEDEF,             iconTypedef},
    { PIX_XENUMLIST,            iconEnum},
    { PIX_XVARIABLE,            iconVariable},
    { PIX_XFIELD,               iconMember},
    { PIX_RTL,			iconList },
    { PIX_GROUP,	 	iconGroup },
    { PIX_SCRATCHGROUP,	 	iconScratchGroup },
    { PIX_WRITE0,		iconWrite15 },
    { PIX_WRITE1,		iconWrite16 },
    { PIX_WRITE2,		iconWrite17 },
    { PIX_WRITE3,		iconWrite18 },
    { PIX_WRITE4,		iconWrite19 },
    { PIX_WRITE5,		iconWrite20 },
    { PIX_WRITE6,		iconWrite21 },
    { PIX_WRITE7,		iconWrite22 },
    { PIX_WRITE8,		iconWrite23 },
    { PIX_WRITE9,		iconWrite24 },
    { PIX_WRITE10,		iconWrite25 },
    { PIX_WRITE11,		iconWrite26 },
    { PIX_INSTANCE, 	iconInstance },
    { PIX_STRING,	 	iconString },
    { PIX_LOCAL,	 	iconLocal },
    { PIX_SQL_C,	 	iconSQL_C },
    { PIX_SQL_CPLUSPLUS, 	iconSQL_CPlusPlus },
    { PIX_DEFECT,               iconDefect },
    { PIX_DOC_JAVA + 1,         iconJavaSource },
    { PIX_DOC_JAVA,             iconJavaSource },
    { PIX_XCOMPONENT,           iconComponent },
    { PIX_XPACKAGE,             iconPackage },
    { PIX_XINTERFACE,           iconInterface }
};

class localInitForListServer {
    public:
	localInitForListServer (void);
} localInitForListServerInstance;

localInitForListServer::localInitForListServer (void)
{
    Initialize (localInitForListServer::localInitForListServer);

    unsigned j;

    for (unsigned i = 0; i < 256; i++) {
	iconTable[i] = iconUnknown;
	for (j = 0; j < (sizeof (iconConv) / sizeof (*iconConv)); j++) {
	    if (iconConv[j].pix == i) {
		iconTable[i] = iconConv[j].icon;
		break;
	    }
	}
    }
}

//-----------------------------------------------------------------------------

static const char* convertIcons (const unsigned char* pix)
{
    Initialize (convertIcons);

    static char buf[5];

    buf[0] = '\0';

    if (!pix)
	return buf;

    int i;

    for (i = 0; (i < 4) && *pix; i++, pix++)
	buf[i] = (char) (iconTable[*pix] + 1);

    buf[i] = '\0';
    return buf;
}

//-----------------------------------------------------------------------------

RTListServer::RTListServer (int clientId) : RTL (0)
{
    Initialize (RTListServer::RTListServer);

    _caller	= 0;
    _clientId	= clientId;
    _ldr	= new ldrList (this);
    _id		= _uniqId++;
    _type	= rtlUnknown;

    _fmtSort	= "";
    _fmtFormat	= "";
    _fmtShow	= "";
    _fmtHide	= "";

    _next	= _list;
    _list	= this;
}

//-----------------------------------------------------------------------------

void RTListServer::set_caller (vchar * caller) {

    if (strcmp ((char *)caller, "local") == 0) {
	_caller = 1;
    }
}

//-----------------------------------------------------------------------------

RTListServer::~RTListServer (void)
{
    Initialize (RTListServer::~RTListServer);

    RTListServer* p;

    for (RTListServer** pp = &_list; p = *pp; pp = &p->_next) {
	if (p == this) {
	    *pp = p->_next;
	    break;
	}
    }
}

//-----------------------------------------------------------------------------

RTListServer* RTListServer::find (int id)
{
    Initialize (RTListServer::find);

    for (RTListServer* p = _list; p; p = p->_next)
	if (p->id() == id)
	    return p;

    return 0;
}

//-----------------------------------------------------------------------------

int RTListServer::getSize (void) const
{
    Initialize (RTListServer::getSize);

    SortedList* v = _ldr->get_list ((genString&)_fmtSort, (genString&)_fmtFormat
,                 
                                    (genString&)_fmtShow, (genString&)_fmtHide,
                                    _fmtDescend);
    if (!v) return 0;
                  
    return v->sorted.size();

//     return _getArr().size ();
}

//-----------------------------------------------------------------------------

const char* RTListServer::getTitle (void) const
{
    Initialize (RTListServer::getTitle);

    return ((RTListServer*) this)->get_filename ();
}

//-----------------------------------------------------------------------------

int RTListServer::get_unfilteredSize () 
{
    Initialize (RTListServer::get_unfilteredSize);

     return _getArr().size ();
}

//-----------------------------------------------------------------------------

int RTListServer::isFiltered ()
{
    Initialize (RTListServer::isFiltered);

    return (((char *)_fmtShow && _fmtShow.length() > 0) || 
            ((char *)_fmtHide && _fmtHide.length() > 0));
}

//-----------------------------------------------------------------------------

void RTListServer::getFilter (genString& sort, genString& format, genString& show, genString& hide) {

    sort = _fmtSort;
    format = _fmtFormat;
    show = _fmtShow;
    hide = _fmtHide;

}

//-----------------------------------------------------------------------------

const char* RTListServer::getItem (int idx) const
{
    Initialize (RTListServer::getItem);

    static const char* const bad_item = "(Not Defined)";

    if (!_ldr)
	return bad_item;

    SortedList* v = _ldr->get_list ((genString&)_fmtSort, (genString&)_fmtFormat,
				    (genString&)_fmtShow, (genString&)_fmtHide,
				    _fmtDescend);
    if (!v)
	return bad_item;

    if (idx < 0 || idx >= v->sorted.size())
        return bad_item;

    symbolPtr* sym = _ldr->get_obj (v->sorted[idx]->index);
    if (!sym)
	return bad_item;

    const unsigned char* ic = get_prefix (*sym);

    static genString name;
    // name = sym->get_name ();
    name = v->text[v->sorted[idx]->offset+1];
    name += convertIcons (ic);

    return name;
}

//-----------------------------------------------------------------------------

int RTListServer::getSelSize () const
{
    Initialize (RTListServer::getSelSize);

    symbolArr arr;

    getSelArr (arr);
  
    return arr.size();
}

void RTListServer::getSelArr (symbolArr& arr) const
{
    Initialize (RTListServer::getSelArr);

    if (!_ldr)
	return;

    SortedList* v = _ldr->get_list ((genString&)_fmtSort, (genString&)_fmtFormat,
				    (genString&)_fmtShow, (genString&)_fmtHide,
				    _fmtDescend);
    if (!v)
        return;

    const genMask&	s = getSel ();
    symbolPtr*		sym;

    for (int i = s.peek(); i >= 0; i = s.peek (i))
	if (i < v->sorted.size())
	    if (sym = _ldr->get_obj (v->sorted[i]->index))
		arr.insert_last (*sym);
}

//-----------------------------------------------------------------------------

static void gen_range (int first, int last, genString& str, int& gen_comma, int& count)
{
    if (gen_comma) {	
	str += ",";
    } else gen_comma = 1;

    genString segment;
    if (first==last) {
	count++;
	segment.printf ("%d", first);
    } else {
	count += last-first+1;
	segment.printf ("%d-%d", first, last);
    }
    str += segment;
}

int RTListServer::select (symbolArr& arr)
{
    if (!_ldr)
        return 0;
 
    SortedList* v = _ldr->get_list ((genString&)_fmtSort, (genString&)_fmtFormat
,
                                    (genString&)_fmtShow, (genString&)_fmtHide,
                                    _fmtDescend);
    if (!v)
        return 0;
 
    genMask mask;

    int i;
    for (i=0; i<v->sorted.size(); i++) {
        symbolPtr* sym = _ldr->get_obj (v->sorted[i]->index);
        if (!sym)
            continue;
 	for (int j=0; j<arr.size(); j++) {
	    if (*sym == arr[j]) {
	        mask += i;
	        arr.remove (*sym);
                break;
	    }
	}
    }

    int count = 0;
    int first, last;
    int gen_comma = 0;
    genString str;
    bool got = false;
    for (i=first=last=mask.peek(); i >= 0; i = mask.peek (i)) {
        got = true;
        if (i > last+1) {
	    gen_range (first, last, str, gen_comma, count);
            first = last = i;
	} else {
	    last = i;
	    continue;
        }
    }
    if (got) {
        gen_range (first, last, str, gen_comma, count);
        genString cmd;
        cmd.printf ("dis_rtl_select %d {%s}", _clientId, (char*)str);
        rcall_dis_DISui_eval_async (Application::findApplication("DISui"), (vstr*)(char*)cmd);
    }

    return count;
}

//-----------------------------------------------------------------------------

int RTListServer::setSel (const genMask& sel)
{
    Initialize (RTListServer::setSel);

    _sel.clear ();

    //------------------------------------------------------------
    // given selection indeces are with respect to the sorted list
    //------------------------------------------------------------

    if (!_ldr)
	return 0;

    SortedList* v = _ldr->get_list ((genString&)_fmtSort, (genString&)_fmtFormat,
				    (genString&)_fmtShow, (genString&)_fmtHide,
				    _fmtDescend);
    if (!v)
	return 0;

    for (int i = sel.peek(); i >= 0; i = sel.peek (i))
	if (i < v->sorted.size())
	    _sel += i; //v->sorted[i]->index;

    return 1;
}

//-----------------------------------------------------------------------------

void RTListServer::insert (const symbolArr& syms)
{
    Initialize (RTListServer::insert);

    _getArr().insert_last (syms);

    _propagate ();
}

//-----------------------------------------------------------------------------

void RTListServer::insert (symbolPtr sym)
{
    Initialize (RTListServer::insert);

    RTLNode* n = checked_cast (RTLNode, get_root());
    if (!n)
	return;

    n->rtl_insert (sym);

    _sel.clear ();
    _propagate ();
}

//-----------------------------------------------------------------------------

void RTListServer::clear (void)
{
    Initialize (RTListServer::clear);

    RTLNode* n = checked_cast (RTLNode, get_root());
    if (!n)
	return;

    n->clear ();

    _sel.clear ();
    //_propagate ();
}

//-----------------------------------------------------------------------------


void RTListServer::queryProjects (const RTListServer* src, int pos)
{
    Initialize (RTListServer::queryProjects);

    app*	ap;
    char*	mytitle = 0;

    if (src) {

        symbolPtr	sym = src->getSortedEntry (pos);
        symbolPtr	xsym = sym.get_xrefSymbol ();
        Relational*	rp = sym;

	if (rp && is_RTL (rp)) {
	    ap = checked_cast (app, rp);
	    mytitle = ap->get_filename ();
	}
	else if (xsym.isnotnull()) {
	    if (xsym.get_kind() != DD_PROJECT)
	        return;
	    ap = sym.get_def_app ();
	    if (!ap || !is_projHeader (ap))
	        return;
	    mytitle = ap->get_filename ();
        }
	else return;
	_type = rtlProject;
    }
    else {
	ap = checked_cast (app, projNode::get_project_rtl());
	if (!ap)
	    return;
	mytitle = "Top Level Projects";
	_type = rtlTopProject;
    }

    RTLNode* nd = checked_cast (RTLNode, ap->get_root());
    if (!nd)
        return;

    _setList (nd, mytitle);
}

//-----------------------------------------------------------------------------

void RTListServer::queryProjects (char *projname, int* retval)
{
    Initialize (RTListServer::queryProjects);

    projNode* proj_node = projNode::find_project(projname);
    RTLNode* nd = checked_cast (RTLNode, proj_node);
    if (!nd){
        *retval = 0;
        return;
    }

    char       mytitle[200];

    if(strcmp(projname, "/")) {
        strcpy(mytitle, projname);
        _type = rtlProject;
    } else { 
        strcpy(mytitle, "Top Level Projects");
        _type = rtlTopProject;
    }

    _setList (nd, mytitle);
    *retval = 1;
}

//-----------------------------------------------------------------------------

void RTListServer::queryParentProject (void)
{
    Initialize (RTListServer::queryParentProject);

    app*	ap = 0;
    char*	mytitle = 0;

    Obj* ob = get_relation (project_of_rtlServer, this);
    if (!ob || !ob->relationalp())
	return;

    Relational* rp = (Relational*) ob;
    if (!is_projNode (rp))
	return;

    Obj* parent_ob = get_relation (parentProject_of_childProject, rp);
    if (parent_ob && parent_ob->relationalp()) {
	Relational* parent_rp = (Relational*) parent_ob;
	if (is_projNode (parent_rp)) {
	    projNode* parent_pn = checked_cast (projNode, parent_rp);
	    ap = checked_cast (app, parent_pn->get_header());
	    if (ap && is_projHeader (ap))
		mytitle = ap->get_filename ();
	}
    }

    if (!ap) {
	ap = checked_cast (app, projNode::get_project_rtl());
	mytitle = "Top Level Projects";
	_type = rtlTopProject;
    }

    RTLNode* nd = checked_cast (RTLNode, ap->get_root());
    if (!nd)
	return;

    _setList (nd, mytitle);
}

//-----------------------------------------------------------------------------

void RTListServer::resetFilter ()
{
    _fmtShow = "";
    _fmtHide = "";
}

//-----------------------------------------------------------------------------

void RTListServer::setFilter (char *sort, char *format, char *show, char *hide) {

    _fmtSort = sort;
    _fmtFormat = format;
    _fmtShow = show;
    _fmtHide = hide;
}

//-----------------------------------------------------------------------------

void RTListServer::_setList (RTLNode* nd, const char* mytitle)
{
    Initialize (RTListServer::_setList);

    if (!nd)
	return;

    rem_relation (project_of_rtlServer, this, 0);

    ParaCancel::set();
    if (is_projNode (nd)) {
	projNode* pn = checked_cast (projNode, nd);
	put_relation (project_of_rtlServer, this, pn);
	pn->refresh ();
    }
    ParaCancel::reset();

    if (mytitle)
        set_filename ((char*) mytitle);
    else set_filename ("");

    clear ();
//    _getArr().insert_last (nd->rtl_contents());
    symbolArr& proj_contents = nd->rtl_contents();

    int sz = proj_contents.size();
    for (int ii = 0; ii < sz; ++ii)
    {
      symbolPtr& el = proj_contents[ii];
      Relational* obj = el;
      if(is_projNode(obj)){
         projNode* pr = (projNode*)obj;

	 if( !pr->is_visible_proj() ) continue;
      }
      _getArr().insert_last(el);
    }

    _propagate ();
}

//-----------------------------------------------------------------------------

symbolArr& RTListServer::_getArr (void) const
{
    Initialize (RTListServer::_getArr);

    static symbolArr empty;

    RTLNode* n = checked_cast (RTLNode, get_root());
    if (!n) {
	empty.removeAll ();
	return empty;
    }

    return n->rtl_contents ();
}

//-----------------------------------------------------------------------------

symbolPtr RTListServer::getSortedEntry (int pos) const
{
    Initialize (RTListServer::getSortedEntry);

    if ((pos < 0) || !_ldr)
	return NULL_symbolPtr;

    SortedList* v = _ldr->get_list ((genString&)_fmtSort, (genString&)_fmtFormat,
				    (genString&)_fmtShow, (genString&)_fmtHide,
				    _fmtDescend);

    if (!v || (pos >= v->sorted.size()))
	return NULL_symbolPtr;

    symbolArr&	arr = _getArr ();
    int		idx = v->sorted[pos]->index;

    if ((idx < 0) || (idx >= arr.size()))
	return NULL_symbolPtr;

    return arr[idx];
}

//-----------------------------------------------------------------------------

void RTListServer::_propagate (void)
{
    Initialize (RTListServer::_propagate);

    RTLNode* n = checked_cast (RTLNode, get_root());
    if (!n)
        return;

    ::obj_insert (this, REPLACE, n, n, 0);

    if (_caller == 1) {
#ifdef NEW_UI
 	rtlClient* rtlc = rtlClient::find (_clientId);
	if (rtlc) rtlc->update();
#endif
    }

    else {
        rcall_gc_rtl_refresh (Application::findApplication("DISui"), _clientId);
    }
}

#endif
