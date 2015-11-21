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
#ifndef _GenericTree_h
#define _GenericTree_h

///////////////////////////////////////////////////////////
//
// FILE:
//
//     GenericTree.h
//
// DESCRIPTION:
//
//     Generic data structure for maintaining trees.
//     Nodes are given meaning by subclassing. Used
//     for providing tree operations via Access/sw.
//
///////////////////////////////////////////////////////////

#include <Relational.h>
#include <genArr.h>

// CLASS:
//
//     GenericTree
//
// DESCRIPTION:
//
//     Encapsulates a node on a tree and its relationship to
//     other nodes on the tree.

class GenericTreePtrArr;

class GenericTree : public Relational
{

  public:

                             GenericTree();
                             GenericTree(const GenericTree &);
    virtual                  ~GenericTree();

    void                     operator=(const GenericTree &);  // does not return *this

    virtual GenericTree*     GetRoot() const;
    virtual GenericTree*     GetParent() const;
    virtual int              GetChildren(GenericTreePtrArr &) const;
    virtual int              GetChildCount() const;
    virtual GenericTree*     GetChild(int) const;
    virtual GenericTree*     GetFirstChild() const;
    virtual GenericTree*     GetLastChild() const;

    virtual GenericTree*     InsertChild(GenericTree *, int = -1);
    virtual void             RemoveChild(int);
    virtual void             RemoveAllChildren();

    virtual void             Remove();

    virtual void             DetachTree();
    virtual void             DetachNode(GenericTreePtrArr &);
    virtual void             DetachNode();

    virtual GenericTree*     CloneTree() const;
    virtual GenericTree*     CloneNode() const;

  protected:

    GenericTree*             parent;
    GenericTree*             firstChild;
    GenericTree*             prevSibling;
    GenericTree*             nextSibling;

};

typedef GenericTree* GenericTreePtr;
genArr(GenericTreePtr);

#endif /* ! _GenericTree_h */
