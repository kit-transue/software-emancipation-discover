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

#ifndef _ggraViewLayer_h
#define _ggraViewLayer_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#undef printf
#undef sprintf
#undef vsprintf

#include <glayer.h>
#include <gString.h>
#include <viewTypes.h>
#include <symbolArr.h>
#include <ggeneral.h>

class View;
class view; //This is aset's view class
class vdomainitem;
class dialog;

class graViewLayer : public Layer {

public:
    graViewLayer (symbolArr&, viewtype, View *, const vchar *title = (const vchar *)"");
    graViewLayer (view *, View *, const vchar *title = (const vchar *)"");
    ~graViewLayer ();

    void 		getName 	(gString&) 	const;
    void 		getResourceName	(gString&) 	const;
    const vname* 	getState 	(void) 		const;
    const char *	findPopupMenu	(const int, const int);

    symbolArr   &GetSelection ();
    viewtype    GetViewType ();
    view*       GetGraView();
    vdomainitem *GetDomainItem();

private:
    void		display		(void);
    void		undisplay	(void);

    gString 		title_;
    symbolArr		symbols_;
    viewtype		vt_;
    view		*m_pView;
    dialog              *dlg;
};

    inline symbolArr &graViewLayer::GetSelection () { return symbols_;}
    inline viewtype graViewLayer::GetViewType () { return vt_;}
    inline view* graViewLayer::GetGraView() { return m_pView; }


view *dis_get_gra_view();

struct dagraph_info
{
  symbolArr source;
  symbolArr target;
  gString device_name;
};

#endif // _ggraViewLayer_h
