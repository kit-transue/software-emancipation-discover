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
//  checklinks.C
//      Created  5/9/94
//

#include <xref.h>
#include <xref_priv.h>
#include <ddKind.h>

// The whole purpose of this file is to provide a single function
//    validate_link(), which is to be called within checkpmod.C
//    to test a particular link


// max num links allow
#define  NUM_LINKS 128

// currently there are 40, but what the heck
#define  NUM_KINDS 50
class kinds
{
public:
  char  arr[NUM_KINDS];     // each "kinds" is a bitmap of ddKind values
  kinds() {reset();}
  void reset();
  int operator [] (int i) {return ((i >= 0 && i < NUM_KINDS) ? arr[i] : 0);}
  kinds & operator | (kinds &);
  void set_kind(int i, int val) {arr[i] = val;}
  void print();
  int operator == (kinds &);
};

void kinds::reset()
{
  for (int i = 0; i < NUM_KINDS; ++i)
    arr[i] = 0;
}

kinds & kinds::operator | (kinds & trg)
{
  for (int i = 0; i < NUM_KINDS; ++i)
    if (trg[i])
      arr[i] = 1;
  return *this;
}

int kinds::operator == (kinds & trg)
{
  for (int i = 0; i < NUM_KINDS; ++i)
    if (trg[i] != arr[i])
      return 0;
  return 1;
}

class  checklinks
{
  public:
    static void init();                              // initialize bitmap table
    static void allow(linkType, kinds&, kinds&);       // allow particular link between kinds
    static void allow2(linkType, kinds&, kinds&);      // allow both the link and its inverse
//    static void disallow(linkType, kinds, kinds);    // disallow a particular link
    static ddKind  get_lowkind(kinds &k);             // convert kinds to one of the ddKind bits
    static kinds &get_kinds( ddKind k, kinds &);           // convert ddKind to kinds
    static bool validate_link(linkType, ddKind, ddKind);    // check whether a particular link is legal
    static void dumpbitmap(int i,int j);     // dump (debug use only)
    static void dump();       // dump (debug use only)
  private:
    static kinds  table[NUM_LINKS][NUM_KINDS];                     // bitmap table, indexed by linktype, src kind
                                                                 //   bits represent destination kind
    static kinds  wild;
};

kinds checklinks::table[NUM_LINKS][NUM_KINDS];
kinds checklinks::wild;

//  This function a candidate for inlining
kinds &checklinks::get_kinds( ddKind k, kinds &ks)
//  turn a kind into a single-bit bitmap.  All other transformations
    // defined in terms of this one.  Invalid values return 0.
{
  ks.reset();
  if (k > 0 && k <= NUM_KINDS)
    ks.set_kind(k, 1);
  return ks;
}

ddKind checklinks::get_lowkind(kinds &k)
// get the lowest kind represented in the bitmap.  Return it as a ddKind,
// turning that bit off in the bitmap that was passed in.  Return -1 if
// no valid match
{
    for (int i=1; i<NUM_KINDS; i++) {
      if (k[i]) {
	k.set_kind(i, 0);
	return (ddKind) i;
      }
    }
    return (ddKind) 0;
}

void merge_2kinds(kinds &res, kinds &trg)
{
  for (int i = 0; i < NUM_KINDS; ++i)
    if (trg[i])
      res.set_kind(i, 1);
}

void merge_4kinds(kinds &res, kinds &k1, kinds &k2, kinds &k3, kinds &k4)
{
  merge_2kinds(res, k1);
  merge_2kinds(res, k2);
  merge_2kinds(res, k3);  
  merge_2kinds(res, k4);    
}

static bool initialized=0;
void checklinks::init()
{
    Initialize(checklinks::init);
    int ii;
    for (ii = 0; ii < NUM_KINDS; ++ii)
      wild.set_kind(ii, 1);
    for (ii = 0; ii < NUM_LINKS; ++ii) {
      for (int jj = 0; jj < NUM_KINDS; ++jj)
	table[ii][jj].reset();
    }
    kinds tmp_kinds;
    kinds ddtypedef = get_kinds( DD_TYPEDEF, tmp_kinds);
    kinds ddmacro = get_kinds( DD_MACRO, tmp_kinds);
    kinds ddvardecl = get_kinds( DD_VAR_DECL, tmp_kinds);
    kinds ddfield = get_kinds( DD_FIELD, tmp_kinds);
    kinds ddfuncdecl = get_kinds( DD_FUNC_DECL, tmp_kinds);
    kinds ddenum = get_kinds( DD_ENUM, tmp_kinds);
    kinds ddenumval = get_kinds( DD_ENUM_VAL, tmp_kinds);
    kinds ddclass = get_kinds( DD_CLASS, tmp_kinds);
    kinds ddinterface = get_kinds( DD_INTERFACE, tmp_kinds);
    kinds ddunion = get_kinds( DD_UNION, tmp_kinds);
    kinds ddcomponent = get_kinds( DD_COMPONENT, tmp_kinds);
    kinds ddsemtype = get_kinds( DD_SEMTYPE, tmp_kinds);
    kinds ddtemplate = get_kinds( DD_TEMPLATE, tmp_kinds);
    kinds ddsubsystem = get_kinds( DD_SUBSYSTEM, tmp_kinds);
    //    kinds ddassoclink = get_kinds( DD_ASSOCLINK, tmp_kinds);
    kinds ddmodule = get_kinds( DD_MODULE, tmp_kinds);
    kinds ddproject = get_kinds( DD_PROJECT, tmp_kinds);
    kinds ddiflsrc = get_kinds( DD_IFL_SRC, tmp_kinds);
    kinds ddifltrg = get_kinds( DD_IFL_TRG, tmp_kinds);
    //kinds ddstyle = get_kinds( DD_STYLE, tmp_kinds);
    kinds ddcategory = get_kinds( DD_CATEGORY, tmp_kinds);
    kinds ddrelation = get_kinds( DD_RELATION, tmp_kinds);
    kinds ddrelsrc = get_kinds( DD_REL_SRC, tmp_kinds);
    kinds ddreltrg = get_kinds( DD_REL_TRG, tmp_kinds);
    //kinds ddassoctype = get_kinds( DD_SOFT_ASSOC_TYPE, tmp_kinds);
    //kinds ddsoftassoc = get_kinds( DD_SOFT_ASSOC, tmp_kinds);
    kinds ddlinknode = get_kinds (DD_LINKNODE, tmp_kinds);
    kinds ddlinkspec = get_kinds (DD_LINKSPEC, tmp_kinds);
    kinds ddprot = get_kinds(DD_SYNTYPE, tmp_kinds);
    
    kinds objects = ddfield ;
    merge_4kinds(objects, ddfuncdecl, ddclass, ddvardecl, ddenumval);
    kinds types = ddsemtype;
    merge_4kinds(types, ddclass, ddunion, ddenum, ddinterface);

    allow2(has_superclass, ddclass, ddclass);
    allow2(has_vpub_superclass, ddclass, ddclass);
    allow2(has_vpri_superclass, ddclass, ddclass);
    allow2(has_pub_superclass, ddclass, ddclass);
    allow2(has_pri_superclass, ddclass, ddclass);
    allow2(has_vpro_superclass, ddclass, ddclass);
    allow2(has_pro_superclass, ddclass, ddclass);
    allow2(has_superclass, ddclass, ddinterface);
    allow2(has_pub_superclass, ddclass, ddinterface);
    allow2(has_pri_superclass, ddclass, ddinterface);
    allow2(has_pro_superclass, ddclass, ddinterface);
    allow2(has_superclass, ddinterface, ddinterface);
    allow2(has_pub_superclass, ddinterface, ddinterface);
    allow2(has_pri_superclass, ddinterface, ddinterface);
    allow2(has_pro_superclass, ddinterface, ddinterface);

    allow2(declares, ddmodule, ddunion);
    allow2(declares, ddmodule, ddenum);
    allow2(declares, ddclass, objects);
    allow2(declares, ddinterface, objects);
    allow2(declares, ddunion, objects);
    allow2(declares, ddfuncdecl, objects);
    allow2(declares, ddmodule, objects);
    allow(has_c_proto, ddfuncdecl, ddprot);

    allow2(template_of, ddtemplate, wild);
    allow2(has_superclass, ddsubsystem, ddsubsystem);
    allow2(used_by, wild, wild);
    tmp_kinds = ddfuncdecl;
    merge_2kinds(tmp_kinds, ddinterface);
    merge_4kinds(tmp_kinds, ddmodule, ddclass, ddunion, ddenum);
    merge_4kinds(tmp_kinds, ddenumval , ddfield, ddfield , ddmacro);
    merge_4kinds(tmp_kinds, ddtypedef, ddvardecl, ddiflsrc , ddifltrg);
    merge_4kinds(tmp_kinds, ddsubsystem, ddrelation, ddiflsrc, ddifltrg);
    allow2(is_defining, ddmodule, wild);
    allow2(has_property, wild, wild);

    allow2(is_including, ddmodule, ddmodule);
    kinds nested;
    merge_2kinds(nested, ddinterface);
    merge_4kinds(nested, ddclass, ddtypedef, ddunion, ddenum);
    allow2(member_of, ddclass, nested);
    allow2(member_of, ddinterface, nested);
    allow2(member_of, ddunion, nested);

    allow2(file_ref, ddmodule, wild);
    allow2(has_type, objects, ddsemtype);
    allow2(has_type, ddtypedef, ddsemtype);
    allow2(has_type, ddenum, ddsemtype);
    
    
    allow2(have_friends, ddclass, objects);
    allow2(have_friends, ddclass, ddclass);
    
    //allow2 (instances_of_assocType, ddassoctype, ddsoftassoc);
    allow2 (linkspec_of_linknode, ddlinknode, ddlinkspec);
    allow2 (reverse_of_linkspec, ddlinkspec, ddlinkspec);

    tmp_kinds = ddtypedef;
    merge_2kinds (tmp_kinds, ddinterface);
    merge_4kinds (tmp_kinds, ddmacro, ddvardecl, ddfield, ddfuncdecl);
    merge_4kinds (tmp_kinds, ddenum, ddenumval, ddclass, ddunion);
    //allow2 (assoc_of_sym, tmp_kinds, ddsoftassoc);
    allow2 (linknode_of_src_sym, tmp_kinds, ddlinknode);
    allow2 (trg_sym_of_linknode, ddlinknode, tmp_kinds);

    tmp_kinds = ddfuncdecl;
    merge_2kinds(tmp_kinds, ddvardecl);
    allow2(have_arg_type, tmp_kinds, types);
    tmp_kinds = ddfield;
    merge_2kinds(tmp_kinds, ddtypedef);
    allow2(have_arg_type, tmp_kinds, types);
    allow2(ifl_targ_of_src, wild, wild);

    allow2(grp_has_peer, ddsubsystem, ddsubsystem);
    allow2(grp_has_server, ddsubsystem, ddsubsystem);
    tmp_kinds = ddtypedef;
    merge_4kinds(tmp_kinds, ddmacro, ddvardecl, ddfuncdecl, ddenum);
    merge_4kinds(tmp_kinds, ddclass, ddunion, ddtemplate, ddinterface);
    allow2(grp_has_pub_mbr, ddsubsystem, tmp_kinds);
    allow2(grp_has_pri_mbr, ddsubsystem, tmp_kinds);
    allow2(grp_has_trans_mbr, ddsubsystem, tmp_kinds);


    initialized=1;
}


extern "C" void dumpbitmap(int i, int j)
{
    checklinks::dumpbitmap(i,j);
}
void checklinks::dumpbitmap(int ,int )
{
}
    
void checklinks::dump()
{
}

void checklinks::allow(linkType lt, kinds &src_kinds, kinds &dest_kinds)
{
    Initialize(checklinks::allow);
    kinds src = src_kinds;
    ddKind src_kind;
    while (src_kind = get_lowkind(src)) {
      for (int ii = 0; ii < NUM_KINDS; ++ii)
	if (dest_kinds[ii])
	  table[lt][src_kind].set_kind(ii, 1);
    }
}
void checklinks::allow2(linkType lt, kinds &src_kinds, kinds &dest_kinds)
{
    Initialize(checklinks::allow2);
    allow(lt, src_kinds, dest_kinds);
    allow(get_reverse_link(lt), dest_kinds, src_kinds);
}

bool checklinks::validate_link(linkType lt, ddKind src, ddKind dest)
{
    Initialize(checklinks::validate_link);
    if (!initialized) 
	init();
    if (lt < 0 || lt>=NUM_LINKS) lt = (linkType)0;
    bool result = (0 != (table[lt][src][dest]));
    return result;
}

bool  validate_link(linkType lt, ddKind src, ddKind dest)
{
    return checklinks::validate_link(lt,src,dest);
}

/*
   START-LOG-------------------------------------------
  
  $Log: checklinks.cxx  $
  Revision 1.20 2000/11/14 10:15:07EST Scott Turner (sturner) 
  adding support for 'language' and 'checksum' properties to the model,
  for bug 19353 and for better detection of the Java ELS language
 * Revision 1.2  1994/05/26  13:17:06  davea
 * Bug track: n/a
 * add hyperlinks to -checkpmod tests
 *
 * Revision 1.1  1994/05/13  13:48:18  davea
 * Initial revision
 *
*/
