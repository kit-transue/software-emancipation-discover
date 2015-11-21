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
#ifndef _objOperate_h
#define _objOperate_h

#include <objRelation.h>
#include <transaction.h>
class objRegenerator;

typedef int objViewType;

class app;
class appTree;
class objTree;

class steSlot;

void obj_assign(app*, appTree*, steSlot*, short);


////////////////////////////////////////////////////////////// MODIFIERS:

class objInserter : public objModifier
{
  public:
    objOperType type;  
    objTree* src_obj;	    // object to insert; may be NULL;
    objTree* targ_obj;    // object to insert src_obj as first/after/replace
    void *data;	    // usually a string, e.g "i=j+1"  XXX: non-const OK?
    void const *pt;
    objInserter *prev_ins;   // pointer to previous inserter
        
    objInserter( objOperType t, objTree* x, objTree* y, 
        void *d = 0, void const *p = 0, objInserter *prev = NULL );

    objInserter(const objInserter &os);

  protected:			// for GNU
    objInserter() {}

  public:
    virtual int apply(Relational*);// { return 0; }
    
    virtual objPropagator* accept(Relational* doc, int level);
    virtual void const *get_data(); // { return data; }
    virtual objTree* get_target(); //  { return targ_obj; }
    virtual objOperType get_op_type(); // { return type; }
};


class objRemover : public objModifier
{
  public:
    objTree*  src_obj;	// object to be removed
    char flags;			// flags: 0x01: pass to ldr,
				//	        do not remove on APP.
    objTree* src_parent;

    objRemover(objTree*, char = 0);

    virtual int apply(Relational*); // { return 0; }
    virtual objPropagator* accept(Relational*, int);
};


class objHiliter : public objWalker
{
  public:
    objViewType type;
    objTree*  src_obj;

    objHiliter(objTree* x, objViewType tp = 0) : type(tp), src_obj(x) {}
  
    virtual int apply(Relational*) const; //  { return 0; }
    virtual objPropagator* accept(Relational*, int);
};

class objFocuser : public objWalker
{
  public:
    objViewType type;
    objTree* src_obj;

    objFocuser(objTree*  x, objViewType tp = 0) : type(tp), src_obj(x) {}

    virtual int apply(Relational*) const; // { return 0; }
    virtual objPropagator* accept(Relational*, int);
};


class objAssigner : public objWalker
{
  public:
    steSlot* slot;
    objTree* src_obj;
    short operation; // 1 = add category, 0 = remove category

    objAssigner(objTree*  x, steSlot* sl, short o) : slot(sl), src_obj(x), operation(o) {}

    virtual int apply(Relational*) const; // { return 0; }
    virtual objPropagator* accept(Relational*, int);
};

class objMerger : public objModifier
{
  public:
    objTree* targ_obj;
    objTree* src_obj;
#ifdef __GNUG__
    objMerger(const objMerger& om) {*this = om;}
#endif
    objMerger(objTree* targ, objTree* src)
	: targ_obj(targ), src_obj(src) {}

    virtual int apply(Relational*); // { return 0; }
    virtual objPropagator* accept(Relational* , int);
};


class objSplitter : public objModifier
{
  public:
    objTree* bottom_obj;
    objTree* top_obj;
#ifdef __GNUG__
    objSplitter(const objSplitter& os) { *this = os; }
#endif   
    objSplitter(objTree* bottom, objTree* top)
	: bottom_obj(bottom), top_obj(top) {}

    virtual int apply(Relational*); // { return 0; }
    virtual objPropagator* accept(Relational*, int);
};


class objRegenerator : public objModifier
{
  public:
    objTree*  targ_obj;	// astStmt; steTextNode, etc.
#ifdef __GNUG__
    objRegenerator(const objRegenerator& os) { *this = os; }
#endif
    objRegenerator(objTree* targ) : targ_obj(targ) {}

    virtual int apply(Relational*); 
    virtual objPropagator* accept(Relational*, int);
};

// XXX: duplicated in objOper.h!
void obj_split(app*, appTree* bottom, appTree*);
void obj_merge(app*, appTree* targ, appTree* src);
void obj_regenerate(app*, appTree* targ);

void obj_insert(app*, objOperType, appTree*, appTree*, 
                void *, void const *, objInserter *);
void obj_insert(app*, objOperType, appTree*, appTree*, void *, void const *);
void obj_insert(app*, objOperType, appTree*, appTree*, void *);

class OperPoint;

class objRegion : public Obj
{
  public :
    OperPoint *start;
    OperPoint *end;

  objRegion( OperPoint *st, OperPoint *en );
  objRegion( appTree* st, int off1, appTree* en, int off2 );
  ~objRegion();

};

#endif
