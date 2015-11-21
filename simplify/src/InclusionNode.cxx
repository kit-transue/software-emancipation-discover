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
#include "InclusionNode.h"

#include "simplify_globals.h"

InclusionNode::~InclusionNode()
{
  int size = m_regions.size();

  for (int i = 0; i < size; ++i)
    {
      Region* reg = m_regions[i];
      delete reg;
    }
}

void InclusionNode::AddRegion(int start, int end, int source_start)
{
  Region* reg = new Region(start, end, source_start);
  if (reg)
    {
      m_regions.push_back(reg);
    }
}

/* find a region among the regions that are covered by this node
   that contains line. Regions are stored in sorted order, so
   we can use binary search */
Region* InclusionNode::FindRegion(int line)
{
  int size = m_regions.size();
  Region* ret = 0;
  Region* reg;
  
  int mid;
  int high = size - 1;
  int low = 0;

  while(low <= high)
    {
      mid = (high + low)/2;

      reg = m_regions[mid];

      if (reg->GetStart() > line)
	{
	  high = mid - 1;
	}
      else if (reg->GetEnd() < line)
	{
	  low = mid + 1;
	}
      else
	{
	  ret = reg;
	  break;
	}
    }
  return ret;
}

// return 1 if all of covered regions follow line, i.e. their
// start line numbers are greater than line. Since regions are
// stored in sorted order, only need to check the first one
int InclusionNode::FollowsLine(int line)
{
  int ret = 0;
  int size = m_regions.size();

  if (size)
    {
      Region* reg = m_regions[0];
      if (reg->FollowsLine(line))
	{
	  ret = 1;
	}
    }

  return ret;
}

// return 1 if all of covered regions proceed line, i.e. there end
// line numbers are smaller then line. Since regions are stored in
// sorted order, only need to check the last one
// if this node does not cover any regions (include an empty file), return 1
int InclusionNode::ProceedsLine(int line)
{
  int ret = 1;
  int size = m_regions.size();

  if (size)
    {
      Region* reg = m_regions[size - 1];
      if (!reg->ProceedsLine(line))
	{
	  ret = 0;
	}
    }

  return ret;
}

int InclusionNode::AddDependency(InclusionNode* node)
{
  if (node == this)
    return 0;

  int size = m_dependson.size();
  for (int i = 0; i < size; ++i)
    {
      const InclusionNode* n = m_dependson[i];
      if (n == node) //already exists
	return 0;
    }

  m_dependson.push_back(node);
  return 1;
}

int InclusionNode::ContainsSourceLine(int line)
{
  int ret = 0;
  int size = m_regions.size();

  Region* reg;

  int mid;
  int high = size - 1;
  int low = 0;

  while(low <= high)
    {
      mid = (high + low)/2;

      reg = m_regions[mid];

      int start = reg->GetSourceStart();
      int end = start + reg->GetEnd() - reg->GetStart();
      
      if (start > line)
	{
	  high = mid - 1;
	}
      else if (end < line)
	{
	  low = mid + 1;
	}
      else
	{
	  ret = 1;
	  break;
	}
    }
  return ret;
}



