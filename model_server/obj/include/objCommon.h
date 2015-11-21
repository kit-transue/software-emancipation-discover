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
#ifndef _objCommon_h
#define _objCommon_h

#include <objTree.h>
#include <genString.h>
#include "representation.h"

#define smtLanguage fileLanguage
#define smt_C FILE_LANGUAGE_C
#define smt_CPLUSPLUS FILE_LANGUAGE_CPP
#define smt_FORTRAN FILE_LANGUAGE_FORTRAN
#define smt_COBOL FILE_LANGUAGE_COBOL
#define smt_UNKNOWN FILE_LANGUAGE_UNKNOWN
#define smt_ESQL_C FILE_LANGUAGE_ESQL_C
#define smt_ESQL_CPLUSPLUS FILE_LANGUAGE_ESQL_CPP
#define smt_ELS FILE_LANGUAGE_ELS

bool is_C_or_CPLUSPLUS(smtLanguage language);
bool is_C(smtLanguage language);
bool is_CPLUSPLUS(smtLanguage language);
bool is_known_language(smtLanguage lang);

class Hierarchical;

class commonTree;
class objArr;

class objHiliter;
class objFocuser;
class objAssigner;

class objInserter;
class objRemover;
class objMerger;
class objSplitter;

class RTLNode;

#include <dis_iosfwd.h>

enum steNodeType
{
    steREG=0, steREF, steHOOK, steSTR, steHEAD, 
    stePAR, steGLUE, steFOOT, steSKIP, steSPLICE
};

class symbolArr;

typedef unsigned char byte;

class commonTree : public objTree
{
  public:
    commonTree();
    commonTree(const commonTree&);
    define_relational(commonTree,objTree);
    declare_copy_member(commonTree);
    void set_read_only( unsigned );

    virtual void send_string(ostream& stream = cout) const ;
    virtual char const *get_name() const;
    virtual int get_node_type(); // str type: steREG/steSTR/...
    virtual int get_extype() const;	 // 0 < extype < 256
    virtual void set_node_type(steNodeType);
    
    // Create list of tokens. For structured nodes:
    //  mode = 0 - only header tokens; mode = 2 - all tokens
    virtual objArr* get_tok_list(int mode = 0); 

    // The following 5 member functions are to be removed, do not use. -jef
    virtual boolean is_RefNode() const; // { return 0; }
    virtual boolean is_steRefNode() const ; //{ return 0; }
    virtual boolean is_steRefTextNode() const ; //{ return 0; }
    virtual boolean is_steRefGraNode() const ; // { return 0; }
    virtual boolean is_ldrRefGraNode() const; // { return 0; }
    virtual boolean is_wrong_ref () const; // { return 0; }

    virtual objArr* search(commonTree* root, void* constraints, 
			   int options, int load_flag);

    virtual Hierarchical* get_header_virtual () const;

    // The following 3 virtual table place holders
    virtual void commonTree_stab1_virtual ();
    virtual void commonTree_stab2_virtual ();
    virtual void commonTree_stab3_virtual ();

    virtual void commonTree_stab4_virtual ();
    virtual void commonTree_stab5_virtual ();
    virtual void commonTree_stab6_virtual ();
    virtual void commonTree_stab7_virtual ();
    virtual void commonTree_stab8_virtual ();
    
    Hierarchical* get_header() const { return get_header_virtual(); }

    byte type;
    byte extype;
    smtLanguage language  :8;
// one byte
    unsigned raw_flag    :3;
    unsigned read_only   :1;
    unsigned smod_vision :3;
    unsigned reserv      :1;
};
generate_descriptor(commonTree,objTree);


class  Hierarchical : public commonTree
{
  public:
    Hierarchical();
    Hierarchical(const Hierarchical&);
    Hierarchical(char const *name);
    define_relational(Hierarchical,Relational);

    virtual char const *get_name() const; // { return title; }
    virtual void set_name(char const *);

    virtual int get_node_type(); // { return steREF; }

    virtual void insert_obj(objInserter *oi, objInserter *ni) = 0;
    virtual int  regenerate_obj(objTree*);
    virtual void remove_obj(objRemover *, objRemover * nr) = 0;
    virtual void hilite_obj(objHiliter * oh, objHiliter *nh);
    virtual void focus_obj(objFocuser * of, objFocuser *nf);
    virtual void assign_obj(objAssigner * oc, objAssigner *nc);

    virtual objTree* get_root() const;
    virtual void put_root(objTree* the_root);

    virtual objSplitter* split_obj(objSplitter *oi, objSplitter *ni);
    virtual objMerger* merge_obj(objMerger *om, objMerger *nm);

    virtual void open(objTree*  subnode = NULL);

    virtual void set_type(int type);
    virtual int get_type();
// old 
    void invalidate_filename ();
    virtual void set_filename (char const *);
    virtual char const *get_filename();

    virtual int save_app();

    virtual Hierarchical* get_header_virtual () const;
     
    long get_inode();

    int is_modified();
    void clear_modified();
    void set_modified();

    int is_imported();
    void clear_imported();
    void set_imported();

    int is_src_modified();
    void clear_src_modified();
    void set_src_modified();

    virtual void notify_softassoc_insert
                             (objInserter *, objInserter *);
    
    virtual void notify_softassoc_remove
                             (objRemover *, objRemover *);
    
    virtual void notify_softassoc_merge
                             (objMerger *, objMerger *);
    
    virtual void notify_softassoc_split
                             (objSplitter *, objSplitter *);

   static symbolArr& prop_rtl_contents();
   static void clear_prop_rtl();
    
    void add_to_prop_rtl();
    
  protected:
    genString title;
    genString filename;
    unsigned int modified:1;		// tree was modified
    unsigned int imported:1;		// set for fresh import
    unsigned int valid_filename:1;	// reserve space
    unsigned int src_modified:1;	// set if source is modified 
    unsigned int reserve_space:28;	// reserve space
    long date;				// creation date, for unique tags
    genString ph_name;

 private:

   static RTLNode* prop_rtl();

   static RTLNode* prop_rtlnode;
};
generate_descriptor(Hierarchical,commonTree);

defrel_one_to_one(Hierarchical,header,commonTree,tree);


#endif
