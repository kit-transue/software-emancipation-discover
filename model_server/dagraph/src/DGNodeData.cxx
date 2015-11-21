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
//
// DGNodeData.C
// ------------
//

#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <DGNodeData.h>
#include <DGNodeDataAVL.h>

//
// PUBLIC:
//

DGNodeData::DGNodeData()
{
}

DGNodeData::~DGNodeData()
{
}

int
DGNodeData::hierarchy()
{
    return BASE;
}

// ---- DGNodeDataAVL ----

//
// PUBLIC:
//

DGNodeDataAVL::DGNodeDataAVL(avl_treeptr x_tree, DGNodeData & x_key)
: avl_node(x_tree, x_key), data_node(x_key)
{
}

//
// returns: 1: this < other, 0: this == other, -1: this > other
//
int
DGNodeDataAVL::compare(const avl_key & other) const
{
    int ret_val = 0;

    ret_val = 0 - datanode().compare((DGNodeData &)other);

    return ret_val;
}
