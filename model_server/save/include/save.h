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
#ifndef _save_h
#define _save_h

#ifndef _objOper_h
#include "objOper.h"
#endif

#define SAVE_RelType   1		// Type for relation type record
#define SAVE_Relation  2		// Type for relation record
#define SAVE_extheader 3		// Type for external header record
#define SAVE_tree      4		// Start of tree record
#define SAVE_header    5		// Header
#define SAVE_endtree   6		// End of tree
#define SAVE_node      64		// Tree node

#define SAVE_EXT_NONE	""
#define SAVE_EXT_REL	",rel"
#define SAVE_EXT_DD	",dd"
#define SAVE_EXT_PRJ	",prj"
#define SAVE_EXT_KWD	",kwd"
#define SAVE_EXT_NOTES	",notes"
#define SAVE_EXT_WCFG	",wcfg"
#define SAVE_EXT_FCFG	",fcfg"
#define SAVE_EXT_PCFG	",pcfg"
#define SAVE_EXT_ALIASES ",aliases"

// global variables

extern Relational * save_header;	// Header address
extern int save_hdef;			// Type number for header definition
extern int save_tdef;			// Type number for tree definition
extern char * save_filename;
extern Relational * save_rheader;	// Header address
extern int save_text;			// 1 if text file needed
extern int save_level;
extern appTree * save_node;

// Save definition table

struct save_def
{
    class_descriptor * dhdr;	// Address of header descriptor
    class_descriptor * dnode;	// Address of node descriptor
    RelType ** trel;		// Type of relation to root
    char * suff;		// Suffix for text file
};

extern save_def save_def_table[];


struct save_description
{
    long magic;			// Magic number
				//  #REL, #DDC, #KWD
    long version;		// Version of save file format
    int date;			// Date of original header creation
    long checksum;		// Checksum of source file
};


struct save_rechdr
{
    unsigned char type;		// For header record: SAVE_hb + number of
				//   first save_def_table entry for header type;
				// For tree node record: SAVE_tb + number of
				//   entry for header + relation + tree;
				// For special record: SAVE_...
    unsigned char level;	// Level of node in tree
    unsigned char lth;		// Length of specific record
    unsigned char nspace;	// Number of spaces before text token
    unsigned short tlth;	// Length of token
    int id;			// ID of node
};


// Remote header

class rmtHeader : public app
{
    static rmtHeader* first;
    rmtHeader* next;

    RelationalPtr rel_copy() const { return NULL; }

  public:
    static rmtHeader* find(char* name, int apptype = 0);
    char* name;
    int   apptype;
    int   extype;

    rmtHeader(app*);
    rmtHeader(char*, int);
    ~rmtHeader();

    define_relational(rmtHeader, app);

    virtual void set_filename (char*);
    virtual char const *get_filename();
    virtual char const *get_name()   const;
    virtual int   get_extype() const;
    virtual int   get_type();

    void insert_obj(objInserter*, objInserter*);
    void remove_obj(objRemover*,  objRemover*);
};
generate_descriptor(rmtHeader, app);


// Remote node

class rmtNode: public appTree
{
    RelationalPtr rel_copy() const {return(NULL);};

  public:
    app * get_main_header();
    define_relational(rmtNode, appTree);
};
generate_descriptor(rmtNode, appTree);


struct save_relation
{
    short rtid;		// Internal identifier of the relation type
    short hdid;		// Internal identifier of header (or 0 if the same)
    int nid;		// ID of node
};


struct saveInd
{
    unsigned char type;
    short ind;			// indentation displacement
    unsigned char nlb : 1;	// at least one newline requested before
    unsigned char nla : 1;	// at least one newline requested after
};


int  strend(const char *, const char *);

int  move_relations(app *, app *, int = 0);
int  move_relations(appTree *, appTree *, int = 0);
int  is_relation_persistent(RelType *);
int  save_save(app *);
void save_restore(char *);
void save_restore(char *, Relational *);
saveInd* save_get_indent(appTree *);
int  save_find_def(Relational *, Relational *, RelType *);
void save_put_rec(int type, int, int, int, void *, int, int);
void save_error(char *);

int  save_make_name(char *, char*, char*);
int  save_make_name(char *, char*, char*, int);
int  save_get_type(char *);
int  save_open(char *, int);
app* save_close(app*);
void save_close();
int  save_read_description(save_description *);
int  save_write_description(save_description *);
int  save_read(void *);
int  save_write(void *);
int  save_open_text(char *, char *);
void save_close_text();
int  save_tget(char *, int);
int* save_tput(char *, int = 0);
Relational* save_common_restore(class_descriptor *,
				void *, app * h, char * = 0);

#endif // _save_h
