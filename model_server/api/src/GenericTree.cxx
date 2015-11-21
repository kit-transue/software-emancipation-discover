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
///////////////////////////////////////////////////////////
//
// FILE:
//
//     GenericTree.C
//
// DESCRIPTION: 
//
//     Generic data structure for maintaining trees.
//     Nodes are given meaning by subclassing. Used
//     for providing tree operations via Access/sw.
//
///////////////////////////////////////////////////////////

#include <GenericTree.h>

// FUNCTION
//
//     GenericTree::GenericTree
//
// DESCRIPTION:
//
//     Default constructor.

GenericTree::GenericTree() :
  parent(NULL), firstChild(NULL), prevSibling(NULL), nextSibling(NULL)
{
}

// FUNCTION:
//
//     GenericTree::GenericTree
//
// DESCRIPTION:
//
//     Copy constructor.

GenericTree::GenericTree(const GenericTree &) :
  parent(NULL), firstChild(NULL), prevSibling(NULL), nextSibling(NULL)
{
}

// FUNCTION
//
//     GenericTree::~GenericTree
//
// DESCRIPTION:
//
//     Destructor. The immediate family (children, siblings,
//     parent) are not notified of the node's demise.

GenericTree::~GenericTree()
{
}

// FUNCTION:
//
//     GenericTree::operator=
//
// DESCRIPTION:
//
//     Assignment operator. Nullifies all links.

void GenericTree::operator=(const GenericTree &)
{
  parent = firstChild = prevSibling = nextSibling = NULL;
}

// FUNCTION:
//
//     GenericTree::GetRoot
//
// DESCRIPTION:
//
//     Retrieve a live pointer to the root
//     node.
//
// RETURNS:
//
//     A pointer to the root node.

GenericTree *GenericTree::GetRoot() const
{
  GenericTree *root = (GenericTree *)this;
  GenericTree *p;

  while (p = root->parent)
  {
    root = p;
  }

  return root;
}

// FUNCTION:
//
//     GenericTree::GetParent
//
// DESCRIPTION:
//
//     Retrieve a live pointer to the node's parent.
//
// RETURNS:
//
//     A pointer to the node's parent or NULL if the
//     node is an orphan.

GenericTree *GenericTree::GetParent() const
{
  return parent;
}

// FUNCTION:
//
//     GenericTree::GetChildren
//
// DESCRIPTION:
//
//     Traverse the tree and collect pointers to
//     all of the node's children.
//
// OUTPUTS:
//
//     children    Reference to an array of node pointers.
//
// RETURNS:
//
//     The number of children found.

int GenericTree::GetChildren(GenericTreePtrArr &children) const
{
  GenericTree *cur = firstChild;
  int n = 0;

  while (cur != NULL)
  {
    children.append(&cur);
    cur = cur->nextSibling;
    ++n;
  }

  return n;
}

// FUNCTION:
//
//     GenericTree::GetChildCount
//
// DESCRIPTION:
//
//     Traverse the tree and count up the number
//     of children this node has.
//
// RETURNS:
//
//     The number of children found.

int GenericTree::GetChildCount() const
{
  GenericTree *cur = firstChild;
  int n = 0;

  while (cur != NULL)
  {
    cur = cur->nextSibling;
    ++n;
  }

  return n;
}

// FUNCTION:
//
//     GenericTree::GetChild
//
// DESCRIPTION:
//
//     Get a pointer to the child node at the specified index.
//
// INPUTS:
//
//     index    Position of child to retrieve. Indices begin
//              at zero.
//
// RETURNS:
//
//     A pointer to the requested child or NULL if no such
//     child exists.

GenericTree *GenericTree::GetChild(int index) const
{
  GenericTree *cur = firstChild;

  while ((index > 0) && (cur != NULL))
  {
    cur = cur->nextSibling;
    --index;
  }

  return cur;
}

GenericTree *GenericTree::GetFirstChild() const
{
  return firstChild;
}

GenericTree *GenericTree::GetLastChild() const
{
  GenericTree *prev = NULL;
  GenericTree *cur = firstChild;

  while (cur != NULL)
  {
    prev = cur;
    cur = cur->nextSibling;
  }

  return prev;
}

// FUNCTION:
//
//     GenericTree::InsertChild
//
// DESCRIPTION:
//
//     Insert the specified node at the specified index. Indices
//     begin at zero. To insert at the end of the list, use an
//     index of -1.
//
// INPUTS:
//
//     newChild    The new child to insert.
//     index       The position at which the new child should appear.

GenericTree *GenericTree::InsertChild(GenericTree *newChild, int index)
{
  newChild->parent = this;

  GenericTree *prev = NULL;
  GenericTree *next = firstChild;

  while ((index != 0) && (next != NULL))
  {
    prev = next;
    next = next->nextSibling;
    --index;
  }

  if (prev != NULL) {
    prev->nextSibling = newChild;
    newChild->prevSibling = prev;
  } else {
    firstChild = newChild;
  }

  if (next != NULL) {
    next->prevSibling = newChild;
    newChild->nextSibling = next;
  }

  newChild->nextSibling = next;
  newChild->prevSibling = prev;

  return newChild;
}

// FUNCTION:
//
//     GenericTree::RemoveChild
//
// DESCRIPTION:
//
//     Remove the child at the specified index.
//
// INPUTS:
//
//     index    The index of the child to remove. 

void GenericTree::RemoveChild(int index)
{
  GenericTree *child = GetChild(index);

  if (child) {
    child->Remove();
    delete child;
  }
}

// FUNCTION:
//
//     GenericTree::RemoveAllChildren
//
// DESCRIPTION:
//
//     Remove all of this node's children. All of the
//     descendents are destroyed.

void GenericTree::RemoveAllChildren()
{
  GenericTree *cur = firstChild;

  while (cur != NULL)
  {
    GenericTree *next = cur->nextSibling;
    cur->RemoveAllChildren();
    delete cur;
    cur = next;
  }

  firstChild = NULL;
}

// FUNCTION:
//
//     GenericTree::Remove
//
// DESCRIPTION:
//
//     Unlink this node and destroy its descendents.
//     You must call delete after invoking this method
//     to de-allocate the node's contents.

void GenericTree::Remove()
{
  DetachTree();
  RemoveAllChildren();
}

// FUNCTION:
//
//     GenericTree::DetachTree
//
// DESCRIPTION:
//
//     Unlink this node and its children from the tree.

void GenericTree::DetachTree()
{
  GenericTree *next = nextSibling;
  GenericTree *prev = prevSibling;

  if (next) {
    next->prevSibling = prev;
  }

  if (prev) {
    prev->nextSibling = next;
  } else if (parent != NULL) {
    parent->firstChild = next;
  }

  parent = NULL;
  prevSibling = NULL;
  nextSibling = NULL;
}

// FUNCTION:
//
//     GenericTree::DetachNode
//
// DESCRIPTION:
//
//     Displace the node from the tree. The children
//     are bumped up and the node is left linkless.
//
// OUTPUTS:
//
//     promoted    The nodes that were promoted.

void GenericTree::DetachNode(GenericTreePtrArr &promoted)
{
  GenericTree *lastChild = NULL;
  GenericTree *cur = firstChild;

  while (cur != NULL) {
    promoted.append(&cur);
    cur->parent = parent;
    lastChild = cur;
    cur = cur->nextSibling;
  }

  if (lastChild != NULL && nextSibling != NULL) {
    nextSibling->prevSibling = lastChild;
    lastChild->nextSibling = nextSibling;
  } else if (nextSibling != NULL) {
    nextSibling->prevSibling = prevSibling;
  } else {
    /* ok */
  }

  if (firstChild != NULL && prevSibling != NULL) {
    prevSibling->nextSibling = firstChild;
    firstChild->prevSibling = prevSibling;
  } else if (prevSibling != NULL) {
    prevSibling->nextSibling = nextSibling;
  } else if (parent) {
    parent->firstChild = firstChild;
  } else {
    /* this node was an orphan */
  }

  // abandonment:

  parent = NULL;
  firstChild = NULL;
  prevSibling = NULL;
  nextSibling = NULL;
}

void GenericTree::DetachNode()
{
  GenericTreePtrArr ignore;
  DetachNode(ignore);
}

// FUNCTION:
//
//     GenericTree::CloneTree
//
// DESCRIPTION:
//
//     Create a complete copy of this node and
//     all of its descendents.
//
// RETURNS:
//
//     A pointer to the new tree. The new tree will
//     have no parent or siblings.

GenericTree *GenericTree::CloneTree() const
{
  GenericTree *newTree = CloneNode();

  GenericTree *cur = firstChild;

  while (cur != NULL)
  {
    GenericTree *newChild = cur->CloneTree();
    newTree->InsertChild(newChild);
    cur = cur->nextSibling;    
  }

  return newTree;
}

// FUNCTION:
//
//     GenericTree::CloneNode
//
// DESCRIPTION:
//
//     Virtual copy constructor.
//
// RETURNS:
//
//     A pointer to a detached orphan node
//     containing no links.

GenericTree *GenericTree::CloneNode() const
{
  return new GenericTree(*this);
}
