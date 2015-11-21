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
#ifndef _ddSelector_h
#define _ddSelector_h

// ddSelector.h
//------------------------------------------
// synopsis:
// Bitmask of ddKinds for querying data dictionary.
//
//------------------------------------------

#ifndef _ddKind_h
#include <ddKind.h>
#endif

//  A number of places in here will have to be changed again if we add more than 64 ddKind values
//     so there is a test in the default constructor against  NUM_OF_DDS
const unsigned int SELECT_MAX = 64;
const unsigned MASK_SIZE = SELECT_MAX/32;

//-----------------------------------------------------------------------------------------------
class ddSelector
{
  public:
    ddSelector();
    ddSelector(const ddSelector&);
    void add_all();
    void add(ddKind k);
    void rem(ddKind k);
    int selects(ddKind k) const;
    int empty();

    ddSelector& operator =(const ddSelector&);
    int operator ==(const ddSelector&);
    int operator !=(const ddSelector&);

  private:
    unsigned long mask[MASK_SIZE];

  public:
    unsigned int want_loaded : 1;
    unsigned int want_unloaded : 1;
    unsigned int want_used : 1; // not fully implemented: used in add_symbols
    unsigned int dummy : 29;
};
//-----------------------------------------------------------------------------------------------

inline ddSelector::ddSelector(const ddSelector& that)
{
    *this = that;
}

//  Note, NUM_OF_DDS is expected to be < SELECT_MAX
inline int ddSelector::operator ==(const ddSelector& that)
{
    return
	mask[0] == that.mask[0] &&
	mask[1] == that.mask[1] &&
	want_loaded == that.want_loaded &&
	want_unloaded == that.want_unloaded;
}

inline int ddSelector::operator !=(const ddSelector& that)
{
    return ! (*this == that);
}

//  add one ddKind to the mask.  No problem if it is already there
inline  void ddSelector::add(ddKind k)
{
    if (k < SELECT_MAX)
	mask[k/32] |= (1 << (k%32));
}

// remove one ddKind from the mask.  No problem if it is not there
inline  void ddSelector::rem(ddKind k)
{ 
    if (k < SELECT_MAX)
	mask[k/32] &= ~(1 << (k%32));
}

//  nonzero means this particular ddKind is in the mask
inline  int ddSelector::selects(ddKind k) const
{
    return (k < SELECT_MAX) ? (0 != (mask[k/32] & (1 << (k%32)))) : 0;
}

//  Note, NUM_OF_DDS is expected to be < SELECT_MAX
//  Nonzero means there is at least one ddKind in the mask
inline  int ddSelector::empty()
{
    return ((mask[0] | mask[1]) == 0);
}

#endif

