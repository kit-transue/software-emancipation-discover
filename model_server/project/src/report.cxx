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
// report.C
//------------------------------------------
// synopsis:
//
// Create new projNode and new projModule when it refresh a projNode.

// INCLUDE FILES
#include <mpb.h>
#include <cLibraryFunctions.h>
#include <msg.h>

#ifndef ISO_CPP_HEADERS
#include <iomanip.h>
#else /* ISO_CPP_HEADERS */
#include <iomanip>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <machdep.h>
#include <transaction.h>
#include <messages.h>

#ifndef _proj_h
#include <proj.h>
#endif


#ifndef __psetmem_h
#include <psetmem.h>
#endif

#ifndef _objDb_h
#include <objDb.h>
#endif


#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _path1_h
#include <path1.h>
#endif

#ifndef _path_h
#include <path.h>
#endif

#ifndef _path0_h
#include <path0.h>
#endif

#ifndef _pathcheck_h
#include <pathcheck.h>
#endif

#ifndef _fileCache_h
#include <fileCache.h>
#endif



#ifndef _pdf_tree_h
#include <pdf_tree.h>
#endif

//#ifndef _vpopen_h
//#include <vpopen.h>
//#endif
#include <shell_calls.h>

#ifndef _RTL_externs_h
#include <RTL_externs.h>
#endif

#ifndef _WIN32
#include <dirent.h>
#else
#ifndef S_ISDIR
#define S_ISDIR(a)	(((a) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(a)	(((a) & S_IFMT) == S_IFREG)
#endif
#endif

#ifdef _WIN32
extern "C" int g_DisFN_RealCase;
#endif

int proj_restore (projNode *);
extern "C" void  driver_exit(int);

// forward declarations
typedef struct
{
    int  type;
    void *node;
    int  used;
    char const *name;
} old_dir_entry;

old_dir_entry *found_old_dir(char const *name, int type);

#ifndef _glob_0_h
#include <glob_0.h>
#endif

#ifndef _genTmpfile_h
#include <genTmpfile.h>
#endif

static int do_duplicate_checking = 0;

// BEGIN WORKING AREA FOR TOP PROJ
typedef struct
{
    char    *name; /* one level */
    char    *dir;  /* directory name */
    projMap *pm;
    char    *full_proj_name;
} top_proj_head_entry;

genArr(top_proj_head_entry);

static genArrOf(top_proj_head_entry) top_proj_heads;

static void init_top_proj_heads()
{
    Initialize(init_top_proj_heads);

    top_proj_heads.reset();
}

static void free_top_proj_names()
{
    Initialize(free_top_proj_names);

    for (int i=0;i<top_proj_heads.size();i++)
    {
        top_proj_head_entry *p = top_proj_heads[i];
        psetfree(p->name);
        psetfree(p->dir);
        psetfree(p->full_proj_name);
    }
}

static char **top_proj_name_buf = 0;
static void init_top_proj_name_buf()
{
    Initialize(init_top_proj_name_buf);

    top_proj_name_buf = (char **)psetmalloc(sizeof(char *) * top_proj_heads.size());
}

static void free_top_proj_name_buf()
{
    Initialize(free_top_proj_name_buf);

    psetfree(top_proj_name_buf);
    top_proj_name_buf = 0;
}

// END WORKING AREA FOR TOP PROJ

// begin of estimate time for refresh projects

static int script_total_time = 0;
static int script_proj_count = 0;
static int total_time = 0;
static int proj_count = 0;
static int stop_timing = 0;

static time_t time_a;
static time_t time_b;

static void log_start_time()
// mark the starting time to create project nodes
{
    Initialize(log_start_time);

    OSapi_time(&time_a);
}

static void log_complete_time(int type)
// add the time to the total that create projnodes
//
// type == 1 for script or filter projects
// type == 0 for all projects
{
    Initialize(log_complete_time);

    OSapi_time(&time_b);
    if (!stop_timing)
    {
        total_time += int(time_b - time_a);
        if (type)
            script_total_time += int(time_b - time_a);
    }
}


// end of estimate time for refresh projects

// begin for inode of link
typedef struct
{
    dev_t st_dev;
    ino_t st_ino;
} link_inode_entry;

genArr(link_inode_entry);

static genArrOf(link_inode_entry) link_inode_heads;

static void init_link_inode_heads()
{
    Initialize(init_link_inode_heads);

    link_inode_heads.reset();
}

static bool old_inode(struct OStype_stat *buf)
{
    Initialize(old_inode);

    for (int i=0;i<link_inode_heads.size();i++)
    {
        link_inode_entry *p = link_inode_heads[i];
        if (p->st_dev == buf->st_dev && p->st_ino == buf->st_ino)
            return TRUE;
    }
    link_inode_entry *p = link_inode_heads.grow();
    p->st_dev = buf->st_dev;
    p->st_ino = buf->st_ino;

    return FALSE;
}

// end for inode of link

// list files -- begins
extern "C"
{
    void proj_ls(char const *dir, char const *wild, glob_t *globv);
    void globfree(glob_t *pglob);
    void proj_find_files(char const *dir, int (*report_func)(char const *));
}

// forward declaration
static projNode *my_make_project(char const *name, projNode *parent, char const *dir, projMap *pm, char const *full_proj_name,
                                 char level_flag, int force_refresh_flag, int bad_file_name);
static void remove_all_pnlist(projMap *pm, pdf_tree_head *h, pn_list *pnl, int num);

static glob_t *global_ggg = 0;
static WildPair *global_m  = 0;
static int buf_len = 0;
#define NEWSIZE 128

void report_globfree(glob_t &ggg)
{
    Initialize(report_globfree);

    if (global_ggg)
    {
        for(int i=0;i<global_ggg->gl_matchc;i++)
            psetfree(global_ggg->gl_pathv[i]);
	psetfree(global_ggg->gl_pathv);
        global_ggg = 0;
    }
    else
        globfree(&ggg);
}

static int two_star_filter(char const *x)
// Check if the path x matches to the pattern defined in global_m.
// If it matches, add it to the buffer global_ggg.
{
    Initialize(two_star_filter);

    char const *q = x;
    int len = global_m->match_to_end(q);
    if (len >= 0)
    {
        if (global_ggg->gl_matchc == buf_len)
        {
            buf_len += NEWSIZE;
            global_ggg->gl_pathv = (char **) (global_ggg->gl_pathv ?
                                   psetrealloc((char *) global_ggg->gl_pathv, buf_len * sizeof(char *)) :
                                   psetmalloc(buf_len * sizeof(char *)));
        }
        global_ggg->gl_pathv[global_ggg->gl_matchc++] = strdup(x);
    }
    return 1;
}

extern char const *proj_get_current_save ();

static void collect_2_star_files_thru_links(char const *full_path, char const *prefix, char const *ptn, glob_t &ggg, int no_link_flag)
{
    Initialize(collect_2_star_files_thru_links);

    // OSapi_fprintf(stderr,"full_path = (%s) prefix = (%s) ptn = (%s)\n", full_path, prefix, ptn);

    genString new_full_path;
    if (strlen(prefix) == 0)
        new_full_path = full_path;
    else
        new_full_path.printf("%s/%s",full_path,prefix);

    genTmpfile tnm("repot");

    //boris: sort find output when we save .prj file. Otherwise files on the top
    //       level are saved after all directories

    int do_sort = 0;
    if (proj_get_current_save())
	do_sort = 1;

#ifndef _WIN32

    shell_cd_find (tnm.name(), new_full_path, ".", do_sort);

    FILE *in = tnm.open_for_read();
    char line[MAXPATHLEN];
    while(fgets(line,MAXPATHLEN-1,in))
    {
        line[MAXPATHLEN-1] = '\0';
        line[strlen(line)-1] = '\0';
        if ( line[strlen(line)-1] == 0x0d )
          line[strlen(line)-1] = '\0';
        if (strcmp(line,".") == 0)
            continue;
        genString path;
        if (strlen(prefix) == 0)
            path = line+2;
        else
            path.printf("%s/%s", prefix, line+2);

        genString filename;
        filename.printf("%s/%s",full_path,path.str());

        struct OStype_stat buf;
        if (OSapi_lstat(filename, &buf) == 0)
        {
            if (S_ISLNK(buf.st_mode))
            {
                if (OSapi_stat(filename, &buf) != 0)
                    continue;
                if (S_ISDIR(buf.st_mode))
                {
                    if (old_inode(&buf)) // had visited
                        continue;
                    collect_2_star_files_thru_links(full_path, path, ptn, ggg, no_link_flag);
                }
                if (no_link_flag)
                    continue;
            }
            if (leading_char(ptn,'/'))
                path = filename.str();
            two_star_filter(path);
        }
    }
    fclose(in);
#else

    shell_cd_find (tnm.name(), new_full_path, ".", do_sort);

    FILE *in = tnm.open_for_read();
    char line[MAXPATHLEN+1];
    int  iName = strlen( new_full_path.str() ); 
    while(fgets(line,MAXPATHLEN-1,in))
    {
        line[MAXPATHLEN-1] = '\0';
        line[strlen(line)-2] = '\0';
        
        two_star_filter( convert_to_uniform_case(convert_back_slashes( &(line[iName]) )) );
    }
    fclose(in);
#endif
}

static void collect_2_star_files(char const *full_path, char const *ptn, glob_t &ggg, int no_link_flag)
// find all file for a pattern that contains **
//
// full_path: is the full_path generated so far
//
// ptn: is the file name pattern
//
// ggg: is the output parameter that contains all matched files
//
// no_link_flag: set if it does not need link. After return, no stat() will be called, which will be faster.
//               if not set, "-type f" will not be issued.
{
    Initialize(collect_2_star_files);

    buf_len = 0;
    ggg.gl_pathv = 0;
    ggg.gl_matchc = 0;
    global_ggg = &ggg;
    init_link_inode_heads();

    global_m = (WildPair *)db_new(WildPair, (ptn));

    genString new_full_path;
    if (leading_char(ptn, '/'))
    {
        new_full_path = ptn;
        char *p = (char *)new_full_path.str();
        p++;
        while(1)
        {
            char *q = strchr(p,'/');
            if (q)
                *q = '\0';
            if (need_expand_dir(p))
            {
                *(p-1) = '\0';
                break;
            }
            if (q)
            {
                *q = '/';
                p = q+1;
                if (*p == '\0')
                {
                    *(p-1) = '\0';
                    break;
                }
            }
            else
                break;
        }
        p = (char *)new_full_path.str();
        if (*p == '\0')
            new_full_path = "/"; // bad: it is /*/../**/.. or /**/..
    }
    else
        new_full_path = full_path;

    struct OStype_stat buf;
    if (OSapi_stat((char const *)new_full_path.str(), &buf) == 0)
        old_inode(&buf);

    collect_2_star_files_thru_links(new_full_path, "", ptn, ggg, no_link_flag);
    delete global_m;

    global_m = 0;
}

int create_clean_ptn_str(wkPtnDef *ptn_strs, genString &dir_ptn)
// construct the whole physical path
// if the path contains "|", the collected path needs to check if it matches to the pattern
//    It will return 1; otherwise, it will return 0.
//
// ptn_strs: is a link list that contains the definition for this physical path
// dir_ptn: contains the constructed path
{
    Initialize(create_clean_ptn_str);

    dir_ptn = 0;
    int need_match = 0;
    while(ptn_strs)
    {
        char const *p =  ptn_strs->get_str();
        if (strchr(p,'|'))
        {
            if (strstr(p,"**"))
                dir_ptn += "**";
            else
                dir_ptn += "*";
            need_match = 1;
        }
        else
            dir_ptn += ptn_strs->get_str();
        ptn_strs = ptn_strs->get_next();
    }
    return need_match;
}

int collect_file_names(glob_t &ggg, char const *full_path, projMap *pm, int /* targ_type */ )
// targ_type could be FILE_TYPE or DIR_TYPE
//
// ggg: is the buffer that contains the found file names
// full_path: is the directory where pm stands
// pm: is the projMap which is being expanded
{
    Initialize(collect_file_names);

    int no_stat_call = 0;
    ggg.gl_matchc = 0;

    if (pm && pm->is_exclude_proj())
        return 0;
    genString phy;
    int need_match = 0;
    if (pm)
        need_match = create_clean_ptn_str(pm->get_phy_rule(0)->get_ptn_strs(), phy);
    else
        phy = "*";

    glob_t tggg;
    if (phy.length() && strstr(phy.str(),"**"))
    {
        int no_link = pm->is_reg_file();
        if (need_match)
            collect_2_star_files(full_path, phy, tggg, no_link); 
        else
            collect_2_star_files(full_path, phy, ggg, no_link);
        if (no_link)
            no_stat_call = 1;
    }
    else if (need_match)
        proj_ls(full_path, phy, &tggg);
    else
        proj_ls(full_path, phy, &ggg);

    if (need_match)
    {
        ggg.gl_pathv = 0;
        ggg.gl_matchc = 0;
        if (tggg.gl_matchc)
        {
            char const *p = pm->get_dir_name();
            WildPair *m  = (WildPair *)db_new(WildPair, (p));
            ggg.gl_pathv = (char **)psetmalloc(tggg.gl_matchc * sizeof(char *));
            for(int i=0;i<tggg.gl_matchc;i++)
            {
                char const *x = tggg.gl_pathv[i];
                int len = m->match_to_end(x);
                if (len >= 0)
                    ggg.gl_pathv[ggg.gl_matchc++] = strdup(x);
            }

            delete m;
        }
        report_globfree(tggg);
        global_ggg = &ggg;
    }

#ifdef _WIN32
  if ( !g_DisFN_RealCase ) // convert path to lowcase
  {
    for(int n=0;n<ggg.gl_matchc;n++)
      convert_to_uniform_case( ggg.gl_pathv[n] );
  }
#endif
    return no_stat_call;
}
// list files -- ends

enum
{
    GEN_ALL_LEVELS,
    GEN_ONE_LEVEL
};

static char start_with_slash = 0;

int count_level(char const *dir_ptn)
{
    Initialize(count_level);

    if (*dir_ptn == '/')
    {
        start_with_slash = 1;
        dir_ptn++;
    }
    else
        start_with_slash = 0;

    int level = 0;

    while(1)
    {
        level++;
        char const *p = strchr(dir_ptn,'/');
        if (p)
            dir_ptn = p + 1;
        else
            break;
    }
    return level;
}

bool need_expand_dir(char const *ptn)
// check if ptn contains any wild card characters "*?[]|"
{
    Initialize(need_expand_dir);

    while(*ptn)
    {
        if (*ptn == '\\')
        {
            ptn++;
            if (*ptn == '\0')
                break;
        }
        else if (*ptn == '*' || *ptn == '?' || *ptn == '[' || *ptn == ']' || *ptn == '|')
            return TRUE;
        ptn++;
    }
    return FALSE;
}


static void gen_all_dirs(projMap *pm, char const *full_path)
// generate all paths that matched the physical definition in the projMap pm
// full path is the physical directory where pm applies
{
    Initialize(gen_all_dirs);

    if (pm && (pm->is_exclude_proj() || (!pm->get_view_flag() && pm->get_parent_project())))
        return;

    if (pm && pm->get_parent_project() && pm->is_leading_slash_in_phy_path())
        full_path = "/";

    glob_t ggg;
    collect_file_names(ggg, full_path, pm, DIR_TYPE);
    for(int i=0;i<ggg.gl_matchc;i++)
    {
        genString new_full_path;
        combine_dir_and_sub_dir(full_path, ggg.gl_pathv[ggg.gl_matchc-1-i], new_full_path);

        struct OStype_stat buf;
        if (OSapi_stat(new_full_path, &buf) == 0)
        {
            if (S_ISDIR(buf.st_mode))
                pm->get_ret_dir()->grow(1)->dir_name = strdup(ggg.gl_pathv[ggg.gl_matchc-1-i]);
        }
    }
    report_globfree(ggg);
}

// BEGIN WORKING AREA FOR THE MULTIPLE MATCHES FOR THE IMPLICIT_SUB_DIR PROJECT HEADS
typedef struct
{
    projMap *h;
} proj_head_entry;

genArr(proj_head_entry);

static genArrOf(proj_head_entry) multi_proj_heads;

static void init_multi_proj_heads()
{
    Initialize(init_multi_proj_heads);

    multi_proj_heads.reset();
}

static void create_multi_match_heads(projMap *proj_head, char const *s)
{
    Initialize(create_multi_match_heads);

    if (proj_head == 0)
        return;

    char const *save_s = s;
    // if (proj_head->is_project()) // need for module also
    {
        genString r;
        update_the_using_rule(r, "", proj_head);

        if (strncmp(s,r,strlen(r)) == 0)
        {
            s += strlen(r);
            if (*s == '\0')
            {
                proj_head_entry *p = multi_proj_heads.grow();
                p->h = proj_head;
            }
            else if (strncmp(s,RULE_SEPARATOR,strlen(RULE_SEPARATOR)) == 0)
                create_multi_match_heads(proj_head->get_child_project(), s);
        }
    }
    create_multi_match_heads(proj_head->get_next_project(), save_s);
}

// END WORKING AREA FOR THE MULTIPLE MATCHES FOR THE IMPLICIT_SUB_DIR PROJECT HEADS

// BEGINNING for flat project

void pdf_flat_init_proj_head()
// clean the working buffer for flattening projects
{
    Initialize(pdf_flat_init_proj_head);

    init_multi_proj_heads();
}

void pdf_flat_add_proj_head(projMap *cur)
// add one projmap into the working buffer for flattening projects
{
    Initialize(pdf_flat_add_proj_head);

    proj_head_entry *p = multi_proj_heads.grow();
    p->h = cur;
}

int pdf_flat_size_of_proj_head()
// return the size of the working buffer for flattening projects
// It is the number of flattened projmaps.
{
    Initialize(pdf_flat_size_of_proj_head);

    return multi_proj_heads.size();
}

projMap *pdf_flat_proj_head_by_idx(int idx)
// return the projmap from the working buffer by index into the buffer for flattening projects
{
    Initialize(pdf_flat_proj_head_by_idx);

    Assert(idx >= 0 && idx < multi_proj_heads.size());

    proj_head_entry *p = multi_proj_heads[idx];
    return p->h; // return the first one
}

// ENDING for flat project

static projMap *match_proj_name(projMap *proj_head, char const *s, int project_only = 0);
static projMap *match_proj_name(projMap *proj_head, char const *s, int project_only)
{
    Initialize(match_proj_name);

    if (!proj_head)
        return 0;

    if (!project_only || proj_head->is_project()) // need for module also
    {
        genString r;
        update_the_using_rule(r, "", proj_head);

        if (strncmp(s,r,strlen(r)) == 0)
        {
            char const *old_s = s;

            s += strlen(r);
            if (*s == '\0')
                return proj_head;

            if (strncmp(s,RULE_SEPARATOR,strlen(RULE_SEPARATOR)))
                return(match_proj_name(proj_head->get_next_project(), old_s, project_only));

            if (proj_head->get_child_project())
            {
                if (proj_head->is_implicit_sub_dir_proj())
                {
                    if (proj_head->is_imp_leaf())
                        return proj_head;
                    else
                    {
                        // All the children NODES are created ==> TOUCH ONES
                        create_multi_match_heads(proj_head->get_child_project(),s);
                        return 0;
                    }
                }

                return(match_proj_name(proj_head->get_child_project(), s, project_only));
            }
            else
            {
                if (proj_head->is_implicit_sub_dir_proj())
                    return(proj_head); // = sign for this project
                return 0;
            }
        }
    }

    if (proj_head->get_next_project())
        return(match_proj_name(proj_head->get_next_project(), s, project_only));
    else
        return 0;
}

projMap *match_one_proj_name(projMap *proj_head, char const *s)
{
    Initialize(match_one_proj_name);

    init_multi_proj_heads(); // clean the working buffer

    projMap *leaf_proj = match_proj_name(proj_head, s);
    if (leaf_proj)
        return (leaf_proj);

    if (multi_proj_heads.size())
    {
        proj_head_entry *p = multi_proj_heads[0];
        return p->h; // return the first one
    }

    return 0;
}

static void pn_phy_path(projNode *pn, genString &s)
{
    Initialize(pn_phy_path);

    char const *proj_name = pn->get_ln();
    while(pn && !pn->get_map())
        pn = pn->find_parent();

    if (pn == 0) // control project /tmp
    {
        s = 0;
        return;
    }

    projMap *proj_head = pn->get_map();
    proj_head->ln_to_fn_imp(proj_name, proj_name, DIR_TYPE, s, 0, 1);
}

projMap *get_projmap_for_projnode(projNode *pn)
{
    Initialize(get_projmap_for_projnode);

    genString full_path;
    pn_phy_path(pn, full_path);

    genString s = projMap::report_path_tran_rule();

    while(pn && !pn->get_map())
        pn = pn->find_parent();

    if (pn == 0) // control project /tmp
        return 0;

    projMap *proj_head = pn->get_map();

   // boris: 041498, bug #15017, only projects PLEASE
    return match_proj_name(proj_head, s, 1);
}

 int num_of_fail_get_log_name = 0;

void my_make_module(char const *full_path, projNode *pn, projMap *proj_head, int force_refresh_flag, int list_proj);


static void report_all_modules_of_one_directory(projNode *pn, projMap *pm, char const *full_path, char proj_type,
                                                char level_flag, int force_refresh_flag)
// If report_all_modules_of_one_directory() is called from report_one() by a member project,
// ignore the return of module_path_is_reported().
//
// pn: the projnode where it stands
// pm: the projmap which is used
// full_path: the directory where pn and pm stand
// proj_type: PROJECT or MEMBER; if it is MEMBER, it will create a new module.
// level_flag: GEN_ALL_LEVELS or GEN_ONE_LEVEL. GEN_ALL_LEVELS is not being used.
// force_refresh_flag: it is called to force refresh the project node again; for example by File->"Update Projects"
{
  Initialize(report_all_modules_of_one_directory);

  if (pm && (pm->is_exclude_proj() || (!pm->get_view_flag() && pm->get_parent_project())))
    return;

  if (pm && pm->get_parent_project() && pm->is_leading_slash_in_phy_path())
    full_path = "/";

  glob_t ggg;
  int no_stat_call = collect_file_names(ggg, full_path, pm, FILE_TYPE);
  char file_type = 0;

  if (no_stat_call)
    file_type = FILE_TYPE;

  for (int i = 0; i < ggg.gl_matchc ; ++i) {
    genString new_full_path;
    combine_dir_and_sub_dir(full_path, ggg.gl_pathv[i], new_full_path);
    convert_to_uniform_case((char const *) new_full_path.str());

    if (!no_stat_call) {
      struct OStype_stat buf;
      genString t_path = new_full_path.str();
      file_type = 0;

      if (OSapi_stat(t_path, &buf) == 0) {
	if (S_ISREG(buf.st_mode))
	  file_type = FILE_TYPE;
      }
    }

    if (file_type && (proj_type == MEMBER || level_flag == GEN_ALL_LEVELS)) {
      int file_type = pm->get_file_type();

      if (no_stat_call)
	pm->set_file_type(DEFAULT_TYPE);

      my_make_module(new_full_path, pn, pm, force_refresh_flag, 0);

      if (no_stat_call)
	pm->set_file_type(file_type);
    }
  }

  report_globfree(ggg);
}

static void one_item_for_this_list_project(projNode *pn, projMap *pm, char const *dir, char const *line, int force_refresh_flag)
// create one module for this script project which is flattern from now on
//
// pn: projnode
// pm: projmap for this projnode
// dir: the directory where this projnode stands
// line: the physical name for this module
// force_refresh_flag: 0 original run; 1 force_refresh run
{
    Initialize(one_item_for_this_list_project);

    genString full_path;

    if (leading_char(line,'/'))
        full_path = line;
    else
        full_path.printf("%s/%s",dir,line);

    genString r_path;
    project_convert_filename((char const *)full_path.str(), r_path);

    genString ln;
    projNode *pno = 0;
    pdf_tree_fn_to_ln(r_path, ln, &pno);
    if (ln.length())
    {
        projNode *par = pno;
        while(par)
        {
            pno = par;
            par = par->find_parent();
        }
        if (pm != pno->get_map())
            pm->fn_to_ln(r_path,pn->get_ln(),ln);
    }
    if (ln.length())
        my_make_module(r_path, pn, pm, force_refresh_flag, 1);
    else if (force_refresh_flag)
    {
        pn->fn_to_ln(r_path, ln, FILE_TYPE, 0, 1);
        if (ln.length() && found_old_dir(ln,DD_MODULE) == 0)
            my_make_module(r_path, pn, pm, force_refresh_flag, 1);
    }
}

static genTmpfile*  tnm_prepared;

static void construct_list_tree(projNode *pn, projMap *pm, char const *s, char const *dir, int force_refresh_flag)
// construct the whole tree: modules for a project which is defined as a script project
// (( such as atria local project ))
//
// pn: projnode
// pm: projmap for this projnode
// s: the defined script
// dir: the directory where this projnode stands
// force_refresh_flag: 1 for force refresh; 0 for the regular case
{
    Initialize(construct_list_tree);

    genTmpfile* tnm = NULL;
    genString lockfn;

    if (tnm_prepared) {
	tnm = tnm_prepared;
	lockfn = tnm_prepared->name();
	lockfn += ".lock";
	int count = 0;
	while (OSapi_access((char const *)lockfn, F_OK)) {
	    OSapi_sleep(1);
	    count++;

	    // display message on the progress bar
	    if (count == 2)
	      mpb_info("Executing script selector in PDF...");
	    else
	      mpb_refresh();

	    if (count >= 7200) {
		// two hours
		msg("ERROR: No file mapping for logical name $1 in project $2") << s << eoarg << " timeout" << eom;
		driver_exit(1);
	    }
	}
    } else {
	Initialize(construct_list_tree_vsystem);
	tnm = new genTmpfile ("repot");
	int tmp_stat = shell_cd_exec_redirect (tnm->name(), dir, s, NULL);
    }

    FILE *in = tnm->open_for_read();
    char line[256];
    while(fgets(line,255,in))
    {
        char *p = line;
        while(1)
        {
            // remove white space
            // leading spaces
            while(*p)
            {
                if (!isspace(*p))
                    break;
                p++;
            }
            if (*p == '\0')
                break;

            char *q = p;
            while(*q)
            {
                if (isspace(*q))
                    break;
                q++;
            }
            char tmp = *q;
            *q = '\0';
	    trn_path_2DIS(p);
            if (strncmp(p,"./",2) == 0)
                p += 2;
            DBG
            {
                msg("($1)") << line << eom;
            }
            one_item_for_this_list_project(pn, pm, dir, p, force_refresh_flag);
            *q = tmp;
            p = q;
        }
    }
    tnm->close();

    tnm->unlink();
    if (lockfn.length())
	OSapi_unlink((char const *)lockfn);
    delete tnm;
    if(tnm_prepared)
	tnm_prepared = NULL;
    pn->update_all_hash_tables(pm);
    mpb_info("Loading DISCOVER...");
}

static void prepare_construct_list_tree(projNode *pn, projMap *pm, char const *s, char const *dir, int force_refresh_flag)
{
    tnm_prepared = new genTmpfile ("repot");
    genString tfn = tnm_prepared->name();
    genString lockfn = tfn;
    lockfn += ".lock";
    (void)shell_cd_exec_touch_background (dir, s, tfn, lockfn);
}

static projNode *my_make_project(char const *name, projNode *parent, char const *dir, projMap *pm, char const *full_proj_name,
                                 char level_flag, int force_refresh_flag, int bad_file_name)
// create a projnode
//
// name: the physical path just for this level
// parent: the parent for this node. It is 0, if this is a top project.
// dir: the physical path for this node
// pm: the projmap for this node
// full_proj_name: the full project name to this node
// level_flag: GEN_ONE_LEVEL only for this level; GEN_ALL_LEVELS for all levels and requires to go recursively down
//             to leaf level (file/module)
// force_refresh_flag: 0 original run; 1 refresh run.
// bad_file_name = 1: only happens when the invisible project contains bad physical path and all children"s
//                    physical paths have leading path and all children are also invisible
{
    Initialize(my_make_project);
    if (pm && !pm->match_file_type(dir,DIR_TYPE)) // file type could be [[ d ]] or [[ D ]] or [[ dD ]]
        return NULL;

    // top level exclude projects
    if (!parent && exclude_this_top_one(dir, pm)) {
	DBG {
	    msg("PDF DEBUG: EXCLUDED project = $1 dir = $2") << setw(10) << name << eoarg << dir << eom;
	    ;
	}
        return NULL;
    }

    if (!bad_file_name) {
        genString ln;
        projMap *par = pm;
        while(par->get_parent_project())
            par = par->get_parent_project();
        par->fn_to_ln(dir,full_proj_name,DIR_TYPE,ln);
        if (!ln.length()) {
	    DBG {
		msg("PDF DEBUG: BAD NAME project = $1 dir = $2") << setw(10) << name << eoarg << dir << eom;
		;
	    }
            return NULL;
	}
    }

    if (force_refresh_flag) {
        old_dir_entry *q = found_old_dir(full_proj_name,DD_PROJECT);
        if (q) {
	    DBG {
		msg("PDF DEBUG: FOUND OLD project = $1 dir = $2") << setw(10) << name << eoarg << dir << eom;
		;
	    }
            return (projNode *)q->node;
	}
    }

    if (!stop_timing) {
        proj_count++; // estimate time for refresh
        if (pm->is_script() || pm->is_filter())
            script_proj_count++;
    }

    DBG {
	msg("PDF DEBUG: MAKING project = $1 dir = $2") << setw(10) << name << eoarg << dir << eom;
	;
    }

    projNode *p = NULL;
    int pmod_type = (pm) ? pm->get_pmod_type() : PMOD_UNKNOWN;
    if (!parent) {
        if (do_duplicate_checking == 1) {
            top_proj_head_entry *pp = top_proj_heads.grow();
            pp->name           = strdup(name);
            pp->dir            = strdup(dir);
            pp->pm             = pm;
            pp->full_proj_name = strdup(full_proj_name);
	    DBG {
		msg("PDF DEBUG: DUPLICATE project = $1 dir = $2") << setw(10) << name << eoarg << dir << eom;
		;
	    }
            return NULL;
        }

        p = proj_make_project(name, parent, dir, pm, full_proj_name, pmod_type);
    } else
        p = proj_make_project(name, parent, dir, 0, full_proj_name, pmod_type);

    if (force_refresh_flag)
        p->update_projnode();

    if (level_flag == GEN_ALL_LEVELS) {
        if (pm && pm->is_leaf())
            report_all_modules_of_one_directory(p, pm, dir, PROJECT, level_flag, force_refresh_flag);
    }
    return p;
}

void construct_list_projects()
{
    Initialize(construct_list_projects);

    log_start_time();
    
    int break_val = 3, val = 1,incr_val = 1, num_incs = 0;
    mpb_incr_values_init(break_val, val, incr_val);
    projNode *proj;
    for (int i=1; proj = projList::search_list->get_proj(i); i++)
    {
        projMap *pm = proj->get_map();
        if (pm && pm->is_script())
        {
            genString s;
            pm->get_script_text(proj, s);
            if (s.length())
            {
                init_module_check_tree();
                genString dir;
                proj->ln_to_fn_imp(proj->get_ln(), dir, DIR_TYPE);
                construct_list_tree(proj, pm, s, dir, 0);
            }
        }
	mpb_step(break_val, incr_val, val, num_incs, i);
    }
    log_complete_time(1);
}

void prepare_construct_list_projects()
{
    projNode *proj;
    for (int i=1; proj = projList::search_list->get_proj(i); i++)
    {
        projMap *pm = proj->get_map();
        if (pm && pm->is_script())
        {
            genString s;
            pm->get_script_text(proj, s);
            if (s.length())
            {
                genString dir;
                proj->ln_to_fn_imp(proj->get_ln(), dir, DIR_TYPE);
                prepare_construct_list_tree(proj, pm, s, dir, 0);
		break;
            }
        }
    }
}

// forward declaration
static void report_one_and_next(projMap *proj_head, projNode *parent, char const *full_path, char const *full_proj_name,
                       char file_type, char level_flag, int force_refresh_flag);

static void report_one_imp_sub(projMap *pm, projNode *parent, char const *full_path, char const *full_proj_name,
                               char file_type, char level_flag, projMap *filter, int force_refresh_flag)
// report one implicit project "=" which will be obsolete in future
//
// pm: the projmap which is used
// parent: the parent projnode
// full_path: the directory where pm stands
// full_proj_name: the full project name where pm stands
// file_type: DIR_SUB or MODULE_SUB
// level_flag: GEN_ALL_LEVELS or GEN_ONE_LEVEL. GEN_ALL_LEVELS is not being used.
// filter: filter for the file names
// force_refresh_flag: it is called to force refresh the project node again; for example by File->"Update Projects"
{
    Initialize(report_one_imp_sub);

    if (pm && (pm->is_exclude_proj() || (!pm->get_view_flag() && pm->get_parent_project())))
        return;

    if (pm && pm->get_parent_project() && pm->is_leading_slash_in_phy_path())
        full_path = "/";

    glob_t ggg;
    collect_file_names(ggg, full_path, 0, FILE_TYPE);
    for(int i=0;i<ggg.gl_matchc;i++)
    {
        genString new_full_path;
        combine_dir_and_sub_dir(full_path, ggg.gl_pathv[ggg.gl_matchc-1-i], new_full_path);

        struct OStype_stat buf;
        char my_file_type = 0;
        if (OSapi_stat( new_full_path, &buf) == 0)
        {
            if (S_ISREG(buf.st_mode))
                my_file_type = FILE_TYPE;
            else if (S_ISDIR(buf.st_mode))
                my_file_type = DIR_TYPE;
        }
        if (my_file_type == FILE_TYPE && (file_type & MODULE_SUB))
        {
            bool do_it = 1;
            if (filter)
            {
                genString ret_path;
                filter->get_matched_name(new_full_path, ret_path, FILE_TYPE);
                if (!ret_path.length())
                    do_it = 0;
            }
            if (do_it)
                my_make_module(new_full_path, parent, pm, force_refresh_flag, 0);
        }
        else if (my_file_type == DIR_TYPE && (file_type & DIR_SUB))
        {
            genString new_full_proj_name;
            combine_dir_and_sub_dir(full_proj_name, ggg.gl_pathv[ggg.gl_matchc-1-i], new_full_proj_name);
            projNode *pn = my_make_project(ggg.gl_pathv[ggg.gl_matchc-1-i], parent, new_full_path, pm,
                                           new_full_proj_name, level_flag, force_refresh_flag, 0);
            if (level_flag == GEN_ALL_LEVELS)
                report_one_imp_sub(pm, pn, new_full_path, new_full_proj_name, file_type, level_flag, filter,
                                   force_refresh_flag);
        }
    }
    report_globfree(ggg);
}

bool file_project(projMap *proj_head, char const *full_path, 
	char construct_new_path)
// If the project contains only one file, it will return true.
// Otherwise, it will return FALSE;
//
// proj_head: is the checked projMap
// full_path: is the directory where this projMap stands
// construct_new_path: if set, the tested path will be full_path + the physical path in this projMap
//                     if not set, the tested path will be full_path
{
    Initialize(file_project);

    if (proj_head->is_leaf())
    {
        wkPtnRule *rule = proj_head->get_phy_rule(0);
        wkPtnDef *cur = rule->get_ptn_strs();
        // If the physical path does not exist, cur will be null.
        if (!cur || cur->get_next() || cur->get_type() != PTN_CONST_STRING)
            return FALSE;

        genString new_full_path;
        if (construct_new_path)
            combine_dir_and_sub_dir(full_path, cur->get_str(), new_full_path);
        else
            new_full_path = full_path;
        struct OStype_stat buf;
        if (OSapi_stat(new_full_path, &buf) == 0)
        {
            if (S_ISREG(buf.st_mode))
                return TRUE;
        }
    }
    return FALSE;
}

static bool all_children_invisible_and_leading_slash(projMap *proj_head)
{
    Initialize(all_children_invisible_and_leading_slash);

    if (proj_head->get_view_flag() || proj_head->some_child_no_leading_slash())
        return FALSE;

    if (proj_head->get_child_project())
    {
        bool status = all_children_invisible_and_leading_slash(proj_head->get_child_project());
        if (!status)
            return FALSE;
    }
    if (proj_head->get_next_project())
    {
        bool status = all_children_invisible_and_leading_slash(proj_head->get_next_project());
        if (!status)
            return FALSE;
    }
    return TRUE;
}

static void report_one(projMap *proj_head, projNode *parent, char const *full_path, char const *full_proj_name,
                       char file_type, char level_flag, int force_refresh_flag)
// try to expand one projMap
//
// proj_head: the projMap to be expanded
// parent: the proj_head belongs to
// full_path: the directory where proj_head stands
// full_proj_name: the proj_name till parent
// file_type: MODULE_SUB | DIR_SUB, MODULE_SUB, or DIR_SUB
// level_flag = GEN_ALL_LEVELS: expand to the leaf
//                            : Otherwise, expand this one only
// force_refresh_flag = 1: if called by file->"refresh projects"
//                    = 0: normal expansion.
{
    Initialize(report_one);

    if (proj_head->is_file() && (file_type & MODULE_SUB))
        report_all_modules_of_one_directory(parent, proj_head, full_path, MEMBER, level_flag, force_refresh_flag);
    else if (proj_head->is_left_way())
        ; // do nothing
    else if (file_project(proj_head, full_path, 1)) // 1: need new full_path
    {
        wkPtnRule *rule = proj_head->get_phy_rule(0);
        wkPtnDef *cur = rule->get_ptn_strs();
        genString new_full_path;
        combine_dir_and_sub_dir(full_path, cur->get_str(), new_full_path);

        if (parent) // parent exists, use the regular way.
            my_make_module(new_full_path, parent, proj_head, force_refresh_flag, 0);
        // else // Should not come here. The parser has reported this error.
    }
    else if (!proj_head->is_file())
    {
        if (leading_char(proj_head->get_dir_name(),'/'))
            start_with_slash = 1;
        else
            start_with_slash = 0;

        proj_head->reset_ret_dir();

        if (start_with_slash)
            gen_all_dirs(proj_head, "/");
        else
            gen_all_dirs(proj_head, full_path);

        projNode *p;

        // if an invisible project contains a bad physical path and all its children contain a leading slash
        // in their phytsical path and all children are invisible
        if (!proj_head->get_ret_dir()->size())
        {
            if (all_children_invisible_and_leading_slash(proj_head))
	    {
		genString new_full_proj_name;
		combine_dir_and_sub_dir(full_proj_name, proj_head->get_org_project_name(), new_full_proj_name);
                p = my_make_project(proj_head->get_org_project_name(), parent, proj_head->get_org_dir_name(),
                                    proj_head, new_full_proj_name, level_flag, force_refresh_flag, 1);
	    }
        }
        else
        for (int i = proj_head->get_ret_dir()->size() - 1; i >= 0; i--)
        {
            char const *dir_n = (*proj_head->get_ret_dir())[i]->dir_name;
            genString proj_n;
            proj_head->get_matched_name(dir_n, proj_n, 0);
            if (proj_n.length() == 0)
                continue;

            genString new_full_path;
            combine_dir_and_sub_dir(full_path, dir_n, new_full_path);
            genString new_full_proj_name;
            combine_dir_and_sub_dir(full_proj_name, (char const *)proj_n.str(), new_full_proj_name);

	    if (file_type & DIR_SUB) {
		if (proj_head->get_view_flag() || proj_head->get_parent_project() == 0)
		    p = my_make_project((char const *)proj_n.str(), parent, new_full_path, proj_head,
					new_full_proj_name, level_flag, force_refresh_flag, 0);

		if (proj_head->get_child_project() && level_flag == GEN_ALL_LEVELS)
		    report_one(proj_head->get_child_project(), p, new_full_path, new_full_proj_name,
			       file_type, level_flag, force_refresh_flag);
	    }
        }

        // If it fails to find dir in this level, and you want to stop it.
        // Remove the next two statements.
        if (proj_head->get_ret_dir()->size() == 0 && level_flag == GEN_ALL_LEVELS) {
	    wkPtnDef *cur_ptn = proj_head->get_phy_rule(0)->get_ptn_strs();

	    // If it has next node, it needs matching.
	    // It comes here, because it fails to get any matched directories.
	    if (cur_ptn->get_next() == 0) {
		projMap *child = proj_head->get_child_project();

		while(child) {
		    if (child->is_project()) {
			wkPtnDef *child_cur_ptn = child->get_phy_rule(0)->get_ptn_strs();

			// If the child starts a root
			if (child_cur_ptn->get_type() == PTN_CONST_STRING && *child_cur_ptn->get_str() == '/') {
			    genString new_full_proj_name;
			    combine_dir_and_sub_dir(full_proj_name, cur_ptn->get_str(), new_full_proj_name);
			    report_one(child, p, "", new_full_proj_name, file_type, level_flag, force_refresh_flag);
			}
		    }

		    child = child->get_next_project();
		}
	    }
        }
    }
}

static void report_one_and_next(projMap *proj_head, projNode *parent, char const *full_path, char const *full_proj_name,
                       char file_type, char level_flag, int force_refresh_flag) 
{
    Initialize(report_one_and_next);
    report_one(proj_head, parent, full_path, full_proj_name, file_type, level_flag, force_refresh_flag);

    // If it is the top level project,
    // forget the next one, because it will be called one by one.
    if (proj_head->get_parent_project() == 0)
	return;

    while (proj_head = proj_head->get_next_project()) {
	report_one(proj_head, parent, full_path, full_proj_name, file_type, level_flag, force_refresh_flag);
    }
}

static void create_gen_subs_error(char const *full_proj_name, int num_of_fail_get_log_name)
{
    Initialize(create_gen_subs_error);

    projMap::reset_parser_err();

    genString err_msg;

    err_msg.printf("Could not find a logical name for %d files in project \"%s\"",
                   num_of_fail_get_log_name,full_proj_name);

    projMap::add_one_parser_error(1, err_msg);
}

static void generate_subs(projNode *pn, projMap *proj_head, char type, char level_flag,
                          int force_refresh_flag)
{
    Initialize(generate_subs);

    if (!proj_head->get_view_flag())
        return;

    genString full_path;
    pn_phy_path(pn, full_path);

    IF (full_path.length() == 0)
        return;

    num_of_fail_get_log_name = 0;
    if (proj_head->is_leaf() || proj_head->is_imp_leaf())
    {
        if ((proj_head->is_implicit_sub_dir_proj()   ||
            proj_head->get_flag() == MEMBER) && proj_head->get_view_flag() ||
            proj_head->get_parent_project() == 0)
        {
            projMap *filter = 0;
            if (proj_head->get_child_project() && proj_head->get_child_project()->get_flag() == MEMBER)
                filter = proj_head->get_child_project();
            report_one_imp_sub(proj_head,
                               pn,
                               full_path,
                               pn->get_ln(),
                               type,
                               level_flag,
                               filter,
                               force_refresh_flag);
        }
        else
            report_all_modules_of_one_directory(pn, proj_head, full_path, PROJECT, GEN_ALL_LEVELS, force_refresh_flag);
    }
    else
    {
        report_one_and_next(proj_head->get_child_project(),
                   pn,
                   full_path,
                   pn->get_ln(),
                   type,
                   level_flag,
                   force_refresh_flag);
    }

    if (num_of_fail_get_log_name)
        create_gen_subs_error(pn->get_ln(), num_of_fail_get_log_name);
}

// force-refresh begins
// type = DD_PROJECT: node is projNode
// type = DD_MODULE:  node is projModule

genArr(old_dir_entry);

static genArrOf(old_dir_entry) old_dir_data;

static void init_old_dir_data()
{
    Initialize(init_old_dir_data);

    old_dir_data.reset();
}

static void free_old_dir_data()
{
    Initialize(free_old_dir_data);

    for (int i=0; i<old_dir_data.size(); i++)
    {
        old_dir_entry *p = old_dir_data[i];
        psetfree((char *)p->name);
    }
}

static int old_dir_cmp(const void *a, const void *b)
{
    Initialize(old_dir_cmp);

    old_dir_entry *p0 = (old_dir_entry *)a;
    old_dir_entry *p1 = (old_dir_entry *)b;

    if (p0->type == DD_PROJECT && p1->type == DD_MODULE)
        return 1;
    if (p1->type == DD_PROJECT && p0->type == DD_MODULE)
        return -1;

    return (strcmp(p0->name,p1->name));
}

static void pr_one_old_dir(old_dir_entry *p)
{
    Initialize(pr_one_old_dir);

    if (p->type == DD_PROJECT)
        msg("PROJECT ") << eom;
    else if (p->type == DD_MODULE)
        msg("MODULE  ") << eom;
    else
        msg("$1  ") << p->type << eom;

    msg("($1)") << p->name << eom;
    ;
}

// pn might not exist
static void add_itself(projNode *pn)
{
    Initialize(add_itself);

    genString phy_dir;
    pn->ln_to_fn_imp(pn->get_ln(), phy_dir, DIR_TYPE);
    struct OStype_stat buf;
    // if stat fails; not exist OR
    // not directory any more
    if (OSapi_stat(phy_dir, &buf) || !S_ISDIR(buf.st_mode))
    {
        old_dir_entry *p = old_dir_data.grow();

        p->used = 0;
        p->type = DD_PROJECT;
        p->node = pn;
        p->name = strdup(((projNode *)p->node)->get_ln());
    }
    else
        return;
    projNode *par = pn->find_parent();
    if (par)
        add_itself(par);
}

static void collect_old_dir(projNode *pn)
{
    Initialize(collect_old_dir);

    symbolArr& proj_contents = pn->rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        if (sym.get_kind() == DD_MODULE || sym.get_kind() == DD_PROJECT)
        {
            old_dir_entry *p = old_dir_data.grow();

            p->used = 0;
            p->type = sym.get_kind();
            if (sym.get_kind() == DD_PROJECT)
            {
                appPtr app_head = sym.get_def_app();
                p->node = (void *)projNodePtr(app_head->get_root());
                p->name = strdup(((projNode *)p->node)->get_ln());
            }
            else // if (sym.get_kind() == DD_MODULE)
            {
		projModule *m = checked_cast(projModule, sym);
		(void) remove_module_from_hashes (m);
                p->node = (void *)m;
                p->name = strdup(sym.get_name());
            }
        }
    }
    add_itself(pn);
    if (old_dir_data.size() > 1)
        OSapi_qsort(old_dir_data[0], old_dir_data.size(), sizeof(old_dir_entry), old_dir_cmp);

    DBG
    {
        for (int i=0; i<old_dir_data.size(); i++)
        {
            old_dir_entry *p = old_dir_data[i];
            pr_one_old_dir(p);
        }
    }
}

old_dir_entry *found_old_dir(char const *name, int type)
{
    Initialize(found_old_dir);

    if (old_dir_data.size() == 0)
        return 0;

    old_dir_entry key;
    key.type = type;
    key.name = name;
    old_dir_entry *ret = (old_dir_entry *)OSapi_bsearch(&key, old_dir_data[0], old_dir_data.size(),
                                          sizeof(old_dir_entry), old_dir_cmp);
    if (ret)
        ret->used = 1;
    return ret;
}

void remove_one_projmap(projNode *pn)
// remove the list of the root projNodes generated by this projMap
//
// pn is one of the top project nodes generated by this projMap
{
    Initialize(remove_one_projmap);

    projMap *pm = pn->get_map();
    pdf_tree_head *h = pm->obtain_pdf_tree_head();
    pn_list *pnl = h->get_pnl();
    while(pnl->get_pn() != pn)
        pnl = pnl->get_next();
    Assert(pnl);
    remove_all_pnlist(pm, h, pnl, 1);
}

static void remove_unexist(char type)
// during refreshing projects, remove the projNode trees whose directories were
// removed
{
    Initialize(remove_unexist);

    start_transaction()
    {
        for (int i=0; i<old_dir_data.size(); i++)
        {
            old_dir_entry *p = old_dir_data[i];
            if (!p->used)
            {
                if (p->type == DD_PROJECT && (type & DIR_SUB))
                {
                    projNode *pn = (projNode *) p->node;
		    if (pn->is_visible_proj())
		      {
			if (pn->get_map()) // top project
			  remove_one_projmap(pn);
			else
			  pn->delete_projnode();
		      }
                }
                else if (p->type == DD_MODULE && (type & MODULE_SUB))
                {
                    projModule *pm = (projModule *) p->node;
                    pm->invalidate_cache();
                    pm->delete_module(1);
                }
            }
        }
    }  end_transaction();
}

void proj_generate_subs_one_level_force_refresh(projNode *pn, char type)
{
    Initialize(proj_generate_subs_one_level_force_refresh);

    stop_timing = 1;
    init_old_dir_data();
    collect_old_dir(pn);
    proj_generate_subs_one_level(pn, type, 1);
    remove_unexist(type);
    free_old_dir_data();
    stop_timing = 0;
}
// force-refresh ends

void proj_generate_subs_one_level(projNode *pn, char type, int force_refresh_flag)
// generate one level of projects including modules
//
// pn: projnode where it stands
// type: MODULE_SUB | DIR_SUB, MODULE_SUB, or DIR_SUB
// force_refresh_flag: 1 for force refresh; 0 for the regular case
{
    Initialize(proj_generate_subs_one_level);

    char const *hidden_lns = pn->get_ln();
    if (hidden_lns[0] == '/')
	hidden_lns++;
    if (::strncmp(hidden_lns, "__", 2) == 0)
	return;

    if (!force_refresh_flag && proj_restore (pn) == 1)
	return;

    init_multi_proj_heads(); // clean the working buffer
    projMap *proj_head = get_projmap_for_projnode(pn);

    // If the leaf is (*)/, the next statement will be true.
    if (proj_head == 0 && multi_proj_heads.size() == 0)
        return;

    log_start_time();

    if (proj_head)
    {
        if (force_refresh_flag && proj_head->is_script())
        {
            genString s;
            proj_head->get_script_text(pn, s);
            genString dir;
            proj_head->ln_to_fn_imp(pn->get_ln(),pn->get_ln(),DIR_TYPE,dir,0,1);
            if (s.length())
                construct_list_tree(pn, proj_head, s, dir, 1);
        }
        else
            generate_subs(pn, proj_head, type, GEN_ONE_LEVEL, force_refresh_flag);
    }
    else
    {
        for(int i=0;i<multi_proj_heads.size();i++)
        {
            proj_head_entry *p = multi_proj_heads[i];
            proj_head = p->h;
            generate_subs(pn, proj_head, type, GEN_ONE_LEVEL, force_refresh_flag);
        }
    }
    if (proj_head->is_script() || proj_head->is_filter())
        log_complete_time(1);
    else
        log_complete_time(0);
}

// num = 1: only remove this one
// num = 0: remove the remain tree
static void remove_all_pnlist(projMap *pm, pdf_tree_head *h, pn_list *pnl, int num)
{
    Initialize(remove_all_pnlist);

    pn_list *cur = h->get_pnl();
    pn_list *last = 0;
    while(cur != pnl)
    {
        last = cur;
        cur = cur->get_next();
    }
    pn_list *n = 0;
    if (num == 1)
        n = pnl->get_next();
    if (last)
        last->set_next(n);
    else
    {
        h->set_pnl(n);
        if (!n)
            log_un_exist_proj_heads(pm);
    }
    pn_list *del_pnl = pnl;
    while(pnl)
    {
//      projHeaderPtr proj_hdr = checked_cast(projHeader, pnl->get_pn()->get_header());
        projList::rem_proj_from_all_lists(pnl->get_pn());
        pnl->get_pn()->delete_projnode();
//      delete proj_hdr;
        if (num == 1)
            break;
        pnl = pnl->get_next();
    }
    if (num == 1)
        del_pnl->set_next(0);
    delete del_pnl;
}

int name_cmp(const void *a, const void *b)
{
    Initialize(name_cmp);

    char const **p0 = (char const **)a;
    char const **p1 = (char const **)b;
    return strcmp((char const *)(*p0), (char const *)(*p1));
}

static pdf_tree_head *report_all_maps_by_callected_names(projMap *proj_head, int &start_idx)
// proj_head: the top projmap that generates all top projnodes
{
    Initialize(report_all_maps_by_callected_names);

    pdf_tree_head *pth_l  = 0;
    if (proj_head->get_next_project())
        pth_l = report_all_maps_by_callected_names(proj_head->get_next_project(), start_idx);

    log_start_time();
    // sort the top project names
    int count = 0;
    int i;
    for (i = start_idx; i < top_proj_heads.size(); i++)
    {
        top_proj_head_entry *pp = top_proj_heads[i];
        if (pp->pm != proj_head)
            top_proj_name_buf[count++] = pp->full_proj_name;
    }
    OSapi_qsort(top_proj_name_buf, count, sizeof(char const *), name_cmp);

    pn_list *pnl = 0;
    pn_list *pn_list_l = 0;

    char found = 0;
    int new_start_idx = start_idx;
    for (i = top_proj_heads.size() - 1; i >= start_idx; i--)
    {
        top_proj_head_entry *pp = top_proj_heads[i];
        if (pp->pm == proj_head)
        {
            found = 1;
            char const **ret = (char const **)OSapi_bsearch(&(pp->full_proj_name), top_proj_name_buf, count, sizeof(char const *), name_cmp);
            if (ret)
                continue;
	    int pmod_type = (pp->pm) ? pp->pm->get_pmod_type() : PMOD_UNKNOWN;
            projNode *p = proj_make_project(pp->name, 0, pp->dir, pp->pm, pp->full_proj_name, pmod_type);

            pn_list *pl = db_new(pn_list,(p, pp->dir));
            if (pnl == 0)
                pnl = pl;
            else
                pn_list_l->set_next(pl);
            pn_list_l = pl;
        }
        else if (!found)
            new_start_idx = i;
    }
    start_idx = new_start_idx;
    pm_list *pml = db_new(pm_list,(proj_head, 0));
    pdf_tree_head *pth = db_new(pdf_tree_head,(pnl,pml,"",pth_l,ACCEPT_LEAF,0,SEARCH_PROJ));
    if (!pnl)
        log_un_exist_proj_heads(proj_head);

    if (proj_head->is_script() || proj_head->is_filter())
        log_complete_time(1);
    else
        log_complete_time(0);

    return pth;
}

static void collect_top_proj_names(projMap *proj_head)
{
    Initialize(collect_top_proj_names);

    if (proj_head->get_next_project())
        collect_top_proj_names(proj_head->get_next_project());

    log_start_time();

    report_one(proj_head,
               0,
               "",
               "",
               DIR_SUB | MODULE_SUB,
               GEN_ONE_LEVEL,
               0);

    if (proj_head->is_script() || proj_head->is_filter())
        log_complete_time(1);
    else
        log_complete_time(0);
}

pdf_tree_head *report_all_maps(projMap *proj_head)
{
    Initialize(report_all_maps);

    /* STEP 1: collect all top project names */
    do_duplicate_checking = 1;
    init_top_proj_heads();

    collect_top_proj_names(proj_head);
    if (!top_proj_heads.size())
    { 
      msg("Discover could not start up because your source files could not be found.") << eom;
      msg("Make sure that your source files are available.") << eom; 
      return 0; 
    } 
    init_top_proj_name_buf();

    int idx = 0;
    pdf_tree_head *pth = report_all_maps_by_callected_names(proj_head, idx);

    free_top_proj_name_buf();
    free_top_proj_names();
    do_duplicate_checking = 0;

    return pth;
}

