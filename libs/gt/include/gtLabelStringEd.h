#ifndef _gtLabelStringEd_h
#define _gtLabelStringEd_h

// gtLabelStringEd.h
//------------------------------------------
// synopsis:
// ...
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#include <gtStringEd.h>
#include <gtLabel.h>
#include <gtHorzBox.h>

class gtLabeledStringEditor {
  public:
    static gtLabeledStringEditor* create(
	gtBase *p, const char *n, const char *c, const char *l)
    { return new gtLabeledStringEditor(p,n,c,l); }

    ~gtLabeledStringEditor();

    gtStringEditor *editor() { return se; }
    void manage() { hb->manage(); }
    char *text() { return se->text(); }
    void text(const char *c) { se->text(c); }
    void columns(int c) { se->columns(c); }
    void set_sensitive(int s) { se->set_sensitive(s);
			    l->set_sensitive(s); }
    void add_key_navig_group() { se->add_key_navig_group(); }

  protected:
    gtLabeledStringEditor();
    gtLabeledStringEditor(gtBase*, const char*, const char*, const char*);

    long get_last_position() { return se->get_last_position(); }
    void set_selection(long first, long last) { se->set_selection(first, last); }
    void set_callback(gtStringCB callback, void *client_data)
    { se->set_callback(callback, client_data); }
    void editable(int ed) { se->editable(ed); }
    void read_only() { se->read_only(); }
    void rem_key_navig_group() { se->rem_key_navig_group(); }

  private:
    gtStringEditor* se;
    gtLabel* l;
    gtHorzBox* hb;
};

#endif // _gtLabelStringEd_h

/*
   START-LOG-------------------------------------------

 $Log: gtLabelStringEd.h  $
 Revision 1.1 1993/07/28 19:47:43EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:13  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:44  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:43  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:51  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:20  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:12:07  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  08:33:28  wmm
// Another access fix for clean compilation.
// 
// Revision 1.3  92/09/11  08:17:50  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.2  92/09/11  04:07:25  builder
// Add ctor and dtor.
// 
// Revision 1.1  92/08/18  12:24:51  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/
