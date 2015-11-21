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
#include <afx.h>
#include <windows.h>
#include <iostream.h>
#include <stdio.h>
#include <io.h>
#include <ctype.h>

void usage() {
	cout << "Usage: dislite [-service [service]]\n";
}

DWORD start_process(char *cmd, int need_wait) 
{
	SECURITY_ATTRIBUTES sattr;
	DWORD dwExitCode;
	memset(&sattr, 0, sizeof(sattr));
	sattr.nLength        = sizeof(sattr);
	sattr.bInheritHandle = TRUE;
	
	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb         = sizeof(sinfo);
	
	PROCESS_INFORMATION pinfo;
	
	if(!CreateProcess(NULL, cmd, NULL, NULL, FALSE,
		CREATE_DEFAULT_ERROR_MODE | DETACHED_PROCESS  | NORMAL_PRIORITY_CLASS, NULL,
		NULL, &sinfo, &pinfo)){
		return -1;
	}
	CloseHandle(pinfo.hThread);
	if(need_wait)	{
		WaitForSingleObject(pinfo.hProcess, INFINITE);
		GetExitCodeProcess(pinfo.hProcess, &dwExitCode);
		CloseHandle(pinfo.hProcess);
		return dwExitCode;
	} else {
		CloseHandle(pinfo.hProcess);
		return 0;
	}
}

DWORD start_emacs(char* dis_emacs)
{
	char cmd[1024];
	char *psethome = getenv("PSETHOME");
	char *dis_install = getenv("DIS_INSTALL");
	for(int i=0;i<strlen(dis_install);i++) if(dis_install[i]=='\\') dis_install[i]='/';
	strcpy(cmd, dis_emacs);	
	strcat(cmd, " -l ");
	strcat(cmd, dis_install);
	strcat(cmd, "\\dislite.el");
	return start_process(cmd, 0);
}

DWORD start_vim(void)
{
	static char str[1024];
	char *psethome = getenv("PSETHOME");
	if(!getenv("VIM")) {
		sprintf(str, "VIM=%s\\lib\\vim", psethome);
		_putenv(str);
	}
	_putenv("DIS_VIM_DEVXPRESS=1");
	sprintf(str, "%s\\bin\\gvim.exe", psethome);
	return start_process(str, 1);
}

DWORD choose_service() 
{
	char cmd[1024];
	char script_path[1024];
	char *psethome = getenv("PSETHOME");
	char *wish = getenv("DIS_WISH");
	char *dis_install = getenv("DIS_INSTALL");
	for(int i=0;i<strlen(dis_install);i++) if(dis_install[i]=='\\') dis_install[i]='/';

  	cmd[0]=0; // empty the string
  	int bHasSpaces = strchr(wish,' ')!=NULL;  // check if path to wish has spaces

	if(bHasSpaces) strcat(cmd,"\"");
	strcat(cmd, wish);
	if(bHasSpaces) strcat(cmd,"\"");

	strcat(cmd," -f ");

	if((dis_install!=NULL) && (strlen(dis_install)>0)) {
	  	bHasSpaces = strchr(dis_install,' ')!=NULL;  // check if path to tcl scripts has spaces
		if(bHasSpaces) strcat(cmd,"\"");
	    strcat(cmd, dis_install);
	} else {
		if( (psethome!=NULL) && (strlen(psethome)>0) ) {
  			bHasSpaces = strchr(psethome,' ')!=NULL;  // check if path to tcl scripts has spaces
			if(bHasSpaces) strcat(cmd,"\"");
	        strcat(cmd, psethome);
	        strcat(cmd, "/lib/dislite");
	        strcpy(script_path, psethome);
	        strcat(script_path, "/lib/dislite");
		} else {
	         strcat(cmd, "c:/discover/lib/dislite");
	         strcpy(script_path,"c:/discover/lib/dislite");
		}
		SetEnvironmentVariable("DIS_INSTALL",script_path);
	}
	strcat(cmd, "/ServiceChooser.tcl");
  	if(bHasSpaces) strcat(cmd,"\"");
	return start_process(cmd, 1);
}

int vcopen=0;
int restart = 0;
int do_prefs(int opt)
{
	char str[_MAX_PATH];
	// Please avoid magic numbers
	const int LINE_LENGTH = 128;
	char serv[LINE_LENGTH];
	int i;
	FILE *f;
	char *home = getenv("HOME");
	char *service = getenv("DIS_SERVICE");

	if(!service || vcopen) {
		sprintf(str,"%s\\dislite\\dislite.prefs", home);
		if(opt) {
			choose_service();
			f = fopen(str, "r");
			if(!f) exit(1);
		} else {
			f = fopen(str, "r");
			if(!f) {
				choose_service();
				f = fopen(str, "r");
				if(!f) exit(1);
			}
		}
		fgets(serv, LINE_LENGTH, f);
		fclose(f);
		for(i=0; serv[i]!='\n'&&serv[i]!='\0'; i++);
		serv[i] ='\0';
		if (serv[0] == '\0') {
			do_prefs(1);
		} else {
			sprintf(str, "DIS_SERVICE=%s", serv);
			_putenv(str);
		}
	} else {
		char tmpserv[LINE_LENGTH];
		strcpy (tmpserv,service);
		if (!strchr (tmpserv,'@')) {
			char dis_s[LINE_LENGTH];
			strcpy (dis_s,"@");
			strcat (dis_s,tmpserv);
			sprintf(str, "DIS_SERVICE=%s", dis_s);
			_putenv(str);
		}
	}
	return 0;
}

static void
setargv(int *argcPtr,char ***argvPtr)
{
    char *cmdLine, *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;
    
    cmdLine = GetCommandLine();
	
    /*
	* Precompute an overly pessimistic guess at the number of arguments
	* in the command line by counting non-space spans.
	*/
	
    size = 2;
    for (p = cmdLine; *p != '\0'; p++) {
		if (isspace(*p)) {
			size++;
			while (isspace(*p)) {
				p++;
			}
			if (*p == '\0') {
				break;
			}
		}
    }
    argSpace = (char *) malloc((unsigned) (size * sizeof(char *) 
		+ strlen(cmdLine) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;
	//this appears to be a string parser. any reason you didn't use strtok?
    p = cmdLine;
    for (argc = 0; argc < size; argc++) {
		argv[argc] = arg = argSpace;
		while (isspace(*p)) {
			p++;
		}
		if (*p == '\0') {
			break;
		}
		
		inquote = 0;
		slashes = 0;
		while (1) {
			copy = 1;
			while (*p == '\\') {
				slashes++;
				p++;
			}
			if (*p == '"') {
				if ((slashes & 1) == 0) {
					copy = 0;
					if ((inquote) && (p[1] == '"')) {
						p++;
						copy = 1;
					} else {
						inquote = !inquote;
					}
                }
                slashes >>= 1;
            }
			
            while (slashes) {
				*arg = '\\';
				arg++;
				slashes--;
			}
			
			if ((*p == '\0') || (!inquote && isspace(*p))) {
				break;
			}
			if (copy != 0) {
				*arg = *p;
				arg++;
			}
			p++;
        }
		*arg = '\0';
		argSpace = arg + 1;
    }
    argv[argc] = NULL;

    *argcPtr = argc;
    *argvPtr = argv;
}

static bool valid_directory (const char *dir) {
	bool retval = false;
	if (dir) {
		if (::_access(dir, 2) == 0) {
			retval = true;
		}
	}
	return retval;
}

int start_vc(void);
int APIENTRY
WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int  nCmdShow)
{
	char str[2048];
	char **argv;
	int argc;

	setargv(&argc, &argv);
	_putenv("DIS_LITESTARTUP=1");
	_putenv("DIS_NLUNIXSTYLE=1");

	char* userProfile = getenv("USERPROFILE");
	if(userProfile!=NULL) {
		char newhome [512];
		strcpy(newhome, "HOME=");
		strcat(newhome, userProfile);
		putenv(newhome);
	}

	// Make sure $HOME exists and is a valid directory
	// If it's no good, maybe HOMEPATH and HOMEDRIVE will help
	if(!valid_directory(getenv("HOME"))) {
		char *homedrive = getenv("HOMEDRIVE");
		char *homepath = getenv("HOMEPATH");
		if (homedrive && homepath) {
			char newhome [512];
			strcpy(newhome, "HOME=");
			strcat(newhome, getenv("HOMEDRIVE"));
			strcat(newhome, getenv("HOMEPATH"));
			putenv(newhome);
		}
		// If they didn't help, maybe c:\temp will be OK
		if(!valid_directory(getenv("HOME"))) {
			_putenv("HOME=c:\\temp");
		}
		if(!valid_directory(getenv("HOME"))) {
			exit(1);
		}
	}
	//next set USER so that the code in the tcl will work
	if (!getenv("USER"))
	{
		char newuser[128];
		strcpy(newuser, "USER=");
		if (getenv("USERNAME"))
		{
			strcat(newuser, getenv("USERNAME"));
		}
		else
		{
			if (getenv("TEMP"))
				strcat(newuser, getenv("TEMP"));
			else
			 //pick a good location
				strcat(newuser, "C:\\");
		}
		putenv(newuser);
	}


	char *psethome = getenv("PSETHOME");
	//GRB: If psethome ends with a slash, remove it.
	if (psethome && psethome[strlen(psethome)-1]== '\\')
	{
		char envstring[512];
		char newpsethome [256];
		strcpy(newpsethome, psethome);
		newpsethome[strlen(newpsethome)-1]='\0';
		strcpy(envstring, "PSETHOME=");
		strcat(envstring, newpsethome);
		putenv(envstring);
		psethome=getenv("PSETHOME");
	}

	char *wish = getenv("DIS_WISH");
	if(!wish) {
		sprintf(str, "DIS_WISH=%s\\bin\\wish83.exe", psethome);
		_putenv(str);
		sprintf(str, "TCL_LIBRARY=%s\\lib\\tcl8.3", psethome);
		_putenv(str);
		sprintf(str, "TK_LIBRARY=%s\\lib\\tk8.3", psethome);
		_putenv(str);
	}

	char* dis_install = getenv("DIS_INSTALL");
	if(dis_install) {
		for(int i=0;i<strlen(dis_install);i++) {
			if(dis_install[i]=='\\') {
				dis_install[i]='/';
			}
		}
	} else {
		sprintf(str, "DIS_INSTALL=%s/lib/dislite", psethome);
		char* pch=str;
		while(*pch!=0) {
			if(*pch=='\\') *pch='/';
			pch++;
		}
		_putenv(str);
	}

	// Add path to our libraries to %PATH%.
	char* path = getenv("PATH");
	sprintf(str, "PATH=%s;%s\\lib;", path, psethome);
	_putenv(str);

// Modified by eduards
	
	char *def_ed = getenv("DIS_EDITOR");
	char ed[2];	
	
	if((def_ed==NULL) || (strlen(def_ed)==0)) {
		if(getenv("DIS_VIM")) {
			strcpy(ed,"2");
		} else {
			strcpy(ed,"0");
		}
	} else {
		strcpy(ed,def_ed);
	}
	sprintf(str, "default_editor=%s", ed);
	_putenv(str);
//

	int error = 0;
	if(argc == 1) {
		do_prefs(0);
	} else if (argc==2) {
		if(!strcmp(argv[1], "-vcopen")) {
		        vcopen = 1;
			do_prefs(0);
		} else if(!strcmp(argv[1], "-service")) {
			do_prefs(1);
		} else {
			error = 1;
		}
	} else if(argc == 3) {
		if(!strcmp(argv[1], "-service")) {
			sprintf(str, "DIS_SERVICE=%s", argv[2]);
			_putenv(str);
			do_prefs(0);
		} else {
			error = 1;
		}
	} else {
		error = 1;
	}

	if(error) {
		usage();
		exit(1);
	}
	
	_putenv("DIS_LITESTARTUP=0");

// Modified by eduards

	int ch_ed ();
	if (ch_ed ()) {
		_putenv("DIS_LITESTARTUP=1");
		do_prefs(1);
		if (ch_ed ()) exit(1);
	}

	return 0;
}

int ch_ed () {

	FILE *f;
	char *home = getenv("HOME");
	char str[1024],ind_editor[2];

	if(!getenv("DIS_EDITOR")) {
		sprintf (str,"%s\\dislite\\editor.prefs", home);
		f = fopen(str, "r");
		if (f) {
			fgets(ind_editor, 2, f);
			fclose(f);
		} else {
			char *def_ed = getenv("default_editor");
			strcpy(ind_editor,def_ed);
		}
		sprintf (str,"DIS_EDITOR=%s", ind_editor);
		_putenv(str);
	} else {
		char *d_e = getenv("DIS_EDITOR");
		strcpy(ind_editor,d_e);
	}

	if (*ind_editor == '1') {
		char *dis_emacs = getenv("DIS_EMACS");
		if(dis_emacs) {
			sprintf (str,"%s", dis_emacs);
			f = fopen(str, "r");
			if (!f) {
				start_emacs("emacs");
			} else {
				start_emacs(dis_emacs);
				fclose(f);
			}
		}			
		else {
			start_emacs("emacs");
		}
	}
	else if (*ind_editor == '2') {
		
		for (int ii=0;ii<5;ii++) {
				if(start_vim()!=10) break;
				FILE *f;
				//PLEASE DON'T USE MAGIC NUMBERS!
				const int LINE_LENGTH=128;
				char serv[LINE_LENGTH];
				char *home = getenv("HOME");
				sprintf(str,"%s\\dislite\\dislite.prefs", home);
				f = fopen(str, "r");
				if(!f) return 1;
				fgets(serv, LINE_LENGTH, f);
				fclose(f);
				for(int i=0; serv[i]!='\n' && serv[i]!='\0'; i++);
				serv[i] ='\0';
				sprintf(str, "DIS_SERVICE=%s", serv);
				_putenv(str);
			}
			if (ii==5) return 1;
		}
	else {
		while(TRUE) {
	      restart = 0;
	      start_vc();
	      if (!restart) break;
	    }
	}
	return 0;
}






