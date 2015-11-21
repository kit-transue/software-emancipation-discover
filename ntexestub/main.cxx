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
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <windows.h>

typedef char *charptr;

void main(int argc, char **argv)
{
	if(argc < 4)
		return;

	int read_handle  = atoi(argv[1]);
	int write_handle = atoi(argv[2]);
	int read_pipe_file_handle = _open_osfhandle((long)read_handle, _O_RDONLY);
	if(read_pipe_file_handle < 0){
		return;
	}
	if(read_pipe_file_handle != 3){
	  if(_dup2(read_pipe_file_handle, 3) != 0)
	    return;
	}
	int write_pipe_file_handle = _open_osfhandle((long)write_handle, _O_WRONLY);
	if(write_pipe_file_handle < 0){
	  return;
	}
	if(write_pipe_file_handle != 4){
	  if(_dup2(write_pipe_file_handle, 4) != 0)
	    return;
	}

        char *exe = argv[3];
	char **new_argv = (char **)malloc(sizeof(charptr) * (argc - 3 + 1));
	for(int i = 3; i < argc; i++) {
          const char * ptr = argv[i];
          int has_space = 0;
          while (ptr && *ptr) {
	    if (isspace(*ptr)) {
              has_space = 1;
              break;
	    }
            ptr++;
          }
          int len = strlen(argv[i]+1);
          if (has_space) len += 2;
          new_argv[i -3] = (char *)malloc(sizeof(char) * len);
          if (has_space) {
	    strcpy(new_argv[i - 3], "\"");
	    strcat(new_argv[i - 3], argv[i]);
	    strcat(new_argv[i - 3], "\"");
          } else {
	    strcpy(new_argv[i - 3], argv[i]);
          }
        }
	new_argv[argc - 3] = NULL;

	if ( _execv(exe, (const char * const *)new_argv) == -1 ) {
          printf("Failed to exec process: %s\n", exe);
          return;
        }
	return;
}
