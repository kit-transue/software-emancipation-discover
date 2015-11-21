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
#ifndef _extract_dialog_h
#define _extract_dialog_h

#ifndef _gt_h
#include <gt.h>
#endif
#ifndef _Relational_h
#include <Relational.h>
#endif


extern "C"
{
    void cancelDecomposition();

    void autosubsys_set_weight(int, int);
    void autosubsys_set_count(int, int);
    void autosubsys_set_threshold(int);
    void autosubsys_set_disjoint(int);
    void autosubsys_set_subsys_count(int);

    int autosubsys_get_weight(int);
    int autosubsys_get_count(int);
    int autosubsys_get_threshold();
    int autosubsys_get_disjoint();
    int autosubsys_get_subsys_count();

    void enable_aus_undo();

    bool aus_weights_read_subsys_setting(const char *filename);
};

/*******************		    ********************/

class weightpair {
  private:
    genString tlable;
    gtScale *s;
    gtLabel *l;
    static gtScale *make_scale(gtBase *parent, const char *name, int top_gap);
    static gtLabel *make_label(gtBase *parent, const char *txt, int top_gap);

  public:
    weightpair() { s=NULL; l=NULL; }
    gtForm *make(gtBase *parent, const char *txt, gtBase* above, int top_gap);
    gtScale *scale() { return s; }
    int value();
    char *name() {  return tlable; }
    void value(int x);
};

class aus_weights {
  protected:
    gtDialogTemplate *shell;

    gtCascadeButton* cascade_button;

    weightpair scales[11];
    gtScale *threshold;

    gtStringEditor *filename_prefix;
    gtStringEditor *logicname_level;

    gtRadioBox *radio;

    gtStringEditor* num_subsys;    

    gtBase *parent;

    gtSeparator *make_separator(gtBase *topwidget);

    void build();

    void load();
    void get();
    void put();

    static void do_apply(gtPushButton *, gtEvent *, void *, gtReason);
    static void do_undo(gtPushButton *, gtEvent *, void *, gtReason);
    static void do_save(gtPushButton *, gtEvent *, void *, gtReason);
    static void do_restore(gtPushButton *, gtEvent *, void *, gtReason);
    static void do_finish(gtPushButton *, gtEvent *, void *, gtReason);

    void save();
    void restore();

    void read_subsys_setting(const char *filename);
    bool get_three_count(int &file_val, int &logic_val, int &num_subs);

  public:
    ~aus_weights();
    void buttonstate(int);
};


#endif // _extract_dialog_h
