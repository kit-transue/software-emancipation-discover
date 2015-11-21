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
#include "galgen.h"

extern char yytext[];
extern int  yyparse();
extern FILE *yyin, *yyout;
extern FILE* fout;
extern FILE* headers;

extern TYPENAME functions[MAX_FUNC_TYPENAME];
extern int functioncount;

extern TYPENAME args[MAX_ARGS_TYPENAME];
extern int argcount;
    
extern char fnname[MAX_FUNC_TYPENAME];
extern char fnbody[MAX_FUNC_TYPENAME];

extern char* service_name;
extern char* exttype;

extern char *Tclnames[MAX_FUNC_TYPENAME];
extern int num_Tclfns;

void init_args () 
{
    int i;

    for (i=0; i<MAX_ARGS_TYPENAME; i++)  {
        args[i].ptr = 0;
        args[i].ref = 0;
        args[i].isstatic = 0;
        args[i].isconst = 0;
        args[i].output = 0;
        args[i].unsign = 0;
    }

    argcount = 0;
    IF_init_args(args);
}

void inc_argcount() { argcount++; }
void set_args_type( int type ) { args[argcount].type = type; }
void set_args_ptr() { args[argcount].ptr++; }
void set_args_ref() { args[argcount].ref++; }
void set_args_isstatic() { args[argcount].isstatic++; }
void set_args_isconst() { args[argcount].isconst++; }
void set_args_output() { args[argcount].output++; }
void set_args_unsign() { args[argcount].unsign++; }
TYPENAME* get_args() { return &args[argcount]; }
void set_args_name( char* name ) 
{ 
  args[argcount].name = strdup(name);
  IF_set_name_pos(argcount);
}
void set_args_typename( char* name ) 
{ 
  args[argcount].typname = strdup(name); 
  IF_set_type_pos(argcount);
}

void  set_service_name( char* name ) 
{
  service_name = strdup(name); 
  IF_set_service_name(service_name); 
}
char* get_service_name() { return service_name; }
void  set_exttype(char* p) {exttype=p;}

void typename_print_converters (char* name)
{
   fprintf (fout, "\nvscrap* marshall_%s(%s& x);\n", name, name);
   fprintf (fout, "void demarshall_%s(%s* x, vscrap* s);\n\n", name, name);
}

void typename_fix (TYPENAME* thing)
{
    if (isfixable(thing->typname)) {
	char buffer[1000];
	strcpy (buffer, "_");
	strcat (buffer, thing->typname);
	strcat (buffer, "ptr");
	thing->ptr--;
	thing->typname = strdup (buffer);
    }
    if ((thing->ptr > 1) || (thing->ref > 1))
	fprintf (stderr, "Warning: too many pointer references - %s\n", thing->name);
}

void typename_printfn (int fn)
{
    int i;

    fprintf (fout, " rcall_");
    typename_print_name (fout, &functions[fn]);
    fprintf (fout, " (Application* app");
    
    for (i=1; i<argcount; i++) {
	fprintf (fout, ", ");
	typename_print_arg (fout, &args[i]);
    }
    fprintf (fout, ")");
}

void typename_copy (TYPENAME* dest, TYPENAME* src)
{
    dest->type = src->type;
    dest->typname = strdup(src->typname);
    dest->name = strdup(src->name);
    dest->ptr = src->ptr;
    dest->ref = src->ref;
    dest->unsign = src->unsign;
    dest->isstatic = src->isstatic;
    dest->isconst = src->isconst;
}

void typename_print_type (FILE* file, TYPENAME* thing)
{
    if (thing->isstatic) fprintf (file, "static ");
    if (thing->isconst) fprintf (file, "const ");
    if (thing->unsign) fprintf (file, "unsigned ");
    fprintf (file, "%s", thing->typname);
}

void typename_print_typeP (FILE* file, TYPENAME* thing)
{
    int i;
    fprintf (file, "%s", thing->typname);
    for (i=0; i<thing->ptr; i++)
	fprintf(file, "P");
}

void typename_print_argdecl (FILE* file, TYPENAME* thing)
{
    fprintf (fout, "    %s", thing->typname);
    fprintf (fout, " %s;\n", thing->name);
}

void typename_print_typeptr (FILE* file, TYPENAME* thing)
{
    int i;
    if (thing->isstatic) fprintf (file, "static ");
    if (thing->isconst) fprintf (file, "const ");
    if (thing->unsign) fprintf (file, "unsigned ");
    fprintf (file, "%s", thing->typname);
    for (i=0; i<thing->ptr; i++)
	fprintf(file, "*");
    for (i=0; i<thing->ref; i++)
	fprintf(file, "&");
}

void typename_print_typeptr_for_retval (FILE* file, TYPENAME* thing)
{
    int i;
    if (thing->isconst) fprintf (file, "const ");
    if (thing->unsign) fprintf (file, "unsigned ");
    fprintf (file, "%s", thing->typname);
    for (i=0; i<thing->ptr; i++)
	fprintf(file, "*");
    for (i=0; i<thing->ref; i++)
	fprintf(file, "&");
}

void typename_print_reference (FILE* file, TYPENAME* thing)
{
    int i;
    for (i=0; i<thing->ptr; i++)
	fprintf(file, "&");
    fprintf (file, "%s", thing->name);
}

void typename_print_ptr (FILE* file, TYPENAME* thing)
{
    int i;
    
    for (i=0; i<thing->ptr; i++)
	fprintf (file, "*");
}

void typename_print_ref (FILE* file, TYPENAME* thing)
{
    int i;
    
    for (i=0; i<thing->ref; i++)
	fprintf (file, "&");
}

void typename_print_name (FILE* file, TYPENAME* thing)
{
    fprintf (file, "%s", thing->name);
}

void typename_print_arg (FILE* file, TYPENAME* thing)
{
    typename_print_type (file, thing);
    typename_print_ptr (file, thing);
    typename_print_ref (file, thing);
    fprintf (file, " ");
    typename_print_name (file, thing);
}

void gen_caller(char* body)
{
    int i;
    int fn;

    fprintf (stderr, "Caller %s\n", args[0].name);
    
    fprintf (fout, "\n\n/*----------------------------- %s ------------------------------*/\n\n", 
	    args[0].name);
    
    fprintf (fout, "#ifdef CLIENT\n");
    
    fn = functioncount;
    typename_copy (&functions[functioncount++], &args[0]);
    
    typename_print_type (fout, &functions[fn]);
    typename_print_ptr (fout, &functions[fn]);
    fprintf (fout," rcall_");
    typename_print_name (fout, &functions[fn]);
    fprintf (fout," (Application* app");
    
    typename_print_type (headers, &functions[fn]);
    typename_print_ptr (headers, &functions[fn]);
    fprintf (headers, " rcall_");
    typename_print_name (headers, &functions[fn]);
    fprintf (headers, " (Application* app");
    
    IF_print_func_def(argcount, "rcall_", "Application*", "app");

    for (i=1; i<argcount; i++) {
	fprintf (fout,", ");
	typename_print_arg (fout, &args[i]);
	
	fprintf (headers, ", ");
	typename_print_arg (headers, &args[i]);
    }
    
    fprintf (headers, ");\n");
    fprintf (fout,")\n{\n");
    fprintf (fout,"    %s_app = app;\n", service_name);
    
    if ((strcmp(args[0].typname,"void") != 0) &&
        (strcmp(args[0].typname,"async") != 0)) {
	fprintf (fout,"    ");
	typename_print_type (fout, &args[0]);
	fprintf (fout," return_val;\n");
    }
    fprintf (fout,"    if (app && !app->mySession()) \n");
    fprintf (fout,"        app->connect (0, (char*)app->getName());\n");
    fprintf (fout,"    if (!app || !app->mySession()) {\n");
    fprintf (fout,"        if (Application::traceMode)\n");
    fprintf (fout,"                fprintf (stderr, \"Cannot call %s in %s - not connected\\n\");\n", args[0].name, service_name);
    if ((strcmp(args[0].typname,"void") == 0) ||
        (strcmp(args[0].typname,"async") == 0)) 
	fprintf (fout,"        return;\n");
    else
	fprintf (fout,"        return 0;\n");
    
    fprintf (fout,"    } else {\n");
    fprintf (fout,"        if (Application::traceMode)\n");
    fprintf (fout,"            printf (\"+++>");
    typename_printfn (fn);
    fprintf (fout,"\\n\");\n");
    fprintf (fout,"        Application::IncrementCount();\n");
    fprintf (fout,"        vsessionStatement *statement = new vsessionStatement;\n");
    fprintf (fout,"        statement->SetSession(app->mySession());\n");
    fprintf (fout,"        statement->SetSignature(app->getSignature());\n");
    fprintf (fout,"        vscrap *real_result=vscrapCreateArray();\n");
    fprintf (fout,"        int array_index=0;\n");
    fprintf (fout,"        int array_index2=0;\n");
    fprintf (fout,"        g_push(marshall__charptr(\"%s\"));\n", args[0].name);
    
    for (i=1; i<argcount; i++) {   
	if (args[i].output) continue;

	fprintf (fout,"          g_push(marshall_");
	typename_print_type(fout, &args[i]);
	fprintf (fout,"(");
	typename_print_ptr (fout, &args[i]);
	fprintf (fout,"%s));\n", args[i].name);
    }
    
    fprintf (fout,"        statement->SetArgs(statement,real_result);\n");
    fprintf (fout,"        if (real_result)delete real_result;\n");
    
    if (strcmp(args[0].typname, "async") != 0)
	fprintf (fout,"        statement->SetObserveStatementProc(std_notify);\n");
    else {
        fprintf (fout,"        statement->SetObserveStatementProc(std_async_notify);\n");
	fprintf (fout,"        statement->SetDestroyWhenFinished(TRUE);\n");
    }
    fprintf (fout,"        statement->Send();\n");
    fprintf (fout,"        answer_scrap = NULL;\n");
    
    if (strcmp(args[0].typname, "async") != 0){
	fprintf (fout,"        Block(statement);\n");
	fprintf (fout,"        vscrap *sarg = (vscrap *)statement->GetData();\n");
	fprintf (fout,"        statement->DeleteLater();\n"); 
	fprintf (fout,"        array_index  = 0;\n");
    } else 
	fprintf (fout,"        vscrap * sarg=answer_scrap;\n");

    fprintf (fout,"\n");
    for (i=1; i<argcount; i++) {
        if (args[i].output) {
	    fprintf (fout,"        demarshall_");
	    typename_print_type(fout, &args[i]);
	    fprintf (fout,"(&%s, g_take());\n", args[i].name);
        }
    }
    fprintf (fout,"\n");


    if ((strcmp(args[0].typname, "async") != 0) &&
        (strcmp(args[0].typname, "void") != 0)) {

	fprintf (fout,"        demarshall_%s (&return_val, g_take());\n", args[0].typname);
        fprintf (fout,"        Application::DecrementCount();\n\n");
	fprintf (fout,"        if (sarg){ sarg->DeleteLater(); }\n");
        fprintf (fout,"        if (Application::traceMode)\n");
        fprintf (fout,"            printf (\"--->");
        typename_printfn (fn);
        fprintf (fout,"\\n\");\n");
	fprintf (fout,"        return return_val;\n");
    }
    else if( strcmp(args[0].typname, "void") == 0 )
    {
	fprintf (fout,"        Application::DecrementCount();\n");
        fprintf (fout,"        if (Application::traceMode)\n");
        fprintf (fout,"            printf (\"--->");
        typename_printfn (fn);
        fprintf (fout,"\\n\");\n");
        
        fprintf (fout,"        return;\n");
    }
    fprintf (fout,"    }\n");
    fprintf (fout,"}\n#endif /* CLIENT */\n");
    
}

void gen_callee(char* body)
{
    int i;
    int fn = functioncount - 1; 

    fprintf (fout,"\n#ifdef SERVER\n");
    fprintf (fout,exttype);
    
    typename_print_typeptr(fout, &args[0]);
    fprintf (fout," %s (", args[0].name);
    
    typename_print_type (headers, &args[0]);
    typename_print_ptr (headers, &args[0]);
    fprintf (headers, " ");
    typename_print_name (headers, &args[0]);
    fprintf (headers, " (");
    
    for (i=1; i<argcount; i++) {
	if (i>1) fprintf (fout,", ");
	typename_print_arg (fout, &args[i]);
	
	if (i>1) fprintf (headers, ", ");
	typename_print_arg (headers, &args[i]);
    }
    
    fprintf (headers, ");\n"); 
    
    if (body) 
	fprintf (fout,")\n%s", body);
    else
	fprintf (fout,")\n{\n}\n");
    
    
    fprintf (fout,"\n\n");
    if (args[0].isstatic) fprintf (fout,"static ");
    fprintf (fout,"void %s_fn (vscrap* sarg, vscrap* real_result, int& array_index, int& array_index2)\n", args[0].name);
    fprintf (fout,"{\n");
    
    for (i=1; i<argcount; i++) {
	typename_print_argdecl (fout, &args[i]);
    }
    fprintf (fout,"\n");
    fprintf (fout,"    if (Application::traceMode)\n");
    fprintf (fout,"        printf (\"+++<");
    typename_printfn (fn);
    fprintf (fout,"\\n\");\n");
    for (i=1; i<argcount; i++) {
	if (!args[i].output) 
	    fprintf (fout, "    demarshall_%s (&%s, g_take());\n", args[i].typname, args[i].name); 
    }
    
    fprintf (fout,"\n    ");
    if ((strcmp(args[0].typname, "async") != 0) &&
        (strcmp(args[0].typname, "void") != 0)) {
	typename_print_typeptr_for_retval (fout, &args[0]);
	fprintf (fout," return_val = ");
    }
    
    fprintf (fout,"%s (", args[0].name);
    for (i=1; i<argcount; i++) {
	if (i>1) fprintf (fout,",");
	typename_print_reference (fout, &args[i]);
    }
    fprintf (fout,");\n");
    
    fprintf (fout,"\n");
    for (i=1; i<argcount; i++) {
	if (args[i].output) {
	    fprintf (fout,"    g_push(marshall_");
	    typename_print_type(fout, &args[i]);
	    fprintf (fout,"(");
	    typename_print_ptr (fout, &args[i]);
	    fprintf (fout,"%s));\n", args[i].name);
	}
    }
    fprintf (fout,"\n");

    fprintf (fout,"    if (Application::traceMode)\n");
    fprintf (fout,"        printf (\"---<");
    typename_printfn (fn);
    fprintf (fout,"\\n\");\n");
    if ((strcmp(args[0].typname, "async") != 0) &&
        (strcmp(args[0].typname, "void") != 0)) {
	fprintf (fout,"    g_push(marshall_");
        typename_print_typeP (fout, &args[0]);
	fprintf (fout,"(return_val));\n");
    }
    fprintf (fout,"}\n");
    
    fprintf (fout,"\n#endif /* SERVER */\n");
}

void gen_Tcl (char* body)
{
    fprintf (stderr, "Tcl %s\n", args[0].name);
    
    Tclnames[num_Tclfns++] = strdup (args[0].name);
    
    fprintf (fout,"\n\n/*----------------------------- %s ------------------------------*/\n\n", args[0].name);
    fprintf (fout,"#ifdef SERVER\n");
    
    fprintf (fout,exttype);
    fprintf (fout,"int %s (ClientData, Tcl_Interp* interp, int argc, char *argv[]", 
	    args[0].name);
    
    if (body) 
	fprintf (fout,")\n%s", body);
    else
	fprintf (fout,")\n{\n}\n");
    
    fprintf (fout,"\n#endif /* SERVER */\n");
}

