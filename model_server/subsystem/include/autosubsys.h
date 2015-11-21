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
#ifndef _autosubsys_h
#define _autosubsys_h

#include "objArr.h"
#include "ddict.h"
#include "autosubsys-weights.h"
#include <ddKind.h>
#include <xrefMapper.h>
#include <symbolPtr.h>


#ifndef _symbolSet_h
#include <symbolSet.h>
#endif

#define DORMANT_CODE 1
#define EXTRACT_SUB  2

class Entity;
class entlist;
class decomposer;
class objSet;


class entity_name_hash_entry
{
  public:
    entity_name_hash_entry(const char *path_name, Entity *e, entity_name_hash_entry *n);
    ~entity_name_hash_entry();
    entity_name_hash_entry *get_next();
    void set_next(entity_name_hash_entry *n);

    const char *get_name() { return name; }
    Entity *get_entity() { return e; }
  private:
    genString name;
    Entity    *e;
    entity_name_hash_entry *next;
};

// Class allentity manages the list of all entities in the system.

class allentity : public objArr, private xrefMapper {
  protected:
    char *name;
  public:
    allentity(char *n);
    ~allentity();
    void add(Entity *e, const symbolPtr&);
    void add_sym_alias(Entity*, const symbolPtr&);
    bool relate(); 
    int getdead(symbolArr roots, objArr* dead, weight*, int);
    Entity* item(const symbolPtr&);
    void register_alternate_symbol(Entity*, const symbolPtr&);

    void hash_file_name(genString name, Entity *e);
    void sort_filenames(int count);
    void reset_filename_hash();

  private:
    // In autosubsys.h.C: decomposer::additems()
    //     create all entities.
    //     e->filename_relation();
    //         allentities->hash_file_name(name, this);
    //     will insert all entities and their names into two hash tables.
    // ==================================================================
    //     allentities.sort_filenames(count);
    //         will sort these objects in the hash tables into two buffers.
    // ==================================================================
    //     Each entity contains four integers for these two sorted buffers
    //         int filename_prefix_entity_start_idx;
    //         int filename_prefix_entity_count;
    //         int logicname_level_entity_start_idx;
    //         int logicname_level_entity_count;
    //     If this entity does not have any name's relation with other's entities,
    //         start_idx will be -1 and count will be 0.
    //     Otherwise, start_idx will be the index into the corresponding sorted buffer
    //         and count will be the number of eneities that have same name's relation.
    //     REMEMBER, itself is in this set of entities. Hence, these two counts -1 is for
    //         the global-cohesion.
    //     For the local-cohesion, after count the entities with the same container, the
    //         returned count needs to substract 1 to exclude itself.
    // ==================================================================
    // next 2 are hash tables
    entity_name_hash_entry **filename_prefix_hash;
    entity_name_hash_entry **logicname_hash;
    // next 2 keep track of the number of entries in above tables
    int fph_count;
    int lnh_count;
    // next 2 are sorted buffers that contain the objects saved in the hash tables
    entity_name_hash_entry **filename_prefix_buf;
    entity_name_hash_entry **logicname_buf;
};

// decomposer is the main class for the whole operation.

class decomposer {
  protected:
    weight *weights;
    int nweights;
    objArr allsubsystems;
    allentity allentities;
    int threshold;
    int subsys_mode;
    RTLNode *subsys_rtl;
    symbolArr roots;
    bool already_inited;
    objArr sys_domainlist;
    objArr extraction_domainlist;
    bool extraction_domainlist_valid;
    symbolSet subsys_members;
    objSet projs_with_pmods;
  public:
    decomposer();
    ~decomposer() { use_sys_domainlist(); }
    int init();
    int additems();
    void add_subsystems();
    int setweights(weight *, int);
    int builddead();
    int work();
    int go_deadcode_only(symbolArr& roots, weight *warr, int loc, bool newUI=false,symbolArr* scope=NULL);
    Entity* item(const symbolPtr& sym);
    bool inited() const { return already_inited; }
    void use_sys_domainlist();
    void use_extraction_domainlist();
    void set_extraction_domainlist(symbolArr& selected_projects);
  private:
};

// The following four external functions are defined in ui/browser/extract-subsystems.C
extern bool Entity_need_cmp_filename_prefix();
extern bool Entity_need_cmp_logicname();
extern int  Entity_filename_prefix_count();
extern int  Entity_logicname_count();

extern void batch_subsystem_extraction(int  extract_sub_flag,
                                       char *ext_proj_name[],
                                       int  ext_proj_name_count,
                                       char *dormant_root_group,
                                       char *weight_file_name,
                                       char *group_name[],
                                       int  group_name_count);

#endif
