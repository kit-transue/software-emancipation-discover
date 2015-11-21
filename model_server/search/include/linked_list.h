/*
 * This is a placeholder file for the original that could not be included
 * for copyright reasons.  Please contact the party below for a copy of
 * this file.
 */
#ifndef __linklist
#define __linklist


// XXX: stubbed in based on usage; should replace with STL structures:
// YYY: crap!  All implemented--sans copyright--in linked_list.h.cxx!

class slink {
 public:
 slink() : fwd_link(0) {}

  slink *next() { return fwd_link; }
  void link_after(slink *p, slink *&head, slink *&tail);
  void unlink_from(slink *p, slink *&head, slink *&tail);
 protected:
  slink *fwd_link;
};

class dlink {
 public:
 dlink() : fwd_link(0), bwd_link(0) {}

  void link_before(dlink *p, dlink *&head, dlink *&tail);
  void link_after(dlink *p, dlink *&head, dlink *&tail);
  void unlink(dlink *&head, dlink *&tail);
 protected:
  dlink *fwd_link;
  dlink *bwd_link;
};

#endif
// linklist.hxx: Linked list declarations

// Copyright (c) 1991, William M. Miller, Sudbury, MA 01776

// Requires:   nothing

