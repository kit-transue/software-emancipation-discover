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

#include "genMask.h"

//-----------------------------------------------------------------------------

const unsigned genMask::_ulsize = sizeof (unsigned long);
const unsigned genMask::_ulbits = 8 * sizeof (unsigned long);

//-----------------------------------------------------------------------------

//
// Merge in the contents of another set.
//

const genMask& genMask::operator += (const genMask& other)
{
    _provide (other._cap, 1);

    for (unsigned i = 0; i < other._words; i++)
	_mask[i] |= other._mask[i];

    return *this;
}

//-----------------------------------------------------------------------------

//
// Remove the contents of another set.
//

const genMask& genMask::operator -= (const genMask& other)
{
    unsigned w;

    if (_words < other._words)
	w = _words;
    else w = other._words;

    for (unsigned i = 0; i < w; i++)
	_mask[i] &= ~other._mask[i];

    return *this;
}

//-----------------------------------------------------------------------------

//
// Remove elements which are not in another set.
//

const genMask& genMask::operator *= (const genMask& o)
{
    unsigned i;

    if (_words > o._words) {
	for (i = 0; i < o._words; i++)
	    _mask[i] &= o._mask[i];
	OS_dependent::bzero (_mask + o._words, (_words - o._words) * _ulsize);
    }
    else for (i = 0; i < _words; i++)
	_mask[i] &= o._mask[i];

    return *this;
}

//-----------------------------------------------------------------------------

//
// Find the lowest element, remove it from the set, and return its value.
// If the set is empty, return -1.
//

int genMask::pop (void)
{
    unsigned long k;

    for (unsigned i = 0; i < _words; i++)
        if ((k = _mask[i]) != 0) {
	    int j;
            for (j = 0; (k & 1) == 0; j++, k >>= 1);
            _mask[i] &= ~(1UL << j);
            return i * _ulbits + j;
        }

    return -1;
}

//-----------------------------------------------------------------------------

//
// Scan for the lowest element whose value is above the one passed in.  If
// the argument is negative, find the lowest element in the set.  If no such
// element exists, return -1.  May be used to loop through the contents of
// a set.
//

int genMask::peek (int prev) const
{
    if (prev < -1) prev = -1;

    unsigned		c = ++prev % _ulbits;
    unsigned long	k, m;

    for (unsigned i = prev / _ulbits; i < _words; i++, c = 0)
	if ((k = _mask[i]) != 0) {
	    for (m = 1UL << c; (m != 0) && ((k & m) == 0); c++, m <<= 1);
	    if (m != 0)
		return i * _ulbits + c;
	}

    return -1;
}

//-----------------------------------------------------------------------------

//
// Determine if the set is empty.
//

int genMask::empty (void) const
{
    for (unsigned i = 0; i < _words; i++)
        if (_mask[i] != 0)
            return 0;

    return 1;
}

//-----------------------------------------------------------------------------

//
// Determine if the set is equal to another set.
//

int genMask::operator == (const genMask& other) const
{
    const genMask* little;
    const genMask* big;

    if (_words < other._words) {
	little = this;
	big = &other;
    }
    else {
	little = &other;
	big = this;
    }

    unsigned i;
    for (i = 0; i < little->_words; i++)
	if (little->_mask[i] != big->_mask[i])
	    return 0;

    for (; i < big->_words; i++)
	if (big->_mask[i] != 0)
	    return 0;

    return 1;
}

//-----------------------------------------------------------------------------

//
// Determine if the set is not equal to another set.
//

int genMask::operator != (const genMask& other) const
{
    const genMask* little;
    const genMask* big;

    if (_words < other._words) {
	little = this;
	big = &other;
    }
    else {
	little = &other;
	big = this;
    }

    unsigned i;
    for (i = 0; i < little->_words; i++)
	if (little->_mask[i] != big->_mask[i])
	    return 1;

    for (; i < big->_words; i++)
	if (big->_mask[i] != 0)
	    return 1;

    return 0;
}

//-----------------------------------------------------------------------------

//
// Make sure the set has sufficient space to hold the given range of values.
// If 'clear' is nonzero, clear out any newly-allocated space.
//

void genMask::_provide (unsigned range, int clear)
{
    if (range > _cap) {

	unsigned g = (range + _ulbits - 1) / _ulbits;
	unsigned w = _words;

	if (w < 1) w = 1;
	while (w < g) w <<= 1;

	if (_mask)
	    _mask = (unsigned long *) psetrealloc ((char *) _mask, w * _ulsize);
	else _mask = (unsigned long *) psetmalloc (w * _ulsize);

	if (clear)
	    OS_dependent::bzero (_mask + _words, (w - _words) * _ulsize);

	_words = w;
	_cap = _words * _ulbits;
    }
}

//-----------------------------------------------------------------------------

//
// Determine the union of two sets.
//

genMask operator + (const genMask& a, const genMask& b)
{
    const genMask* little;
    const genMask* big;

    if (a._words < b._words) {
	little = &a;
	big = &b;
    }
    else {
	little = &b;
	big = &a;
    }

    genMask temp (big->_cap, 0);

    unsigned i;
    for (i = 0; i < little->_words; i++)
	temp._mask[i] = a._mask[i] | b._mask[i];

    for (; i < big->_words; i++)
	temp._mask[i] = big->_mask[i];

    return temp;
}

//-----------------------------------------------------------------------------

//
// Determine the difference of two sets.
//

genMask operator - (const genMask& a, const genMask& b)
{
    unsigned w;

    if (a._words < b._words)
	w = a._words;
    else w = b._words;

    genMask temp = a;

    for (unsigned i = 0; i < w; i++)
	temp._mask[i] &= ~b._mask[i];

    return temp;
} 

//-----------------------------------------------------------------------------

//
// Determine the intersection of two sets.
//

genMask operator * (const genMask& a, const genMask& b)
{
    const genMask* little;
    const genMask* big;

    if (a._words < b._words) {
	little = &a;
	big = &b;
    }
    else {
	little = &b;
	big = &a;
    }

    genMask temp = *little;

    for (unsigned i = 0; i < temp._words; i++)
	temp._mask[i] &= big->_mask[i];

    return temp;
}
