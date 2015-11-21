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
/*  XrefTable.h */

#ifndef _XrefTable_h
#define _XrefTable_h
#include <Relational.h>
#include <objSet.h>
#include <objArr_Int.h>
#include <genString.h>
#include <genArr.h>
#include <Link.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include <symHeaderInfo.h>
#include <linkHeaderInfo.h>
#include <genArr.h>
#include <objRelation.h>

class XrefTable;
class xrefSymbol;
class symbolPtr;
class Xref;
class projNode;
class RTLNode;
typedef RTLNode * RTLNodePtr;

class ddSelector;
class ddSymbol;
class ddElement;
class symbolArr;
class symHeaderInfo;
class indHeaderInfo;
class hashPair;
class ddRoot;
class headerInfo;
class linkTypes;

class hashElmt;
class Link;
genArr(Link);
typedef LinkArr* SymbolIndex;
genArr(SymbolIndex);
#if !defined(intArr_defined)
genArr(int);
#define intArr_defined
#endif

extern char const *PSET_MSYNC_XREF;
#ifdef _WIN32
typedef unsigned short mode_t;
#endif

class headerInfo;
class hashElmt;

/**  XrefTable - class to map the pmod file into memory, and support searching
 * and modifying it
 **/
class XrefTable : public Relational
{
    // from XrefTable:
 public:
    XrefTable(Xref* xr);
    XrefTable(Xref* xr, char const *sym_file, char const *ind_file, char const *link_file, int readonly = 0, projNode* proj=0);
    virtual ~XrefTable();
    xrefSymbol* get_symbol(Relational* obj);
    xrefSymbol* lookup_module(char const *file_name);
    xrefSymbol * find_symbol(Relational *);
    xrefSymbol* get_def_file_symbol(Relational* rel);
    SymbolIndexArr *symbolIndex_table; //array of SymbolIndex to store symbol info
    Xref* my_Xref;   // reverse pointer up to my Xref instance (for performance)
    static XrefTable* first_XrefTable;
    XrefTable* next;                 // link to next XrefTable in sequence
    projNode* my_proj;               // connection to the project that owns me
    define_relational(XrefTable,Relational);
    // end XrefTable
 public:
    void is_consistent_hashElmt(hashElmt* xe);            // testing functions only
    void is_consistent_hashElmts();
    char const *extend_file(int symgrow, int indgrow, bool movable);
    xrefSymbol* newSymbol(ddKind kind, int nl, char const *ns);
    xrefSymbol* halfNewSymbol(ddKind kind, int nl, char const *ns);
    void deleteSymbol(xrefSymbol* sym);
    void deleteSymbol(Relational*);
    virtual xrefSymbol* insert_symbol(Relational* obj, int lk = 0);
    virtual xrefSymbol* lookup(symbolArr&, ddKind k, char const *ns, xrefSymbol* dfs );
    virtual xrefSymbol* insert_symbol(xrefSymbol*, XrefTable*);
    virtual xrefSymbol* insert_symbol(ddKind k, int nl, char const *ns);
    virtual Xref * get_Xref();
    void init_link(Relational * obj); // obj here is the app level of symbol
    void link_xref(XrefTable* xt);
    virtual void init_link(xrefSymbol* sym, Relational * obj);
    virtual void add_link(xrefSymbol* sym,linkType lt,  Relational* obj);
    virtual int collect_symbols(xrefSymbol*, linkTypes&, int, symbolArr&, xrefSymbol*, int, XrefTable*);
    int to_read(char const *);
    int to_write(char const *);
    virtual void insert_module(app* head);
    virtual void insert_module(app* head, int);
    void remove_old_links(hashPair*);
    void call_rel_hooks(ddRoot*);
    void remove_module(char const *filename);
    void build_remove_module(char const *filename, hashPair *);  
    void remove_module1(char const *filename);

    void tidy_xref(XrefTable* nx);
    void merge_xref(XrefTable* xref);
    int Mhash(char const *s) ;

    // test if there's enough room for xref table to grow without moving
    bool is_room(int size);
    //  if there isn't enough room, move the xref table now
    bool make_room(int size);

    int lookup(objArr_Int&, Relational* rel, xrefSymbol* dfs );
    int lookup(objArr_Int&, ddKind k, char const *ns, xrefSymbol* dfs );
    uint get_symbols(symbolArr& as);        // historical wrapper for following function, which should have default 2nd arg
    uint get_symbols(symbolArr& as, const Xref* xr) const;
    void copy_link(xrefSymbol* sym, linkType lt, XrefTable* Xr, xrefSymbol* lt_of=0);
    void copy_module(char const *module_name, XrefTable* to);
    void copy_module1(char const *module_name, XrefTable* to);

    headerInfo * get_header() { return header;}
    void deleteHashTable();
    void write_whole_XrefTable(char const *sym_file, char const *ind_file, char const *link_file);
    void remap_to_private();     // remap from SHARED to PRIVATE
    char const *do_backup ();
    int dump_new_links_to_pmod(genString&);
    XrefTable* map_backup ();
    XrefTable* get_backup () {  return backup ? backup : map_backup (); }
    unsigned int max_offset();
    int need_to_mv_pmod_to_perm() ;
    void set_need_to_mv_pmod_to_perm() {next_offset_sym = 1;}

    symbolPtr insert_def_file_link(ddSymbol *, symbolPtr);
    int mv_pmod_to_perm();
    void truncate_pmod(char const *indfile, char const *symfile);
    void insert_SWT_Entity_Status (app *);
    int copy_pmod_to_perm();
    int copy_sym_mmap_to(genString &);
    int copy_ind_mmap_to(genString &);  
    int copy_link_mmap_to(genString &);
    uint make_new_symbolIndex();
    //private:
    //  symInd* insert_symInd(uint so, uint h);
    xrefSymbol* insert_xrefSymbol();
    void insert_xrefSymbol_name(uint , char const *, uint);
    xrefSymbol* get_symbol_from_offset(int offset);
    void put_def_file_for_sym(ddSymbol* dd, xrefSymbol* sym);
    int get_offset_from_symbol(xrefSymbol* sym);
    xrefSymbol* confirm_is_symbol(xrefSymbol* el, ddKind k,
                                  char const *name, char const *file_sym);
    int get_sym_offset_from_ind(int ind);
    xrefSymbol *get_symbol_from_index(int); 
    uint init_symbolIndex_table();
    void update_link_offsets_of_syms(int *);
    int get_orig_link_offset(uint);
    virtual Link* get_link_addr() { return link_addr;}
    virtual xrefSymbol* get_ind_addr() { return ind_addr;}
    virtual char const *get_sym_addr();
    void re_insert(hashElmt* xe);
    void rehash_xref();
    void reset_pmod(char const *, char const *, char const *);
    void msync_xref();
  
    void insert_time_t(time_t *t) {sym_header->insert_time_t(t);}
    time_t * extract_time_t(time_t *t) {return sym_header->extract_time_t(t);}
    void insert_sdo_update_time();
    time_t *get_last_sdo_update(time_t *t);  
    void file_trigger(symbolArr &, genString &);
    void code_trigger(symbolArr &modified, genString &, int ti);
    
    /* orig_link_offsets and saved_next_loffset are used to save the 
       original values of offset_to_first_link and next_offset_link
       since these values change when pmods are periodically saved.
    */
    intArr orig_link_offsets;
    int saved_next_loffset;  

    headerInfo* header; // point to header of sym map file
    hashElmt **table; // hash table for mapped file
    char *sym_addr; // beginning of mapped area (used to be sizeof(headerInfo) + header)
    xrefSymbol *ind_addr; // starting address of mapped file for symbol
    Link* link_addr; //starting address of mapped file for links
#ifndef _WIN32
    int unit;
    int unit1;
    int unit2;
#else
    HANDLE unit;
    HANDLE unit1;
    HANDLE unit2;
#endif
    unsigned int fresh:1; // 1 -> fresh .sym & .ind files, 0 -> already existing
    unsigned int nosaveflag:1; // 1 -> save mechanism disable, 0 -> save mechanism in use
    // (when disabled, we map shared, with write-thru to file)
    unsigned int filler0:1;		// unused
    unsigned int delete_in_destructor:1;  // flag whether files are to be deleted in destructor
    unsigned int filler:28;

    XrefTable* backup;
    genString symfile_name;
    genString indfile_name;
    genString linkfile_name;
    genString symbackup;
    genString indbackup;
    genString linkbackup;
    // start of header info for each XrefTable
    int sym_is_writable;
    int ind_is_writable;
    int link_is_writable;
    int units_are_backup;
    symHeaderInfo *sym_header;
    indHeaderInfo *ind_header;          // ptr to header area of ind file
    linkHeaderInfo *link_header;
    int sym_map_size;                // total size of mapped areas
    int ind_map_size;
    int link_map_size;

    uint next_offset_sym; // store orig value for comparison when exiting
  

    virtual void add_symbols(symbolArr& arr, const ddSelector& selector);   // add symbols corresponding to this selector
    uint update_sym_addr_offset(int n);    // move current-end pointer of xref 
  
    int pmod_num;
    int insert_to_pmod_array() const;
    void remove_from_pmod_array() const;
    static  XrefTable* get_XrefTable_from_num(int); 
    int get_pmod_num () { return pmod_num; }

    //
    // PRIVATE:
    //

 private:

    mode_t mode;                    // File creation mode value

};


inline int XrefTable::need_to_mv_pmod_to_perm()
{
    return (next_offset_sym  &&
            symfile_name.length() && symbackup.length() &&
            strcmp(symfile_name,symbackup));
}

inline int XrefTable::get_orig_link_offset(uint ind)
{
    return *orig_link_offsets[ind];
}

void check_old_version_of_pmod(char const *, char const *, Xref*);
#ifdef _WIN32
HANDLE open_timeout(char const *name, int flags, int mode);
#else
int open_timeout(char const *name, int flags, int mode);
#endif

generate_descriptor(XrefTable,Relational);
#endif
