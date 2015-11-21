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
#ifndef _checkpmod_h
#define _checkpmod_h

class  testpmod
{
  public:
    testpmod(projNode* pr, Xref* xr, bool verbose, int quiet=0, char* label="test");   // constructor
    ~testpmod();
    bool checkall();
    void simple_check_headers();
    void find_all_syms();
    bool simple_check_links(xrefSymbol* sym);
    void terminate(const char* errmsg);   // terminate paraset with final error message

    bool checksym(xrefSymbol* sym);
    bool checklink(xrefSymbol* sym, Link lnk);
    bool chaselinks(xrefSymbol* sym);
    bool checksymhead(xrefSymbol* sym);
    bool process_one_module();
    void mark_processing(int i);             // mark this symbol as being processed
    void mark_processing(xrefSymbol* sym);     // mark this symbol as being processed
    bool processed(xrefSymbol* sym);    // check if this symbol has been processed
    bool processed(int i);    // check if this symbol has been processed
    void mark_pending(xrefSymbol* sym);     // mark this symbol as being pending
    void mark_bad_symbol(xrefSymbol* sym);   // mark this symbol as having a problem
    void mark_bad_module(xrefSymbol* sym);   // mark this symbol as being a module with a problem
    bool is_bad_module(xrefSymbol* sym);
    bool do_thorough_test(xrefSymbol* sym);
    bool mark_bytemap(const void* address, int size);  // mark these bytes as used, return nonzero if already were
    void tell_changed(int index, int state);
    void show_unused_bytemap();
    bool mark_sym_link_space(const symbolPtr symptr);
    unsigned int elapsed_time();
    void check_null(char* name, char* limit);
    int make_symbol(xrefSymbol* sym);
    int get_symbol(xrefSymbol* sym) const;
    xrefSymbol* get_symbol(int i) const;
    unsigned get_flag(int i, int flag=0xffffffff) const;
    int get_next_flag(int i, int flag) const;
    void set_flag(int i, int flag);
    void reset_flag(int i, int flag);
    void show_bad_symbols();
    
    bool test_link(xrefSymbol* sym, linkType lt, xrefSymbol* destsym);
    bool test_dup_link(Link* link, linkType lt, xrefSymbol* destsym);
    void sym_print(xrefSymbol*);
    xrefSymbol* get_namesymbol_indirect(xrefSymbol* sym, bool cautious=0);    // find a renamed symbol
    char* get_name(xrefSymbol* sym,bool cautious=0);                          // get name, even if renamed
    char* get_name_safe(xrefSymbol* sym);

    int get_index(Link* lnk);
    xrefSymbol* get_symbol_of_link(Link* lnk);
    int get_first_link_offset(xrefSymbol*);
    int get_index_of_symbol(xrefSymbol*);
    int get_name_length_indirect(xrefSymbol* sym);      // get length of renamed symbol
    static int get_name_length(xrefSymbol* sym);    // get length of original symbol (mainly to find links)
    void repair();                   // entry point called by -batch -repair
    
    bool verbose;
    projNode* pr;
    Xref*  xr;
    XrefTable *xrt;
    symHeaderInfo* symhdr;
    headerInfo* hi;
    indHeaderInfo* indhdr;
    linkHeaderInfo* linkhdr;

    unsigned int  maxindex;   // total size of following
    int*  index;           // index array, same size as pmod body, index into all_syms

    unsigned int maxsym;      // capacity of this array
    unsigned int lastsym;      // last symbol to be inserted here
    xrefSymbol**  all_syms;       // pointer to a (dynamic) array of pointers to valid symbols
    int*  flags;      // pointer to a (dynamic) array of flags, matched to above
#define  ALL_SYM  1<<0
#define  PENDING  1<<4
#define  PROCESSED 1<<8
#define  BADSYMBOL 1<<12
#define  BADMODULE 1<<16
	//  note:  these flags are really two bits each, but I'm using 4 bits to make debugging easier

    int bad_symbols;
    char* sym_name;        // ptr to name portion of xrt (might be either symfile_name, or symbackup)
    char* ind_name;        // ptr to name portion of xrt (might be either indfile_name, or indbackup)
    char* link_name;       // ptr to name portion of xrt (might be either linkfile_name, or linkbackup)

    xrefSymbol* beginpmod;
    xrefSymbol* endpmod;
    char* linkpmod_begin;
    char* linkpmod_end;
    char* pmod_bytemap;  // boolean array, which bytes of pmod are already accounted for
    static int global_errors;  // count of total errors in the run
    void symbol_error(xrefSymbol* sym, const char* errmsg=0);

    time_t start_time;          // time the constructor was called
    int suppress;           // flag to make it quieter if being run invisibly, through set-home
    char* label;            // label for test ("test", "repair")
};


#endif
