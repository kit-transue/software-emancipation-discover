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
#ifndef _instanceBrowser_h
#define _instanceBrowser_h

#ifndef _Relational_h
#include <Relational.h>
#endif

#ifndef _miniBrowser_h
#include <miniBrowser.h>
#endif

class InstanceController;
class InstanceAPI;
RelClass(InstanceRep);

class InstanceRep : public appTree {
  public:
    define_relational(InstanceRep, appTree);
    
    InstanceRep(symbolPtr sym, InstanceController *c, int i);
    ~InstanceRep();
    
    virtual symbolPtr get_associated_symbol(void);
    virtual char const *get_name() const;
    virtual unsigned  char get_icon();
    virtual int       use_own_icon();
    virtual int       use_own_name();
    virtual void      handle_remove();
    virtual void      set_index(int i) { ind = i; };
    virtual int       get_index() { return ind; };
    virtual void      print(ostream& os=cout, int lev=0) const;
  private:
    virtual void       notify(int, Relational*, objPropagator*, RelType*);
    
    InstanceController *controller;
    int                ind;
    genString          label;
};
generate_descriptor(InstanceRep,appTree);

RelClass(InstanceController);

class InstanceController : public miniBrowserRTL_API {
  public:
    InstanceController(symbolArr& inst, char *title, char *full_name = NULL, miniBrowser *browser = NULL);
    virtual ~InstanceController();

    virtual void       selection_callback(RTL* rtl, symbolArr& selected, miniBrowser *browser); 
    virtual int        rtl_dying(RTL *);
    virtual int        use_own_icons();
    virtual int        use_own_names();
    
    virtual void       browse(void);
    virtual symbolPtr  get_symbol(int ind);
    virtual unsigned   char get_sym_icon(int ind);
    virtual char       *get_sym_name(int ind);
    virtual void       handle_remove_rep(int ind);
    virtual void       handle_modification(int ind, InstanceRep *rep);
    virtual Relational *representation_get_real(symbolPtr sym);
    virtual int        representation_get_index(symbolPtr sym);
  private:
    virtual symbolArr& get_representation(void);
    
    symbolArr   representation;
    int         in_destructor;
  protected:
    symbolArr   instances;
    miniBrowser *browser;
    genString   title;
    genString   full_name;
};

RelClass(InstanceAPI);

class InstanceAPI : public miniBrowserRTL_API {
  public: 
    InstanceAPI(symbolArr& domain);
    virtual ~InstanceAPI();

    virtual int        action_callback(RTL* rtl, symbolArr& selected, miniBrowser *browser); 
    virtual int        create_instance_array(symbolPtr selection);
    virtual int        rtl_dying(RTL *);
    InstanceController *create_instance_controller(symbolArr& instances, char *name, char *full_name, symbolPtr selection,
						   miniBrowser *browser);
    virtual void       create_list_name(symbolArr &selection, genString& name);
    virtual void       sort_instances(void);
  private:
    symbolArr domain_selection;
    symbolArr instances;
};

#endif /* _instanceBrowser_h */


