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
#ifndef _ldr_h
#define _ldr_h


#include <relArr.h>

#include <app.h>

class app;
class appTree;
class OperPoint;
class ldrSelection;

class ldrTree;

class ldr : public Hierarchical
{
  public:
    ldr();
    ~ldr();
    define_relational(ldr,Hierarchical);

    virtual objTree* find_ldr(objTree *);
    objTree*         find_ldr_to_rebuild(objTree *);  // -jef

    virtual void hilite_obj(objHiliter * oh, objHiliter *nh);
    virtual void focus_obj(objFocuser * of, objFocuser *nf);
    virtual void assign_obj(objAssigner * oc, objAssigner *nc);

    virtual objSplitter* split_obj(objSplitter *oi, objSplitter *ni);
    virtual objMerger* merge_obj(objMerger *om, objMerger *nm);
    virtual int regenerate_obj(objTree*);
    virtual ldrTree* generate_tree(appTree*, int = 0);

    virtual objArr* search(commonTree* root, void* constraints, 
			   int options, int load_flag);

    app* get_appHeader() const;

    // The following function returns ldrHeader of specified type
    // for app and for specified appTree* root
    static ldr* find_ldrHeader(app*, int, appTree*);

    static ldr* find_ldrHeader(app*, int);
};
generate_descriptor(ldr,Hierarchical);


declare_rel(ldrTree,ldrtree,appTree,apptree);
declare_rel(viewTree,viewtree,ldrTree,ldrtree);

class ldrTree : public commonTree
{
  
  define_ptr(ldrTree,ldrtree,appTree,apptree);
#if 1 
  /* temporary gcc bug */
  define_ptr(ldrTree,ldrtree,viewTree,viewtree);
#else
  friend Obj * ldrtree_getit_viewtree (const ldrTree * ldrtree) ;
 friend    int off_viewtree_of_ldrtree (ldrTree *) {return (int)&((ldrTree * )0)->_viewtree_of_ldrtree .ptr;} 
struct st_viewtree_of_ldrtree   {
 Obj * ptr; 
 void*operator new(size_t); void operator delete(void*);
st_viewtree_of_ldrtree () : ptr(0) {} st_viewtree_of_ldrtree  (const st_viewtree_of_ldrtree  &) : ptr(0){} st_viewtree_of_ldrtree  & operator = (const st_viewtree_of_ldrtree  &) {ptr=0;return *this;} ~st_viewtree_of_ldrtree () {if(ptr) obj_rem_ptr((RelType*) viewtree_of_ldrtree ,&ptr);} } _viewtree_of_ldrtree  ;
#endif

  public:
    unsigned int collapsed : 1;	// Node should be collapsed by default       
    unsigned int readonly : 1;	// Node should be collapsed by default       

    ldrTree(const ldrTree&);
    ldrTree();
    ~ldrTree();
    define_relational(ldrTree,commonTree);

    virtual appTree* get_appTree() const;
    virtual void set_appTree(appTree*);
    virtual void send_string(ostream&) const;

    ldr* get_header() { return (ldr*)(commonTree::get_header()); }
};
#if 1 
    /* temporary gcc bug */
#else
inline Obj * ldrtree_getit_viewtree (const ldrTree * ldrtree) {return ldrtree->_viewtree_of_ldrtree .ptr;}
#endif

generate_descriptor(ldrTree,commonTree);

defrel_one_to_many(appTree,apptree,ldrTree,ldrtree);
defrel_many_to_one(ldr,ldr,app,app);

#endif
