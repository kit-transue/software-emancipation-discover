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
#ifndef _Region_h
#define _Region_h

struct Region
{
public:
  Region() : m_start(0), m_end(0), m_source_start(0) {}
  Region(int start, int end, int ss) : m_start(start), m_end(end),
                                       m_source_start(ss) {}

  int GetStart()              { return m_start; }
  int GetEnd()                { return m_end; }
  int GetSourceStart()        { return m_source_start; }

  void SetStart(int start)    { m_start = start; }
  void SetEnd(int end)        { m_end = end; }
  void SetSourceStart(int s)  { m_source_start = s; }

  int ContainsLine(int line)  { return line >= m_start && line <= m_end; }
  int FollowsLine(int line)   { return m_start > line; }
  int ProceedsLine(int line)  { return m_end < line; }

  int ContainsSourceLine(int line);

private: 
  int m_start;        //start line number (in final expanded file)
  int m_end;          //end line number (in final expanded file)
  int m_source_start; //start line of this region in the original file
};

inline int Region::ContainsSourceLine(int line)
{
  return line >= m_source_start && line <= m_source_start + (m_end - m_start);
}

#endif
