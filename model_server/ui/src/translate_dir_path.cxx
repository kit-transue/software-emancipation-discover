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
// translate_dir_path.C

// INCLUDE FILES
#ifndef _proj_h
#include <proj.h>
#endif

#include <msg.h>
#include <messages.h>

#ifndef _path_h
#include <path.h>
#endif

#ifndef _gtPushButton_h
#include <gtPushButton.h>
#endif

//#define TXT(a) a

// FUNCTION DEFINITIONS

bool translate_dir_path(const char* dir, const char* file, const char* name,
                        const char* suffix, bool to_logical, genString& translated_dir)
// translate the directory path between physical and logical
//
// dir: the directory name
//
// file: the entered file name from which the extension is defined
//
// name: entered name
//
// suffix: file extension
//
// to_logical = 1: from physical to logical
//            = 0: from logical  to physical
//
// translated_dir: returned the translated path
{
    Initialize(translate_dir_path);

    if (!dir || !*dir)
        return true; // map NULL or "" to ""

    projNodePtr proj = projNode::get_home_proj();
    if (!proj)
    {
	msg("ERROR: Cannot translate '$1:' home project not set") << dir << eom;
        return false;
    }

    genString converted_dir;
    if (to_logical)
    {
        project_convert_filename ((char*)dir, converted_dir);
        if (proj->is_script())
            proj->fn_to_ln(converted_dir, translated_dir, DIR_TYPE,0,1);
        else
            proj->fn_to_ln(converted_dir, translated_dir, DIR_TYPE);
    }
    else
    {
        if (proj->is_script())
            proj->ln_to_fn(dir, translated_dir, DIR_TYPE,0,1);
        else
            proj->ln_to_fn(dir, translated_dir, DIR_TYPE);
    }

    if (translated_dir.length() == 0)
    {
        if (to_logical)
        {
            genString src;
            if (file && strlen(file))
                src.printf("%s/%s",converted_dir.str(),file);
            else
                src.printf("%s/%s%s",converted_dir.str(),name,suffix);
            if (proj->is_script())
                proj->fn_to_ln(src, translated_dir,FILE_TYPE,0,1);
            else
                proj->fn_to_ln(src, translated_dir);
        }
        else
        {
            genString log;
            if (file && strlen(file))
                log.printf("%s/%s",dir,file);
            else
                log.printf("%s/%s%s",dir,name,suffix);
            if (proj->is_script())
                proj->ln_to_fn(log, translated_dir,FILE_TYPE,0,1);
            else
                proj->ln_to_fn(log, translated_dir);
        }

        if (translated_dir.length())
        {
            char *p = strrchr((char *)translated_dir, '/');
            if (p)
                *p = '\0';
        }
    }

    size_t pathlen = translated_dir.length();
    if (pathlen == 0) {
	msg("ERROR: $1 translation of '$2' failed") << (to_logical ? TXT("Physical-to-logical") : TXT("Logical-to-physical")) << eoarg << dir << eom;
    }

    return pathlen > 0;
}

/*
   START-LOG-------------------------------------------

   $Log: translate_dir_path.C  $
   Revision 1.11 2000/07/12 18:15:57EDT ktrans 
   merge from stream_message branch
// Revision 1.7  1994/06/24  14:15:19  so
// Bug track: 7700
// fix bug 7700
//
// Revision 1.6  1993/04/28  04:55:24  builder
// Fix mapping problems
//
// Revision 1.5  1993/04/16  20:31:21  so
// fix bug #3407
// When fail to convert the dir name, try to convert it as a partial dir name.
//
// Revision 1.4  1993/03/31  16:13:57  wmm
// Fix bug 3009 (succeed with no action on NULL or empty dir string).
//
// Revision 1.3  1993/03/15  22:18:05  so
// Don't append dummy file name to do the directory mapping.
//
// Revision 1.2  1993/02/24  21:43:24  wmm
// Adapt to change from current project to home project.
//
// Revision 1.1  1993/01/18  17:33:53  wmm
// Initial revision
//
   END-LOG---------------------------------------------
*/
