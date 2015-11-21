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
#include <machdep.h>
#ifndef _WIN32
#include <sys/param.h>
#endif
#include <genString.h>

// UTILITY
// The next two routines are defined in cpp/src/cccp.c also.
static int count_separator(char *p, char s)
{
    int count = 0;
    while(*p)
    {
        if (*p == s)
            count++;
        p++;
    }
    return count;
}

static const char* local_paraset_resolvepath(const char* path, char* ret_buf)
{
    if (path == 0)
        return 0;

    char pathname[MAXPATHLEN];
    char *p;
    int  i;
    int  idx;
    int  status;
    int  count;
    genString ret_path;

    //convert_home_dir((char *)path, ret_path);
    //path = ret_path;

    if (*path == '/')
        strcpy(pathname,path);
    else
    {
        p = (char *)OS_dependent::getwd(pathname);
        if (p == 0)
            return 0;
        p += strlen(p);
        *p++ = '/';
        strcpy(p,path);
    }
    p = pathname;

    count = count_separator(p,'/');

    char **tok_buf = new char* [sizeof(char *) * count];
    memset(tok_buf,0,sizeof(char *) * count);
    idx = 0;
    while(*p)
    {
        if (*p == '/')
        {
            tok_buf[idx++] = p+1;
            *p = '\0';
        }
        p++;
    }

    char *flag = new char [sizeof(char) * count];
    memset(flag,0,sizeof(char) * count);

    status = 1; // assuming successful
    for(i=0;i<count;i++)
    {
        if (strlen(tok_buf[i]) == 0)
            flag[i] = 1; // remove one slash out of '//'
        else if (strcmp(tok_buf[i],".") == 0)
            flag[i] = 1; // remove /./
        else if (strcmp(tok_buf[i],"..") == 0) // the most complicated one
        {
            char found = 0;
            int  j;
            flag[i] = 1; // remove /../
            for(j=i-1;j>=0;j--)
            {
                if (flag[j] == 0)
                {
                    flag[j] = 1;
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                status = 0; // fail
                break;
            }
        }
    }

    if (status)
    {
        p = ret_buf;
        *p = '\0';
        for (i=0;i<count;i++)
        {
            if (flag[i] == 0)
            {
                *p++ += '/';
                strcpy(p,tok_buf[i]);
                p += strlen(p);
            }
        }
    }

    delete [] tok_buf;
    delete [] flag;

    if (status == 0 || strlen(ret_buf) == 0)
        return 0;

    return ret_buf;
}

extern "C" const char* paraset_resolvepath(const char* path, char* ret_buf)
{
    if (path == 0)
        return 0;

    if (*path != '/')
        path = local_paraset_resolvepath(path, ret_buf);

    if (path == 0)
        return 0;

    genString work_path = path;
    genString new_path;
    while(1)
    {
        char *s = (char *)work_path.str();
        char *p;
        while(1)
        {
            p = strstr(s,"..");
            if (!p || (*(p-1) == '/' && (*(p+2) == '\0' || *(p+2) == '/')))
                break;
            s = p+2;
        }
        if (!p)
        {
            if (local_paraset_resolvepath(work_path.str(), ret_buf) == 0)
                return 0;

#ifndef _WIN32
	    char link_buf[MAXPATHLEN];
    	    char resolve_buf[MAXPATHLEN];
	    
            int no_chars = OSapi_readlink (ret_buf, link_buf, MAXPATHLEN);
            if (no_chars > 0) {
		link_buf[no_chars] = 0;
	  	if (link_buf[0] == '/')
		    return paraset_resolvepath(link_buf, ret_buf);
		strcpy (resolve_buf, ret_buf);
		char *p = strrchr (resolve_buf, '/');
	        if (p) 
		    *p = 0;
		strcat (resolve_buf, "/");
		strcat (resolve_buf, link_buf);
		if (strcmp(resolve_buf, path) == 0) {
		    strcpy (ret_buf, resolve_buf);
		    return ret_buf;
		}
		return paraset_resolvepath(resolve_buf, ret_buf);
            }
#endif
            return ret_buf;
        }
        else
        {
            *(p-1) = '\0';
            if (local_paraset_resolvepath(work_path.str(), ret_buf) == 0)
                return 0;
            new_path = ret_buf;
#ifndef _WIN32
            int no_char = OSapi_readlink(new_path.str(), ret_buf, MAXPATHLEN);
            if (no_char > 0)
            {
                ret_buf[no_char] = '\0';
                if (*ret_buf == '/')
                    new_path = ret_buf;
                else
                    new_path += ret_buf;
            }
#endif

            *(p-1) = '/';
            char *q;
            if (*(p+2) == '\0')
                q = p+2;
            else
            {
                q = p+3;
                while(1)
                {
                    if (strncmp(q,"../",3) == 0)
                        q += 3;
                    else if (strcmp(q,"..") == 0)
                    {
                        q += 2;
                        break;
                    }
                    else
                        break;
                }
                if (*(q-1) == '/')
                    *(q-1) = '\0';
            }
            new_path += p-1;
            if (local_paraset_resolvepath(new_path.str(), ret_buf) == 0)
                return 0;

            if (*q == '\0')
                break;

            new_path = ret_buf;
            *(q-1) = '/';
            new_path += q-1;
            work_path = new_path.str();
        }
    }

    char link_buf[MAXPATHLEN];
    char resolve_buf[MAXPATHLEN];

#ifndef _WIN32
    int no_chars = OSapi_readlink (ret_buf, link_buf, MAXPATHLEN);
    if (no_chars > 0) {
	link_buf[no_chars] = 0;
	if (link_buf[0] == '/')
	    return paraset_resolvepath(link_buf, ret_buf);
	strcpy (resolve_buf, ret_buf);
	char *p = strrchr (resolve_buf, '/');
	if (p) 
	    *p = 0;
	strcat (resolve_buf, "/");
	strcat (resolve_buf, link_buf);
	if (strcmp(resolve_buf, path) == 0) {
	    strcpy (ret_buf, resolve_buf);
	    return ret_buf;
	}
	return paraset_resolvepath(resolve_buf, ret_buf);
    }
#endif
    return ret_buf;
}

