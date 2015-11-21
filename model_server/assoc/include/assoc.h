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
#ifndef _assoc_h
#define _assoc_h

// assoc.h
//------------------------------------------
// synopsis:
// 
// Manual association implementation
//
//------------------------------------------

#ifndef _general_h
#include <general.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _objOper_h
#include <objOper.h>
#endif

#ifndef _xref_h
#include <xref.h>
#endif

#ifndef _genArrCharPtr_h
#include <genArrCharPtr.h>
#endif

// Predefine some classes
class RTL;
class gtRTL;
class objArr;

// Define associativity program types

enum AssocProgram {
   ASSOC_PRG_SOFT, 
   ASSOC_PRG_HARD, 
   ASSOC_PRG_TCL 
};


// CLASS: assocType
//
// This class describes an association type. Each association instance will
// have a unique name which represents an instance of this class

class assocType : public app
{ 
friend class assocAnalyzer;
friend class assocCreator;
friend class assocEditor;
friend class assocRemover;
friend class assocNode;
friend class change_propagator;

public:

//Define standard relational informational and constructors

   define_relational(assocType, app);
   assocType(const char *uniqe_name);
   assocType(const assocType &original);
   ~assocType();
   copy_member(assocType);

   virtual void insert_obj(objInserter *oi, objInserter *ni);
   virtual void remove_obj(objRemover *, objRemover * nr);

//Define information setting routines

   void name (const char *);
   const char *name () const;

   void category (const char *);
   const char *category () const;

   void impact (bool);
   bool impact () const;

   void program (AssocProgram, const char*);
   AssocProgram program (const char*&) const;

// Define startup initialization routine

   static void init();
   static const char* generate_symbol(const char *prefix);

   void load_type_instances (RTL*);

    virtual void send_string(ostream& stream = cout) const;

protected:
   static RTL* assoc_rtl;
   static RTL* hard_object_impact_rtl;
   static RTL* soft_object_impact_rtl;
   static RTL* file_impact_rtl;
   static class objSet* processed;

private:
   genString unique_name;   // Unique symbol name for Xref purposed

   genString uname;         // User define name of association type 
   genString cat_name;      // User defined category name
   bool impact_on; 	    // User defined flag for impact analysis on/off 
   AssocProgram propagate_flag; // hard propagation should occur
   AssocProgram program_type;// User defined type of program to follow 
			    // (SOFT/HARD for now)
   genString program_string;// The program (description for SOFT/HARD)

   static void do_notify_insert (char *, appTree*, void*, objSet*, int force_rename);

 public:
   static void assoc_hook (int op, Relational* src, Relational* trg, RelType*);
   static assocType* db_restore(char* title, byte language, char*);
   static const char *map_data (char *, appTree*);
   static const char *trim_data (const char *);
   static const char *map_data (char *, char *, genString &);  
};
generate_descriptor(assocType,app);
   
// CLASS: assocInstance
//
// This class represents the definition part for an association instance.

class assocInstance : public app
{
friend class assocNode;

public:

//Define standard relational informational and constructors

   define_relational(assocInstance, app);
   assocInstance(const char *instance_name);
   assocInstance(const assocInstance &original);
   ~assocInstance();
   copy_member(assocInstance);

   virtual void insert_obj(objInserter *oi, objInserter *ni);
   virtual void remove_obj(objRemover *, objRemover * nr);

//Define information setting routines

   void user_name (const char *);
   const char * user_name () const;

   void unique_name (const char *);
   const char * unique_name () const;

   void assoc_type_name (const char *);
   const char * assoc_type_name () const;

   void comment (const char *);
   const char * comment () const;

   void load_instance_files (RTL*);
   void load_instance_objects (RTL*);

   virtual void send_string(ostream& stream = cout) const;

   static assocInstance* db_restore(char* title, byte language, char*);

private:
   genString uniq_name;       // Unique symbol name for Xref purposed
   genString type_name;       // Unique name of association type
 
   genString name;            // User defined instance name
   genString desc;            // User defined instance description
};
generate_descriptor(assocInstance,app);


// CLASS: assocNode
//
// This class represents the root of the assocType and assocInstance

class assocNode : public appTree
{
public:

//Define standard relational informational and constructors

   define_relational(assocNode, appTree);
   assocNode();
   assocNode(const assocNode &original);
   ~assocNode();
   copy_member(assocNode);

// Given a ddElement, get the assocInstance or assocType if it exists

   static assocInstance* get_instance (ddElement*);
   static assocType*     get_type (ddElement*);

   static void db_save(app*, appTree*, db_app&, db_buffer&);
   static assocNode* db_restore(app*, appTree* parent, db_app&, db_buffer&);
};
generate_descriptor(assocNode,appTree);

/*
   START-LOG-------------------------------------------
 
   $Log: assoc.h  $
   Revision 1.8 2001/07/25 20:40:54EDT Sudha Kallem (sudha) 
   Changes to support model_server.
 * Revision 1.10  1994/03/15  14:54:21  davea
 * Bug track: 6615
 * change include "" to include <>
 *
 * Revision 1.9  1993/08/27  23:38:29  andrea
 * change arg prop proj
 *
 * Revision 1.8  1993/08/24  14:44:51  kws
 * Added trim_data function
 *
 * Revision 1.7  1993/03/08  14:27:48  aharlap
 * added phys_name argument to hd_restore ()
 *
 * Revision 1.6  1993/02/28  18:39:15  sharris
 * (bug 2715) make map_data static and available for general use
 *
 * Revision 1.5  1993/02/16  01:25:39  kws
 * Combine automatic rename and manual associativity
 *
 * Revision 1.4  1993/01/16  19:04:15  aharlap
 * added save
 *
 * Revision 1.3  1993/01/12  02:55:05  smit
 * *** empty log message ***
 *
 * Revision 1.2  1993/01/07  14:38:06  glenn
 * Added remove object definition
 *
 * Revision 1.1  1992/12/21  03:18:08  kws
 * Initial revision
 *
   END-LOG---------------------------------------------
*/
#endif // _assoc_h
