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
#ifndef _XREFQUERY_H
#define _XREFQUERY_H

#include <xref_queries.h>
#include <genMask.h>
#include <symbolArr.h>
#include <symbolSet.h>
#include <ddKind.h>
#include <projectBrowser.h>
#include <linkTypes.h>

class XrefQuery
{
  public:
    static void DoQueries(const symbolArr& selected_symbols, 
			  genMask queries, RTLPtr rtl_head);

  private:
    static bool Init();
    static bool do_1_query(symbolSet& accum_results, symbolArr& domain, int action);
    static bool get_link(symbolArr& domain, linkType link, 
			 ddKind kind, symbolArr& rtl_root_arr, int trg);
    static int  get_link_local(symbolArr& domain, linkTypes& la, int knd, 
			       symbolArr& rtl_root_arr, int trg);
    static void remove_paraset_internals(symbolArr& result);
    static void filter_kind(ddKind kind, symbolArr& result, 
			    symbolArr& rtl_root_arr, const symbolPtr xrsym, 
			    const int trg, const linkType& link);
    static bool do_before_get_link(symbolSet& accum_results, 
				   symbolArr& filtered_domain, int action);
    static int filter_src_before_query(const symbolArr& sel_syms, 
				       symbolArr& result, int action);

    static int GetIndex(int action);
  public:

    struct Query
    {
	uint Type;
	genMask PreFilter;
	ddKind Kind;
        genMask Links;
    };


    //    friend int sub_query_local(symbolArr&, linkTypes&, int, symbolArr&, int);
  private:
    
    static Query m_Queries[SHOW_LAST];
};

#endif

