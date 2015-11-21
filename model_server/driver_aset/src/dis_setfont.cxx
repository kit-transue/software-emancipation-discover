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
#if ! _STAND_ALONE_
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <cmd.h>
#include <customize.h>
#include <psetmem.h>
#endif

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <X11/Xlib.h>

#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#include <sys/param.h>
#include <dirent.h>
#include <sys/utsname.h>

static int local_host = 0;

static int aseticons_available = 0;
int is_aseticons_available() { return aseticons_available; }

//
// make the standalone exe: CC -o dis_setfont -D{$Arch} -D_STAND_ALONE_ -g dis_setfont.C -lX11
//


// return:   0 if directory exists
static int stat_dir (const char *dir_name) 
{
    int ret = -1;
    if (dir_name && dir_name[0]) {
	DIR *dirp = opendir(dir_name);
	if (dirp) {
	    closedir(dirp);
	    ret = 0;
	}
    }
    return ret;
}

char *mymalloc (unsigned sz)
{
#if _STAND_ALONE_
    char *ptr = (char *)malloc(sz);
#else
    char *ptr = psetmalloc (sz);
#endif
    return ptr;
}

void myfree (void *ptr)
{
#if _STAND_ALONE_
    free((char *)ptr);
#else
    psetfree (ptr);
#endif
}

struct args_INFO {
    char *display;
    char *font_nm;
    int   force;

    args_INFO() : display(0), font_nm("aseticons-20"), force(0) { }
};

static const char *get_envvar (const char *nm)
{
#if _STAND_ALONE_
    const char *value = getenv(nm);
#else
    const char *value = OSapi_getenv((char *)nm);
#endif
    return value;
}

static const char* arch[] = {
    "",
    "sun5",
    "hp10",
    "irix6"
};

enum ARCH {
    archUNKNOWN,
    archSUN5,
    archHP10,
    archIRIX6,

    archLAST
};

static enum ARCH get_platform()
{
    enum ARCH ind   = archUNKNOWN;

#ifdef sun5
    ind = archSUN5;
#endif
#if defined(hp10)
    ind = archHP10;
#endif
#if defined(irix6)
    ind = archIRIX6;
#endif
    
    return ind;
}

static int get_dis_fontpath (char *dis_path)
{
    int ret = 0;
    const char *psethome = get_envvar("PSETHOME");
    if (!psethome || !psethome[0]) {
	msg("Env. var. PSETHOME is not set.", error_sev) << eom;
	ret = -1;
	return ret;
    }

    strcpy(dis_path,   psethome);
    strcat(dis_path, "/lib/fonts/");
 
    msg("DISCOVER font location $1", normal_sev) << dis_path << eom;
    return ret;
}

#if _STAND_ALONE_
static void help (char *nm)
{
    char *prog;
    if (!nm || !nm[0]) return;

    prog = strrchr (nm, '/');
    if (prog == 0) 
	prog = nm;
    else
	prog ++ ;

    printf("\nNAME\n");
    printf("     %s - query X server for DISCOVER icon font\n", prog);
    printf("\nSYNOPSYS\n");
    printf("     %s [-d[isplay] <DISPLAY>] [-s[ilent]] [-fn <FONT>] [-f[orce]]\n\n", prog);
    printf("\nAVAILABILITY\n");
    printf("     DISCOVER Release 7.2\n");
    printf("\nDESCRIPTION\n");
    printf("     The %s DISCOVER utility queries X server for font \"aseticons-20\"\n", prog);
    printf("\nOPTIONS\n");
    printf("     -d[isplay]    specifies the host, display number, and screen\n\n");
    printf("     -s[ilent]     supresses printing protocol on stdout.\n\n");
    printf("     -fn <FONT>    specifies the font name to be used instead of default\n");
    printf("                   font \"aseticons-20\"\n\n");
    printf("     -f[orce]      adds to X server font path the DISCOVER font paths one by\n");
    printf("                   untill the font \"aseticons-20\" is available\n");
    printf("\nENVIRONMENT\n");
    printf("    DISPLAY To get the default host, display number, and screen\n");
    printf("\nEXIT STATUS\n");
    printf("     The following exit values are returned:\n");
    printf("\n");
    printf("     0   The font is available.\n");
    printf("\n");
    printf("     1   The font is not available.\n");
    printf("\n");
    printf("    -1   The X server connection is refused.\n");
    printf("\n");
    printf("    -2   Neither command option \'-d[isplay]\' nor environment variable DISPLAY\n");
    printf("         is not specified.\n");
    printf("    -3   Command line options are wrong.\n\n");

}

static int check_help (int argc, char **argv)
{
    if ( argc == 2 && ( (strcmp(argv[1],"-help") == 0) ||
	                (strcmp(argv[1],"help")  == 0) ||
	                (strcmp(argv[1],"-HELP") == 0) ||
	                (strcmp(argv[1],"HELP")  == 0) ||
	                (strcmp(argv[1],"/\?")  == 0) ) )
	{
	    help(argv[0]);
	    return 1;
	}
    return 0;
}

static int check_args (int argc, char **argv, args_INFO& inf)
{
    int res = 0;
    char *cur;
    int i;
    
    for (i = 1 ; i < argc ; i ++ ) {
	if (strstr("-silent", argv[i])) {
	    silent = 1;
	    break;
	}
    }

    for (i = 1 ; i < argc ; i ++ ) {
	if (!argv[i] || !*argv[i]) {
	    msg ("$1th command line option empty", error_sev) << i-1 << eoarg << eom;
	    res = 1;
	    break;
	}
	if (strcmp(argv[i], "-fn") == 0) {
	    i ++ ;
	    if ( i >= argc || !argv[i] || !*argv[i] ) {
		msg ("There is no font name after \'-fn\'", error_sev) << eom;
		res = 1;
		break;
	    }
	    inf.font_nm = argv[i];
	} else if (strstr("-display", argv[i])) {
	    i ++ ;
	    if ( i >= argc || !argv[i] || !*argv[i] ) {
		msg ("There is no X server name after \'-display\'", error_sev) << eom;
		res = 1;
		break;
	    }
	    inf.display = argv[i];
	} else if (strstr("-force",argv[i])) {
	    inf.force = 1;
	} else if ( !strstr("-silent", argv[i]) ) {
	    msg ("Unknown command line option \'$1\'", error_sev) << argv[i] << eoarg << eom;
	    res = 1;
	    break;
	}
    }
    return res;
}
#endif


static void print_X_info (Display *dpy)
{
    if (!dpy) return;
    msg("Server_Vendor:dis_setfont.C", normal_sev) << ServerVendor(dpy) << eom;
    msg("Vendor_Release:dis_setfont.C", normal_sev) << VendorRelease(dpy) << eom;
    msg("Protocol_Version:dis_setfont.C", normal_sev) << ProtocolVersion(dpy) << eom;
    msg("Protocol_Revision:dis_setfont.C", normal_sev) << ProtocolRevision(dpy) << eom;
}

static int font_err = 0;

static int local_xerror(Display *dpy, XErrorEvent *rep)
{
    if (rep->error_code == BadName) {
	msg("X error handler - BadName", error_sev) << eom;
	font_err = 1;
    } else if (rep->error_code == BadValue) {
	msg("X error handler - BadValue", error_sev) << eom;
	font_err = 2;
    } else {
	msg("X error handler - error code $1", error_sev) << (int)rep->error_code << eom;
	font_err = 3;
    }

    return 0;
}

static int load_font_check ( Display *dpy, args_INFO& inf)
{
    int res = 0;

    int cnt = 0;
    char **lst = XListFonts(dpy, inf.font_nm, 1, &cnt);

    if (!lst || cnt == 0 || font_err) {
	msg("Failed: font \'$1\' is not available.", warning_sev) << inf.font_nm << eom;
	res = 1;
    } else {
	msg("Succeeded: font \'$1\' is available.", normal_sev) << inf.font_nm << eom;
	res = 0;
    }

    XFreeFontNames(lst); 
    return res;
}


// returns:  0 if the font inf.font_nm exists with the font path added infront to existing path
//             reset the font path back to existing if font is not found
static int check_fontpath (Display *dpy, args_INFO& inf, const char *p)
{
    int ret = -1;
    if (!p || !p[0]) return ret;

    if (p[0] == '/' && stat_dir(p)) {
	msg("Directory $1 does not exist.", warning_sev) << p << eom;
	ret = -1;
	return ret;
    }

    int npaths = 0;
    char **orig_fp = XGetFontPath(dpy, &npaths);
    if (font_err) return ret;

    int i = 0; int len = strlen(p);
    for(i = 0; i < npaths; i++) {
	if ( strncmp(p, orig_fp[i], len) == 0 )
	    break;
    }

    if (!npaths || i >= npaths) {
	// prepend our path to font path
	msg("Append font path with $1", normal_sev) << p << eom;

	char **new_font_path = (char **)mymalloc((npaths+1)*sizeof(char *));
	if (npaths) {
	    for (i = 0; i < npaths; i++) {
		new_font_path[i] = (char *)mymalloc(strlen(orig_fp[i]) + 1);
		strcpy(new_font_path[i], orig_fp[i]);
	    }
	}
	new_font_path[npaths] = (char *)mymalloc(len + 13);
	strcpy(new_font_path[npaths], p);

	XSetFontPath (dpy, new_font_path, npaths+1);
	if (font_err) {
	    msg("Failed to set new font path.", error_sev) << eom;
	    if (orig_fp) XFreeFontPath (orig_fp);
	    return ret;
	}
	for (i = 0; i <= npaths; i++)
	    myfree (new_font_path[i]);
	myfree(new_font_path);
    } else {
	msg("Rehashing font path that already contains path $1", normal_sev) << p << eoarg << eom;

	//rehash font path
	npaths = 0;
	orig_fp = XGetFontPath(dpy, &npaths);
	if (npaths) XSetFontPath (dpy, orig_fp, npaths);
    }
 
    ret = load_font_check (dpy, inf);

    if (ret) {
	msg("Removing $1 from X server font path.", warning_sev) << p << eom;

	int good = 0;
	char **new_font_path = 0;

	if ( npaths > 1) {
	    new_font_path = (char **)mymalloc((npaths)*sizeof(char *));
	    for (i = 0; i < npaths; i++) {
		if (strncmp(p, orig_fp[i], len) != 0) {
		    new_font_path[i] = (char *)mymalloc(strlen(orig_fp[i]) + 1);
		    strcpy(new_font_path[i], orig_fp[i]);
		    good ++;
		}
	    }
	}

	XSetFontPath (dpy, new_font_path, good);
	for (i = 0; i < good; i++)
	    myfree (new_font_path[i]);
	myfree(new_font_path);
    }
    if (orig_fp) XFreeFontPath (orig_fp);

    return ret;
}

int play_with_fontpath (Display *dpy, args_INFO& inf)
{
    int ret = 0;

    const char *dis_fp = get_envvar("DISfp");
    if (dis_fp && dis_fp[0]) {
	msg("Env: font path from DISfp: $1", normal_sev) << dis_fp << eom;
	ret = check_fontpath (dpy, inf, dis_fp);
	return ret;
    }

    if ( !(local_host || inf.force) ) return 1;

    char fnt_path[MAXPATHLEN];
    char dis_path[MAXPATHLEN];
    ret = get_dis_fontpath (dis_path);
    if (ret) return ret;

    if (stat_dir(dis_path) != 0) {
	ret = -1;
	msg("DISCOVER font location directory $1 does not exist for X server.", error_sev) << dis_path << eoarg << eom;
	return ret;
    }

    strcpy(fnt_path, dis_path);
    enum ARCH ind        = get_platform ();
    const char *platform = arch[ind];
    if (!platform || !platform[0]) {
	msg("get_dis_fontpath: cannot figure out the host architecture.", error_sev) << eom;
	ret = -1;
	return ret;
    }
    strcat(fnt_path, platform);

    ret = check_fontpath (dpy, inf, fnt_path);
    if ( inf.force && ret ) {
	for ( int i = archSUN5 ; i < archLAST ; i ++ ) {
	    platform = arch[i];

	    strcpy(fnt_path, dis_path);
	    strcat(fnt_path, platform);

	    ret = check_fontpath (dpy, inf, fnt_path);

	    if (ret == 0) break;
	}
	if (ret) {
	    msg("Failed to find font $1.", error_sev) << inf.font_nm << eom;
	}
    }
    return ret;
}


static void print_header ()
{
    msg("==== DISCOVER set font ====", normal_sev) << eom;
}


static struct utsname ustr;

static void check_hostname(args_INFO& inf)
{
    int ret = uname (&ustr);
    
    if (ret == -1) {
	msg("Failed to get information about current system.", error_sev) << eom;
	return;
    }
    
    int count = strlen (ustr.nodename);
    if (inf.display[0] == ':' ||
	(strncmp(ustr.nodename,inf.display,count) == 0 && inf.display[count] == ':' ))
	local_host = 1;
    
    const char *knd = (local_host) ? "local" : "remote";
    msg("Running on $1 host: $2 $3 $4", normal_sev) << knd << eoarg << ustr.nodename << eoarg << ustr.sysname << eoarg << ustr.release << eom;
}


typedef int(*HANDLER)(Display*, XErrorEvent*);

int check_aset_icons (args_INFO& inf) 
{
    int res = 0;
    Display *dpy = XOpenDisplay(inf.display);
    if (dpy == (Display *)0) {
	msg("Failed to open DISPLAY $1.", error_sev) << inf.display << eom;
	res = -1;
    } else {
	HANDLER old = XSetErrorHandler(local_xerror);

	print_header();
	check_hostname(inf);

	msg("Displaying on X server: $1", normal_sev) << inf.display << eom;
	print_X_info (dpy);

	res = load_font_check(dpy, inf);

	if (res) res = play_with_fontpath (dpy, inf);

	if (res) {
#if _STAND_ALONE_	    
	    printf (" WRN: Failed to find DISCOVER font \'%s\'.\n", inf.font_nm);
#else
	    msg("Failed to find DISCOVER font \'$1\'.", warning_sev) << inf.font_nm << eom;
	    ;
#endif
	}

	(void) XSetErrorHandler( old );
    }

    return res;
}

#if _STAND_ALONE_
main (int argc, char **argv)
{
    if (check_help(argc, argv)) exit (0);

    args_INFO inf;

    if (check_args(argc, argv, inf)) exit (-3);
    if (inf.display == 0) {
	inf.display = getenv("DISPLAY");
	if (!inf.display || !inf.display[0]) {
	    printf(" ERR: Neither DISPLAY is set nor the display name is specified\n");
	    exit (-2);
	}
    }

    int res = check_aset_icons (inf);

    return res;
}
#else
int dis_setfont ()
{
    args_INFO inf;
    inf.display = OSapi_getenv("DISPLAY");
    int res = check_aset_icons (inf);
    switch (res) {
    case 0:
	aseticons_available = 1;
	break;
    case 1:
	break;
    case -1:
    case -2:
    case -3:
    default:
	msg("Failed to use DISPLAY", error_sev) << inf.display << eoarg << eom;
	break;
    }
    return res;
}
#endif
