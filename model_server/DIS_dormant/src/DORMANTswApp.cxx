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
/**********
 *
 * DORMANTswApp.C - DORMANT/sw Application class def.
 *
 **********/

// Local DORMANT includes.
// This include has galaxy defs.
#include <gRTListServer.h>


#include "../include/DORMANTswApp.h"
#include <genError.h>
#include <projList.h>


//--------------     class DORMANTswApp static members   ---------------------

// Private status flag
bool DORMANTswApp::DORMANTswIsRunning = vFALSE;

// Returns vTRUE iff DORMANT/sw application already running.
bool DORMANTswApp::isRunning()
{
    Initialize(DORMANTswApp::isRunning);

    return (DORMANTswIsRunning);
}



//----------     class DORMANTswApp Public Members     ----------


// class constructor.
DORMANTswApp::DORMANTswApp()
{
    Initialize(DORMANTswApp::DORMANTswApp);

    isInitialized = false;
}



// class destructor.
DORMANTswApp::~DORMANTswApp()
{
    Initialize(DORMANTswApp::~DORMANTswApp);

}



// Start up the DORMANT/sw application services.
bool DORMANTswApp::Startup()
{
    Initialize(DORMANTswApp::Startup);

    if (DORMANTswIsRunning == vFALSE) {
        DORMANTswIsRunning = vTRUE;
    }

    return(vTRUE);
}



// Shutdown the DORMANT/sw application services.
bool DORMANTswApp::Shutdown()
{
    Initialize(DORMANTswApp::Shutdown);

    return (vTRUE);
}



// Update the Roots RTL.
bool DORMANTswApp::UpdateRoots(int rootId, int sourceId)
{
    Initialize(DORMANTswApp::UpdateRoots);

    RTListServer* rootRTL = RTListServer::find(rootId);
    RTListServer* sourceRTL = RTListServer::find(sourceId);

    // Set the selection array of the Source RTL to the Root RTL.
    symbolArr source(0);
    sourceRTL->getSelArr(source);
    rootRTL->clear();
    rootRTL->insert(source);

    return (vTRUE);
}


// Do analysis and extract the dead code
bool DORMANTswApp::Extract(int rootId)
{
    Initialize(DORMANTswApp::Extract);

    RTListServer* rootRTL = RTListServer::find(rootId);

    if (rootRTL) {
        symbolArr roots(0);
        roots = rootRTL->getArr();
        decomposer *d = new decomposer;
        setupDefaults();
        d->go_deadcode_only(roots, defaultweights, NWEIGHTS); 
    }

    return(vTRUE);
}

//---------     class DORMANTswApp Privane Members     ----------


/*
 * (Stolen from ui/browser/extract-subsystem.C)
 * This is where the weights for subsystem extraction are actually kept.
 * Each weight structure contains a relation and an integer weight to be
 * used for calculating bindings.
 *
 * isInitialized is a flag set when the weight array has been initialized
 * to its default values. One of the "features" of C++ is that you can't
 * initialize arrays of objects except with a no-arguments constructor.
 *
 */

#define DEFWT(a,b,c,d,e) 		\
   defaultweights[a].outgoing = b;		\
   defaultweights[a+NWEIGHTS/2].outgoing=c;	\
   defaultweights[a].count = e;		\
   defaultweights[a+NWEIGHTS/2].count=e;\
   defaultweights[a].m_weight=d;

void DORMANTswApp::setupDefaults() {
  if (isInitialized) return;
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
  isInitialized=true;
}





/**********     end of DORMANTswApp.C     **********/
