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
#include <link_type_converter.h> 
#include <Link.h>
#include <ddKind.h>

Link NULL_Link;
//  constructor, called once while program is starting
link_type_converter::link_type_converter()
{
  NULL_Link.assign((linkType) 0);  
    for (int i=0; i<sizeof(belong_to_file); i++) {
	belong_to_file[i] = 0;
	forward_link[i] = 0;
	reverse_link[i] = 0;
    }
    belong_to_file[ref_file] = 1;
    belong_to_file[is_including] = 1;
    belong_to_file[included_by] = 1;
    belong_to_file[is_defined_in] = 1;

    forward_link[is_using] = 1;
    forward_link[has_superclass] = 1;
    forward_link[is_defining] = 1;
    forward_link[is_including] = 1;
    forward_link[parent_of] = 1;
    forward_link[file_ref] = 1;
    forward_link[has_type] = 1;
    forward_link[have_friends] = 1;
    forward_link[assoc_of_sym] = 1;
    forward_link[have_arg_type] = 1;
    forward_link[instances_of_assocType] = 1;
    forward_link[ifl_src_of_targ] = 1;    
    forward_link[has_vpub_superclass] = 1;
    forward_link[has_vpri_superclass] = 1;
    forward_link[has_pub_superclass] = 1;
    forward_link[has_pri_superclass] = 1;
    forward_link[has_vpro_superclass] = 1;
    forward_link[has_pro_superclass] = 1;
    forward_link[trg_sym_of_linknode] = 1;
    forward_link[reverse_of_linkspec] = 1;
    forward_link[grp_has_pub_mbr] = 1;
    forward_link[grp_has_pri_mbr] = 1;
    forward_link[grp_has_trans_mbr] = 1;
    forward_link[grp_has_client] = 1;
    forward_link[grp_has_server] = 1;
    forward_link[grp_has_peer] = 1;
    forward_link[grp_is_peer_of] = 1;
    forward_link[declares] = 1;
    forward_link[has_template] = 1;
    forward_link[has_property] = 1;

    reverse_link[used_by] = 1;
    reverse_link[has_subclass] = 1;
    reverse_link[is_defined_in] = 1;
    reverse_link[included_by] = 1;
    reverse_link[member_of] = 1;
    reverse_link[ref_file] = 1;
    reverse_link[type_has] = 1;
    reverse_link[friends_of] = 1;
    reverse_link[sym_of_assoc] = 1;
    reverse_link[arg_type_of] = 1;
    reverse_link[assocType_of_instances] = 1;
    reverse_link[ifl_targ_of_src] = 1;    
    reverse_link[has_pro_subclass] = 1;
    reverse_link[has_vpub_subclass] = 1;
    reverse_link[has_vpri_subclass] = 1;
    reverse_link[has_pub_subclass] = 1;
    reverse_link[has_pri_subclass] = 1;
    reverse_link[has_vpro_subclass] = 1;
    reverse_link[linknode_of_trg_sym] = 1;
    reverse_link[linkspec_of_reverse] = 1;
    reverse_link[grp_pub_mbr_of] = 1;
    reverse_link[grp_pub_mbr_of] = 1;
    reverse_link[grp_trans_mbr_of] = 1;
    reverse_link[declared_in] = 1;
    reverse_link[template_of] = 1;
    reverse_link[has_possessor] = 1;
}



