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
#ifndef _Link_h
#define _Link_h

#include <ddKind.h>
typedef unsigned int uint;
class xrefSymbol;
class XrefTable;

#include <dis_iosfwd.h>

#define CVP(x) ((char *) (&(x))) 
#define CP(x) ((char *) (x))
class OldLink
{
friend class xrefSymbol;
  private:
  unsigned char link_index_byte_0;//  unsigned lt:7;
//  unsigned noway:1; // 1 for up (point to symbol), 0 for down (to link)
  unsigned char link_index_byte_1;
  unsigned char link_index_byte_2;
  unsigned char link_index_byte_3;
  void set_link_type(linkType);
  void set_way(uint);
public:
  linkType lt() const;
  uint  way() const;
  int operator==(const OldLink & ln) const;
  void assign(linkType lt, int way, uint index);
   void assign(linkType lt, int way);
  void assign(OldLink & ln);
  void assign(linkType lt, xrefSymbol* sym);
  void make_last_link();
  void set_index(uint ui);
  int get_index() const;
  xrefSymbol* get_symbol_of_link();
  int symbol_link();
  int num_link();
  void print(ostream &);
};
inline uint OldLink::way() const
{
    return link_index_byte_0 & 1;
}
inline void OldLink::set_way(uint newway)
{
    if (newway)
	link_index_byte_0 |= 1;
    else
	link_index_byte_0 &= 0xfe;
}

inline linkType OldLink::lt() const
{
    return (linkType)((link_index_byte_0 >> 1) & 0x7f);
}
inline void OldLink::set_link_type(linkType newtype)
{
    link_index_byte_0 = way() | (newtype<<1);
}

inline int OldLink::get_index() const
{
    return ((((link_index_byte_1 << 8 ) | link_index_byte_2) << 8) | link_index_byte_3);
}
inline void OldLink::set_index(uint index)
{
    index &= 0xffffff;
    link_index_byte_1 = index >> 16;
    link_index_byte_2 =(index >> 8) & 0xff;
    link_index_byte_3 = index & 0xff;
}
inline void OldLink::assign(linkType lt, int way, uint index){
    link_index_byte_0 = (lt<< 1) |(int)(way !=0);
    set_index(index);
}

inline void OldLink::assign(linkType lt, int way)//  initialize Link, with index of zero
{
    link_index_byte_0 = (lt << 1) |(int)(way !=0);
    link_index_byte_1 = link_index_byte_2 = link_index_byte_3=0;
}

inline
int OldLink::operator==(const OldLink & ln) const
{
  return (lt() == ln.lt() && way() == ln.way() && get_index() == ln.get_index());
}

inline void OldLink::assign(OldLink & ln)
{
  assign(ln.lt(), ln.way(), ln.get_index());
}

inline int OldLink::symbol_link()
{

  linkType tt = lt();
  return (! (tt == lt_lmd || tt == lt_lmdhigh || tt == lt_sym 
	     || tt == lt_junk || tt == ltRelational || tt == lt_next 
	     || tt == lt_filetype  || tt == lt_misc || tt == lt_attribute 
	     || tt == lt_own_offset));
}

inline int OldLink::num_link()
{
  linkType tt = lt();
  return ( (tt == lt_lmd || tt == lt_lmdhigh || tt == lt_filetype || tt == lt_attribute ));
}

inline void OldLink::assign(linkType l, xrefSymbol* sym)
{
  set_link_type(l);
  if (CP(sym) < CP(this)) {
    set_way(1);
    set_index((uint) (CVP(*this) - CVP(*sym)));
  }
  else {
    set_way(0);
    set_index((uint) (CVP(*sym) - CVP(*this)));
  }
}


/* The Link class has been renamed to OldLink and a new Link class is created ( very similar to previous one) to accomodate to new pmod structure */
class Link
{
  friend class xrefSymbol;
 private:
  unsigned char link_index_byte_0;
  unsigned char link_index_byte_1;
  unsigned char link_index_byte_2;
  unsigned char link_index_byte_3;
  void set_link_type(linkType);
  
 public:
  linkType lt() const;
  int operator==(const Link & ln) const;
  void assign(linkType lt, uint index);
  void assign(linkType lt);
  void assign(Link & ln);
  void assign(linkType lt, xrefSymbol* sym);
  void make_last_link();
  void set_index(uint ui);
  int get_index() const;
  xrefSymbol* get_symbol_of_link(XrefTable*);
  int symbol_link();
  int num_link();
  void print(ostream &);
};

inline linkType Link::lt() const
{
    return (linkType)(link_index_byte_0 & 0xff);
}
inline void Link::set_link_type(linkType newtype)
{
    link_index_byte_0 = newtype;
}

inline int Link::get_index() const
{
  return ((((link_index_byte_1 << 8 ) | link_index_byte_2) << 8) | link_index_byte_3);
}
inline void Link::set_index(uint index)
{
  index &= 0xffffff;
  link_index_byte_1 = index >> 16;
  link_index_byte_2 =(index >> 8) & 0xff;
  link_index_byte_3 = index & 0xff;
}

inline void Link::assign(linkType lt, uint index)
{
  link_index_byte_0 = lt;
  set_index(index);
}

inline void Link::assign(linkType lt)//  initialize Link, with index of zero
{
  link_index_byte_0 = lt;
  link_index_byte_1 = link_index_byte_2 = link_index_byte_3=0;
}

inline void Link::make_last_link()//  Create the special link that marks the end of the linklist
{
  assign(lt_last);
}

inline
int Link::operator==(const Link & ln) const
{
  return (lt() == ln.lt() && get_index() == ln.get_index());
}

#endif

