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
#include <cLibraryFunctions.h>
#include <msg.h>
#include "gt.h"
#include "xref.h"
#include "ddict.h"
#include "view_creation.h"
#include "ste_interface.h"
#include "systemMessages.h"
#include "RTLdialog.h"

//-----------------------------------------------------------------------------

//
// dialog context
//

struct odContext {
    RTLdialog*	dialog;
};

typedef odContext* odContextPtr;

//-----------------------------------------------------------------------------

//
// array of contexts
//

static genArr (odContextPtr) ocArr;

//-----------------------------------------------------------------------------

//
// Destroy a dialog in response to WM_DESTROY.
//

static int dialog_destroy_CB (void *data)
{
    Initialize (dialog_destroy_CB);

    RTLdialog*		dlg;
    odContextPtr	ocp;

    for (int i = 0; i < ocArr.size (); i++) {
	if ((ocp = *ocArr[i]) == NULL)
	    continue;
	if ((dlg = ocp->dialog) == NULL)
	    continue;
	if (dlg->get_shell () == (gtDialogTemplate*) data) {
	    ocArr.remove (i);
	    delete dlg;
	    delete ocp;
	    return false;
	}
    }

    return true;
}

//-----------------------------------------------------------------------------

static void open_entity_CB (int button, symbolArr* sel,
						void* ctx, RTLdialog* dlg)
{
    Initialize (open_entity_CB);

    if (ctx == NULL)
	return;

    odContextPtr context = (odContextPtr) ctx;

    if (button >= 0) {
        if ((sel != NULL) && (sel->size () > 0)) {
            view_create ((*sel)[0]);
            view_create_flush ();
	}
    }

    if (button != 0) {
        for (int i = 0; i < ocArr.size (); i++)
	    if (*ocArr[i] == context)
	       ocArr.remove (i);
	delete dlg;
	delete context;
    }
}

//-----------------------------------------------------------------------------

void open_def_of_text (symbolPtr sym)
{
    Initialize (open_def_of_text);

    commonTreePtr	ctp = sym;
    ddElementPtr	assoc_ddep;
    RTLdialog*		dlg;
    symbolArr		syms;
    symbolPtr		sp, assoc;
    Obj*		rel;
    Obj*		ob;

    sp = sym.get_xrefSymbol ();
    if (sp.isnotnull ()) {
	view_create (sp);
	view_create_flush ();
	return;
    }

    if (ctp == NULL)
	return;
    if (ctp->get_first () == NULL)
	ctp = get_ste_parent (ctp);
    if (ctp == NULL)
	return;

    rel = get_relation (soft_assoc_of_app, ctp);

    if (rel != NULL) {
	ForEach (ob, *rel) {
	    if (!ob->relationalp ()) continue;
	    assoc_ddep = checked_cast (ddElement, ob);
	    assoc = assoc_ddep->get_xrefSymbol ();
	    if (assoc.isnull ()) continue;
	    assoc->get_link (sym_of_assoc, syms);
	}
    }

    syms.remove_dup_syms ();
    if (syms.size () < 1) {
	msg("ERROR: No associated entities found.") << eom;
	return;
    }

    odContextPtr	ocp = new odContext;
    odContextPtr*	ocpp = ocArr.append (&ocp);

    dlg = new RTLdialog ("sdoc", TXT("Open Definition"), open_entity_CB, ocp);
    ocp->dialog = dlg;
    
    dlg->init (TXT("Select Entity to Open"), syms);
    dlg->list_style (gtSingle);
    dlg->override_WM_destroy (dialog_destroy_CB);
    dlg->popup ();
}
