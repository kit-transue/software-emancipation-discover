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
// This file is only included for SERVER code. It should be moved!
#ifndef _RTListServer_h
#define _RTListServer_h

#ifndef vstrINCLUDED    // vstr not included yet.
   #include <vport.h>
   #include vstrHEADER
#endif

// Make sure that galaxy undefs are called.
// Also, make sure that aset/system stuff is defined.
#include <galaxy_undefs.h>

#ifndef _genMask_h
#include "genMask.h"
#endif

#ifndef _symbolArr_h
#include "symbolArr.h"
#endif

#ifndef __ldrList
#include "ldrList.h"
#endif

#ifndef _rtlCommon_h
#include "grtlCommon.h"
#endif

//-----------------------------------------------------------------------------

class RTListServer : public RTL {

    public:
    
        RTListServer (int);

	define_relational (RTListServer, RTL);

        int		id (void) const;
	int		getType (void) const;

	void		insert (const symbolArr&);
	void		insert (symbolPtr);
	void		clear (void);

	void		queryProjects (const RTListServer* = 0, int = -1);
        void            queryProjects (char *, int *);
	void		queryParentProject (void);
	void		resetFilter ();
        void            setFilter (char *, char *, char *, char *);
        void 		getFilter (genString&, genString&, genString&, genString&);

        int		getSize (void) const;
        const char*	getItem (int) const;
        const char*	getTitle (void) const;
        int    		isFiltered();
        int             get_unfilteredSize ();

        int		setSel		(const genMask&);
        const genMask&	getSel		(void)		const;
        int		getSelSize      (void) const;
	void		getSelArr	(symbolArr&)	const;
	symbolArr&	getArr		(void)          const {return _getArr();}

        int		select (symbolArr&);

   	void 		set_caller	(vchar *);
	int		get_caller	();

	void		_propagate (void);

        ~RTListServer (void);

        static RTListServer* find (int id);
	symbolPtr		getSortedEntry (int) const;

    private:

	int			_caller;
        int			_id;
        int			_clientId;
        genMask			_sel;
	ldrList*		_ldr;
	genString		_fmtSort;
	genString		_fmtFormat;
	genString		_fmtShow;
	genString		_fmtHide;
	int			_fmtDescend;
	int			_type;
        RTListServer*		_next;

        static int		_uniqId;
        static RTListServer*	_list;

	void			_setList (RTLNode*, const char*);

	
	symbolArr&		_getArr (void) const;
};

//-----------------------------------------------------------------------------

inline int RTListServer::get_caller() {return _caller;}

//-----------------------------------------------------------------------------

inline int RTListServer::id (void) const
{
    return _id;
}

//-----------------------------------------------------------------------------

inline const genMask& RTListServer::getSel (void) const
{
    return _sel;
}

//-----------------------------------------------------------------------------

inline int RTListServer::getType (void) const
{
    return _type;
}

#endif
