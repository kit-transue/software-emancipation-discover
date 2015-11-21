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
#ifndef _smt_h
#define _smt_h

#ifndef _smtScdTypes_h
#include <smtScdTypes.h>
#endif

typedef unsigned int uint;

#include <Relational.h>

#include <objOper.h>


#define smt_out_stmt(h, m, t, s1, s2) smt_def_stmt(h, m, t, s1, s2);

typedef unsigned char byte;

class astTree;
class db_buffer;
class db_app;


// SMT tree node:

struct RelSegment : Relational
{
    struct mem_segments* memseg;
    RelSegment (struct mem_segments*);
    define_relational(RelSegment,Relational);
};

generate_descriptor(RelSegment,Relational);

class smtTree : public appTree
{
  friend void smt_tidy_merge(objTree *t1, objTree *t2);
  RelationalPtr rel_copy() const {return 0;}
public:
//   void * operator new(size_t size);
//   void   operator delete(void *);

  static void      db_save(app*h, appTree*node, db_app&, db_buffer&);
  static smtTree*  db_restore(app*h, appTree *parent, db_app&, db_buffer&);
  void db_after_restore(db_buffer&); // optional member on root

  int tbeg;		// begin of text relative address (in characters)

  int tlth;		// length of text in characters
  int tnfirst;		// first token number
  int tnmax;		// maximum token number
  byte spaces;
  byte newlines;// Mumber newlines and spaces before token
  unsigned arn : 1;	// AST regeneration needed
  unsigned vrn : 1;	// View regenerating needed
  unsigned ndm : 1;	// Node modified (for ldr)
  unsigned is_macro : 1;
//
  unsigned temp_vis : 1; // visibility flag Used for Short Smod Extraction
//
  byte was_type;
  astTree *ref_ast;    // Reference to AST
  int src_line_num;// Line number in the "src" file
  char *old_string;
  define_relational(smtTree, appTree);  

  int oldStart;         // The start (as returned by start()) 
                        // as it was before propigation takes place.
  int oldLength;        // The length (as returned by length()) 
                        // as it was before propigation takes place.

//
// Put all declarations below this line!
//----------------------------------------------------------------------

  obj_tree (smtTree);
  smtTree(int type = SMT_null);	// constructor
  smtTree(smtTree *);		// The special internal constructor for 
				// copying a module
  smtTree(const smtTree& r);    // the copy constructor
  ~smtTree();			// Destructor
  app *get_main_header();
  virtual objArr *get_tok_list(int mode = 0);
  virtual int get_extype() const;
  virtual int get_node_type();
  virtual void put_signature(ostream& os) const; 
  void print(ostream&, int) const;
  void send_string(ostream&) const;
  smtTree* replace_text(void const *t = "", int type = 0, int ref = 0, int frmt=0);
  void clear_arn();
  void clear_vrn();
  virtual appTreePtr rel_copy_for_header (appPtr, objCopier *);
  smtTree *get_refparent ();

  static Relational *restore(void *rec, app *h, char const *txt = 0); 
  virtual char const *get_name() const ;

// Returns the the node to open when open_view for this is called
  virtual appTree *get_world ();

  int untok_length_in_lines ();
  smtTree *subtree_at_line (int line);
  int get_last_line (smtTree *&last_leaf, int& last_len);
  int contains_line (int line, smtTree *& the_tok);

  int ilength () { return tlth; }
  int length () { return spaces + tlth + newlines; }
  int istart () { return tbeg; }
  int start () { return tbeg - spaces; }
  void disconnect_from_ast ();
  virtual int construct_symbolPtr(symbolPtr&) const;
  virtual int get_linenum () const;
};



// SMT header
class smtHeader: public app
{
  RelationalPtr rel_copy() const {return(NULL);};

public:
  static smtHeader* db_restore(char const *title, byte language, char const* ph_name);
  static int add_to_htable(smtHeader*);
  static smtHeader** htable;

  static int htablth;
  static int newtabndx;

  int htindex;
  //  static int smt_mode;
  char *srcbuf;		// memory buffer for source text
  int src_size, src_asize;	// size of text and allocated size of buffer
  int get_count;
  smtTree *get_node, *get_subtree;
  unsigned parsed : 1;		// File was parsed
  unsigned ast_exist : 1;
  unsigned arn : 1;	// AST regeneration needed (should use get_arn() and set_arn())
  unsigned vrn : 1;		// View regenerating needed
  unsigned sam : 1;		// Suspend AST modifying
  unsigned scope : 1;
  unsigned drn : 1;             // DD regeneration / mapping needed
  unsigned tok : 1;		// Allow tokenizing
  unsigned actual_mapping_priv : 1;  // MUST use get_actual_mapping() and set_actual_mapping(uint)
  unsigned just_restored : 1;
  smtTree *last_token;
  objArr *globals;		// Array of AST globals
  unsigned foreign : 1;		// Foreign file
  unsigned untok_flag : 1;          // File has untok tokens
  int ttablth;			// Size of tokens table;
  smtTree ** ttable;		// Tokens table
  char *sh_filename;

  smtHeader(char const *, smtLanguage = smtLanguage(-1), smtHeader * = 0);
  smtHeader(char const *, char const*, smtLanguage = smtLanguage(-1), projNode**pn=0);
  ~smtHeader();  
  define_relational(smtHeader, app);

  void print(ostream&, int) const;
  static smtHeader *find(char const *); // find smtHeader matching  name
  static smtHeader *find_from_realpath (char const *); // find smtHeader matching physical filename
  void cut_ast();
  void cut_ast_internal();
  void smt_delayed_map ();
  
  static void set_touched_ddElements(smtTree *trg);
  int load();                    // Load program from file
  int load(char const *);              // Load program from string
  int parse();                   // Parse program and create SMT tree
  void insert_obj(objInserter *oi, objInserter *ni);
  void remove_obj(objRemover *, objRemover *nr);
  void insert_obj_real(objInserter *oi, objInserter *ni, int do_tidy);
  void remove_obj_real(objRemover *, objRemover *nr, int do_tidy);
  void obj_insert_light(objOperType tp, smtTree*src, smtTree*trg, 
			void*data, void*d1=0);
  void obj_remove_light(smtTree*, int=0);
  virtual objSplitter* split_obj(objSplitter *, objSplitter *);
  virtual objMerger* merge_obj(objMerger *om, objMerger *nm);

  virtual char const* get_name(smtTree *) const;
  virtual char const* get_name() const;
  void get_init(smtTree *);
  char get_char();
  void tidy_tree();
  smtTree *tn_na(int tn);
  smtTree *cn_na(int tn);
  smtTree *cp_na(int tn);
  void check_margin (smtTree *& tok, int& off, int end_flag = 0);  
  void make_ast();
  static void modify_view();
  void suspend_parsing();
  int test_status();  
  void src_exp_buffer(int);

  boolean is_correct(appTree*, appTree*); // checks if correct reference. 
  smtTree *tree_at_line (int line);
  void tok_mode(int);	// Enable / disable tokenizing
  int make_tree();	// Create SMT tree (by importing / restoring)
  void expand_ttable();
  void set_ast_exist(int);
  void set_arn(int);
  uint get_arn() const;
  void set_actual_mapping(uint);
  uint get_actual_mapping() const;
  void set_ttable(smtTree *t)
    {
    while(t->tnfirst > ttablth)
      expand_ttable();
    ttable[t->tnfirst - 1] = t;
    }
  void enum_src ();
  void disconnect_from_ast ();
};

RelClass(smtHeader);

define_relation(smt_goto_of, smt_label_of);
define_relation(smt_declaration_of, smt_definition_of);
define_relation(declaration_of_reference,reference_of_declaration);
define_relation(name_of_smt_tree,smt_tree_of_name);
define_relation(scope_of_smtHeader, smtHeader_of_scope);
define_relation(kr_pdecl_of_ref, kr_ref_of_pdecl);
define_relation(id_of_decl, decl_of_id);
define_relation(smt_includes, smt_included);
define_relation(xref_of_smt, smt_of_xref);

define_relation(segment_of_smt, smt_of_segment);

generate_descriptor(smtHeader,app);
generate_descriptor(smtTree, appTree);

#ifdef _PSET_DEBUG
#define VALIDATE_id_smtHeader(x)    (( (x) && (x->get_id() > 0)) ? 0 : gen_if_handler(_my_name, "deleted smtHeader"))
#else
#define VALIDATE_id_smtHeader(x)
#endif



inline uint smtHeader::get_arn() const
{
    return arn;
}
inline uint smtHeader::get_actual_mapping() const
{
    return actual_mapping_priv;
}


void smt_tidy_level(smtTree *);
void smt_create_tree(smtHeader *);
extern "C" int smt_cpp_main(int, char const **, char const **, int *);
void smt_shallow_parser(smtHeader *);
void  smt_destroy_parsing(smtHeader *);
smtTree *smt_def_stmt(smtHeader *, int, int, int, int);
int smt_def_token(smtHeader *, int, int, int, int, int = 0, int = 0);
void smt_tokenizer(smtHeader *, smtTree *);

// makes SMT_temp over collection
smtTree* smt_unite(Obj*, objArr_Int&);

// makes SMT_temp over collection and SMT_temp over each
// region that containes more than one node
smtTree* smt_unite(Obj*, objArr_Int&, int do_regions); 
                                  
void smt_ast_text(ostream&, smtTree *);
int smt_ast_translate(smtHeader *);
boolean smt_is_statement(smtTree *);
boolean smt_is_cstmt(smtTree *);
boolean smt_is_cdecl(smtTree *);
boolean smt_is_cdefi(smtTree *);
void start_smt_editing();
void commit_smt_editing();
void try_modify_ast();

void smt_cut_region(int off1, smtTree *node1, int off2, smtTree *node2);
void smt_cut_region(int off1, smtTree *node1, int off2, smtTree *node2, int do_tidy);
void smt_paste_region(smtTree *src, int trg_off, smtTree *trg);
smtTree *smt_replace_text(smtTree *toknode,char const *data,int format=0,int do_tidy=1);

smtHeader *smt_prog_import(char const *, char const *, smtLanguage, smtTree * = 0);

void smt_remove(smtTree *);	// Cut subtree
smtTree *smt_cut(smtTree *);	// Cut subtree including single parents
void smt_splice(smtTree *);  	// Delete structured node (save children)
void smt_set_ast(smtTree *, astTree *);
void smt_set_mode(int);
void smt_reset_mode(int);
int smt_check_file_ext(char const *); // Check possibility to parse separately
void smt_kill_comment(smtTree *);
int smt_modify_ast(smtHeader *, int = 0);

// smt_ast2 declarations

void smt_get_smt_includes(smtHeaderPtr, objSet&);
void smt_clear_include_drn(smtHeaderPtr);
int smt_to_ast(smtHeaderPtr, smtTreePtr);
extern "C" smtLanguage get_language();
int smt_to_ast_xref(smtHeaderPtr, smtTreePtr);

struct smtExtModifier
{
  objOperType type;
  unsigned ref : 1;
  int node_type;
  void const *data;  
  smtExtModifier(objOperType t = NULLOP, void *d = 0,
                 int nt = 0, unsigned r = 0) :
    type(t), ref(r), node_type(nt), data(d) {}
};

void smt_touch(smtTree *t, int ast = 0);
inline void smt_touch_parent(smtTree *t, int ast = 0)
{
  smt_touch((smtTree *)t->get_parent(), ast);
}

void restore_tokenize (smtHeader*);
void cpp_reset ();

// mapping functions

void smt_map_ast(char const *, astTree*, int&, char const *&, int&, char const *&, int=0, int=0, int=0);
smtTree* smt_mark_stree(astTree*, int, int, char const *, int, char const *);
extern "C" void smt_prt_mark(smtHeader *smt_header, astTree* root, int mode, 
             int type, smtTree *smt_tree,
             int  start_line,  char const *start_file, int  end_line, char const *end_file);

void smt_get_src_info(astTree* node, int& st, int& en, char const *&  file);

smtTree* smt_find_stmt_node(smtHeader*, int);
void smt_prt_mark(smtHeader* sh, int start_line, int end_line, int type);

extern "C" void smt_prt_tree(smtTree* tree);

void smt_unset (smtTree *);
void ast_unset (astTree *);
void smt_set_ast(smtTree *, astTree *);

smtTree* ast_get_smt(astTree *);
astTree* smt_get_ast(smtTree *);

smtTree* smt_mark_stree(astTree* root, int mode, int type,
	int  start_line,  char const *start_file, int  end_line, char const *end_file);
 
smtTree* smt_mark_stree(astTree* root, int mode, int type,
			int  start_line,  char const *start_file, int  end_line,
			char const *end_file, smtHeader* s_h);

int smt_map_template(smtHeaderPtr s_h, char const *lname, int smt_mode, astTree* root, 
                 char const *&  start_file, char const *&  end_file, int& smt_type);

smtTree* smt_find_stmt_node(smtHeader*, int);
//void smt_kill_stmt(smtTree*);
 
void smt_fill_decl_info(smtTree* smt_decl);
int smt_get_id_line(astTree* decl);

int smt_map_switch(smtHeaderPtr s_h, char const *lname, int smt_mode, astTree* root, 
   int& st_line, char const *&  st_file, int& en_line, char const *&  en_file, int& smt_type);

int smt_map_type(smtHeaderPtr s_h, char const *lname, int smt_mode, astTree* root, 
                 char const *&  start_file, char const *&  end_file, int& smt_type);

int smt_map_function(smtHeaderPtr s_h, char const *lname, int smt_mode, astTree* root, 
         int&  start_line, char const *&  start_file,int&  end_line, char const *&end_file,
                int smt_type, int method_context);

enum smt_asynch_type {
	NOT_FILLED,
	REALLY_REPARSE_MAKE_AST,
	REALLY_REPARSE_IMPORT
};

struct smt_asynch_data {
	void* make_ast_header;
	void* parse_header;
	smt_asynch_type oper;
	int data;
	smt_asynch_data () : make_ast_header(0), parse_header(0), oper(NOT_FILLED),
			     data (0) {}
};
int smt_setup_segment (smtHeader*);
extern "C" void set_smt_asynch_data (void*, void*, smt_asynch_type, int);
extern "C" void clear_smt_asynch_data ();
extern "C" smt_asynch_data* get_smt_asynch_data ();
extern "C" void set_smt_asynch_parse_header (void*);
extern "C" int check_smt_asynch_data ();
extern astTree* smt_get_ast_new (smtTree*);

#endif

