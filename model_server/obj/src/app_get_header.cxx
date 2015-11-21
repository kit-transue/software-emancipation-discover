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
// app_get_header.C
//
//#include <string.h>
#include <cLibraryFunctions.h>

#include "objArr.h"
#include "objOper.h"     


// Returns header given app type and name
//
// static class function

appPtr app::get_header(appType type, char const *name)
{
    Initialize(app::get_header);

    Obj* al = get_app_list();

    if (name != NULL)
    {
	Obj* el;
	ForEach(el, *al)
	{
	    appPtr ap = checked_cast(app,el);
	    if(type == ap->get_type())
	    {
		char const *app_name = ap->get_name();
		if(app_name && !strcmp(app_name, name))
		    ReturnValue(ap);
	    }
	}
    }

    ReturnValue(NULL);
}

appPtr app::find_from_phys_name(char const *phys_name)
{

    Initialize(app_from_phys_name);
    if (phys_name== NULL) return NULL;

    app *app_header = NULL;

    Obj *al = app::get_app_list();


	Obj* el;
	ForEach(el, *al)
	{
	    appPtr ap = checked_cast(app,el);
	    char const *app_name = ap->get_phys_name();
            if(!app_name)
	      app_name = ap->get_name();

	    if(app_name && !strcmp(app_name, phys_name)){
               app_header = ap;
               break;
            }
	}
    return app_header;
}

boolean app_needs_to_be_saved(app *);

boolean app::needs_to_be_saved()
{
  return app_needs_to_be_saved(this);
}

boolean app::is_scratch() {
   return false;
}

// Return a NULL-terminated array of pointers to the app's that are modified,
// or NULL if there are no modified app's.
// NOTE: Caller is responsible for deleting the array.
//
// static class function

objArr* app::get_modified_headers()
{
    Initialize(app::get_modified_headers);

    Obj *al = get_app_list();
    objArr *array = new objArr;

    ObjPtr elem;
    ForEach(elem, *al)
    {
	appPtr ah = checked_cast(app,elem);
	if (ah->needs_to_be_saved())
	{
	    array->insert_last(ah);
	}
    }

    ReturnValue(array);
}

