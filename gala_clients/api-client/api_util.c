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

#include <string.h>
#include <stdio.h>
#include <ctype.h>

int fun_ind = 0;
int no_args = 0;

char* args[100];
char* fun_names[1000];

char * fname = 0;

static FILE *client_header;

void print_client_function(file)
  FILE * file;
{
     int ii;

     fprintf(client_header,"void %s();\n",fname);


     fprintf(file, "void %s(", fname);
     for(ii=0; ii<no_args; ++ii){
         fprintf(file, " arg%d",  ii);
         if(ii < no_args-1)
           fprintf(file, ", ");
     }
     fprintf(file, ")\n");

     for(ii=0; ii<no_args; ++ii){
         fprintf(file, "%s arg%d", args[ii], ii);
	 fprintf(file, ";\n ");
     }

     fprintf(file, "{\n");
     
     fprintf(file," house_keep(); \n");

     for(ii=0; ii<no_args; ++ii)
         fprintf(file, " %s_marshal(&arg%d);\n", args[ii], ii);

     fprintf(file, "   api_call_server(%d);\n", fun_ind);

     for(ii=0; ii<no_args; ++ii)
         fprintf(file, " %s_demarshal(&arg%d);\n", args[ii], ii);

     fprintf(file, "}\n\n");
}

void print_server_function(file)
  FILE *file;
{
     int ii;
     fprintf(file, "void %s ();\n",fname);

     fprintf(file, "int %s_handler(", fname);

     fprintf(file, ")\n{\n");
     
     for(ii=0; ii<no_args; ++ii){
         fprintf(file, "%s arg%d;\n", args[ii], ii);
     }

     for(ii=0; ii<no_args; ++ii)
         fprintf(file, " %s_demarshal(&arg%d);\n", args[ii], ii);

     fprintf(file, "   %s(", fname);

     for(ii=0; ii<no_args; ++ii){
         fprintf(file, "arg%d", ii);
         if(ii < no_args - 1)
           fprintf(file, ", ");
     }
     fprintf(file, ");\n");

     fprintf(file," house_keep(); \n");

     for(ii=0; ii<no_args; ++ii)
         fprintf(file, " %s_marshal(&arg%d);\n", args[ii], ii);

     fprintf(file, "   return 0;\n");
     fprintf(file, "}\n\n");
}
print_server_array(file)
   FILE * file;
{
  int ii;

  fprintf(file, "  FUNPTR api_functions[] = {\n");
  
  for(ii=0; ii<fun_ind; ++ii){
    fprintf(file, "   %s_handler,\n", fun_names[ii]);
  }
  fprintf(file, "   };\n");
  fprintf(file, "   int api_function_no = %d;\n\n", fun_ind);
}
main()
{

    FILE * client = fopen("client.c", "w");
    FILE * server = fopen("server.c", "w");
    client_header=fopen("client_header.h","w");

    fprintf(client, "\n typedef char *charp; \n typedef int DI_object; \n typedef int * intp; \n typedef DI_object * DI_objectp; \n void house_keep();\n DI_objectp_marshal();\n api_call_server(); \n DI_objectp_demarshal(); \n DI_object_marshal(); \n DI_object_demarshal(); \n int_marshal(); \n int_demarshal(); \n intp_marshal(); \n intp_demarshal(); \n charp_marshal(); \n charp_demarshal();\n");
    

    fprintf(client_header,"\n typedef char *charp; \n typedef int DI_object; \n typedef int * intp; \n typedef DI_object * DI_objectp;\n int DI_connect(); \n extern void (*api_error_handler)();");

    fprintf(server,"\n typedef char *charp; \n typedef int DI_object; \n typedef int * intp; \n typedef DI_object * DI_objectp;\n void house_keep();\n void DI_objectp_marshal();\n void DI_objectp_demarshal();\n void DI_object_marshal();\n void DI_object_demarshal();\n void int_marshal();\n void int_demarshal();\n void intp_marshal();\n void intp_demarshal();\n void charp_marshal(); \n void charp_demarshal();\n typedef int (*FUNPTR)();\n extern FUNPTR api_functions[];\n");

     
    while(yylex()){
	print_client_function(client);
	print_server_function(server);
    }
    print_server_array(server);
    exit(0);
}


