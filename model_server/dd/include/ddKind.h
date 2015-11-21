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
#ifndef _ddKind_h
#define _ddKind_h

#include "general.h"

/*
// ddKind.h
//------------------------------------------
// synopsis:
// Data dictionary elements, attributes
//
//------------------------------------------
*/
#ifndef ddEnum
#define ddEnum(x) enum x
#define tok(x) x
#define generate_prt_fun(x) const char* paste(x,_name)(int n);

#endif



ddEnum(ddKind)
// kinds for xrefSymbols and ddElements
{
    tok(DD_UNKNOWN),        // 0  illegal value
    tok(DD_TYPEDEF),        // 1
    tok(DD_MACRO),          // 2
    tok(DD_VAR_DECL),       // 3
    tok(DD_FIELD),          // 4
    tok(DD_FUNC_DECL),      // 5
    tok(DD_PARAM_DECL),     // 6
    tok(DD_ENUM),           // 7
    tok(DD_ENUM_VAL),       // 8
    tok(DD_CLASS),          // 9
    tok(DD_MACRO_LIST),     //10
    tok(DD_FUNC_DECL_LIST), //11
    tok(DD_VAR_DECL_LIST),  //12
    tok(DD_TYPEDEF_LIST),   //13
    tok(DD_UNION),          //14
    tok(DD_SUBSYSTEM),      //15
    tok(XXX_DD_ASSOCLINK),      //16
    tok(DD_MODULE),         //17
    tok(DD_PROJECT),        //18
    tok(DD_IFL_SRC),        //19
    tok(DD_LINK),           //20 // for internal use only
    tok(DD_IFL_TRG),        //21
    tok(XXX_DD_STYLE),          //22
    tok(DD_CATEGORY),       //23
    tok(DD_SEMTYPE),        //24
    tok(DD_SYNTYPE),        //25
    tok(DD_EXTFILE),        //26
    tok(DD_LANGUAGE),       //27
    tok(DD_UNUSED_1),       //28
    tok(DD_ROOT),           //29
    tok(DD_RELATION),       //30
    tok(DD_REL_SRC),        //31
    tok(DD_REL_TRG),        //32
    tok(DD_TEMPLATE),       //33
    tok(DD_COMPONENT),      //34
    tok(DD_BUGS), 	    //35
    tok(DD_SW_PROJ),        //36
    tok(DD_SUBMISSION),     //37
    tok(XXX_DD_ASSOC_CONST),    //38
    tok(DD_CHECKSUM),	    //39
    tok(XXX_DD_SOFT_ASSOC),	    //40
    tok(XXX_DD_SOFT_ASSOC_TYPE),//41

    //------------------------------------------
    // *** JJS 3/21/95 ***
    // definitions for Unified Linking Mechanism
    //------------------------------------------

    tok (DD_LINKSPEC),      //42
    tok (DD_LINKNODE),      //43

    //-------------------------------------------------
    // *** JJS 3/21/95 ***
    // end of definitions for Unified Linking Mechanism
    //-------------------------------------------------


    tok (DD_STRING),        //44
    tok (DD_LOCAL),         //45
    tok (DD_SCOPE),         //46
    tok (DD_INSTANCE),      //47
    tok (DD_SMT),           //48
    tok (DD_REGION),        //49
    tok (DD_EXTGROUP),      //50
    tok (DD_EXTGROUP_TMP),  //51
    tok (DD_AST),           //52
    tok (DD_DATA),          //53
    tok (DD_PACKAGE),       //54
    tok (DD_CURSOR),        //55
    tok (DD_SQL_TABLE),     //56
    tok (DD_EXCEPTION),     //57
    tok (DD_EXTERN_REF),    //58
    tok (DD_LABEL),         //59
    tok (DD_VERSION),       //60
    tok (DD_NUMBER),         //61
    tok (DD_NAMESPACE),     //62
    tok (DD_INTERFACE),     //63
    tok (NUM_OF_DDS)        //64   // MUST BE LAST, used to do range check
};
generate_prt_fun(ddKind);

ddEnum(ddProtection)
{
    tok(DD_PUBLIC), tok(DD_PROTECTED), tok(DD_PACKAGE_PROT), tok(DD_PRIVATE)
};
generate_prt_fun(ddProtection);

ddEnum(ddCharacteristic)
{
    tok(DD_CONSTANT), tok(DD_VOLATILE),
    tok(DD_NONCONSTANT), tok(NON_VOLATILE), 
                                // either will clear constant or volatile
    tok(DD_STATIC), tok(DD_VIRTUAL), tok(DD_PURE_VIRTUAL),
    tok(DD_NON_STATIC), tok(DD_NON_VIRTUAL), 
                                // either will clear static and virtual
    tok(DD_INLINE), tok(DD_NOT_INLINE)
};
generate_prt_fun(ddCharacteristic);

ddEnum(dd_member_rel_type)
{
#ifdef _WIN32
    tok(__UNKNOWN),
#else
    tok(UNKNOWN),
#endif
    tok(OBJECT_OF),			// T x;
    tok(POINTER_TO),			// T* x;
    tok(REFERENCE_TO),			// T& x;
    tok(FUNCTION_RETURNING),		// T f() or T* f() or T& f()
    tok(FUNCTION_TAKING_ARG),		// f(T) or f(T*), or f(T&) or ...
    tok(OTHER)
};
generate_prt_fun(dd_member_rel_type);

ddEnum(linkType)
// types of links    Normally, symbolic link types come in pairs, even&odd
// These links are used in pmods, to give extra meanings and interconnections between
//   symbols
{                        // numeric values in comments below are just aid to debugging
  tok(ltRelational),		// 0
  tok(lt_next),			// 1
  tok(lt_junk),			// 2
  tok(lt_link),			// 3 next symbol is a link
  tok(used_by),			// 4 a is used_by b means b is calling a : b() { a(); }
  tok(is_using),		// 5 a is_using b means a is calling b : a() { b(); }
  tok(has_superclass),          // 6
  tok(has_subclass),            // 7
  tok(is_defining),             // 8
  tok(is_defined_in),           // 9  A is defined in module B
  tok(is_including),            // 10
  tok(included_by),             // 11 modules include each other
  tok(member_of),		// 12 from parent get all members
  tok(parent_of),		// 13 from member, get back to parent
  tok(ref_file),		// 14 all files referencing this symbol
  tok(file_ref),		// 15 all symbols referenced in this file
  tok(lt_lmd),			// 16 last_modified date. low 24 bits
  tok(lt_sym),                  // 17
  tok(lt_filetype),             // 18 for a file, this says if it is C, C++, ...
  tok(lt_misc),   // (poor mnemonic) // 19 for a renamed symbol, points to new name
  tok(has_type),                // 20
  tok(type_has),                // 21
  tok(have_friends),            // 22
  tok(friends_of),              // 23
  tok(lt_lmdhigh),		// 24 last_modified date. high 8 bits
  tok(lt_attribute),		// 25 Symbol attributes (bit fields)
  tok(assoc_of_sym),            // 26
  tok(sym_of_assoc),            // 27
  tok(have_arg_type),           // 28 class A; foo(A&); ==> foo have_arg_type A
  tok(arg_type_of),             // 29 A is arg_type_of foo
    tok(maintain_person_entity_NO_LONGER_USED), // 30
    tok(maintain_entity_person_NO_LONGER_USED), // 31
    tok(interest_person_entity_NO_LONGER_USED), // 32
    tok(interest_entity_person_NO_LONGER_USED), // 33
    tok(fix_person_bug_NO_LONGER_USED),         // 34
    tok(fix_bug_person_NO_LONGER_USED),         // 35
    tok(consult_person_bug_NO_LONGER_USED),     // 36
    tok(consult_bug_person_NO_LONGER_USED),     // 37
    tok(change_entity_bug_NO_LONGER_USED),      // 38
    tok(change_bug_entity_NO_LONGER_USED),      // 39
    tok(has_property),          // 40  possessor has_property property
    tok(has_possessor),         // 41  property has_possessor popssessor
    tok(assocType_of_instances), // 42
    tok(instances_of_assocType), // 43
    tok(ifl_src_of_targ), // 44
    tok(ifl_targ_of_src), // 45
    tok(has_vpub_superclass), // 46
    tok(has_vpub_subclass), // 47
    tok(has_vpri_superclass), // 48
    tok(has_vpri_subclass), // 49
    tok(has_pub_superclass), // 50
    tok(has_pub_subclass), // 51
    tok(has_pri_superclass), // 52
    tok(has_pri_subclass), // 53
    tok(has_vpro_superclass), // 54
    tok(has_vpro_subclass), // 55
    tok(has_pro_superclass), // 56
    tok(has_pro_subclass), // 57

    //------------------------------------------
    // *** JJS 3/21/95 ***
    // definitions for Unified Linking Mechanism
    //------------------------------------------

    tok (linknode_of_src_sym), //58
    tok (src_sym_of_linknode), //59
    tok (linknode_of_trg_sym), //60
    tok (trg_sym_of_linknode), //61
    tok (linkspec_of_linknode), //62
    tok (linknode_of_linkspec), //63
    tok (reverse_of_linkspec), // 64
    tok (linkspec_of_reverse), //65
    
    //-------------------------------------------------
    // *** JJS 3/21/95 ***
    // end of definitions for Unified Linking Mechanism
    //-------------------------------------------------

    tok(grp_has_peer),  // 66
    tok(grp_is_peer_of),  // 67
    tok(grp_has_server),  // 68
    tok(grp_has_client),  // 69
    tok(grp_has_pub_mbr),  // 70
    tok(grp_pub_mbr_of),  // 71
    tok(grp_has_pri_mbr),  // 72
    tok(grp_pri_mbr_of),  // 73
    tok(grp_has_trans_mbr),  // 74
    tok(grp_trans_mbr_of),  // 75
    tok(declared_in),  // 76  
    tok(declares), // 77
    tok(has_c_proto), // 78
    tok(c_proto_has), // 79
    tok(lt_own_pmod_NO_LONGER_USED), // 80
    tok(lt_own_offset), //81
    tok(has_template),          // 82  instance has_template template
    tok(template_of),           // 83  template template_of instance
    tok(lt_last),    // 84
    tok(NUM_OF_LINKS)      // MUST BE LAST, USED FOR RANGE CHECK
	                   // must be < 128
  };
generate_prt_fun(linkType);

//set of attributes for symbols in pmod
enum symAtt
{
PRIV_ATT = 0,
PROT_ATT =1,
VIRT_ATT =2,
PVIR_ATT =3,
STAT_ATT =4,
CNST_ATT =5,
CONS_DES =6,
METHOD =7,
POINTER =8,
REF =9,
LANGUAGE =10,  // width of 3   
SWT_Entity_Status = 13, // width of 2
COMP_GEN_ATT = 15, // compiler generated
NATV_ATT = 16,
IS_SUBSYS_ATT = 17, // available for reuse: IS_SUBSYS_ATT is referenced, but 
                    // the distinction it makes does not actually occur now.
VOLT_ATT =18,
INLI_ATT =19,
PAKG_ATT = 20,
SYNC_ATT = 21,
NEXT = 22,
ALL_ATTRIB = 0,  ALL_ATTRIB_LEN = 24
};

ddEnum(SwtEntityStatus)
{
  tok(SWT_NEW),
  tok(SWT_MODIFIED),
  tok(SWT_UNCHANGE)
};

generate_prt_fun(SwtEntityStatus);

#undef ddEnum
#undef tok
#undef generate_prt_fun


#endif // _ddKind_h
