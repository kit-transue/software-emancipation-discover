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
#ifndef _InclusionTree_h
#define _InclusionTree_h

#include <InclusionNode.h>

class InclusionTree
{
 public:
  InclusionTree();
  InclusionTree(InclusionNode* root);
  virtual ~InclusionTree();

  void SetRoot(InclusionNode* node) { m_root = node; }
  InclusionNode* GetRoot() { return m_root; }

  InclusionNode* FindNodeByLine(int line) { return FindNodeByLine(m_root, line); }
  InclusionNode* FindNodeById(int id) { return FindNodeById(m_root, id); }
  void FindAllByFile(const char* file, std::vector<InclusionNode *>&);

  static void AddChild(InclusionNode* parent, InclusionNode* child);

  static InclusionNode* FindNodeByLine(InclusionNode* startFrom, int line);
  static InclusionNode* FindParentById(InclusionNode* startFrom, int id);
  static InclusionNode* FindNodeById(InclusionNode* startFrom, int id);
  static InclusionNode* FindChildByFile(InclusionNode* startFrom, const char* file);
  static InclusionNode* FindParentByFile(InclusionNode* node, const char*);
  static void FindAllByFile(InclusionNode* node, const char* file, std::vector<InclusionNode *>&);
  
protected:
  static void Destroy(InclusionNode* startFrom);

private:
  InclusionNode* m_root;
};

inline void InclusionTree::FindAllByFile(const char* file, std::vector<InclusionNode *>& res)
{
  FindAllByFile(m_root, file, res);
}

#endif

