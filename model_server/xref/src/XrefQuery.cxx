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
#include <XrefQuery.h>
#include <ParaCancel.h>
#include <msg.h>
#include <systemMessages.h>


extern "C" void push_busy_cursor();
extern "C" void pop_cursor();
void project_query(symbolPtr, linkType, ddKind, symbolArr&);
extern int add_one_level_syms(linkType lt, ddSelector sel_src, ddSelector sel_trg, symbolArr & syms);
extern int get_modified_objects(symbolArr& , symbolArr&);
void find_overrides(symbolArr& src, symbolArr & dst);
void find_instances(symbolPtr& s, symbolArr *res);

XrefQuery::Query XrefQuery::m_Queries[];

int XrefQuery::GetIndex(int action)
{
    int index = -1;
    for (int i = 0; i < SHOW_LAST; i++)
    {
	if (m_Queries[i].Type == action)
	{
	    index = i;
	    break;
	}
    }
    return index;
}

// Decode links (a bitmap) into individual sub-queries and return the
// union of these queries into rtl_head
void XrefQuery::DoQueries(const symbolArr& selected_symbols, 
			  genMask queries, RTLPtr rtl_head)
{
    Initialize(XrefQuery::DoQueries);
    static bool initialized = Init();

    RTLNodePtr	rtl_root = checked_cast (RTLNode, rtl_head->get_root ());

    if (queries.includes ((unsigned) SHOW_REF_FILE))
	queries.add ((unsigned) SHOW_DOCUMENTS);

    rtl_root->clear ();
    symbolSet accum_results;
    
    for (int i = queries.peek(); i >= 0; i = queries.peek(i))
    {
	symbolSet action_results;

	symbolArr domain = selected_symbols;
	
	if (!do_1_query(action_results, domain, i)) 
	  return;
	
	accum_results.insert(action_results);
	
	if(ParaCancel::is_cancelled())
	    return;
    } /* for */
    
    rtl_root->rtl_contents() = accum_results;
}

//////////////////////////////////////////////////////////////////////////////////
//perform a query and append result to accum_results
bool XrefQuery::do_1_query(symbolSet& accum_results, symbolArr& domain, int action)
{
    Initialize(XrefQuery::do_1_query);

    symbolArr results;
    symbolArr filtered_domain;
   
    if (!filter_src_before_query(domain, filtered_domain, action)) 
    {
	domain.removeAll();
	return false;
    }

    if (do_before_get_link(accum_results, filtered_domain, action))
    {
	int index = GetIndex(action);
	if (index == -1) return false;
	genMask links = m_Queries[index].Links;
	ddKind kind = m_Queries[index].Kind;
	for (int i = links.peek(); i >= 0; i = links.peek(i))
	{
	    get_link(filtered_domain, linkType(i), kind, results, action);
	}
    }

    domain.removeAll();
    symbolPtr result;
    ForEachS(result, results) 
    {
	if (!accum_results.includes(result)) 
	{
	    accum_results.insert(result);
	    domain.insert_last(result);
	}
    }

    return true;
}

bool XrefQuery::get_link(symbolArr& domain, linkType link, 
			 ddKind kind, symbolArr& rtl_root_arr, int trg)
//
// Perform a single sub-query and add< the results to an RTL.
//
{
    Initialize(XrefQuery::get_link);

    push_busy_cursor();

    int retval = 0;
    linkTypes la;
    la = link;
    int knd = (int)kind;

    retval = get_link_local(domain, la, knd, rtl_root_arr, trg);
    
    pop_cursor();
    return (retval != 0);
}

///////////////////////////////////////////////////////////////////////////
// Perform a single sub-query and add< the results to an RTL.
int  XrefQuery::get_link_local(symbolArr& domain, linkTypes& la, int knd, 
				symbolArr& rtl_root_arr, int trg)
{
    Initialize(XrefQuery::get_link_local);
    linkType link = la;
    ddKind kind = (ddKind)knd;
    
    if (link == have_arg_type) 
    { // do transitive closure for structures
      ddSelector sel_src;
      ddSelector sel_trg;
      sel_src.add(DD_CLASS);
      sel_src.add(DD_INTERFACE);
      sel_src.add(DD_UNION);
      sel_trg.add(DD_FUNC_DECL);
      sel_trg.add(DD_FIELD);
      sel_trg.add(DD_VAR_DECL);
      
      add_one_level_syms(is_using, sel_src, sel_trg, domain);
    }
    
    symbolPtr dsym;
    ForEachS(dsym, domain)
    {
	// queries work on the xrefSymbol for each sym
	symbolPtr xrsym = dsym.get_xrefSymbol();

	if(xrsym.isnull()) 
	    continue;
	
	// get all links of this type
        symbolArr result;
	
	// if the object is a project, it gets special treatment
        if(xrsym.get_kind() == DD_PROJECT) 
	{
	    project_query(xrsym, link, kind, result);
	}
	else if (xrsym.get_kind() == DD_MODULE &&
		 dsym.relationalp() && 
		 is_projModule((Relational *)dsym)) 
	{
	    // if the object in question is a module, find the xrefsymbol 
	    // with the right links 

            // projNode *project = xrsym->get_projNode();
	    genString xrsym_name;
	    xrsym.get_name(xrsym_name);
	    // projModule *mod = project->find_module(xrsym_name);
            projModule *mod = checked_cast(projModule, dsym);
	    if (mod == 0) 
	    {
		msg("WARN: File $1 does not exist.") << xrsym_name.str() << eom;
	    } 
	    else 
	    {
		projNode *project = mod->get_project();
		xrsym = mod->get_def_file_from_proj(project);
		if(xrsym.isnull())
		    continue;
		else
		    xrsym->get_link_chase_typedefs(link, result);
	    }
	}
#ifdef XXX_assoc //	else if (xrsym.get_kind () == DD_SOFT_ASSOC) 
	{
	    if (link == assoc_of_sym)
		result.insert_first (xrsym);
	    else 
		xrsym->get_link_chase_typedefs (link, result);
#endif //	}
	else if (xrsym.get_kind () == DD_MODULE) 
	{
	    if (link == is_defining)
		xrsym->get_link (link, result);
	    else
		xrsym->get_link_chase_typedefs (link, result);
	}
	else if ((xrsym.get_kind() == DD_CLASS) ||
    	    	 (xrsym.get_kind() == DD_INTERFACE)) 
	{
            // Do not include typedefs for member_of (nested classes) link.
            if (link == member_of) {
                xrsym->get_link(link, result);
            }
            else {
	        if (link == used_by)
		    xrsym->get_link_chase_typedefs (arg_type_of, result);
			
	        xrsym->get_link_chase_typedefs (link, result);
            }
	}
	else if (xrsym.get_kind() == DD_SEMTYPE) 
	{
	    xrsym->get_link_chase_typedefs (link, result);
	    if (link == used_by)
		xrsym->get_link_chase_typedefs(type_has, result);
	}
	else if (link == grp_has_pub_mbr || link == grp_has_pri_mbr) 
	{
	    xrsym->get_link(link, result);	/*  don't chase typedefs */
	}
	else 
	    xrsym->get_link_chase_typedefs(link, result);

	//remove PARASET internal objects from the result list
	remove_paraset_internals(result);

	if(ParaCancel::is_cancelled())
	{
	    return 0;
	}

	// filter found links according to the input kind
	filter_kind(kind, result, rtl_root_arr, xrsym, trg, link);

    } // ForEachS(dsym, domain)

    return 1;
}

///////////////////////////////////////////////////////////////////////////
//remove PARASET internal objects from the result list
void XrefQuery::remove_paraset_internals(symbolArr& result)
{
    Initialize(XrefQuery::remove_paraset_internals);

    symbolPtr rsym;
    symbolArr tmp_arr = result;
    int ind, j;

    for (ind = 0, j = 0; ind < tmp_arr.size(); ++ind, ++j)
    {
	rsym = tmp_arr[ind];
        ddKind curr_kind = rsym.get_kind();

        if (curr_kind == DD_IFL_SRC || curr_kind == DD_IFL_TRG ||
            curr_kind == DD_REL_SRC || curr_kind == DD_REL_TRG ||
	    /*curr_kind == DD_ASSOC_CONST ||*/ curr_kind == DD_LINKNODE) 
	{
	    result.remove_index(j);
            j --;
	}
     }
}

///////////////////////////////////////////////////////////////////////////////////
// filter found links according to the input kind
void XrefQuery::filter_kind(ddKind kind, symbolArr& result, 
			    symbolArr& rtl_root_arr, const symbolPtr xrsym, 
			    const int trg, const linkType& link)
{
    Initialize(XrefQuery::filter_kind);
    symbolPtr rsym;

    if(kind >= 0)
    {
	ddKind xrsym_kind = xrsym.get_kind();
	char const *xrsym_name = xrsym.get_name();

	ForEachS(rsym, result)
	{
	    if ( rsym.get_kind() == kind) 
	    {
		if ((trg == SHOW_MEMBER_FUNCS || trg == SHOW_MEMBER_DATA) && 
		    (xrsym_kind == DD_CLASS || xrsym_kind == DD_INTERFACE) &&
		    (link == is_using)) 
		{
		    char const *rsym_name = rsym.get_name();
                    if (xrsym_name && rsym_name) {
                        if (!strchr(xrsym_name, '@')) {
			    if (strstr(rsym_name, xrsym_name) == rsym_name) {
			        rtl_root_arr.insert_last(rsym);
                             }
                        }
                        else {
                            /* Handle symbols that have context attached to the 
                               name of the symbol, for example, anonymous classes
                               in Java. The name and the context should be 
                               matched separately.
                             */
                            const char *xr_context = strchr(xrsym_name, '@');
                            int xrlen = xr_context - xrsym_name - 1;
                            xr_context++;

                            const char *r_context = strchr(rsym_name, '@');
                            if (r_context) r_context++;

                            if (r_context && xr_context &&
                                (strncmp(rsym_name, xrsym_name, xrlen) == 0) &&
                                (strcmp(r_context, xr_context) == 0)) {
			        rtl_root_arr.insert_last(rsym);
                            }
                        }
                    }
		}
		else
		    rtl_root_arr.insert_last(rsym);
	    }
	}
    }
    else
    {
        ForEachS(rsym, result)
	    rtl_root_arr.insert_last(rsym);
    }
}

//////////////////////////////////////////////////////////////////////////////////
// special processing for some queries
bool XrefQuery::do_before_get_link(symbolSet& accum_results, 
				   symbolArr& filtered_domain, int action)
{
    Initialize(XrefQuery::do_before_get_link);

    bool ret = true;

    switch (action)
    {
      case SHOW_MODIFIED_OBJECTS: 
    {
	symbolArr modified_objects;
	get_modified_objects(filtered_domain, modified_objects);
	accum_results.insert(modified_objects);
	break;
    }
      case SHOW_OVERRIDES:
    {
	symbolArr overrides;
	find_overrides(filtered_domain, overrides);
	accum_results.insert(overrides);
	break;
    } 
      case SHOW_INSTANCES: 
    {
	symbolArr instances;
	symbolPtr sym;
	ForEachS(sym, filtered_domain) 
	{
	    find_instances(sym, &instances);
	}
	accum_results.insert(instances);
	break;
    }
      default:
        break;
    } //switch (action)

    return ret;
}

///////////////////////////////////////////////////////////////////
// check that selected symbols make sense cosidering given query
int XrefQuery::filter_src_before_query (const symbolArr& sel_syms, 
					symbolArr& result, int action)
{
    Initialize(XrefQuery::filter_src_before_query);

    int index = GetIndex(action);
    if (index == -1) return 0;
    genMask filter = m_Queries[index].PreFilter;
    if (filter.empty())
    {
	result = sel_syms;
    }
    else
    {
	symbolPtr sym;
	for (int i = 0; i < sel_syms.size(); ++i)
	{
	    sym = (sel_syms[i]).get_xrefSymbol();
	    if (sym.isnull()) 
		continue;
	    ddKind ki = sym.get_kind();
	    if (filter.includes(ki))
		result.insert_last(sym);
	}
    }
    
    return result.size();
}
	
/////////////////////////////////
bool XrefQuery::Init()
{
    m_Queries[0].Type = SHOW_IS_DEFINED_IN;
    m_Queries[0].PreFilter.clear();
    m_Queries[0].Kind = ddKind(-1);
    m_Queries[0].Links.add(is_defined_in);

    m_Queries[1].Type = SHOW_REF_FILE;
    m_Queries[1].PreFilter.clear();
    m_Queries[1].Kind = ddKind(-1);
    m_Queries[1].Links.add(ref_file);

    m_Queries[2].Type = SHOW_INCLUDED_BY;
    m_Queries[2].PreFilter.add(DD_MODULE);
    m_Queries[2].Kind = ddKind(DD_MODULE);
    m_Queries[2].Links.add(included_by);

    m_Queries[3].Type = SHOW_USED_BY;
    m_Queries[3].PreFilter.clear();
    m_Queries[3].Kind = ddKind(-1);
    m_Queries[3].Links.add(used_by);

    m_Queries[4].Type = SHOW_IS_USING;
    m_Queries[4].PreFilter.clear();
    m_Queries[4].Kind = ddKind(-1);
    m_Queries[4].Links.add(is_using);
    m_Queries[4].Links.add(file_ref);

    m_Queries[5].Type = SHOW_IS_USING_FUNCTIONS;
    m_Queries[5].PreFilter.clear();
    m_Queries[5].Kind = ddKind(DD_FUNC_DECL);
    m_Queries[5].Links.add(is_using);
    m_Queries[5].Links.add(file_ref);

    m_Queries[6].Type = SHOW_IS_USING_DATA_MEMBERS;
    m_Queries[6].PreFilter.clear();
    m_Queries[6].Kind = ddKind(DD_FIELD);
    m_Queries[6].Links.add(is_using);
    m_Queries[6].Links.add(file_ref);


    m_Queries[7].Type = SHOW_IS_USING_VARIABLES;
    m_Queries[7].PreFilter.clear();
    m_Queries[7].Kind = ddKind(DD_VAR_DECL);
    m_Queries[7].Links.add(is_using);
    m_Queries[7].Links.add(file_ref);
    
    m_Queries[8].Type = SHOW_IS_USING_CLASSES;
    m_Queries[8].PreFilter.clear();
    m_Queries[8].Kind = ddKind(DD_CLASS);
    m_Queries[8].Links.add(is_using);
    m_Queries[8].Links.add(file_ref);
    
    m_Queries[9].Type = SHOW_IS_USING_STRUCTURES;
    m_Queries[9].PreFilter.clear();
    m_Queries[9].Kind = ddKind(DD_CLASS);
    m_Queries[9].Links.add(is_using);
    m_Queries[9].Links.add(file_ref);

    m_Queries[10].Type = SHOW_IS_USING_UNIONS;
    m_Queries[10].PreFilter.clear();
    m_Queries[10].Kind = ddKind(DD_UNION);
    m_Queries[10].Links.add(is_using);
    m_Queries[10].Links.add(file_ref);
    
    m_Queries[11].Type = SHOW_IS_USING_ENUMS;
    m_Queries[11].PreFilter.clear();
    m_Queries[11].Kind = ddKind(DD_ENUM);
    m_Queries[11].Links.add(is_using);
    m_Queries[11].Links.add(file_ref);
   
    m_Queries[12].Type = SHOW_IS_USING_ENUM_VAL;
    m_Queries[12].PreFilter.clear();
    m_Queries[12].Kind = ddKind(DD_ENUM_VAL);
    m_Queries[12].Links.add(is_using);
    m_Queries[12].Links.add(file_ref);

    m_Queries[13].Type = SHOW_IS_USING_TYPEDEFS;
    m_Queries[13].PreFilter.clear();
    m_Queries[13].Kind = ddKind(DD_TYPEDEF);
    m_Queries[13].Links.add(is_using);
    m_Queries[13].Links.add(file_ref);

    m_Queries[14].Type = SHOW_IS_USING_MACROS;
    m_Queries[14].PreFilter.clear();
    m_Queries[14].Kind = ddKind(DD_MACRO);
    m_Queries[14].Links.add(is_using);
    m_Queries[14].Links.add(file_ref);
  
    m_Queries[15].Type = SHOW_IS_USING_STRINGS;
    m_Queries[15].PreFilter.clear();
    m_Queries[15].Kind = ddKind(DD_STRING);
    m_Queries[15].Links.add(is_using);
    m_Queries[15].Links.add(file_ref);

    m_Queries[16].Type = SHOW_IS_USING_LOCALS;
    m_Queries[16].PreFilter.clear();
    m_Queries[16].Kind = ddKind(DD_LOCAL);
    m_Queries[16].Links.add(is_using);
    m_Queries[16].Links.add(file_ref);
    
    m_Queries[17].Type = SHOW_IS_DEFINING;
    m_Queries[17].PreFilter.add(DD_MODULE);
    m_Queries[17].PreFilter.add(DD_PROJECT);
    m_Queries[17].Kind = ddKind(-1);
    m_Queries[17].Links.add(is_defining);

    m_Queries[18].Type = SHOW_IS_DEFINING_FUNCTIONS;
    m_Queries[18].PreFilter.add(DD_MODULE);
    m_Queries[18].PreFilter.add(DD_PROJECT);
    m_Queries[18].Kind = ddKind(DD_FUNC_DECL);
    m_Queries[18].Links.add(is_defining);

    m_Queries[19].Type = SHOW_IS_DEFINING_VARIABLES;
    m_Queries[19].PreFilter.add(DD_MODULE);
    m_Queries[19].PreFilter.add(DD_PROJECT);
    m_Queries[19].Kind = ddKind(DD_VAR_DECL);
    m_Queries[19].Links.add(is_defining);

    m_Queries[20].Type = SHOW_IS_DEFINING_CLASSES;
    m_Queries[20].PreFilter.add(DD_MODULE);
    m_Queries[20].PreFilter.add(DD_PROJECT);
    m_Queries[20].Kind = ddKind(DD_CLASS);
    m_Queries[20].Links.add(is_defining);

    m_Queries[21].Type = SHOW_IS_DEFINING_STRUCTURES;
    m_Queries[21].PreFilter.add(DD_MODULE);
    m_Queries[21].PreFilter.add(DD_PROJECT);
    m_Queries[21].Kind = ddKind(DD_CLASS);
    m_Queries[21].Links.add(is_defining);

    m_Queries[22].Type = SHOW_IS_DEFINING_UNIONS;
    m_Queries[22].PreFilter.add(DD_MODULE);
    m_Queries[22].PreFilter.add(DD_PROJECT);
    m_Queries[22].Kind = ddKind(DD_UNION);
    m_Queries[22].Links.add(is_defining);

    m_Queries[23].Type = SHOW_IS_DEFINING_ENUMS;
    m_Queries[23].PreFilter.add(DD_MODULE);
    m_Queries[23].PreFilter.add(DD_PROJECT);
    m_Queries[23].Kind = ddKind(DD_ENUM);
    m_Queries[23].Links.add(is_defining);

    m_Queries[24].Type = SHOW_IS_DEFINING_ENUM_VALS;
    m_Queries[24].PreFilter.clear();
    m_Queries[24].Kind = ddKind(DD_ENUM_VAL);
    m_Queries[24].Links.add(is_defining);

    m_Queries[25].Type = SHOW_IS_DEFINING_FIELDS;
    m_Queries[25].PreFilter.clear();
    m_Queries[25].Kind = ddKind(DD_FIELD);
    m_Queries[25].Links.add(is_defining);

    m_Queries[26].Type = SHOW_IS_DEFINING_TYPEDEFS;
    m_Queries[26].PreFilter.add(DD_MODULE);
    m_Queries[26].PreFilter.add(DD_PROJECT);
    m_Queries[26].Kind = ddKind(DD_TYPEDEF);
    m_Queries[26].Links.add(is_defining);

    m_Queries[27].Type = SHOW_IS_DEFINING_MACROS;
    m_Queries[27].PreFilter.add(DD_MODULE);
    m_Queries[27].PreFilter.add(DD_PROJECT);
    m_Queries[27].Kind = ddKind(DD_MACRO);
    m_Queries[27].Links.add(is_defining);

    m_Queries[28].Type = SHOW_NESTED;
    m_Queries[28].PreFilter.clear();
    m_Queries[28].Kind = ddKind(-1);
    m_Queries[28].Links.add(member_of);

    m_Queries[29].Type = SHOW_NESTED_CLASSES;
    m_Queries[29].PreFilter.clear();
    m_Queries[29].Kind = ddKind(DD_CLASS);
    m_Queries[29].Links.add(member_of);

    m_Queries[30].Type = SHOW_NESTED_UNIONS;
    m_Queries[30].PreFilter.clear();
    m_Queries[30].Kind = ddKind(DD_UNION);
    m_Queries[30].Links.add(member_of);

    m_Queries[31].Type = SHOW_NESTED_ENUMS;
    m_Queries[31].PreFilter.clear();
    m_Queries[31].Kind = ddKind(DD_ENUM);
    m_Queries[31].Links.add(member_of);

    m_Queries[32].Type = SHOW_NESTED_TYPEDEFS;
    m_Queries[32].PreFilter.clear();
    m_Queries[32].Kind = ddKind(DD_TYPEDEF);
    m_Queries[32].Links.add(member_of);

    m_Queries[33].Type = SHOW_CONTAINING;
    m_Queries[33].PreFilter.clear();
    m_Queries[33].Kind = ddKind(-1);
    m_Queries[33].Links.add(parent_of);

    m_Queries[34].Type = SHOW_IS_INCLUDING;
    m_Queries[34].PreFilter.add(DD_MODULE);
    m_Queries[34].Kind = ddKind(-1);
    m_Queries[34].Links.add(is_including);

    m_Queries[35].Type = SHOW_MEMBER_FUNCS;
    m_Queries[35].PreFilter.add(DD_CLASS);
    m_Queries[35].PreFilter.add(DD_INTERFACE);
    m_Queries[35].PreFilter.add(DD_UNION);
    m_Queries[35].PreFilter.add(DD_TEMPLATE);
    m_Queries[35].Kind = ddKind(DD_FUNC_DECL);
    m_Queries[35].Links.add(is_using);

    m_Queries[36].Type = SHOW_MEMBER_DATA;
    m_Queries[36].PreFilter.add(DD_CLASS);
    m_Queries[36].PreFilter.add(DD_UNION);
    m_Queries[36].PreFilter.add(DD_TEMPLATE);
    m_Queries[36].Kind = ddKind(DD_FIELD);
    m_Queries[36].Links.add(is_using);

    m_Queries[37].Type = SHOW_FRIENDS;
    m_Queries[37].PreFilter.add(DD_CLASS);
    m_Queries[37].PreFilter.add(DD_UNION);
    m_Queries[37].PreFilter.add(DD_TEMPLATE);
    m_Queries[37].Kind = ddKind(-1);
    m_Queries[37].Links.add(have_friends);

    m_Queries[38].Type = SHOW_HAS_SUPERCLASS;
    m_Queries[38].PreFilter.add(DD_CLASS);
    m_Queries[38].PreFilter.add(DD_INTERFACE);
    m_Queries[38].PreFilter.add(DD_UNION);
    m_Queries[38].PreFilter.add(DD_TEMPLATE);
    m_Queries[38].Kind = ddKind(-1);
    m_Queries[38].Links.add(has_superclass);

    m_Queries[39].Type = SHOW_HAS_SUBCLASS;
    m_Queries[39].PreFilter.add(DD_CLASS);
    m_Queries[39].PreFilter.add(DD_INTERFACE);
    m_Queries[39].PreFilter.add(DD_UNION);
    m_Queries[39].PreFilter.add(DD_TEMPLATE);
    m_Queries[39].Kind = ddKind(-1);
    m_Queries[39].Links.add(has_subclass);
#ifdef XXX_assoc
    m_Queries[40].Type = SHOW_DEFINED_AS;
    m_Queries[40].PreFilter.add(DD_FUNC_DECL);
    m_Queries[40].PreFilter.add(DD_FIELD);
    m_Queries[40].PreFilter.add(DD_VAR_DECL);
    m_Queries[40].PreFilter.add(DD_TYPEDEF);
    m_Queries[40].PreFilter.add(DD_SEMTYPE);
    m_Queries[40].PreFilter.add(DD_LOCAL);
    m_Queries[40].PreFilter.add(DD_NUMBER);
    m_Queries[40].PreFilter.add(DD_STRING);
    m_Queries[40].PreFilter.add(DD_SOFT_ASSOC);
    m_Queries[40].Kind = ddKind(-1);
    m_Queries[40].Links.add(has_type);
    m_Queries[40].Links.add(assocType_of_instances);
#endif
    m_Queries[41].Type = SHOW_ENUM_MEMBERS;
    m_Queries[41].PreFilter.add(DD_ENUM);
    m_Queries[41].Kind = ddKind(DD_ENUM_VAL);
    m_Queries[41].Links.add(is_using);

    m_Queries[42].Type = SHOW_TEMPLofINST;
    m_Queries[42].PreFilter.clear();
    m_Queries[42].Kind = ddKind(DD_TEMPLATE);
    m_Queries[42].Links.add(has_template);
      
    m_Queries[43].Type = SHOW_INSTofTEMPL;
    m_Queries[43].PreFilter.add(DD_TEMPLATE);
    m_Queries[43].Kind = ddKind(-1);
    m_Queries[43].Links.add(template_of);
    m_Queries[44].Type = SHOW_DECL_USAGE;

    m_Queries[44].PreFilter.add(DD_CLASS);
    m_Queries[44].PreFilter.add(DD_FUNC_DECL);
    m_Queries[44].PreFilter.add(DD_FIELD);
    m_Queries[44].PreFilter.add(DD_VAR_DECL);
    m_Queries[44].Kind = ddKind(DD_CLASS);
    m_Queries[44].Links.add(have_arg_type);
#ifdef XXX_assoc
    m_Queries[45].Type = SHOW_ASSOC_OF_SYM;
    m_Queries[45].PreFilter.clear();
    m_Queries[45].Kind = ddKind(-1);
    m_Queries[45].Links.add(assoc_of_sym);
    m_Queries[45].Links.add(instances_of_assocType);

    m_Queries[46].Type = SHOW_SYM_OF_ASSOC;
    m_Queries[46].PreFilter.add(DD_SOFT_ASSOC);
    m_Queries[46].Kind = ddKind(-1);
    m_Queries[46].Links.add(sym_of_assoc);
#endif
    m_Queries[47].Type = SHOW_DOCUMENTS;
    m_Queries[47].PreFilter.clear();
    m_Queries[47].Kind = ddKind(-1);
    m_Queries[47].Links.add(ref_file);
    m_Queries[47].Links.add(is_defined_in);

    m_Queries[48].Type = SHOW_INSTANCES;
    m_Queries[48].PreFilter.clear();
    m_Queries[48].Kind = ddKind(-1);
    m_Queries[48].Links.clear();

    m_Queries[49].Type = SHOW_IS_DEFINING_SUBSYSTEMS;  
    m_Queries[49].PreFilter.clear();
    m_Queries[49].Kind = ddKind(DD_SUBSYSTEM);
    m_Queries[49].Links.add(is_defining);

    m_Queries[50].Type = SHOW_GRP_MEMBERS;
    m_Queries[50].PreFilter.add(DD_SUBSYSTEM);
    m_Queries[50].Kind = ddKind(-1);
    m_Queries[50].Links.add(grp_has_pub_mbr);
    m_Queries[50].Links.add(grp_has_pri_mbr);

    m_Queries[51].Type = SHOW_GRP_PEERS;
    m_Queries[51].PreFilter.add(DD_SUBSYSTEM);
    m_Queries[51].Kind = ddKind(DD_SUBSYSTEM);
    m_Queries[51].Links.add(grp_has_peer);
    m_Queries[51].Links.add(grp_is_peer_of);

    m_Queries[52].Type = SHOW_GRP_CLIENTS;
    m_Queries[52].PreFilter.add(DD_SUBSYSTEM);
    m_Queries[52].Kind = ddKind(DD_SUBSYSTEM);
    m_Queries[52].Links.add(grp_has_client);

    m_Queries[53].Type = SHOW_GRP_SERVERS;
    m_Queries[53].PreFilter.add(DD_SUBSYSTEM);
    m_Queries[53].Kind = ddKind(DD_SUBSYSTEM);
    m_Queries[53].Links.add(grp_has_server);

    m_Queries[54].Type = SHOW_GRP_OF_MEMBER;
    m_Queries[54].PreFilter.clear();
    m_Queries[54].Kind = ddKind(DD_SUBSYSTEM);
    m_Queries[54].Links.add(grp_pub_mbr_of);
    m_Queries[54].Links.add(grp_pri_mbr_of);

    m_Queries[55].Type = SHOW_IS_DEFINING_RELATIONS;
    m_Queries[55].PreFilter.clear();
    m_Queries[55].Kind = ddKind(DD_RELATION);
    m_Queries[55].Links.add(is_defining);

    m_Queries[56].Type = SHOW_IS_DEFINING_ERRORS;
    m_Queries[56].PreFilter.clear();
    m_Queries[56].Kind = ddKind(DD_BUGS); // bogus; placeholder in query that should not be used
    m_Queries[56].Links.add(is_defining);

    m_Queries[57].Type = SHOW_MODIFIED_OBJECTS;
    m_Queries[57].PreFilter.add(DD_MODULE);
    m_Queries[57].Kind = ddKind(-1);
    m_Queries[57].Links.clear();

    m_Queries[58].Type = SHOW_OVERRIDES;
    m_Queries[58].PreFilter.clear();
    m_Queries[58].Kind = ddKind(-1);
    m_Queries[58].Links.clear();
    
    m_Queries[59].Type = SHOW_DECLARED_IN;
    m_Queries[59].PreFilter.clear();
    m_Queries[59].Kind = ddKind(-1);
    m_Queries[59].Links.add(declared_in);

    m_Queries[60].Type = SHOW_DECLARES;
    m_Queries[60].PreFilter.clear();
    m_Queries[60].Kind = ddKind(-1);
    m_Queries[60].Links.add(declares);

    m_Queries[61].Type = SHOW_EXTENDS_SUPERCLASS;
    m_Queries[61].PreFilter.add(DD_CLASS);
    m_Queries[61].Kind = ddKind(DD_CLASS);
    m_Queries[61].Links.add(has_superclass);

    m_Queries[62].Type = SHOW_IMPLEMENTED_BY_SUBCLASS;
    m_Queries[62].PreFilter.add(DD_INTERFACE);
    m_Queries[62].Kind = ddKind(DD_CLASS);
    m_Queries[62].Links.add(has_subclass);

    m_Queries[63].Type = SHOW_IMPLEMENTS_SUPERINTERFACE;
    m_Queries[63].PreFilter.add(DD_CLASS);
    m_Queries[63].Kind = ddKind(DD_INTERFACE);
    m_Queries[63].Links.add(has_superclass);

    m_Queries[64].Type = SHOW_EXTENDED_BY_SUBINTERFACE;
    m_Queries[64].PreFilter.add(DD_INTERFACE);
    m_Queries[64].Kind = ddKind(DD_INTERFACE);
    m_Queries[64].Links.add(has_subclass);

    m_Queries[65].Type = SHOW_NESTED_INTERFACES;
    m_Queries[65].PreFilter.clear();
    m_Queries[65].Kind = ddKind(DD_INTERFACE);
    m_Queries[65].Links.add(member_of);

    m_Queries[66].Type = SHOW_IS_DEFINING_INTERFACES;
    m_Queries[66].PreFilter.add(DD_MODULE);
    m_Queries[66].PreFilter.add(DD_PROJECT);
    m_Queries[66].Kind = ddKind(DD_INTERFACE);
    m_Queries[66].Links.add(is_defining);
    
    m_Queries[67].Type = SHOW_IS_USING_INTERFACES;
    m_Queries[67].PreFilter.clear();
    m_Queries[67].Kind = ddKind(DD_INTERFACE);
    m_Queries[67].Links.add(is_using);
    m_Queries[67].Links.add(file_ref);

    m_Queries[68].Type = SHOW_NESTED_PACKAGES;
    m_Queries[68].PreFilter.clear();
    m_Queries[68].Kind = ddKind(DD_PACKAGE);
    m_Queries[68].Links.add(member_of);

    m_Queries[69].Type = SHOW_GENERATED;
    m_Queries[69].PreFilter.add(DD_MACRO);
    m_Queries[69].Kind = ddKind(-1);
    m_Queries[69].Links.add(have_friends);

    m_Queries[70].Type = SHOW_MACROS;
    m_Queries[70].PreFilter.clear();
    m_Queries[70].Kind = ddKind(DD_MACRO);
    m_Queries[70].Links.add(friends_of);
    
    m_Queries[71].Type = SHOW_IS_USING_MODULES;
    m_Queries[71].PreFilter.clear();
    m_Queries[71].Kind = ddKind(DD_MODULE);
    m_Queries[71].Links.add(is_using);
    m_Queries[71].Links.add(file_ref);

    return true;
}


