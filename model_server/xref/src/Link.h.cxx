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
/* this file was pulled out of Xref.h.C *09.17.95 *NY*/

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "machdep.h"
#include "general.h"
#include "genError.h"
#include "Link.h"
#include <xrefSymbol.h>

#define CVP(x) ((char *) (&(x))) 
#define CP(x) ((char *) (x)) 

void Link::assign(Link & ln)
{
  assign(ln.lt(), ln.get_index());
}

int Link::symbol_link()
{
  linkType tt = lt();
  return (! (tt == lt_lmd || tt == lt_lmdhigh || tt == lt_sym 
	     || tt == lt_junk || tt == ltRelational || tt == lt_next 
	     || tt == lt_filetype  || tt == lt_misc || tt == lt_attribute 
	     || tt == lt_own_offset));
}

int Link::num_link()
{
  Initialize(Link::num_link);
  linkType tt = lt();
  return ( (tt == lt_lmd || tt == lt_lmdhigh || tt == lt_filetype || tt == lt_attribute ));
}

void Link::assign(linkType l, xrefSymbol* sym)
{
  Initialize(Link::assign);
  if (sym) {
    set_link_type(l);
    set_index((uint)sym->get_symbol_index());
  }
  else
    assign(lt_junk);
}

void Link::print(ostream & os) 
{
  os << (void *) this;
  os << ' ' << linkType_name(lt()) << ' ' << get_index() << endl;
}

void OldLink::print(ostream & os) 
{
  os << (void *) this;
  os << ' ' << linkType_name(lt()) << ' ' << way() << ' ' << get_index() << endl;
}
