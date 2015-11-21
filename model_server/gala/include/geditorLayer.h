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
// geditorLayer.h
//
// Editor layer class definition.
//-----------------------------------------------------------------------------

#ifndef _editorLayer_h
#define _editorLayer_h

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

#ifndef _TextFile_h
#include "TextFile.h"
#endif

//-----------------------------------------------------------------------------

class viewLayer;
class vtext;

//-----------------------------------------------------------------------------

class editorLayer : public Layer {

    public:

        editorLayer (void);

        virtual void removeFromView (void);
        void displayViewLayer (viewLayer*);

        int		isEditor	(void)		const;
        void		getName		(gString&)	const;
        const vname*	getState	(void)		const;

        void SetTextData(TextFile*);

        ~editorLayer (void);

    private:

        static unsigned long	serial_;

        gString		name_;
        viewLayer*  viewlayer_;
        int			window_;
        int			has_frame_;
        TextFile* oldTextData_;

        void display (void);
        void undisplay (void);
};

//-----------------------------------------------------------------------------

class transientEditorLayer : public editorLayer {
    public:
        transientEditorLayer (void);
	int isTransient (void) const;
};

//-----------------------------------------------------------------------------

class viewLayer 
{
    public:

        static viewLayer* findViewLayer (int);
        static viewLayer* findViewLayer (const vchar*);
        static viewLayer* viewLayerList_;

        viewLayer (void);

        const vchar*getTitle (void) const;
        void		setTitle (const vchar*);

        void		displayInView (View*);
        viewLayer*	getNext (void) const;

        ~viewLayer();

    private:
    
        static int	uniqId_;

        int		id_;
        vchar*		title_;
        viewLayer*	next_;
        TextFile* textFile_;
};

#endif // _editorLayer_h
