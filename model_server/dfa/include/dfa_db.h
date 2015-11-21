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
#ifndef _dfa_db_h
#define _dfa_db_h

#include "Relational.h"
#include "objOper.h"
#include "symbolPtr.h"
#include "objArr.h"
#include "genArr.h"
#include "Hash.h"

class symbolArr;
class dataCell;

class objIndSet : public Set {

public:

  const Relational *lookup(const Relational *) const;

  const Relational *sym(const Object *oo) const 
    {
      return 
	(Relational *) arr[((int) oo)-1];
    }

  int index(const Relational *oo) const;
  bool insert(const Relational *);
  virtual unsigned hash_element(const Object *) const;
  virtual bool isEqualObjects(const Object &, const Object &) const;

  objArr arr;
};


typedef unsigned char byte;

class db_buffer;
class db_app;
class astNode;
class astXref;
class smtHeader;

typedef struct _dfa_data {
    int ast_category;
    int offset;
} dfa_data;

genArr(dfa_data);

class astRoot;

class astTokenHash : public Set {

public:

  astTokenHash(astRoot *r);
  virtual unsigned hash_element(const Object *) const;
  virtual bool isEqualObjects(const Object &, const Object &) const;

  int find(unsigned hash, int &idx, Object *&ob) const;
  int next(unsigned hash, int &idx, Object *&ob) const;
  int lookup(astXref *axr, int off, symbolArr &res);

private:

  void insert(astNode *);

};

class astRoot : public appTree {

  friend class astXref;

public:

  astRoot(smtHeader *h);	// constructor
  astRoot(const astRoot &);     // the copy constructor
  ~astRoot();			// Destructor

  static void db_save(app *h, appTree *node, db_app &, db_buffer &);
  static astRoot *db_restore(app *h, appTree *parent, db_app &, db_buffer& );
  static astRoot *get_create_astRoot(smtHeader *h, db_buffer *buf = 0);

  astNode *get_astNode () 
    {
      return dfa_root;
    }

  astXref *get_xref()
    {
      return xref;
    }

  void get_tokens(int off, symbolArr &results);
  void reset();
  app *get_main_header();
 
  define_relational(astRoot, appTree);  

  objIndSet smts;
  genArrOf(dfa_data) data;
  objArr dataCells;

private:

  astNode *dfa_root;
  db_buffer *buffer;
  int _size;
  astXref *xref;
  astTokenHash *tokens;

};

generate_descriptor(astRoot, appTree);

class astXref : public Relational {

  symbolPtr module;
  astRoot *ast_table;

public:

  astXref(const symbolPtr &, astRoot *);
  astRoot *get_ast_table(bool mode = true);
  astRoot *load();
  bool is_loaded() { return ast_table != NULL; }
  void set_ast_table (astRoot* ast) {ast_table = ast;}
  virtual char const *get_name() const;
  define_relational(astXref, Relational);
  astXref(const astXref&);
  void notify_unload();
  int get_offset(astNode*);
  astNode* get_node_from_offset(int offset);
  int generate_array(astNode*,symbolArr&);
  symbolPtr get_module();
  int get_astNodes(ddElement*,symbolArr&);
  dataCell* get_dataCell_from_id(int id);
  ~astXref();	
};

generate_descriptor(astXref, Relational);

int relClass(astRoot);

define_relation(astRoot_of_smtHeader,smtHeader_of_astRoot);
astXref *app_get_astXref(app *);
astNode *astNode_of_sym(const symbolPtr &);
//int generate_array_astNodes(const symbolPtr &, symbolArr &);

void db_set_incremental ();
void db_unset_incremental();

struct dfaLoader {

  dfaLoader()
    {
      db_set_incremental();
    }

 ~dfaLoader()
   {
     db_unset_incremental();
   }

};

#endif
