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
#include <gviewerShell.h>
#include <ggraViewLayer.h>
#include <gview.h>

gviewerShell *gviewerShell::currentViewer_ = NULL;

gviewerShell::gviewerShell (viewtype vt, symbolArr& symbols) {

    currentViewer_ = this;
    createView (vt, symbols);
}

gviewerShell::gviewerShell (view *vp) {

    currentViewer_ = this;
    createView (vp);
}

gviewerShell::~gviewerShell() {
    if (currentViewer_ == this) currentViewer_ = NULL;
}

void gviewerShell::createView (viewtype vt, symbolArr& symbols) {

    vt_ = vt;
    symbols_ = symbols;

    View *v = find_view (GetViewCode()); // is the current view
    const vchar *label;

    switch (vt) {
      case viewCallTree:
	label = (const vchar *) "Tree Diagram";
      break;

      case viewFlowchart:
	label = (const vchar *) "Flow Chart";
      break;

      case viewOutline:
	label = (const vchar *) "Outline";
      break;

      case viewInheritance:
	label = (const vchar *) "Inheritance";
      break;

      case viewDatachart:
	label = (const vchar *) "Data Chart";
      break;

      case viewGraph:
	label = (const vchar *) "Graph";
      break;

      case viewERD:
	label = (const vchar *) "ERD";
      break;

      case viewSubsysBrowser:
	label = (const vchar *) "Subsystem";
      break;

      default:
	label = (const vchar *) "Unknown";
      break;
    }

    graViewLayer* vl = new graViewLayer (symbols, vt, v, label);
    // vl->displayInView (v);

}

void gviewerShell::createView (view *vp)
{
    View *v = find_view (GetViewCode()); // is the current view

    graViewLayer* vl = new graViewLayer (vp, v);
}
