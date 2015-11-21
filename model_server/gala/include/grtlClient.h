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
#ifndef _rtlClient_h
#define _rtlClient_h

#ifndef vportINCLUDED
#include <vport.h>
#endif
#ifndef vlistitemINCLUDED
#include vlistitemHEADER
#endif
#ifndef vcharINCLUDED
#include vcharHEADER
#endif
#ifndef vdictINCLUDED
#include vdictHEADER
#endif
#ifndef vbuttonINCLUDED
#include vbuttonHEADER
#endif
#ifndef vkeyINCLUDED
#include vkeyHEADER
#endif

#ifndef _generic_h
#include "ggeneric.h"
#endif
#ifndef _rtlCommon_h
#include "grtlCommon.h"
#endif
#ifndef _gArray_h
#include "gArray.h"
#endif
#ifndef _gString_h
#include "gString.h"
#endif

#include <gcontrolObjects.h>

//-----------------------------------------------------------------------------

class vevent;
class vimage;
class vbutton;
class vcontainer;
class vspringdom;
class vspringdomObject;

class Pane;
class Layer;
class filterButton;
class clientButton;
class gCenteredLabel;

struct Tcl_Interp;

//-----------------------------------------------------------------------------

typedef vspringdomObject* sdObjPtr;
gArray (sdObjPtr);

//-----------------------------------------------------------------------------

class rtlClient : public gdListItem {

    public:    

        vkindTYPED_FULL_DECLARATION (rtlClient);
        vloadableINLINE_CONSTRUCTORS (rtlClient, gdListItem);
												
	//--------------
	// class methods
	//--------------

        static rtlClient*	find		(int);
        static void		addCommands	(Tcl_Interp*);

	//-----------------------
	// initialization methods
	//-----------------------

        void	postLoadInit		(Layer*);
        void	doInit			(void);
        void    setFilterButton 	(vchar *filterName);
	void 	set_caller		(vchar *);

	//----------------------
	// custom button methods
	//----------------------

        clientButton*	addButton		(clientButton*);
        clientButton*	addImageButton		(vimage*);
        clientButton*	addTextButtonScribed	(vscribe*);
        void		removeButton		(clientButton*, int = 1);
        void		deleteAllButtons	(void);

	//--------------------
	// information methods
	//--------------------

	int		getId			(void) const;
	int		getServerId		(void) const;
        Layer*		getLayer		(void) const;
        const vchar*	getItem			(int);
	int             isFiltered		(void);

	//--------
	// queries
	//--------

        void		queryLinks		(rtlClient*, const vchar*);
        void		queryLink		(rtlClient*, const vchar*);
        void		queryCategories		(const vchar*);
        int		queryProjects		(rtlClient* = 0);
        void		queryParentProject	(void);

	//--------
	// actions
	//--------

        void		clear			(void);
        void		update			(void);
        void		select			(const vchar* selections);
        void            select                  (int selection);
        void		unselect		(void);
	void		openItem		(void);
	int             EditCopy		(rtlClient *);
	int             EditPaste		(rtlClient *);
	void            preserveSelection       (void);

        //------------------------------------
        // base class virtual method overrides
        //------------------------------------

        void		DrawCellContents	(int, int, int, vrect*);
        void		ObserveSelect		(int);
	int		HandleButtonDown	(vevent*);
	int             HandleKeyDown           (vevent*);
        void		SetRect			(const vrect*);
        void		Init			(void);
        void		LoadInit		(vresource);
        void		Destroy		   	(void);
											  
	void        Open                (void);

    private:

	//-----------
	// class data
	//-----------

        static rtlClient*	list_;
        static int		serial_;

	//--------------
	// instance data
	//--------------

        int			id_;
        int			serverId_;
        Layer*			layer_;
        vcontainer*		parent_;
        filterButton*		filter_;
        // We will allow either type of label for a while, clean up later.
        gdLabel*		label_;
        gCenteredLabel*		centeredLabel_;
        gString			selCmd_;
        gString			actCmd_;
        int			inUpdate_;
        int			localOnly_;
        int			contentWidth_;
        int			interiorWidth_;
	vkeyModifiers		lastModifiers_;
        vspringdom*		springDomain_;
        gArrayOf (sdObjPtr)	sdObjArray_;
        sdObjPtr		sdObj_;
        vdict*			iconDict_;
        rtlClient*		next_;
        int                     startat_;
        int                     flag_;
	int                     update_called; // used to check that list is changed during getItem operation
	int                     preserve_selection;

	//----------------
	// private methods
	//----------------

        void			unselectLocal_		(void);
        void			refresh_		(void);
};

//-----------------------------------------------------------------------------

class clientButton : public vbutton {

    public:    

	vkindTYPED_FULL_DECLARATION (clientButton);
	vloadableINLINE_CONSTRUCTORS (clientButton, vbutton);

	void			setParent	(rtlClient*);
	rtlClient*		getParent	(void) const;

	//------------------------------------
	// base class virtual method overrides
	//------------------------------------

	void			Init		(void);

    private:

	rtlClient*		parent_;
};

//-----------------------------------------------------------------------------

inline int rtlClient::getServerId (void) const
{
    gInit (rtlClient::getServerId);

    return serverId_;
}

//-----------------------------------------------------------------------------

inline int rtlClient::getId (void) const
{
    gInit (rtlClient::getId);

    return id_;
}

//-----------------------------------------------------------------------------

inline Layer* rtlClient::getLayer (void) const
{
    gInit (rtlClient::getLayer);

    return layer_;
}

//-----------------------------------------------------------------------------

inline void clientButton::setParent (rtlClient* parent)
{
    gInit (clientButton::setParent);

    parent_ = parent;
}

//-----------------------------------------------------------------------------

inline rtlClient* clientButton::getParent (void) const
{
    gInit (clientButton::getParent);

    return parent_;
}

#endif // _rtlClient_h
