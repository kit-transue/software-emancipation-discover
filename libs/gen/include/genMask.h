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

#ifndef _genMask_h
#define _genMask_h

#ifndef __psetmem_h
#include "psetmem.h"
#endif // __psetmem_h

#ifndef _machdep_h
#include "machdep.h"
#endif // _machdep_h

//---------------------------------------------------------------------------//
//								  	     //
//                          Generic Mask Class			  	     //
//			    ------------------				     //
//								  	     //
// This class provides a fast implementation of a set of integers between    //
// zero and some small positive number.					     //
//								  	     //
//---------------------------------------------------------------------------//

class genMask {

public:

			genMask		(unsigned range = 256, int clear = 1);
			genMask		(const genMask&);

    const genMask&	operator =	(const genMask&);

    void		add		(unsigned value);
    void		remove		(unsigned value);
    void		clear		(void);

    const genMask&	operator +=	(unsigned value);
    const genMask&	operator -=	(unsigned value);

    const genMask&	operator +=	(const genMask&); 
    const genMask&	operator -=	(const genMask&); 
    const genMask&	operator *=	(const genMask&); 

    int			pop		(void);
    int			peek		(int prev = -1)		const;

    int			includes	(unsigned value)	const;
    int			empty		(void)			const;

    int			operator ==	(const genMask&)	const;
    int			operator !=	(const genMask&)	const;

			~genMask	(void);

    friend genMask	operator +	(const genMask&, const genMask&);
    friend genMask	operator -	(const genMask&, const genMask&);
    friend genMask	operator *	(const genMask&, const genMask&);

private:

    void		_provide	(unsigned range, int clear = 0);

    unsigned			_cap;
    unsigned			_words;
    unsigned long *		_mask;
    static const unsigned	_ulsize;
    static const unsigned	_ulbits;
};

//-----------------------------------------------------------------------------

inline genMask::genMask (unsigned range, int clear) :
				        _cap (0), _words (0), _mask (0)
{
    if ((range &= 0xffff) < 1)
	range = 1;

    _provide (range, clear);
}

//-----------------------------------------------------------------------------

inline genMask::genMask (const genMask& s) :
               _cap (0), _words (0), _mask (0)
{
    _provide (s._cap);

    OS_dependent::bcopy (s._mask, _mask, s._words * _ulsize);

    if (_words > s._words)
	OS_dependent::bzero (_mask + s._words, (_words - s._words) * _ulsize);
}

//-----------------------------------------------------------------------------

inline const genMask& genMask::operator = (const genMask& s)
{
    _provide (s._cap);

    OS_dependent::bcopy (s._mask, _mask, s._words * _ulsize);

    if (_words > s._words)
	OS_dependent::bzero (_mask + s._words, (_words - s._words) * _ulsize);

    return *this;
}

//-----------------------------------------------------------------------------

inline void genMask::add (unsigned value)
{
    _provide ((value &= 0xffff) + 1, 1);

    _mask [value / _ulbits] |= (1UL << (value % _ulbits));
}

//-----------------------------------------------------------------------------

inline void genMask::remove (unsigned value)
{
    if ((value &= 0xffff) < _cap)
	_mask [value / _ulbits] &= ~(1UL << (value % _ulbits));
}

//-----------------------------------------------------------------------------

inline void genMask::clear (void)
{
    OS_dependent::bzero (_mask, _words * _ulsize);
}

//-----------------------------------------------------------------------------

inline const genMask& genMask::operator += (unsigned value)
{
    add (value);
    return *this;
}

//-----------------------------------------------------------------------------

inline const genMask& genMask::operator -= (unsigned value)
{
    remove (value);
    return *this;
}

//-----------------------------------------------------------------------------

inline int genMask::includes (unsigned value) const
{
    if ((value &= 0xffff) < _cap)
	return _mask [value / _ulbits] & (1UL << (value % _ulbits));
    else return 0;
}

//-----------------------------------------------------------------------------

inline genMask::~genMask (void)
{
    psetfree (_mask);
}

#endif // _genMask_h
