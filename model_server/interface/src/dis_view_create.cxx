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
#ifndef NEW_UI
#include <view_creation.h>

extern view*   view_create_noselect(app *);
extern int     create_view_of_def(symbolPtr);

#else
#include "representation.h"       /*only for NEW_UI*/
#include "xref.h"
#include "smt.h"
#include "ddict.h"
#include "assocbridge.h"
#include "viewTypes.h"
#include "ide_editor.h"
#endif


#include <genError.h>
#include <dis_view_create.h>

#include "symbolPtr.h"
#include "messages.h"
#include "machdep.h"

class view;
class app;
class symbol;

#ifdef NEW_UI
#include "ide_editor.h"
static int dis_new_view_create( symbolPtr );
static appPtr proj_get_raw_app(const char* f_name);
extern int dis_report_view_create( appPtr, appTreePtr, repType );
extern appTreePtr dd_get_loaded_definition (symbolPtr);
extern int get_file_fn(const symbolPtr &, genString &);
extern int is_external( char* );
#endif /*NEW_UI*/

class dataCell;
dataCell *dataCell_of_symbol(const symbolPtr &symbol, bool load);
void dataCell_offload(dataCell *&cell, bool mode);
const symbolPtr &dataCell_get_symbol(dataCell *cell);

view *dis_view_create(symbolPtr sym, repType rep, int i) 
{
    Initialize (dis_view_create);

#ifndef NEW_UI
    viewPtr retVal = view_create (sym, rep, i);
    view_create_flush();
    return retVal;
#else
    return NULL;
#endif
}

view *dis_view_create(app *app_head)
{
    Initialize(dis_view_create);

#ifndef NEW_UI
    viewPtr retVal = view_create (app_head);
    view_create_flush();
    return retVal;
#else
    return NULL;
#endif
}

view *dis_view_create (const char *str)
{
    Initialize(dis_view_create);

#ifndef NEW_UI
    viewPtr retVal = view_create (str);
    view_create_flush();
    return retVal;
#else
    return NULL;
#endif
}

view *dis_view_create_noselect (app* app)
{
    Initialize(dis_view_create_noselect);

#ifndef NEW_UI
    viewPtr retVal = view_create_noselect (app);
    view_create_flush();
    return retVal;
#else
    return NULL;
#endif
}

view *dis_view_create_noselect (const char *str)
{
    Initialize (dis_view_create_noselect);

#ifndef NEW_UI
    viewPtr retVal = view_create_noselect (str);
    view_create_flush();
    return retVal;
#else
    return NULL;
#endif
}

void dis_view_create_flush ()
{
    Initialize (dis_view_create_flush);

#ifndef NEW_UI
    view_create_flush();
#endif
}

int dis_create_view_of_def (symbolPtr sym)
{
    Initialize (dis_create_view_of_def);

#ifndef NEW_UI
    return create_view_of_def (sym);
#else
    return dis_new_view_create( sym );
#endif
}

#ifdef NEW_UI

int
dis_focus_node( Relational * obj )
{
    Initialize(dis_focus_node);

    int ret_val = -1;

    if (obj) {
	if (is_smtTree(obj)) {
	    smtTreePtr smt = checked_cast(smtTree,obj);
	    appPtr     hdr = checked_cast(app,smt->get_header());
	    if (hdr) {
		ret_val = dis_report_view_create(hdr, smt, Rep_UNKNOWN);
	    }
	}
    }

    return ret_val;
}

const char *ATT_cname( symbolPtr& sym);
const char *ATT_physical_filename( symbolPtr& sym);
int   astNode_get_line_number(const symbolPtr& sym);
int   say_selection_arguments(appPtr app_head, appTreePtr app_node, repType rep_type, int tabs, char* fname, int* lin, int* col, char* tok);

#ifdef _WIN32


//---------------------------------------------------------------------------------------
int dis_transform_instance( symbolPtr sym, int tabs, char* fname, int* lin, int* col, char* tok  ) {
int i;
    Initialize(dis_focus_instance);

    int ret_val = -1;

    if (sym.isnotnull()) {
	    // -- extract filename in which the instance lives
	    symbolPtr scope = sym.scope();
	    if (scope.isnotnull()) {
		    scope = scope.get_xrefSymbol();
		    if (scope.isnotnull()) {
		        if (scope->get_kind() != DD_MODULE)
			       scope = scope->get_def_file();
			}
	    }
	    // -- focus instance
	    if (scope.isnotnull()) {
		   genString file;
		   if (get_file_fn(scope, file) > 0) {
	           memmove(fname,file,file.length());
			   fname[file.length()]=0;
	           *lin = sym.line_number(); // To prevent erroneous optimization from MSVC
			   *col = 0;
			   tok[0]=0;
		       ret_val = 0;
		   }
		}
	}
    return ret_val;
}
//---------------------------------------------------------------------------------------


int
dis_transform_to_MSDEV( symbolPtr sym, int tabs, char* fname, int* lin, int* col, char* tok ) {
    Initialize(dis_new_view_create);

   


    int nResult = -1;

    if( sym.isnotnull() ) {


		if(sym.is_instance()) return dis_transform_instance(sym,tabs,fname,lin,col,tok);


	    appPtr    head = (appPtr)0;
	    appTree*  node = (appTree *)0;

        ddKind    kind = sym.get_kind();
        Relational *ob = sym;
        appTree *nd    = (appTree *)ob;
        

	if(kind == DD_AST){
	  const char *filen     = ATT_physical_filename(sym);
	  int line_number = astNode_get_line_number(sym);
	  *lin = line_number;
	  *col=1;
	  *tok=0;
	  memmove(fname,filen,strlen(filen));
	  fname[strlen(filen)]=0;
	  return 0;
	}
	if(ob && is_smtTree(ob))
	  return dis_focus_node(ob);

        if (kind == DD_MODULE) 
        {
          genString phnm;
	  if (sym.relationalp()) {
	      Relational * relp = sym;
	      if (relp) {
		  if (is_projModule(relp)) {
		      (checked_cast(projModule,relp))->get_phys_filename(phnm);
		  }
	      }
          }
          if (phnm.length() <= 0)
            projHeader::ln_to_fn(sym.get_name(), phnm);
        }

	// -- get module app, and the corresponding smt
	
	if( nResult < 0 )
        {
            if (kind == DD_MODULE) {
	        head = sym.get_def_app();
	    } else {
	        ddElement * ddel = sym.get_def_dd();
	        if (ddel) {
		    head = ddel->get_main_header();
		    node = dd_get_smt(ddel);
		    if (!node) {
		      int dd_line = ddel->xref_index;
		      if (dd_line > 0 ) {
			if (head && is_smtHeader(head)) {
			  smtHeader* h = checked_cast(smtHeader, head);
			  node = h->tree_at_line (dd_line);
			}
		      }
		    }
	        } else if (sym.xrisnotnull()) {
		    symbolPtr symf = sym->get_def_file();
		    if (symf.isnotnull())
		        head = symf.get_def_app();
	        }
	    }

	    // -- open view
	    if (head)
			nResult = say_selection_arguments(head, node, Rep_UNKNOWN,tabs,fname,lin,col,tok);
	    else 
            {
                genString phnm;
                if (sym.relationalp()) 
                {
		  Relational * relp = sym;
		  if (relp) {
		      if (is_projModule(relp)) {
			  (checked_cast(projModule,relp))->get_phys_filename(phnm);
		      }
		  }
		}

	        // -- try global search through all possible projects
	        if (phnm.length() <= 0)
		    projHeader::ln_to_fn(sym.get_name(), phnm);
	        // -- OPEN
	        if (nResult<0 && phnm.length() > 0) {
	           *lin=1;
	           *col=1;
	           *tok=0;
	           memmove(fname,phnm,strlen(phnm));
   	           nResult = 0;
	        }
	    }
        }
    }

    return nResult;
}
#endif


extern bool is_els_non_ascii_suffix (const char *full_path);

int
dis_new_view_create(symbolPtr sym)
{
  Initialize(dis_new_view_create(symbolPtr));
  int nResult = -1;

  if (sym.isnotnull()) {
    appPtr head = (appPtr) 0;
    appTree *node = (appTree *) 0;

    ddKind kind = sym.get_kind();
    Relational *ob = sym;
    appTree *nd = (appTree *) ob;        

    if (kind == DD_AST) {
      genString fname = ATT_physical_filename(sym);
      int line_number = astNode_get_line_number(sym);
      dis_integration_open_file((char *) fname, line_number);
      return 0;
    }

    if (kind == DD_DATA) {
      dataCell *cell = dataCell_of_symbol(sym, false);

      if (!cell)
	return 0;

      symbolPtr &symbol = (symbolPtr &)dataCell_get_symbol(cell);
      symbolPtr node = symbol.get_astNode();
      dataCell_offload(cell, false);

      return
	dis_new_view_create(node);
    }

    if (ob && is_smtTree(ob))
      return
	dis_focus_node(ob);

    if (kind == DD_MODULE) {
      genString phnm;

      if (sym.relationalp()) {
	Relational *relp = sym;

	if (relp) {
	  if (is_projModule(relp)) {
	    (checked_cast(projModule,relp))->get_phys_filename(phnm);
	  }
	}
      }

      if (phnm.length() <= 0)
	projHeader::ln_to_fn(sym.get_name(), phnm);
    }

    // -- get module app, and the corresponding smt
	
    if (nResult < 0) {
      if (kind == DD_MODULE)
	head = sym.get_def_app();
      else {
	ddElement *ddel = sym.get_def_dd();

	if (ddel) {
	  head = ddel->get_main_header();
	  node = dd_get_smt(ddel);
	  if (!node) {
	    int dd_line = ddel->xref_index;
	    if (dd_line > 0 ) {
	      if (head && is_smtHeader(head)) {
		smtHeader* h = checked_cast(smtHeader, head);
		node = h->tree_at_line (dd_line);
	      }
	    }
	  }
	} else if (sym.xrisnotnull()) {
	  symbolPtr symf = sym->get_def_file();

	  if (symf.isnotnull())
	    head = symf.get_def_app();
	}
      }

      // -- open view
      if (head)
	nResult = dis_report_view_create(head, node, Rep_UNKNOWN);
      else {
	genString phnm;

	if (sym.relationalp()) {
	  Relational *relp = sym;

	  if (relp) {
	    if (is_projModule(relp))
	      (checked_cast(projModule,relp))->get_phys_filename(phnm);
	  }
	}

	// -- try global search through all possible projects
	if (phnm.length() <= 0)
	  projHeader::ln_to_fn(sym.get_name(), phnm);

	// -- OPEN
	if (nResult<0 && phnm.length() > 0 ) {
	  if (is_els_non_ascii_suffix(phnm))
	    dis_message ("Viewer.Help.OpenDefinition",  MSG_ERROR, "M_NOTVIEWABLEFILE", realOSPATH(phnm) );
	  else
	    dis_integration_open_file(phnm);
	  nResult = 0;
	}
      }
    }
  }

  return nResult;
}


int
dis_focus_instance( symbolPtr sym ) {
int i;
    Initialize(dis_focus_instance);

    int ret_val = -1;

    if (sym.isnotnull()) {
	if (sym.is_instance()) {
	    // -- extract filename in which the instance lives
	    symbolPtr scope = sym.scope();
	    if (scope.isnotnull()) {
		scope = scope.get_xrefSymbol();
		if (scope.isnotnull()) {
		    if (scope->get_kind() != DD_MODULE)
			scope = scope->get_def_file();
		}
	    }
	    // -- focus instance
	    if (scope.isnotnull()) {
		genString fname;
		if (get_file_fn(scope, fname) > 0) {
	          int line_number = sym.line_number(); // To prevent erroneous optimization from MSVC
		  int line_offset = sym.line_offset();
		  const char* token     = ATT_cname(sym);
                  dis_integration_open_file ((char *)fname,line_number );

		  // We will need it to calculate 
		  // position from offset (calculating TABS)
		  if(line_offset>=0) {
                     dis_integration_read_selection(0);
		     char* line =  dis_integration_getsel_token();
                     int tabsize= dis_integration_getsel_tabsize();
		     int pos=0;
		     for(i=0;i<line_offset;i++) {
		       if(line[i]==0)    break;
		       if(line[i]=='\t') pos+=tabsize;
		       else              pos++;
		    }
		    if ( strncmp(&(line[i]), token, strlen(token)) && line[i] == '"')
		    {
		      int npos = pos +1;
		      while ( line[++i] )
		      {
			if ( line[i] == ' ' ) npos++;
			else if ( line[i] == '\t' ) npos+=tabsize;
			else break;
		      }
		      if ( strncmp( &(line[i]), token, strlen(token) ) == 0 )
			pos = npos;
		    }
		    dis_integration_make_selection((char *)fname,line_number,pos,token);
		}
		ret_val = 0;
	      }
	    }
	}
 }

 return ret_val;
}

extern void open_view (viewtype, symbolArr&);
void dis_open_view(symbolPtr sym, const char* mode)
{
	int arglen = 0;
	if (mode)
		arglen = strlen(mode);
	if (arglen < 3)
	{
		mode = "-text";
		arglen = strlen(mode);
	}

	if (strncmp(mode, "-text", arglen) == 0 || strncmp(mode, "-definition", arglen) == 0) 
    {
            int vws_val = -1;
        
            if (sym.is_instance()) 
            {
			    if (dis_focus_instance(sym) < 0)
				    sym = sym.sym_of_instance();
                else
                    vws_val = 0;
            } 
            else if(sym.relationalp() && sym.get_kind() != DD_MODULE) 
            {
			     Relational *ob = sym;
			     if(ob!=0 && is_appTree(ob))
                 {
				     appTree * node = (appTree *)ob;
			         vws_val = dis_create_view_of_def(node);
			     }
            }
            if (vws_val < 0)
				vws_val = dis_create_view_of_def(sym);

	    if( vws_val < 0 )
			dis_message ("Viewer.Help.OpenDefinition",  MSG_ERROR, "M_COULDNOTOPENDEF" );  
    } 
	else 
	{
		viewtype type;
        if (strncmp(mode, "-outline", arglen) == 0) type = viewOutline;
        else if (strncmp(mode, "-flowchart", arglen) == 0) type = viewFlowchart;
        else if (strncmp(mode, "-dependency", arglen) == 0) type = viewCallTree;
        else if (strncmp(mode, "-inheritance", arglen) == 0) type = viewInheritance;
        else if (strncmp(mode, "-relations", arglen) == 0) type = viewERD;
        else if (strncmp(mode, "-datachart", arglen) == 0) type = viewDatachart;
        else if (strncmp(mode, "-subsystems", arglen) == 0) type = viewSubsysBrowser;
   	    else return;

	    if(sym.relationalp() && sym.get_kind() != DD_MODULE) 
		{
			Relational *ob = sym;
			if(ob!=0 && is_appTree(ob))
			{
			    appTree * node = (appTree *)ob;
			    appTree * prev = node;
			    while(node->get_parent())
				{
					prev = (appTree *)node;
					node = (appTree *)node->get_parent();
			    }
			    sym = prev;
				symbolArr syms;
				syms.insert_last(sym);
			    open_view(type, syms);
			}
		} 
		else
		{
			symbolArr syms;
			syms.insert_last(sym);
			open_view(type, syms);
		}
	}
}


#endif /*NEW_UI*/ 
