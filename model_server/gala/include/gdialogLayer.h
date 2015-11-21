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
//-----------------------------------------------------------------------------
// gdialogLayer.h
//
// Dialog layer class definition.
//-----------------------------------------------------------------------------

#ifndef _gdialogLayer_h
#define _gdialogLayer_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vkindINCLUDED
#include vkindHEADER
#endif

#ifndef vnameINCLUDED
#include vnameHEADER
#endif

#ifndef _gArray_h
#include "gArray.h"
#endif

#ifndef _gString_h
#include "gString.h"
#endif

#ifndef _layer_h
#include "glayer.h"
#endif

//-----------------------------------------------------------------------------

class glistItem;
class rtlClient;

//-----------------------------------------------------------------------------

class dialogLayer : public Layer {

public:

    dialogLayer (const char*);

    void		getName		(gString&)	const;
    void		getResourceName	(gString&)	const;
    const vname*	getState	(void)		const;
	void reopen ();

    const char *findPopupMenu (const int x, const int y); 

    ~dialogLayer (void);

private:

    static unsigned long	serial_;
    gString			name_;
    int                         unloadedPane_;
    vchar *                     dialogName_;

    void display (void);
    void undisplay (void);
};

#endif // _dialogLayer_h
