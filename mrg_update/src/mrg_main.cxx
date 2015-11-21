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
#include "msg.h"
#include "mrg_update.h"

int	do_debug=0;
FILE* 	log_fd=stderr;

int apply_merge(char* src_name, char* delta_name, FILE* out_file, int flag_comment, char *comment,int flag_ifdef, char *def)
{
	char *src, *delta;
	FILE *src_file, *delta_file;
	int src_size, delta_size;
	struct STAT buf1, buf2;

	src_file = fopen(src_name, "rb");
	if( !src_file ) {
		msg("Unable to open $1", error_sev) << src_name << eom;
		return 1;
	}

	delta_file = fopen(delta_name, "rt");
	if( !delta_file ) {
		msg("Unable to open $1", error_sev) << delta_name << eom;
		return 1;
	}

#if 0
#ifdef _WIN32
	if(getenv("DIS_NLUNIXSTYLE")) {
	  _setmode( _fileno(src_file), _O_TEXT );
	}
#endif
#endif

	STAT( src_name, &buf1 );
	STAT( delta_name, &buf2 );
	
	src = new char[buf1.st_size+1];
	delta = new char[buf2.st_size+1]; 

	src_size = fread(src, 1, buf1.st_size, src_file);
	delta_size = fread(delta, 1, buf2.st_size, delta_file);

	src[src_size] = 0;
	delta[delta_size] = 0;

	do_merge(src, delta, out_file, flag_comment, comment, flag_ifdef, def);

	delete[] src;
	delete[] delta;
	fclose(src_file);
	fclose(delta_file);

	return 0;
}

int apply_mapP_to_P(int n, char** p, char *delta_name)
{
	char* delta;
	FILE* delta_file;
	int delta_size;
	struct STAT buf;

	delta_file = fopen(delta_name, "rt");
	if( !delta_file ) {
	  msg("Unable to open $1", error_sev) << delta_name << eom;
	  return 1;
	}

	STAT( delta_name, &buf );

	delta = new char[buf.st_size+1]; 
	delta_size = fread(delta, 1, buf.st_size, delta_file);
	delta[delta_size] = 0;

	int *pPos = new int[n];
	position* pPosition = new position[n];

	int i;
	for(i=0; i<n; i++) {
		pPos[i] = atoi(p[i]);
		pPosition[i].line_num = i;
		pPosition[i].pos = pPos[i];
	}

	qsort(pPosition, n, sizeof(position), sort_pos );
	GetNewPositionArray(pPosition, n, delta);
	qsort(pPosition, n, sizeof(position), sort_line );
#if 0
	cout << "	before		after\n";
	for(i=0; i<n; i++) 
		cout << "	" << pPos[i] <<	"		" << pPosition[i].pos << "\n";
#endif
	for(i=0; i<n; i++) {
		//
		// DO NOT convert to msg() call;  this is the proper
		// output of mrg_update, and needs to go to stdout
		//
		cout << pPosition[i].pos << endl;
	}

	delete[] pPos;
	delete[] pPosition;
	delete[] delta;
	fclose(delta_file);

	return 0;
}

int line_to_pos(int n, int *pLine, int *pPos, char *src)
{
	int line = 0;
	int pos = 0;
	char * index = src;
	for(int i=0; i<n; i++) {
	  pPos[i] = 0;
	  for(int j=line; j<pLine[i]; j++) {
	    for(;*index != '\n'; index++) pos++;
	    index++;
	    pos++;
	    pPos[i]=pos;
	  }
	  line = pLine[i];
	}
	return 0;
}

int pos_to_line(int n, int* pPos, int* pLine, char *target)
{
  int line = 0;
  int pos = 0;
  char *index = target;

  for(int i=0; i<n; i++) {
    if(pPos[i]<0) {
      pLine[i] = -1;
    } else {
      pLine[i] = 0;
      for(int j=pos; j<pPos[i]; j++) {
	if (*index++ == '\n') line++;
      }
      pLine[i] = line;
      pos = pPos[i];
    }
  }
  return 0;
}

int print_instances(int n, int *pPos, int *pLine, char* target)
{
  int line = 0;
  int pos = 0;
  char *index = target;
  char linebuf[1024];

  for(int i=0; i<n; i++) {
    if(pPos[i]<0) {
      pLine[i] = -1;
      //
      // DO NOT convert to msg() call;  this is the proper
      // output of mrg_update, and needs to go to stdout
      //
      cout << -1 << ":" << "* The instance may deleted *" << endl;
    } else {
      pLine[i] = 0;
      for(int j=pos; j<pPos[i]; j++) {
	if (*index++ == '\n') line++;
      }
      pLine[i] = line;
      pos = pPos[i];
      char *p = index;
      if(p> target) --p;
      for(;*p!='\n' && p>target; --p);
      int count = 0;
      if(*p=='\n') ++p;
      for(;*p!='\n' && *p!='\0';++p) linebuf[count++]=*p;
      linebuf[count] = '\0';
      //
      // DO NOT convert to msg() call;  this is the proper
      // output of mrg_update, and needs to go to stdout
      //
      cout << line+1 << ":" << linebuf << endl;
    }
  }
  return 0;
}

int print_line(int n, int *pLine, char *filename)
{
  FILE* f = fopen(filename, "r");
  if( !f ) {
    msg("Unable to open $1", error_sev) << filename << eom;
    return 1;
  }
  
#ifdef _WIN32
  if(getenv("DIS_NLUNIXSTYLE")) {
    _setmode( _fileno(f), _O_TEXT );
  }
#endif

  char linebuf[1024];
  int line = 0;
  for(int i=0; i<n; i++) {
    for(int j=line; j<=pLine[i]; j++) {
      if(!fgets(linebuf, 1024, f)) {
	fclose(f);
	return 1;
      }
    }
    //
    // DO NOT convert to msg() call;  this is the proper
    // output of mrg_update, and needs to go to stdout
    //
    cout << pLine[i]+1 << ":" << linebuf;
    line = pLine[i]+1;
  }
  cout << flush;
  fclose(f);
  return 0;
}

int apply_mapL_to_P(int n, int* pLine, int *pPos, char *src_name, char *delta_name)
{
  char *src, *delta;
  FILE *src_file, *delta_file;
  int src_size, delta_size;
  struct STAT buf1, buf2;

  src_file = fopen(src_name, "rb");
  if( !src_file ) {
    msg("Unable to open $1", error_sev) << src_name << eom;
    return 1;
  }

#ifdef _WIN32
  if(getenv("DIS_NLUNIXSTYLE")) {
    _setmode( _fileno(src_file), _O_TEXT );
  }
#endif
  
  delta_file = fopen(delta_name, "rt");
  if( !delta_file ) {
    msg("Unable to open $1", error_sev) << delta_name << eom;
    return 1;
  }
  
  STAT( src_name, &buf1 );
  STAT( delta_name, &buf2 );
	
  src = new char[buf1.st_size+1];
  delta = new char[buf2.st_size+1]; 
  
  src_size = fread(src, 1, buf1.st_size, src_file);
  delta_size = fread(delta, 1, buf2.st_size, delta_file);
  
  src[src_size] = 0;
  delta[delta_size] = 0;

  line_to_pos(n, pLine, pPos, src);
  
  position* pPosition = new position[n];

  int i;
  for(i=0; i<n; i++) {
    pPosition[i].line_num = i;
    pPosition[i].pos = pPos[i];
  }
  
  qsort(pPosition, n, sizeof(position), sort_pos );
  GetNewPositionArray(pPosition, n, delta);
  qsort(pPosition, n, sizeof(position), sort_line );

  for(i=0; i<n; i++) 
    pPos[i] = pPosition[i].pos;

  //	int newline = pos_to_line(newpos, newsrc);
  //	cout << newpos << "\n";
  delete[] pPosition;
  delete[] src;
  delete[] delta;
  fclose(src_file);
  fclose(delta_file);
  return 0;
}	

int apply_mapL_to_L(int n, int* pOldLine, int* pLine, char *src_name, char *delta_name, char *target_name, int instances_flag=0)
{
  int *pPos= new int[n];
  if(apply_mapL_to_P(n, pOldLine, pPos, src_name, delta_name)) return 1;

  FILE* target_file = fopen(target_name, "r");
  if( !target_file ) {
    msg("Unable to open $1", error_sev) << target_name << eom;
    return 1;
  }
  
#ifdef _WIN32
  if(getenv("DIS_NLUNIXSTYLE")) {
    _setmode( _fileno(target_file), _O_TEXT );
  }
#endif

  struct STAT buf;
  STAT( target_name, &buf);
  char *target = new char[buf.st_size+1]; 
  int target_size = fread(target, 1, buf.st_size, target_file);
  target[target_size] = 0;
  if(instances_flag) {
    print_instances(n, pPos, pLine, target);
  } else {
    pos_to_line(n, pPos, pLine, target);
  }
  delete[] pPos;
  delete[] target;
  fclose(target_file);
  return 0;
}

int apply_GAT(char* GAT_list_name, char* file_info_name, FILE* flog)
{
	int ret = 0;
	genArrOf(pObject) objPtr_arr;

	if(ret = hash_file(file_info_name, objPtr_arr, flog))
		return ret;

	if(ret = GAT_list_update(GAT_list_name, objPtr_arr, flog))
		return ret;

	return 0;
}

void Usage()
{
	msg("Usage is:  mrg_update -merge [-comment comment] [-ifdef XXX] <SRC_FILE> <DELTA_FILE> [OUT_FILE] or", normal_sev) << eom;
	msg("           mrg_update -mapP_to_P pos1 pos2 ... <DELTA_FILE> or", normal_sev) << eom;
	msg("           mrg_update -mapL_to_P lineno1 lineno2 ... <SRC_FILE> <DELTA_FILE> or", normal_sev) << eom;
	msg("           mrg_update -mapL_to_L lineno1 lineno2 ... <SRC_FILE> <DELTA_FILE> <TARGET_FILE>or", normal_sev) << eom;
	msg("           mrg_update -gat <GAT_list> <file_info> [<out_log>] or", normal_sev) << eom; 
	msg("(all positions are 0 based and all line numbers are 1 based)", normal_sev) << eom;
	msg("Return value for -merge and -map switch:\n\t0: succeeded\n\t1: failed", normal_sev) << eom;
	msg("Return value for -gat switch:\n\t0: all succeeded\n\t-1: none succeeded", normal_sev) << eom;
	msg("\tpositive number: number of groups and attributes successfully updated", normal_sev) << eom;
}


/* gat:
 * 0 - all succeed;  minus - none succeed;  positive - numbers of groups and attibutes sucessfully updated
 * 
 * merge or map:
 * 0 - succeed;	1 - fails
 */

int main(int argc, char* argv[])
{
	int ret = 0;
	int i;
	
	if(argc > 3 && !strcmp(argv[1], "-merge")) {
		int flag_comment = 0;
		int flag_ifdef = 0;
		char *comment = NULL;
		char *def = NULL;
		if(*argv[2] == '-') {
			if(!strcmp(argv[2], "-comment")) {
				flag_comment = 1;
				comment = argv[3];
			} else if(!strcmp(argv[2], "-ifdef")) {
				flag_ifdef = 1;
				def = argv[3];
			} else {
				Usage();
				return 1;
			}
			if(argc >= 8) {
				if(*argv[4] == '-') {
					if(!strcmp(argv[4], "-comment")) {
						flag_comment = 1;
						comment = argv[5];
					} else if(!strcmp(argv[4], "-ifdef")) {
						flag_ifdef = 1;
						def = argv[5];
					} else {
						Usage();
						return 1;
					}
				}
			}
		}

		if(argc%2 == 0) {
#ifdef _WIN32 //without this, it would add extra ^M at the end of every line	    
		        _setmode(_fileno(stdout), _O_BINARY);
#endif
			ret = apply_merge(argv[argc-2], argv[argc-1],stdout, flag_comment, comment,flag_ifdef,def);
		} else {
			FILE *fout = fopen(argv[argc-1], "w+b");
			if(!fout) {
				fprintf(fout, "can not write %s\n", argv[argc-1]);
				return 1;
			}

			ret = apply_merge(argv[argc-3], argv[argc-2], fout, flag_comment, comment,flag_ifdef,def);
			fclose(fout);
		}
	} else if(argc > 3  && !strcmp(argv[1], "-print_line")) {
	        int n = argc - 3;
		int *pLine = new int[n];
		for(i=0; i<n;i++) pLine[i] = atoi(argv[i+2])-1;
	        ret = print_line(n, pLine, argv[argc-1]);
	}else if(argc > 3  && !strcmp(argv[1], "-mapP_to_P") ) {
		ret = apply_mapP_to_P(argc-3, argv+2, argv[argc-1]);
	}  else if (argc > 4 && !strcmp(argv[1], "-mapL_to_P")) {
		int n = argc - 4;
		int *pPos = new int[n];
		int *pLine = new int[n];
		for(i=0; i<n;i++) pLine[i] = atoi(argv[i+2])-1;
		ret = apply_mapL_to_P(n, pLine, pPos, argv[argc-2], argv[argc-1]);
		if (ret == 0) {
		  for(i=0; i<n; i++) {
		    //
		    // DO NOT convert to msg() call;  this is the proper
		    // output of mrg_update, and needs to go to stdout
		    //
		    cout << pPos[i] << endl;
		  }
		  delete pPos;
		  delete pLine;
		}
	} else if (argc > 5  && !strcmp(argv[1], "-mapL_to_L")) {
	        int n = argc - 5;
		int* pOldLine = new int[n];
		int* pLine = new int[n];
		for(i=0; i<n;i++) pOldLine[i] = atoi(argv[i+2])-1;
		ret = apply_mapL_to_L(n, pOldLine, pLine, argv[argc-3], argv[argc-2], argv[argc-1]);
		if (ret==0) {
		  for(i=0; i<n; i++) {
		    if (pLine[i]<0) {
		      //
		      // DO NOT convert to msg() call;  this is the proper
		      // output of mrg_update, and needs to go to stdout
		      //
		      cout << -1 << endl;
		    } else {
		      //
		      // DO NOT convert to msg() call;  this is the proper
		      // output of mrg_update, and needs to go to stdout
		      //
		      cout << pLine[i]+1 << endl;
		    }
		  }
		  delete pOldLine;
		  delete pLine;
		}
	} else if (argc > 5  && !strcmp(argv[1], "-instances")) {
	        int n = argc - 5;
		int* pOldLine = new int[n];
		int* pLine = new int[n];
		for(i=0; i<n;i++) pOldLine[i] = atoi(argv[i+2])-1;
		ret = apply_mapL_to_L(n, pOldLine, pLine, argv[argc-3], argv[argc-2], argv[argc-1], 1);
	} else if(argc == 4 && !strcmp(argv[1], "-gat") ) {
		ret = apply_GAT(argv[2], argv[3], stdout);	
	} else if(argc == 5 && !strcmp(argv[1], "-gat") ) {
		FILE* flog = fopen(argv[4], "w+t");
		ret = apply_GAT(argv[2], argv[3], flog);
		fclose(flog);
	} else {
		Usage();
		ret = 1;
	}
	return ret;
}

extern "C" void driver_exit(int code) {exit(code);}

