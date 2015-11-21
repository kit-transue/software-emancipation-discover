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
#include <stdio.h>
#include <stdlib.h>

static char * priv = "loc";
static int main_done = 0;
static int branch_is_writable(name)
char * name;
{
#if 0
       char buf[1024];
       char * user = getenv("USER");
       int status = 0;
       sprintf(buf, "cleartool lstype -brtype -fmt %%u %s | grep -s %s",
	       name, user);
       status = system(buf);
       return ! status;
#else
       return 0;
#endif
}

void main_branch()
{
   if(main_done) return;
   main_done = 1;
   
   if(branch_is_writable("main"))
       printf("@ @ \n");
   else
       printf("@ @ [[ R ]]\n");
}
void process_branch(name)
char * name;
{
    char * dir = "(0)";

    if(strcmp(name, "CHECKEDOUT")==0){
       printf("%s @ [[ cleartool lspriv -short -vob (0) ]]\n", priv);
/*       printf("%s @ [[ cleartool lspriv -short | sed \"s!^%s/!!\"  ]]\n", priv, dir);*/
/*       printf("co @ [[ list atria_list %s %s ]]\n", name, dir); */
       return;
    } else if (name[0] != '/'){
       main_branch();
       return;
    }

    name ++;
    if(strcmp(name,"main")==0){
       main_branch();
    } else {
       if(branch_is_writable(name))
           printf("%s @ [[ filter $PSETCFG/util/atria_filter %s %s ]]\n", name, name, dir);
       else
           printf("%s @ [[ R ]] [[ filter $PSETCFG/util/atria_filter %s %s ]]\n", name, name, dir);
    }
}
