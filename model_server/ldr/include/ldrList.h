#ifndef __ldrList
#define __ldrList

//
// -----------------------------------------------------
// ldrList.h: definition of ldr for RTLs
// encompasses sorting, filtering, and formatting
// -----------------------------------------------------
//

#include <representation.h>
#include <ldrHeader.h>
#include <RTL.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _ddKind_h
#include <ddKind.h>
#endif

RelClass (RTL);
RelClass (ldrList);
RelClass (ldrListNode);

class symbolAttribute;
class symbolFormat;
struct expr;

genArr (char);
#if !defined(intArr_defined)
genArr (int);
#define intArr_defined
#endif

typedef struct { 
    symbolPtr* sym;
    int index;
    int prefix;
    int oname;
} ListRecord;
genArr (ListRecord);

class symbolFormat;

typedef struct {
    int index;
    int offset;
    int sort_offset;
} SortedRecord;
genArr (SortedRecord);

#define N_SORT_FIELDS    15

// -- external declarations
extern const char * sym_get_form_str(symbolPtr &);
extern const char * sym_get_extension(symbolPtr &);
extern const char * sym_get_symname(symbolPtr &);
extern const char * sym_get_filename(symbolPtr &);
extern int    sym_get_line(symbolPtr &);

struct srt_info
{
    symbolAttribute * field_type;
    int forward;
    int index;
};

class SortedList {
    
    friend class ldrListNode;
  public:
    SortedList(ldrListNodePtr, genString& sort, genString& format, 
	       genString& show, genString& hide, int descending);
    ~SortedList();
    
    int insert_symbol (int);
    int update_symbol (int);
    void remove_symbol (int);
    
    void sort();
    void parse_specs ();
    static int compare(const void*, const void*);
    
  public:
    genString sort_str;
    genString format_str;
    genString show_str;
    genString hide_str;
    genString show_and_cli_str;
    int descending_sort;
    srt_info s_info[N_SORT_FIELDS];
    static SortedList* current_sort_list;
    symbolFormat * att_format;
    expr * cli_expr;

  public:
    ldrListNode* list_node;
    SortedList* next;

    genArrOf(SortedRecord) sorted;
    genArrOf(char) text;
};

class ldrList : public ldrHeader 
{
  public:
    
    ldrList (RTLPtr rtl);
    ldrList (const ldrList &ll);
    ~ldrList();
    
    virtual void build_selection(const ldrSelection&, OperPoint&);
    
    virtual void insert_obj(objInserter *, objInserter *);
    virtual void remove_obj(objRemover *, objRemover *);
    virtual void hilite_obj(objHiliter *, objHiliter *);
    virtual void focus_obj(objFocuser *, objFocuser *);
    
    virtual void send_string(ostream& stream) const;
    
    define_relational(ldrList,ldrHeader);
    copy_member(ldrList);

    symbolPtr* get_obj (int index);
    int find_ldr_index (symbolPtr);
    int find_rtl_index (symbolPtr);

    SortedList* get_list (genString& sort, genString& format, 
	genString& show, genString& hide, int descending_sort);
};
generate_descriptor(ldrList, ldrHeader);


class ldrListNode : public ldrTree 
{
    friend class ldrList;
friend class SortedList;
    friend void lde_list_extract (ldrListNodePtr, RTLNodePtr);

  public:
    
    ldrListNode ();
    ldrListNode (const ldrListNode &ll);
    ~ldrListNode ();
    
    int insert_symbol (symbolPtr, int);
    int update_symbol (symbolPtr, int);
    void remove_symbol (symbolPtr, int);
   
    void insert_sorted(int idx, int sort = 1);
    void insert_sorted_instance(SortedList*, int idx, int sort = 1);
    void update_sorted(int, int sort = 1);
    void remove_sorted(int);
    
    virtual void send_string(ostream& stream) const;
    virtual void print(ostream& =cout, int level=0) const;

    void send_string(ostream& stream, int index) const;
    
    copy_member(ldrListNode);
    define_relational(ldrListNode,ldrTree);

    SortedList* get_list (genString& sort, genString& format,
        genString& show, genString& hide, int descending_sort);
    const char* get_str (int index);

  public:
    static ldrListNode* current_ldr_node;
    static void get_sym_name (symbolPtr sym, const unsigned char *& sym_prefix, const char *& sym_name);

  private:
    void get_curr_name (ListRecord *, const unsigned char *& sym_prefix, const char *& sym_name);
    void fill_text (symbolPtr sym, ListRecord* rec);
    void expand_text (symbolPtr sym, ListRecord* rec);
    SortedList* create_view (genString& sort, genString& format,
			     genString& show, genString& hide, int descending_sort);

    int    display_flag;   // what to display
    int    descending;     // sorting order
    genString format_spec; // field format spec
    genString sort_spec;   // field sort spec

  protected:    
    // these fields used by derived class ldrEpochList
    genArrOf(ListRecord) list;
    genArrOf(char) text;
    SortedList* views;
};

generate_descriptor(ldrListNode, ldrNode);

#define isicon(c) ((((unsigned char)c >= 0x80) && ((unsigned char)c <= 0x9f)) || \
                      (((unsigned char)c >= 0xd1) && ((unsigned char)c <= 0xe0)) || \
                      ((unsigned char)c == 0xe2) || \
                      (((unsigned char)c >= 0xe4) && ((unsigned char)c <= 0xe7)) || \
                      (((unsigned char)c >= 0xec) && ((unsigned char)c <= 0xf0)) || \
                      (((unsigned char)c >= 0xf2) && ((unsigned char)c <= 0xf4)) || \
                      (((unsigned char)c >= 0xf6) && ((unsigned char)c <= 0xf7)))

#define PIX_FOLDER                 0xdd
#define PIX_DOC_UNKNOWN  0xdf
#define PIX_DOC_CPP              0xe4
#define PIX_DOC_C                   0xe6
#define PIX_DOC_STD              0xec
#define PIX_DOC_FRAME        0xee
#define PIX_DOC_MAKEFILE   0xf2    // Not used
#define PIX_MODULE                0xd1
#define PIX_WRITE0                  (PIX_MODULE + 0x0)
#define PIX_WRITE1                  (PIX_MODULE + 0x1)
#define PIX_WRITE2                  (PIX_MODULE + 0x2)
#define PIX_WRITE3                  (PIX_MODULE + 0x3)
#define PIX_WRITE4                  (PIX_MODULE + 0x4)
#define PIX_WRITE5                  (PIX_MODULE + 0x5)
#define PIX_WRITE6                  (PIX_MODULE + 0x6)
#define PIX_WRITE7                  (PIX_MODULE + 0x7)
#define PIX_WRITE8                  (PIX_MODULE + 0x8)
#define PIX_WRITE9                  (PIX_MODULE + 0x9)
#define PIX_WRITE10                (PIX_MODULE + 0xa)
#define PIX_WRITE11                (PIX_MODULE + 0xb)

#define PIX_XCOMPONENT    PIX_WRITE11
#define PIX_XINTERFACE       0xf0
#define PIX_XPACKAGE          0xf4

#define PIX_XFUNCTION        0x80
#define PIX_XCLASS           0x81
#define PIX_XVARIABLE        0x82
#define PIX_XENUMLIST        0x83
#define PIX_XUNKNOWN         0x84
#define PIX_XSUBSYSTEM       0x85
#define PIX_XMACRO           0x86
#define PIX_XUNION           0x87
#define PIX_XENUMITEM        0x88
#define PIX_XTYPEDEF         0x89

#define PIX_DEFECT           0x8a
#define PIX_XFIELD           0x8b
#define PIX_XSEMTYPE         0x8c
#define PIX_RTL              0x8d
#define PIX_SPARE_8E         0x8e    // Not used
#define PIX_GROUP            0x8f
#define PIX_SCRATCHGROUP     0x90 

#define PIX_XHARDASSOC	     0x91
#define PIX_XSOFTASSOC	     0x92
#define PIX_XSOFTASSOCTYPE   0x93
#define PIX_XLINKSPEC        0x94
#define PIX_INSTANCE         0x95
#define PIX_STRING           0x96
#define PIX_LOCAL            0x97
#define PIX_SPARE_98         0x98     // Not used.
#define PIX_SET              0x99
#define PIX_SQL_CPLUSPLUS    0x9a 
// 0x9b is "loaded" version
#define PIX_SQL_C            0x9c 
// 0x9d is "loaded" version
#define PIX_DOC_JAVA         0x9e 
// 0x9f is "loaded" version

extern unsigned char dd_icon(ddKind);
extern const unsigned char *get_prefix(symbolPtr);

/*
   START-LOG-------------------------------------------

   $Log: ldrList.h  $
   Revision 1.28 2002/04/11 12:02:56EDT andrey 
   Merge from 'aseticonsReorder' branch.
 * Revision 1.4  1994/07/27  22:37:15  bakshi
 * Bug track: n/a
 * epoch list project
 *
 * Revision 1.3  1994/03/20  23:19:48  boris
 * Bug track: #6721
 * Fixed filter reset on Edit operations and "blinking" problem in viewerShell
 *
 * Revision 1.2  1994/03/02  17:08:34  andrea
 * Bug track: 6528
 * fixed sorting in filter dialog box
 *
 * Revision 1.1  1994/01/21  00:28:58  kws
 * Initial revision
 *
*/

#endif

