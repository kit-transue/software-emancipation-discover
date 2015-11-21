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
DECLARE_STRUCT(db_str_description)
    DECLARE_FIELD_CHARARR(magic,4)      // Magic string number #REL
    DECLARE_FIELD_LONG(version)    // Version of save file format
    DECLARE_FIELD_INT(date)        // Date of original header creation
    DECLARE_FIELD_USHORT(old_checksum) // Checksum of contents -- unused
    DECLARE_FIELD_USHORTBITS(sparebyte,8)
    DECLARE_FIELD_USHORTBITS(need_reparse,1)
    DECLARE_FIELD_USHORTBITS(not_scramble,1)
    DECLARE_FIELD_USHORTBITS(bit2,1)
    DECLARE_FIELD_USHORTBITS(bit3,1)
    DECLARE_FIELD_USHORTBITS(bit4,1)
    DECLARE_FIELD_USHORTBITS(bit5,1)
    DECLARE_FIELD_USHORTBITS(bit6,1)
    DECLARE_FIELD_USHORTBITS(written,1)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT


DECLARE_STRUCT(db_str_desc_extension)
    DECLARE_FIELD_UNSIGNED(new_checksum)
    DECLARE_FIELD_CHARARR(reserved,32)
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_Root)
    DECLARE_FIELD_USHORT(table)
    DECLARE_FIELD_USHORT(ind)
    DECLARE_FIELD_INT(int_len)
    DECLARE_FIELD_INT(root_id)
    DECLARE_FIELD_INT(app_len)
    DECLARE_FIELD_INT(last_id)
    DECLARE_FIELD_INT(reserv)
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_flags)
    DECLARE_FIELD_BITS(db_format,1)
    DECLARE_FIELD_BITS(db_rel_format,2)
    DECLARE_FIELD_BITS(reserv,29)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_RecHeader3b) // newlines->flags, spaces->ctrl
    DECLARE_FIELD_BITS(extype,5)
    DECLARE_FIELD_BITS(type,4)
    DECLARE_FIELD_BITS(rec_len,7)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_app)
    DECLARE_FIELD_BITS(is_node,1)
    DECLARE_FIELD_BITS(dh,2)
    DECLARE_FIELD_BITS(is_leaf,1)
    DECLARE_FIELD_BITS(is_last,1)
    DECLARE_FIELD_BITS(flag,3)
    DECLARE_FIELD_BITS(val,24)
    DECLARE_FIELD_BITS(extype,8)
    DECLARE_FIELD_BITS(type,8)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_app_part)
//    DECLARE_FIELD_BITS(is_node,1)
//    DECLARE_FIELD_BITS(dh,2)
//    DECLARE_FIELD_BITS(is_leaf,1)
//    DECLARE_FIELD_BITS(is_last,1)
//    DECLARE_FIELD_BITS(flag,3)
    DECLARE_FIELD_BITS(val,24)
    DECLARE_FIELD_BITS(extype,8)
    DECLARE_FIELD_BITS(type,8)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_Rel3_14_8)
    DECLARE_FIELD_BITS(is_node,1)
    DECLARE_FIELD_BITS(node_id,14)
    DECLARE_FIELD_BITS(direction,1)
    DECLARE_END_BITFIELD
    DECLARE_FIELD_BYTE(rel_id)
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_Rel3_15_7)
    DECLARE_FIELD_BITS(is_node,1)
    DECLARE_FIELD_BITS(node_id,15)
    DECLARE_FIELD_BITS(direction,1)
    DECLARE_FIELD_BITS(rel_id,7)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_Rel3_16_6)
    DECLARE_FIELD_BITS(is_node,1)
    DECLARE_FIELD_BITS(node_id,16)
    DECLARE_FIELD_BITS(direction,1)
    DECLARE_FIELD_BITS(rel_id,6)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_Rel4_22_8)
    DECLARE_FIELD_BITS(is_node,1)
    DECLARE_FIELD_BITS(node_id,22)
    DECLARE_FIELD_BITS(direction,1)
    DECLARE_END_BITFIELD
    DECLARE_FIELD_BYTE(rel_id)
DECLARE_END_STRUCT

DECLARE_STRUCT(db_str_ddElement)
    DECLARE_FIELD_INT(kind)   // ddKind enum
    DECLARE_FIELD_INT(struct_rel_type)  // dd_member_rel_type enum
    DECLARE_FIELD_INT(local_index)
    DECLARE_FIELD_INT(xref_index)
    DECLARE_FIELD_BITS(record,1)
    DECLARE_FIELD_BITS(enumerated,1)
    DECLARE_FIELD_BITS(private_member,1)
    DECLARE_FIELD_BITS(protected_member,1)
    DECLARE_FIELD_BITS(virtual_attr,1)
    DECLARE_FIELD_BITS(static_attr,1)
    DECLARE_FIELD_BITS(const_attr,1)
    DECLARE_FIELD_BITS(mangled_attr,1)
    DECLARE_FIELD_BITS(obsolete,1)
    DECLARE_FIELD_BITS(expect_friend,1)
    DECLARE_FIELD_BITS(pure_virtual_attr,1)
    DECLARE_FIELD_BITS(volatile_attr,1)
    DECLARE_FIELD_BITS(inline_attr,1)
    DECLARE_FIELD_BITS(extern_def,1)
    DECLARE_FIELD_BITS(is_def,1)
    DECLARE_FIELD_BITS(is_old,1)
    DECLARE_FIELD_BITS(cons_des,1)
    DECLARE_FIELD_BITS(kr,1)
    DECLARE_FIELD_BITS(drn,1)
    DECLARE_FIELD_BITS(from_method,1)
    DECLARE_FIELD_BITS(_ste_flags,2)
    DECLARE_FIELD_BITS(has_def,1)
    DECLARE_FIELD_BITS(datatype,4)
    DECLARE_FIELD_BITS(changed_since_saved,1)
    DECLARE_FIELD_BITS(changed_since_checked_in,1)
    DECLARE_FIELD_BITS(file_not_checked_in,1)
    DECLARE_FIELD_BITS(filler,2)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_app_header)
    DECLARE_FIELD_BITS(is_node, 1)
    DECLARE_FIELD_BITS(dh, 2)
    DECLARE_FIELD_BITS(is_leaf, 1)
    DECLARE_FIELD_BITS(is_last, 1)
    DECLARE_FIELD_BITS(flag, 3)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_app_F2)
    DECLARE_FIELD_BITS(is_node, 1)
    DECLARE_FIELD_BITS(dh, 2)
    DECLARE_FIELD_BITS(is_leaf, 1)
    DECLARE_FIELD_BITS(is_last, 1)
    DECLARE_FIELD_BITS(flag, 3)
    DECLARE_FIELD_BITS(extype, 8)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT

DECLARE_STRUCT(db_rec_header2a)
    DECLARE_FIELD_BITS(extype, 5)
    DECLARE_FIELD_BITS(rec_len, 3)
    DECLARE_END_BITFIELD
DECLARE_END_STRUCT


