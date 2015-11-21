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
/*
 * Extract-Subsystems.C
 *
 * application <-> ui interface layer
 *
 */

#include <cLibraryFunctions.h>
#include <xxinterface.h>
#include "Entity.h"
#include "autosubsys.h"
#include "autosubsys-macros.h"
#include "waiter.h"
#include "extract-dialog.h"
#include <browserShell.h>
#include <RTL_externs.h>
#include <top_widgets.h>
#ifndef AUTOSUBSYS_WEIGHTS_H
#include <autosubsys-weights.h>
#endif

/*
 * These are where the weights for subsystem extraction are actually kept.
 * Each weight structure contains a relation and an integer weight to be
 * used for calculating bindings.
 *
 * isinitialized is a flag set when the weight array has been initialized
 * to its default values. One of the "features" of C++ is that you can't
 * initialize arrays of objects except with a no-arguments constructor.
 *
 * threshold is a variable to hold the "threshold" of binding for the 
 * analysis. 
 *
 * disjointflag determines what is done with existing subsystems. See
 * autosubsys-macros.h.
 * 
 */

weight defaultweights[NWEIGHTS];
static int isinitialized=0;
int threshold;
int num_subsys;
static int disjointflag=0;
projModulePtr  app_get_mod(appPtr);
/*
 * At some point it would not be unreasonable to have each weight contain
 * both the forward and reverse relations, rather than have the first half
 * of the array contain the forward ones, and the second half the reverse
 * ones. 
 * 
 * If you insert new relations here, make sure the "outbound" reference
 * goes first (in the low half of the array) and the "inbound" reference goes
 * in the second half. Several things - including the dead-code analysis -
 * depend on this.
 *
 */

#define DEFWT(a,b,c,d,e) 		\
   defaultweights[a].outgoing = b;		\
   defaultweights[a+NWEIGHTS/2].outgoing=c;	\
   defaultweights[a].count = e;		\
   defaultweights[a+NWEIGHTS/2].count=e;\
   defaultweights[a].m_weight=d;

extern "C" void setupdefaults() {
  if (isinitialized) return;
  DEFWT(AUS_FCALL,	true,	false,	10, -1);
  DEFWT(AUS_DATAREF,	true,	false, 	10, -1);
  DEFWT(AUS_INSTANCE,	true,	false,	10, -1);
  DEFWT(AUS_ARGTYPE,	true,	false, 	10, -1);
  DEFWT(AUS_RETTYPE,	true,	false,	10, -1);
  DEFWT(AUS_ELEMENT,	false,	true,	10, -1);
  DEFWT(AUS_FRIEND,	false,	false,	15, -1);
  DEFWT(AUS_SUBCLASS,	true,	false,	10, -1);
  DEFWT(AUS_MEMBER,	true,	false,	15, -1);
  DEFWT(AUS_FILENAME,   false,	false,	10, 0);
  DEFWT(AUS_LOGICNAME,  false,	false,	10, 0);
  threshold = 15;
  isinitialized=1;
}


/*
 * The commandContext class is supposed to provide an hourglass cursor and
 * stuff like that. This derived class is just a wrapper around the decomposer
 * class.
 *
 */

class decomposerContext: public commandContext {
  protected:
    decomposer* d;
    RTLNode *set2;
    symbolArr roots;
    browserShell* bs;
  public:
    decomposerContext(gtBase *parent, int);  /* dead code only */
    ~decomposerContext() {}
    virtual void execute();
    void set_roots(const symbolArr&);
    void set_browser_shell(browserShell*);
};

/*
 * for starting a full analysis
 *
 */

static decomposer* pending_decomposer;

/*
 * for starting a dead-code analysis
 *
 * the extra arg is to help insure the two constructors don't accidentally
 * become confused.
 */

decomposerContext::decomposerContext(gtBase *parent, int) :
      commandContext(parent, "Dormant", 5), d(pending_decomposer), bs(NULL) {
  Initialize(decomposerContext::decomposerContext [dormant code]);

  if (!d) {
     d = new decomposer;
     pending_decomposer = d;
  }
  set2 = NULL;
}

/*
 * start the operation
 *
 */

void decomposerContext::execute(void) {
   Initialize(decomposerContext::execute);
   
   if (bs) {
      projectBrowser * pb = bs->get_project_browser();
      if(pb) d->set_extraction_domainlist(pb->project_selection());
      bs->update_domains();
      (bs->top_level())->popdown();
   }
   
   d->go_deadcode_only(roots, defaultweights, NWEIGHTS);
   delete pending_decomposer;
   pending_decomposer = NULL;
}

//------------------------------------------
// decomposerContext::set_roots(...)
//------------------------------------------

void decomposerContext::set_roots(const symbolArr& new_roots) {
   Initialize(decomposerContext::set_roots);

   roots = new_roots;
}

void decomposerContext::set_browser_shell(browserShell* shell) {
   bs = shell;
}


/* interface functions to ui stuff */

extern "C" {
    
    void decompose_roots(void* decomp, symbolArr& roots) {
	Initialize(decompose_roots);
	
	if (decomp) {
	  decomposerContext* decon = (decomposerContext*) decomp;
	  decon->set_roots(roots);
	  decon->start();
	  delete decon;
	}
	_li(LIC_DORMANT);    
    }

  void doFindDeadCode(gtBase *parent) {
    setupdefaults();
    decomposerContext* deadcode = new decomposerContext(parent, 0); /* 0 has no effect */
    browserShell* bsp = new browserShell(parent,
       				   "Select project and roots for scan",
       				   decompose_roots, deadcode);
#ifndef NEW_UI
    ((gtDialog *)bsp->top_level())->set_default_parent(NULL); // To prevent error message disappearing.
#endif
    bsp->disable_apply_button();
    deadcode->set_browser_shell(bsp);
  }

/*
 * Note that we create this RTL to hold the subsystems we create.
 * We delete the RTL when we're done. (yes, it's important that 
 * finishDecomposition() gets called.) If the user wants to undo
 * the run, we delete the subsystems it contains as well. 
 * Otherwise it would be very difficult to pick out the subsystems
 * we actually created - and especially hard to tell them from 
 * ones we created on past runs and shouldn't step on.
 *
 * The RTL pointer gets held by the user interface until disposed of.
 *
 */

  void finishDecomposition(RTL *r) {
    obj_delete(r);
    delete pending_decomposer;
    pending_decomposer = NULL;
  }

  void cancelDecomposition() {
     delete pending_decomposer;
     pending_decomposer = NULL;
  }

  int autosubsys_get_threshold(void) {
    return threshold;
  }

  void autosubsys_set_threshold(int t) {
    threshold = t;
  }

  int autosubsys_get_disjoint(void) {
    return disjointflag;
  }

  void autosubsys_set_disjoint(int d) {
    disjointflag = d;
  }

  int autosubsys_get_weight(int which) {
    setupdefaults();
    return (which<0 ? -1 : defaultweights[which].m_weight);
  }

  void autosubsys_set_weight(int which, int val) {
    setupdefaults();
    if (which>=0) {
      defaultweights[which].m_weight = val;
      defaultweights[which+NWEIGHTS/2].m_weight = val;
	/* fwd and reverse relations have same weight */
    }
  }

  int autosubsys_get_count(int which) {
    setupdefaults();
    return (which<0 ? -1 : defaultweights[which].count);
  }

  void autosubsys_set_count(int which, int val) {
    setupdefaults();
    if (which>=0) {
      defaultweights[which].count = val;
      defaultweights[which+NWEIGHTS/2].count = val;
    }
  }

  int autosubsys_get_subsys_count() {
     return num_subsys;
  }

  void autosubsys_set_subsys_count(int cnt) {
     num_subsys = cnt;
  }

}

/*
 * Note: this file is fairly dependent on the stuff in the subsystem 
 * directory; one should make sure it gets recompiled at the appropriate
 * times. In the long run, now that the UI is in C++, what's here should
 * be split between the actual interface and the subsystem directory.
 *
 */
