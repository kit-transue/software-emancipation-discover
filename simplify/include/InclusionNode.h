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
#ifndef _InclusionNode_h
#define _InclusionNode_h

#include <vector>

#include <Region.h>

class InclusionNode
{
 public:
 InclusionNode(int id) : m_id(id), m_regions(), m_dependson(), m_marked(0),
                          m_included_at_line(-1), m_done(0), m_preinclude(0),
                          m_parent(0), m_next(0), m_prev(0), m_first(0), m_last(0){}
  ~InclusionNode();

  int GetId()               { return m_id; }
  std::vector<Region *> &GetRegions()      { return m_regions; }
  int IsMarked()            { return m_marked; }
  void Mark()               { m_marked = 1; }
  std::vector<InclusionNode*> &GetDependencies() { return m_dependson; }

  int AddDependency(InclusionNode* );
  
  void AddRegion(int start, int end, int source_start);
  int ContainsLine(int line) { return FindRegion(line) != 0; }
  int ContainsSourceLine(int line);
  int FollowsLine(int line);
  int ProceedsLine(int line);
  Region* FindRegion(int line);

  int GetIncludedAtLine()          { return m_included_at_line; }
  void SetIncludedAtLine(int line) { m_included_at_line = line; }
  void SetDone()                   { m_done = 1; }
  int IsDone()                     { return m_done; }
  int IsPreinclude()               { return m_preinclude; }

  InclusionNode* GetParent() { return m_parent; }
  InclusionNode* GetNext() { return m_next; }
  InclusionNode* GetPrev() { return m_prev; }
  InclusionNode* GetFirst() { return m_first; }
  InclusionNode* GetLast() { return m_last; }

  void SetParent(InclusionNode* parent) { m_parent = parent; }
  void SetNext(InclusionNode* next) { m_next = next; }
  void SetPrev(InclusionNode* prev) { m_prev = prev; }
  void SetFirst(InclusionNode* first) { m_first = first; }
  void SetLast(InclusionNode* last) { m_last = last; }
  void SetPreinclude(int flag) { m_preinclude = flag; } 

 private:
  int m_id;        //symbol ID for this file from iff_parser
  std::vector<Region *> m_regions; //array of objects of type Region* to store line numbers
                    //in expaned file that are covered by this node
                    //this array is populated using MAP lines directive
  std::vector<InclusionNode*> m_dependson;
  int m_marked;
  int m_preinclude;

  int m_included_at_line;
  int m_done;

  InclusionNode* m_parent; //the parent of this node
  InclusionNode* m_next;  //next sibling
  InclusionNode* m_prev;  //previous sibling
  InclusionNode* m_first; //first child of this node
  InclusionNode* m_last;  //last child of this node
};

#endif









