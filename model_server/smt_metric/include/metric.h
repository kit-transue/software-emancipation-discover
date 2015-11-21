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
#ifndef _metric_h
#define _metric_h


/************************************************************

When modifying this file, recompile files where it's included.

************************************************************/

#include <general.h>
#include <genArr.h>
#include <symbolPtr.h>
#include <Link.h>
#include <smt.h>
#include <Relational.h>
#include <objOper.h>
#include <projHeader.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

typedef unsigned int uint;
class genArrOf(int);

enum metricType 
{
    MET_CC,         // Cyclomatic Complexity
    MET_SRC_LINE_NUM, // lines of code
    MET_MYERS,        // MET_CC + MET_AND_ORS
    MET_FANIN,        // Base classes
    MET_FANOUT,       // Derived Classes
    MET_BYTELEN,      // Func/File length in bytes.
    MET_DEPTH,        // Depth of nested control structures.
    MET_DANG_ELSEIF,  // Dangling else-ifs
    MET_DANG_SWITCH,  // Switch without default clause
    MET_START_LINE,   // starting line of definition.
    MET_RETURNS,      // Number of Returns in code.
    MET_ULOC,         // Uncommented lines of code (total - comments - empty lines).
    MET_XLOC,         // Same but don't count ifdef'd out parts, function declarations and lines with only '{' or '}'.
    MET_IFS,
    MET_SWITCHES,
    MET_LOOPS,
    MET_DEFAULTS,

    NUM_OF_METRICS,    // Must be last. 
      
    // The following are the special types; all new metric types must be added above.

    MET_END_ARRAY = 128,    /* Internally used to end arrays of metrics. It is 128 because it's 
			       not written anywhere*/
    MET_FREE = 127,         // Empty spot  
    MET_OBJECT = 126,       // "Title"
    MET_ELEMENT = 125,      // 
    MET_MOVED = 124,        /* If the block has moved, the index is the new ABSOLUTE 
			       position in the table. */
    MET_CHILDREN = 123,     /* a block of children of a 
			       symbol (s.t. funcs in a file). */
    MET_lmd = 122,
    MET_lmdhigh = 121,       /* Date representation */


      /* Y2K additions */

    MET_INSTANCE_BLOCK = 120,
    MET_INSTANCE = 119,

    MET_INSTANCE_OFFSET = 118,       /* offset in characters from the beginning of the module */
    MET_INSTANCE_LINE = 117,         /* line number in file */
    MET_INSTANCE_LN_OFFSET = 116,    /* number of characters from start of line */
    MET_INSTANCE_SCOPE = 115,        /* offset of xrefSymbol for the scope of instnace */
    MET_INSTANCE_SYMBOL = 114,       /* offset of xrefSymbol for the instnace */
    MET_INSTANCE_LENGTH = 113,       /* number of characters in symbol's name */

    MET_UNDEFINED_ELEMENT = 112,     /* Start of block for elements referenced in file
                                         but not defined there. */        

    MET_LOCAL_DEC_BLOCK = 111,
    MET_DECL_INSTANCE = 110,
    MET_AST_CATEGORY = 109,
    MET_USED_BY_SYMBOL = 108

};

enum astAtt
{
ASSIGN_ATT = 0,
DECL_ATT = 1,
ARG_ATT = 2,
VALUE_ATT = 3,
ADDR_ATT = 4,
ASSIGN_LEVEL_ATT = 16, // length 4
VALUE_LEVEL_ATT =20,   // length 4
LAST_AST_ATT = 24
};

enum met_ent_kind { MI_file = 0, MI_class = 1, MI_func = 2, MI_num = 3 };

typedef void (*met_fcn)(symbolPtr&, smtTree*&, class Relational*, void *);

struct metric_definition {
    metricType type;
    const char * name;
    const char * abbrev;
    bool type_is_int;
    met_fcn fcn;
};

void get_smt_of_sym(symbolPtr&, smtTree *&);
void get_smt_of_sym(symbolPtr&, smtTree *&, smtHeader *&);


#define metric_callback(name) \
static void name(symbolPtr& sym, smtTree*& node, Relational* met, void* res)

class MetricElement;

class Metric : public Relational {
  public:
    metric_callback(cyclomatic_complexity);
    metric_callback(number_of_lines);
    metric_callback(fanin);
    metric_callback(fanout);
    metric_callback(myers);
    metric_callback(bytelen);
    metric_callback(depth);
    metric_callback(dang_elseif);
    metric_callback(dang_switch);
    metric_callback(starting_line);
    metric_callback(returns);
    metric_callback(uncom_lines);
    metric_callback(exec_lines);
    metric_callback(ifs);
    metric_callback(switches);
    metric_callback(defaults);
    metric_callback(loops);

  public:
    static void node_metric(symbolPtr& sym, smtTree *& node, void * res, int type);
    static void token_metric(symbolPtr& sym, smtTree *& node, void * res, int ext_type, char const * name);


  public:
    int cur_idx;
    metricType * seq;
    static metric_definition * def_array[NUM_OF_METRICS]; // a sorted version of definitions
    static metric_definition  definitions[];
    static metricType  file_sequence[];
    static metricType  func_sequence[];
    static metricType  class_sequence[];

  private:
    static bool need_compute_def[MI_num][NUM_OF_METRICS];
    static bool need_compute_static[MI_num][NUM_OF_METRICS];
    
    symbolPtr sym;
    
    bool * need_compute; 
    bool * my_need_compute;
    bool uses_defaults;

    union {
	int int_value;
	double dbl_value;
    } result[NUM_OF_METRICS];
    
  public:
    static int init_statics();

  public:
    Metric(symbolPtr& sym);
    virtual ~Metric();
    static bool is_int(int index);
    void set_value (metricType type, int val);
    void set_value (metricType type, double val);
    int get_i_value(metricType type);
    double get_f_value(metricType type);
    void need_to_compute(metricType type, bool flag);
    bool need_to_compute(metricType type);
    static int compute_int_metric(metricType, symbolPtr&);
    void compute_all_metrics();
    static void compute_one_metric(metricType type, symbolPtr& sym, smtTree *& node,
				   Relational * met, void * res);
    static void find_one_metric(metricType type, symbolPtr& sym, smtTree *& node,
				   Relational * met, void * res);
    static int compute_one_metric_int(metricType type, symbolPtr& sym, smtTree *& node,
				   Relational * met);
    static void get_metric(metricType, symbolPtr&, void *);
    static void get_metric_nocompute(metricType, symbolPtr&, void *);
    static void need_to_compute_static(int idx, int type, bool flag);
    static bool need_to_compute_static(int idx, int type);
    static bool need_to_compute_def(int idx, int type) ;
    const char * name();
    const char * abbrev();
    int int_value();
    double dbl_value();
    bool is_int();
    bool is_computed();

    static void get_instance_list(symbolPtr& module, symbolPtr& sym, 
				  symbolArr& results);



    static void get_decls(MetricElement mel, MetricSet* set, symbolArr& results);
    static void get_instances(MetricElement mel, MetricSet* set, symbolArr& results);
    static MetricElement get_mel_of_undefined_sym(symbolPtr& module, symbolPtr& sym);
    
    static char* get_instance_info(symbolPtr& sym);
    
    define_relational(Metric, Relational);

};
generate_descriptor(Metric, Relational);

// The following is an iterator that allows to walk all the metrics in Metric metric.
#define ForEachMetric(metric) \
  for ((metric).cur_idx = 0; \
       (metric).seq[(metric).cur_idx] != MET_END_ARRAY; \
       (metric).cur_idx ++) \
    if ((metric).need_to_compute((metric).seq[(metric).cur_idx])) 

class MetricHash : public Object {
  public:
    unsigned offset;   // offset in MetricTable

    // the key is computes using MetricSet::hash_element

  public:
    MetricHash(unsigned off = 0, unsigned is_key = 0) : offset(off * 2 + is_key) {}
    virtual ~MetricHash();
    virtual unsigned hash() const;
    virtual bool isEqual(const Object &) const;
};

// the following is just a wrapper for OldLink
struct MLink {
  private:
    OldLink link;
    
  public:
    metricType type() const                           { return (metricType)link.lt(); }
    uint  is_block() const                            { return link.way(); }
    void assign(metricType t, int b, uint index)      { link.assign ( (linkType)t, b, index); } 
    void assign(metricType t, int b)                  { link.assign ( (linkType)t, b); }
    void set_index(uint ui)                           { link.set_index(ui); }
    int get_index() const                             { return link.get_index(); }
};

genArr(MLink);

class MetricTable : public MLinkArr {
  private:
    int compress_priv(int, int, int);
  public:
    bool notify;   /* Whether to leave a "MET_MOVED" token when moving 
		      an element in the table */

  public:
    MetricTable(int sz = 0, bool nott = false) : MLinkArr(sz), notify(nott) { length = sz; }
    int next_index(int i); // if current i points to a block, skip 
    int update_index(int & i); // if the element has been moved, update the pointer
    int compress(); // After execution, all MetricElements become invalid
   
    /* this would be true for types that specify a block that should not be compressed*/    
    bool do_not_compress(metricType ) { return false; }

    void set_free_block(int from, int len);
};

typedef unsigned char uchar;

class MetricElement : public Relational {
   
  private:
    MetricTable * table;   
    MetricTable * priv_table;
    
    int offset;      // Offset in table
      
  private:
    void make_room_in_table(int sz, bool look_for_space = false);
    int get_size_in_table() { return (*table)[offset]->get_index(); }
    int new_size(int /* old_capacity */, int required_size) { return required_size; }
     
  public:
    MetricElement() : table(NULL), priv_table(NULL), offset(0) {}
    MetricElement(metricType block_type);
    MetricElement(symbolPtr& sym);
    MetricElement(MetricTable * tbl, int off) : table(tbl), priv_table(NULL), offset(off) {}
    MetricElement(const MetricElement & ob);
    virtual ~MetricElement();

    void set();
    void set(metricType block_type);
    void set(MetricTable * tbl, int off);
    void set_ast_attribute(uint value, uint bitnum, uint numbits);
    uint get_ast_attribute(uint bitnum, uint numbits);
    static int init_statics();
    int get_offset() { return offset; }
    MLink & operator[](int idx);
    void set_int_at(int i, int val);
    int  get_int_at(int i);
    // Set/get a char at offset d BYTES from the i'th element of the array 
    void set_char_at(int i, int d, uchar val);
    uchar  get_char_at(int i, int d) ;
    void set_free_block(int from, int len) ;
    
    void set_value(metricType type, int value, bool overwrite = true);
    void compute_set_val(metricType type, symbolPtr& sym);
    void compute_set_values(symbolPtr& sym);
    int get_value(metricType type);
    void add_instance_list (symbolPtr& sym, symbolPtr& scope, objArr_Int *smt_line_off);
    void add_instance_list (ddElement* ddel, symbolPtr& scope, objArr_Int *smt_line_off);
    void add_local_var_instance_list (ddElement* ddel, symbolPtr& scope, objArr_Int *smt_line_off);
    void generate_instance (MetricElement& imel, smtTree* stree, 
				       ddElement* ddel, symbolPtr scope, objArr_Int *smt_line_off);
    bool value_exists(metricType type);
    void make_room(int sz);   // Make enough room to store sz metrics
    int get_size();
    void get();
    void put(MetricTable * tbl = NULL, int off = -1);   // where to put
    void unget() { if (priv_table) delete priv_table; priv_table = NULL; }
    bool isnotnull() { return (table!=NULL || priv_table!=NULL); }
    bool isnull() { return (table==NULL && priv_table==NULL); }
    bool uses_priv_table() { return (priv_table!=NULL); }
    int next_index(int i);
    void update_offset() { if (table) table->update_index(offset); }    
    void append(MetricElement &);
    void extract(int, MetricElement&);
    void get_block(metricType, MetricElement&);
    void clear();
    void set_last_mod_date(time_t date);
    time_t get_last_mod_date();
    MetricElement FindParentElement();
    MetricElement inst_get_decl();
    MetricElement find_inst_by_offset( symbolPtr& mod , int off); //NEW arg symbolPtr
    MetricElement find_inst_by_line_offset( symbolPtr&, int, int); //NEW arg symbolPtr
    static symbolPtr get_instance(char const * file, int off);
    static symbolPtr get_instance(char const *, int, int);
    void find_inst_between_offsets( symbolPtr& , int startOffset , int endOffset , symbolArr&);//NEW
    static void get_instances(char const * file, int offStart , int offEnd , symbolArr& arr);//NEW

    int get_inst_symbol_length( symbolPtr& mod ) const; //NEW
  public:
    static bool stored[MI_num][NUM_OF_METRICS];
    static metricType  file_array[];
    static metricType  func_array[];
    static metricType  class_array[];

    define_relational(MetricElement, Relational);
};
generate_descriptor(MetricElement, Relational);

class MSet : public Set {
  public:
    virtual unsigned int hash_element (const Object * ob) const;
    virtual bool isEqualObjects(const Object& ob1, const Object& ob2) const;
  public:
    MetricTable table;
};

class MetricSet : public Relational {
  private:
    MSet set;
    genString file_name;

  public:
    MetricSet(projNode * pr);
    virtual ~MetricSet();

    bool load_table();
    bool save_table();

    MetricTable* get_table() {return &set.table;}
    
    void add_element(MetricElement & mel, int old_offset = -1);
    void add_element(symbolPtr& sym, int old_offset = -1, MetricElement * mel = NULL);
    void find_element(symbolPtr& sym, MetricElement &);
    void remove_element(symbolPtr&, bool only_children = false);

    static void set_metrics_of_smtHeader(smtHeader *, bool from_build = false);
    static void get_metrics_of_sym(symbolPtr&, MetricElement&);
    static void get_metrics_of_sym_aux(symbolPtr def_file, symbolPtr sym);
    static void get_metrics_of_sym_nocompute(symbolPtr&, MetricElement&);
    define_relational(MetricSet, Relational);
};
generate_descriptor(MetricSet, Relational);

defrel_one_to_one(MetricSet,set,projNode,proj);

class needLoad {
    bool old_need_load;
  public:
    static bool need_load_files;
    needLoad(bool need_load) { old_need_load = need_load_files; need_load_files = need_load; }
    ~needLoad() { need_load_files = old_need_load; }
    static bool ask() { return need_load_files; }
};

static int metric_element_initializer = MetricElement::init_statics();

static int metric_initializer = Metric::init_statics();

bool store_metrics();

#endif



