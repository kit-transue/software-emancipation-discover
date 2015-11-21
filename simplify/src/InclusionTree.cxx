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
#include <InclusionTree.h>
#include <simplify_globals.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <symbolStruct.h>

InclusionNode* InclusionTree::FindParentById(InclusionNode* startFrom, int id)
{
  InclusionNode* ret = 0;

  while (startFrom)
    {
      int myid = startFrom->GetId();
      if (myid == id)
	{
	  ret = startFrom;
	  break;
	}
      startFrom = startFrom->GetParent();
    }

  return ret;
}

// Search for an inclusion tree node having the given id,
// among the given node, its later siblings, and their descendants
// in depth-first order.  Assumes the id sought is greater than
// the ids of all of its ancestors their earlier siblings, to speed
// the algorithm.
InclusionNode* InclusionTree::FindNodeById(InclusionNode* startFrom, int id)
{
  InclusionNode* ret = 0;

  while (startFrom)
    {
      if (startFrom->GetId() == id)
	{
	  ret = startFrom;
	  break;
	}

      InclusionNode* next = startFrom->GetNext();
      InclusionNode* first = startFrom->GetFirst();
      if (next && next->GetId() <= id)
	{
	  startFrom = next;
	}
      else if (first && first->GetId() <= id)
	{
	  ret = FindNodeById(startFrom->GetFirst(), id);
	  break;
	}
      else
	{
	  break;
	}
    }

  return ret;
}

InclusionNode* InclusionTree::FindParentByFile(InclusionNode* node, 
					       const char* file)
{
  InclusionNode* ret = 0;

  while (node)
    {
      int id = node->GetId();
      symbol* sym = GetSymbols()[id];
      const char* name = sym->name;
      if (strcmp(name, file) == 0)
	{
	  ret = node;
	  break;
	}
      node = node->GetParent();
    }

  return ret;
}

//only searching immidiate children
InclusionNode* InclusionTree::FindChildByFile(InclusionNode* startFrom,
					      const char* file)
{
  InclusionNode* ret = 0;
  
  startFrom = startFrom->GetFirst();

  while (startFrom)
    {
      int id = startFrom->GetId();
      symbol* sym = GetSymbols()[id];
      const char* name = sym->name;
      if (!startFrom->IsDone() && strcmp(name, file) == 0)
	{
	  ret = startFrom;
	  break;
	}
      startFrom = startFrom->GetNext();
    }

  return ret;
}

//find all nodes that correspond to this file starting with node
void InclusionTree::FindAllByFile(InclusionNode* node, const char* file, vector<InclusionNode *> &res)
{
  while (node)
    {
      int id = node->GetId();
      symbol* sym = GetSymbols()[id];
      const char* name = sym->name;
      if (strcmp(name, file) == 0)
	{
	  res.push_back(node);
	}

      FindAllByFile(node->GetFirst(), file, res);
      
      node = node->GetNext();
    }
}

InclusionTree::~InclusionTree()
{
  InclusionTree::Destroy(m_root);
}

void InclusionTree::Destroy(InclusionNode* startFrom)
{
  if (startFrom == 0)
    return;

  InclusionNode* next = startFrom->GetNext();
  InclusionNode* first = startFrom->GetFirst();

  delete startFrom;

  Destroy(next);
  Destroy(first);
}

//append new child at the end of children list
void InclusionTree::AddChild(InclusionNode* parent, InclusionNode* child)
{
  InclusionNode* node = parent->GetLast();

  child->SetParent(parent);

  if (node == 0)
    {
      parent->SetFirst(child);
      parent->SetLast(child);
    }
  else
    {
      node->SetNext(child);
      child->SetPrev(node);
      parent->SetLast(child);
    }
}
  
InclusionNode* InclusionTree::FindNodeByLine(InclusionNode* startFrom, int line)
{
  if (startFrom == 0)
    return 0;

  if (startFrom->ContainsLine(line))
    return startFrom;
  
  InclusionNode* ret = 0;

  if (startFrom->ProceedsLine(line))
    ret = FindNodeByLine(startFrom->GetNext(), line);

  if (ret == 0)
    ret = FindNodeByLine(startFrom->GetFirst(), line);

  return ret;
}

InclusionTree::
InclusionTree() : m_root(0)
{
}

InclusionTree::
InclusionTree(InclusionNode* root) : m_root(root)
{
}
