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
// glayer.h
//
// Layer class definition.
//-----------------------------------------------------------------------------

#ifndef _layer_h
#define _layer_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vnameINCLUDED
#include vnameHEADER
#endif

#ifndef _generic_h
#include "ggeneric.h"
#endif

#ifndef _gArray_h
#include "gArray.h"
#endif

#ifndef _gString_h
#include "gString.h"
#endif

#ifndef _interp_h
#include "ginterp.h"
#endif

//-----------------------------------------------------------------------------

class View;
class Pane;
class Layer;
class layerStack;

//-----------------------------------------------------------------------------

typedef Layer* layerPtr;
gArray (layerPtr);

//-----------------------------------------------------------------------------

class Layer {

    public:

	//--------------
	// class methods
	//--------------

	static layerStack*		getList			(void);
	static void			updateLayerList		(void);

	//------------
	// constructor
	//------------

	Layer (void);

	//--------------------
	// information methods
	//--------------------

	int			getId		(void)		const;
	View*			getView		(void)		const;
	Pane*			getPane		(void)		const;
	Tcl_Interp*		getInterp	(void)		const;

	virtual int		isTransient	(void)		const;
	virtual int		isEditor	(void)		const;
	virtual void		getName		(gString&)	const;
	virtual void		getResourceName	(gString&)	const;
	virtual const vname*	getState	(void)		const;
	virtual void reopen (void);

	//-----------------
	// evil method which evaluates a predifined script:
	//-----------------
	int PrepareShutdown();

	//-----------------
	// instance methods
	//-----------------

	virtual void displayInView(View*);    // Attaches layer to view and displays it.
	virtual void removeFromView(void);    // Detaches layer from view.

	static Layer* find_layer (int id);
	static Layer* find_layer (const char* resource_name);
	static Layer* findPreviousLayer(Layer* l);

	virtual const char *findPopupMenu (const int x, const int y);

	void setInited();
	int isInited();

	//-----------
	// destructor
	//-----------

	virtual ~Layer (void);

    friend class View;
    friend class editorLayer;

    protected:
	void addToView(View*);        // Add layer to view.
	void moveToTop(void);         // Moves layer to top of stack and displays it.

	void		setInterp	(Tcl_Interp*);
	void		setView		(View*);
	void		setPane		(Pane*);

	virtual void	display		(void) = 0;
	virtual void	undisplay	(void) = 0;

    protected:

	//-----------
	// class data
	//-----------

	static int		uniqId_;
	static layerStack*	list_;

	//--------------
	// instance data
	//--------------

	int		id_;
	View*		view_;
	Pane*		pane_;
	Tcl_Interp*	interp_;
	int 	        inited_;
};

//-----------------------------------------------------------------------------

class layerStack : public gArrayOf (layerPtr) {

   public:

	void	push		(Layer*);

	void	pop		(void);
	void	takeOut		(Layer*);

	Layer*	top 		(void)		const;
};

//-----------------------------------------------------------------------------

inline int Layer::getId (void) const
{
    gInit (Layer::getId);
 
    return id_;
}
 
//-----------------------------------------------------------------------------
 
inline View* Layer::getView (void) const
{
    gInit (Layer::getView);
 
    return view_;
}
 
//-----------------------------------------------------------------------------

inline Pane* Layer::getPane (void) const
{
    gInit (Layer::getPane);

    return pane_;
}
 
//-----------------------------------------------------------------------------
 
inline Tcl_Interp* Layer::getInterp (void) const
{
    gInit (Layer::getInterp);
 
    return interp_;
}

//-----------------------------------------------------------------------------

inline void Layer::setInterp (Tcl_Interp* interp)
{
    gInit (Layer::setInterp);

    interp_ = interp;
}
 
//-----------------------------------------------------------------------------

inline void Layer::setPane (Pane* pane)
{
    gInit (Layer::setPane);

    pane_ = pane;
}
 
#endif // _layer_h
