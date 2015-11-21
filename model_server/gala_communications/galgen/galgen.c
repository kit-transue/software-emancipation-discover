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

/* extern char yytext[]; */
FILE* fout;
FILE* headers;

TYPENAME functions[1000];
int functioncount;

TYPENAME args[100];
int argcount;
    
char fnname[1000];
char fnbody[1000];

char *temp1, *temp2;
char* service_name;
char* exttype;

char *Tclnames[1000];
int num_Tclfns = 0;

int ret_status = 0;
main(int argc, char* argv[] )
{
    int i;
    int iin=0, iout=0, ih=0, iif=0;
    FILE *fin=0;

//    _asm int 3;
    fout = 0;

    for ( i=1; i < argc; i++ )
    {
      if ( stricmp(argv[i], "-in") == 0 )
      {
	if ( ++i < argc && *(argv[i]) != '-')
	  iin=i;
	else
	  fprintf (stderr, "Warning: unknown file name for option %s\n", argv[--i] );
      }
      else if ( stricmp(argv[i], "-out") == 0 )
      {
	if ( ++i < argc && *(argv[i]) != '-' )
	  iout=i;
	else
	  fprintf (stderr, "Warning: unknown file name for option %s\n", argv[--i] );
      }
      else if ( strcmp(argv[i], "-h") == 0 )
      {
	if ( ++i < argc && *(argv[i]) != '-' )
	  ih=i;
	else
	  fprintf (stderr, "Warning: unknown file name for option %s\n", argv[--i] );
      }
      else if ( strcmp(argv[i], "-if") == 0 )
      {
	if ( ++i < argc && *(argv[i]) != '-' )
	  iif=i;
	else
	  fprintf (stderr, "Warning: unknown file name for option %s\n", argv[--i] );
      }
      else
	fprintf (stderr, "Warning: unknown option %s\n", argv[i] );
    }

    if ( iin > 1 )
    {
     fin = fopen ( argv[iin], "r" );
     if ( fin )
       set_in(fin);
    }

    if ( iout > 1 )
     fout = fopen ( argv[iout], "w" );
    if ( !fout )
     fout= stdout;

    if ( ih > 1 )
     headers = fopen ( argv[ih], "w" );
    else
     headers = fopen ("interface.h", "w");

    IF_init_all(iin, iif, argv);

    maketype("char",0);
    maketype("char",1);
    maketype("short",0);
    maketype("int",0);
    maketype("long",0);
    maketype("float",0);
    maketype("double",0);
    maketype("vstr",0);
    maketype("vstr",1);
    maketype("async",0);
    maketype("void",0);
    
    fprintf (headers, "class Application;\n");
    
    fprintf (fout,"#include <vport.h>\n");
    fprintf (fout," \n");
    fprintf (fout,"#include vstartupHEADER\n");
    fprintf (fout,"#include vstdlibHEADER\n");
    fprintf (fout,"#include vsessionHEADER\n");
    fprintf (fout,"#include vserviceHEADER\n");
    fprintf (fout,"#include vscrapHEADER\n");
    fprintf (fout,"#include vdasservHEADER\n");
    fprintf (fout,"#include vstdioHEADER\n");
    fprintf (fout,"\n");

    fprintf (fout,"#include <signal.h>\n");
    fprintf (fout,"#include <iostream.h>\n");
    /* fprintf (fout,"#ifndef _WIN32\n"); */
    fprintf (fout,"#include \"Application.h\"\n");
    /* fprintf (fout,"#else\n");
    fprintf (fout,"#include \"../communications/include/Application.h\"\n");
    fprintf (fout,"#endif\n"); */
    fprintf (headers, "#include \"Application.h\"\n");
    fprintf (fout,"\n");

    fprintf (fout,"#ifdef SERVER\n");
    fprintf (fout,"#include <tcl.h>\n");
    fprintf (fout,"#endif /* SERVER */\n");
    fprintf (fout," \n");

    fprintf (fout,"%s\n", "\
#ifndef _WIN32\n\
#ifndef SIG_TYP\n\
#ifdef sun4\n\
typedef void (*SIGHANDLER)(int,...);\n\
#else\n\
typedef void (*SIGHANDLER)(int);\n\
#endif\n\
#define SIG_TYP SIGHANDLER\n\
#endif\n\
#endif\n\
\n\
static int Block(vsessionStatement *statement)\n\
{\n\
#ifndef _WIN32\n\
    SIG_TYP old = signal(SIGALRM,SIG_IGN);\n\
#endif\n\
    int retval = statement->Block(NULL);\n\
#ifndef _WIN32\n\
    signal(SIGALRM,old);\n\
#endif\n\
    return retval;\n\
}\n\
"   );
    fprintf (fout,"typedef void async;\n");
    fprintf (fout,"typedef char* _charptr;\n");
    fprintf (fout,"typedef vstr* _vstrptr;\n");
    fprintf (headers, "typedef void async;\n");
    fprintf (headers, "typedef char* _charptr;\n");
    fprintf (headers, "typedef vstr* _vstrptr;\n");
    fprintf (fout,"\n");
    fprintf (fout,"#define g_push(s) {vscrapSetArrayCount(real_result,array_index2+1);vscrapSetElementAt(real_result,array_index2,s);array_index2++;}\n");
    fprintf (fout,"#define g_take() sarg? vscrapGetElementAt(sarg,array_index++): 0\n");
    fprintf (fout," \n");
    
    fprintf (fout,"static vscrap*  marshall_char(char x){return vscrapFromCharacter(x);}\n");
    fprintf (fout,"static vscrap*  marshall_short(short x){return vscrapFromInteger((int)x);}\n");
    fprintf (fout,"static vscrap*  marshall_int(int x){return vscrapFromInteger(x);}\n");
    fprintf (fout,"static vscrap*  marshall_float(float x){return vscrapFromFloat(x);}\n");
    fprintf (fout,"static vscrap*  marshall_double(double x){return vscrapFromFloat(x);}\n");
    fprintf (fout,"static vscrap*  marshall__charptr(_charptr x){if (!x)x=\"\";vstr*_x=vstrClone((vchar*)x);vscrap* s=vscrapFromString(_x);vstrDestroy(_x);return s;}\n");
    fprintf (fout,"static vscrap*  marshall__vstrptr(_vstrptr x){if (!x)x=(_vstrptr)\"\";return vscrapFromString(x);}\n");
    fprintf (fout," \n");
    
    fprintf (fout,"static void demarshall_char(char* x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        *x = vscrapGetCharacter(s);delete s;\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = '\\0'; \n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");
    fprintf (fout," \n");

    fprintf (fout,"static void demarshall_short(short* x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        *x = vscrapGetInteger(s);delete s;\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = 0; \n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");
    fprintf (fout," \n");

    fprintf (fout,"static void demarshall_int(int* x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        *x = vscrapGetInteger(s);delete s;\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = 0; \n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");
    fprintf (fout," \n");

    fprintf (fout,"static void demarshall_float(float* x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        *x = vscrapGetFloat(s);delete s;\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = 0.0; \n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");

    fprintf (fout,"static void demarshall_double(double* x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        *x = vscrapGetFloat(s);delete s;\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = 0.0; \n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");
    fprintf (fout," \n");

    fprintf (fout,"extern \"C\" char* strdup(const char*);\n");
    fprintf (fout,"static void demarshall__charptr(char** x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        vstr*y = vstrCloneScribed(vscrapScribeString(s));delete s;\n");
    fprintf (fout,"        *x = strdup((char*)y);\n");
    fprintf (fout,"        vstrDestroy(y);\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = 0;\n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");
    fprintf (fout," \n");


    fprintf (fout,"static void demarshall__vstrptr(vstr** x, vscrap* s) \n");
    fprintf (fout,"{ \n");
    fprintf (fout,"    if (s) { \n");
    fprintf (fout,"        *x = vstrCloneScribed(vscrapScribeString(s));delete s;\n");
    fprintf (fout,"    } else { \n");
    fprintf (fout,"        *x = 0;\n");
    fprintf (fout,"    } \n");
    fprintf (fout,"} \n");
    fprintf (fout," \n");

    fprintf (fout,"static vscrap *answer_scrap;\n");
    fprintf (fout,"\n\n");

    fprintf (fout,"%s\n", "static void errmsg(char*msg){cerr << msg << endl;}");

    fprintf (fout,"static void std_notify (vsessionStatement* statement, vsessionStatementEvent* event)\n");
    fprintf (fout,"{\n");
    fprintf (fout,"    vscrap *sync_answer_scrap = NULL;\n");
    fprintf (fout,"\n");
    fprintf (fout,"    switch (vsessionStatement::GetEventCode (event)) {\n");
    fprintf (fout,"        case vsessionStatementEVENT_COMPLETED:\n");
    fprintf (fout,"	    statement->GetReturnValue(statement, &sync_answer_scrap);\n");
    fprintf (fout,"	    statement->SetData(sync_answer_scrap);\n");
    fprintf (fout,"	    break;\n");
    fprintf (fout,"	case vsessionStatementEVENT_CANNOT_SEND:\n");
    fprintf (fout,"	    errmsg(\"*** cannot send!\\n\");\n");
    fprintf (fout,"	    Application::SetCannotSend(vTRUE);\n");
    fprintf (fout,"	    break;\n");
    fprintf (fout,"	case vsessionStatementEVENT_END_SESSION:\n");
    fprintf (fout,"	    errmsg(\"*** end session!\\n\");\n");
    fprintf (fout,"	    break;\n");
    fprintf (fout,"	case vsessionStatementEVENT_FAILED:\n");
    fprintf (fout,"	    errmsg(\"*** failed!\\n\");\n");
    fprintf (fout,"	    break;\n");
    fprintf (fout,"    }\n");
    fprintf (fout,"}\n\n");

    fprintf (fout,"static void std_async_notify (vsessionStatement* statement, vsessionStatementEvent* event)\n");
    fprintf (fout,"{\n\n");
    fprintf (fout,"    Application::DecrementCount();\n");
    fprintf (fout,"}\n\n");

    while (do_parse());


/* clean up 'exttype'. */
    exttype = "";
    init_args();

/*  Generate the eval function. */

    sprintf (fnname, "dis_servereval");
    sprintf (fnbody, "{\n\
    Application* myapp = Application::findApplication (service);\n\
    static vstr* myresult = NULL;\n\
    if (!myapp) \n\
	myapp = %s_app;\n\
    int e = myapp->eval((vchar*)str);\n\
    myresult = NULL;\n\
    myresult = myapp->getEvalResult();\n\
    return myresult;\n\
}\n", service_name);

    argcount = 3;
    args[0].typname = "_vstrptr";
    args[0].name = fnname;
    args[0].isstatic = 1;

    args[1].typname = "_vstrptr";
    args[1].name = "service";
    
    args[2].typname = "_vstrptr";
    args[2].name = "str";
    
    
    gen_caller ("");
    gen_callee (fnbody);

/*  Generate the servereval_async function */

    init_args();

    sprintf (fnname, "dis_servereval_async");
    sprintf (fnbody, "{\n\
    Application* myapp = Application::findApplication (service);\n\
    if (!myapp) \n\
	myapp = %s_app;\n\
    int e = myapp->eval((vchar*)str);\n\
    return;\n\
}\n", service_name);

    argcount = 3;
    args[0].typname = "async";
    args[0].name = fnname;
    args[0].isstatic = 1;

    args[1].typname = "_vstrptr";
    args[1].name = "service";
    
    args[2].typname = "_vstrptr";
    args[2].name = "str";
    
    gen_caller ("");
    gen_callee (fnbody);

/*  Generate the eval function. */

    init_args();

    sprintf (fnname, "dis_%s_eval", service_name);
    sprintf (fnbody, "{\n\
    int e = %s_app->eval((vchar*)str);\n\
    static vstr* result = NULL;\n\
    result = %s_app->getEvalResult();\n\
    return result;\n\
}\n", service_name, service_name);

    argcount = 2;
    args[0].typname = "_vstrptr";
    args[0].name = fnname;

    args[1].typname = "_vstrptr";
    args[1].name = "str";
    
    
    gen_caller ("");
    gen_callee (fnbody);

/*  Generate the eval_async function */

    init_args();

    sprintf (fnname, "dis_%s_eval_async", service_name);
    sprintf (fnbody, "{\n\
   int e = %s_app->eval((vchar*)str);\n\
}\n", service_name, service_name);
    argcount = 2;
    args[0].typname = "async";
    args[0].name = fnname;

    args[1].typname = "_vstrptr";
    args[1].name = "str";
    
    gen_caller ("");
    gen_callee (fnbody);

    fprintf (fout,"\n#ifdef SERVER\n\n");
    fprintf (fout,"    static int serverInited = 0;\n\n");
    fprintf (fout,"void %sServerInit()", service_name); 
    fprintf(headers, "void %sServerInit();\n", service_name); 
    fprintf (fout,"{\n");
    fprintf (fout,"    %s_app = Application::findApplication(\"%s\");\n", service_name, service_name);
    fprintf (fout,"    if (!serverInited) {\n");

    for (i = 0; i < num_Tclfns; i++) {
        fprintf (fout,"        Tcl_CreateCommand (%s_app->interp(), %c%s%c, %s, NULL, NULL);\n", 
                service_name,
                '"',
                Tclnames[i],
                '"',
                Tclnames[i]);
    }
    fprintf (fout,"\tserverInited = 1;\n");
    fprintf (fout,"    }\n");
    fprintf (fout,"}\n");
    fprintf (fout,"\n\n");


    fprintf (fout,"vscrap* %sServer(", service_name); 
    fprintf (fout,"    vservicePrimitive *primitive,\n");
    fprintf (fout,"    vsession          *session,\n");
    fprintf (fout,"    vscrap            *scrapArgs)\n");
    fprintf (fout,"{\n");

    fprintf (fout,"    %sServerInit();\n\n", service_name);
    fprintf (fout,"    %s_app = Application::findApplication(session);\n", service_name);
    fprintf (fout,"    vscrapArray *sarg=0;\n");
    fprintf (fout,"    int array_index,max_index,array_index2;\n");
    fprintf (fout,"    vstr *command;\n");

    fprintf (fout,"    primitive->GetArgs(scrapArgs,&sarg);\n");
    fprintf (fout,"    array_index=0;array_index2=0;\n");
    fprintf (fout,"    max_index=vscrapGetArrayCount(sarg);\n");
    fprintf (fout,"    demarshall__vstrptr (&command, g_take());\n");
    fprintf (fout,"    vscrap *real_result=vscrapCreateArray();\n\n");

    for (i=0; i<functioncount; i++) {
	fprintf (fout,"    if (strcmp((char*)command, \"%s\") == 0) %s_fn(sarg,real_result,array_index,array_index2);\n", 
	       functions[i].name, functions[i].name);
        if(i < functioncount - 1)
           fprintf (fout,"       else\n");
    }
    fprintf (fout,"    else if (strcmp((char*)command, \"server_eval\") == 0) dis_servereval_fn(sarg,real_result,array_index,array_index2);\n");
    fprintf (fout,"    else if (strcmp((char*)command, \"server_eval_async\") == 0) dis_servereval_async_fn(sarg,real_result,array_index,array_index2);\n");
    

    fprintf (fout,"\n");
    fprintf (fout,"    vscrap *temp= primitive->MakeReturnScrap(session,real_result);\n");
    fprintf (fout,"    if (real_result) delete real_result;\n");
    fprintf (fout,"    if (command) vstrDestroy (command);\n");
    fprintf (fout,"    if (sarg) delete sarg;\n");
    fprintf (fout,"    return temp;\n");

    fprintf (fout,"}\n");
    fprintf (fout,"\n#endif /* SERVER */\n");

    IF_destroy_all();
    fclose(headers);
    if ( fout != stdout )
      fclose( fout );
    return (ret_status);
}


