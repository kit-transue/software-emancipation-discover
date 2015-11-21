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
#ifndef _LDR_TYPES_H_
#define _LDR_TYPES_H_

// This file contains various enumeration types for ldr nodes.

// ldrSymbolType is set on symbol nodes. In View, it determines
// the image of the node on the screen (i.e., is an index of the
// pictures contained in symbols.dat and maintained by graWorld).

#define ldrNOSYMBOL 10000

enum ldrSymbolType {
   ldrSOMETHING,
   ldrIF,
   ldrLOOP,
   ldrBLOCK,
   ldrENTRY_SYMBOL,  
   ldrEXIT_SYMBOL,
   ldrTEXT,
   ldrCONTAINER,
   ldrLIST,
   ldrPUBLIC_METHOD,
   ldrPROTECTED_METHOD,
   ldrPACKAGE_PROT_METHOD,
   ldrPRIVATE_METHOD,
   ldrPUBLIC_VIRTUAL_CLASS,
   ldrPROTECTED_VIRTUAL_CLASS,
   ldrPACKAGE_PROT_VIRTUAL_CLASS,
   ldrPRIVATE_VIRTUAL_CLASS,
   ldrCURRENT_CLASS,
   ldrPUBLIC_CLASS,
   ldrPROTECTED_CLASS,
   ldrPACKAGE_PROT_CLASS,
   ldrPRIVATE_CLASS,
   ldrPUBLIC_MEMBER,
   ldrPROTECTED_MEMBER,
   ldrPACKAGE_PROT_MEMBER,
   ldrPRIVATE_MEMBER,
   ldrNOTE,
   ldrCURRENT_SUBSYSTEM,
   ldrPUBLIC_SUBSYSTEM,
   ldrPROTECTED_SUBSYSTEM,
   ldrPACKAGE_PROT_SUBSYSTEM,
   ldrPRIVATE_SUBSYSTEM,
   ldrRELATION,
   ldrPUBLIC_METHOD_PARAMETER,
   ldrPROTECTED_METHOD_PARAMETER,
   ldrPACKAGE_PROT_METHOD_PARAMETER,
   ldrPRIVATE_METHOD_PARAMETER,
   ldrPUBLIC_NESTED_TYPE,
   ldrPROTECTED_NESTED_TYPE,
   ldrPACKAGE_PROT_NESTED_TYPE,
   ldrPRIVATE_NESTED_TYPE,
   ldrPUBLIC_VIRTUAL_TEMPLATE_PARAMETER,
   ldrPROTECTED_VIRTUAL_TEMPLATE_PARAMETER,
   ldrPACKAGE_PROT_VIRTUAL_TEMPLATE_PARAMETER,
   ldrPRIVATE_VIRTUAL_TEMPLATE_PARAMETER,
   ldrCURRENT_TEMPLATE_PARAMETER,
   ldrPUBLIC_TEMPLATE_PARAMETER,
   ldrPROTECTED_TEMPLATE_PARAMETER,
   ldrPACKAGE_PROT_TEMPLATE_PARAMETER,
   ldrPRIVATE_TEMPLATE_PARAMETER,
   ldrBULLET,
   ldrIF_FLOW,
   ldrLOOP_BOX,
   ldrOVAL,
   ldrHIDDEN_METHOD,
   ldrHIDDEN_METHOD_PARAMETER,
   ldrHIDDEN_MEMBER,
   ldrHIDDEN_NESTED_TYPE,
   ldrFLOW_EMPTY,
   ldrFLOW_LABEL,
   ldrFLOW_YES,
   ldrFLOW_SWITCH,
   ldrFLOW_CASE,
   ldrFLOW_DEFAULT,
   ldrFLOW_LOOP,
   ldrFUNC_CALL,
   ldrERD_CLASS,
   ldrERD_MEMBER,
   ldrERD_ELLIPSIS,
   ldrPROJECT,
   ldrMODULE,
   ldrSTATEMENT,
   ldrFUNCTION,
   ldrEXPRESSION,
   ldrMODULE_STD,
   ldrMODULE_SMT,
   ldrMODULE_SEARCH,
   ldrMODULE_PROJECT,
   ldrSM_SUBSYSTEM,
   ldrPUBLIC_METHOD_NOVICE,
   ldrPROTECTED_METHOD_NOVICE,
   ldrPACKAGE_PROT_METHOD_NOVICE,
   ldrPRIVATE_METHOD_NOVICE,
   ldrPUBLIC_MEMBER_NOVICE,
   ldrPROTECTED_MEMBER_NOVICE,
   ldrPACKAGE_PROT_MEMBER_NOVICE,
   ldrPRIVATE_MEMBER_NOVICE,
   ldrPUBLIC_METHOD_PARAMETER_NOVICE,
   ldrPROTECTED_METHOD_PARAMETER_NOVICE,
   ldrPACKAGE_PROT_METHOD_PARAMETER_NOVICE,
   ldrPRIVATE_METHOD_PARAMETER_NOVICE,
   ldrPUBLIC_NESTED_TYPE_NOVICE,
   ldrPROTECTED_NESTED_TYPE_NOVICE,
   ldrPACKAGE_PROT_NESTED_TYPE_NOVICE,
   ldrPRIVATE_NESTED_TYPE_NOVICE,
   ldrHIDDEN_METHOD_NOVICE,
   ldrHIDDEN_METHOD_PARAMETER_NOVICE,
   ldrHIDDEN_MEMBER_NOVICE,
   ldrHIDDEN_NESTED_TYPE_NOVICE,
   ldrPUBLIC_SUBSYS_FUNC,
   ldrPRIVATE_SUBSYS_FUNC,
   ldrPUBLIC_SUBSYS_CLASS,
   ldrPRIVATE_SUBSYS_CLASS,
   ldrPUBLIC_SUBSYS_VAR,
   ldrPRIVATE_SUBSYS_VAR,
   ldrPUBLIC_SUBSYS_ENUM,
   ldrSUBSYS_TEST,
   ldrXREF_METHOD,
   ldrXREF_CLASS,
   ldrXREF_MEMBER,
   ldrXREF_NESTED_TYPE,
   ldrXREF_ERD_CLASS,
   ldrXREF_ERD_MEMBER,
   ldrXREF_METHOD_NOVICE,
   ldrXREF_MEMBER_NOVICE,
   ldrXREF_NESTED_TYPE_NOVICE,
   ldrPUBLIC_SUBSYS_UNKNOWN,
   ldrPRIVATE_SUBSYS_UNKNOWN,
   ldrPRIVATE_SUBSYS_ENUM,
   ldrXREF_CURRENT_CLASS,
   ldrFUNC_CALL_ROOT,
   ldrMULTIPLE_IF,
   ldrFUNCTION_TYPE,
   ldrCLASS_TYPE,
   ldrVARIABLE_TYPE,
   ldrENUM_TYPE,
   ldrUNKNOWN_TYPE,
   ldrSUBSYSTEM_TYPE,
   ldrMACRO_TYPE,
   ldrUNION_TYPE,
   ldrENUMVALUE_TYPE,
   ldrTYPEDEF_TYPE,
   ldrBLANK1,
   ldrFIELD_TYPE,
   ldrSEM_TYPE,
   ldrLIST1,
   ldrLIST2,
   ldrOPEN_FOLDER,
   ldrCLOSED_FOLDER,
   ldrFILE,
   ldrCPP_FILE,
   ldrC_FILE,
   ldrSTE_FILE,
   ldrFRAME_FILE,
   ldrMAKEFILE,
   ldrPUBLIC_SUBSYS_MACRO,
   ldrPRIVATE_SUBSYS_MACRO,
   ldrPUBLIC_SUBSYS_UNION,
   ldrPRIVATE_SUBSYS_UNION,
   ldrPUBLIC_SUBSYS_TYPEDEF,
   ldrPRIVATE_SUBSYS_TYPEDEF,
   ldrPUBLIC_SUBSYS_FILE_UNKNOWN,
   ldrPRIVATE_SUBSYS_FILE_UNKNOWN,
   ldrPUBLIC_SUBSYS_FILE_CPP,
   ldrPRIVATE_SUBSYS_FILE_CPP,
   ldrPUBLIC_SUBSYS_FILE_C,
   ldrPRIVATE_SUBSYS_FILE_C,
   ldrPUBLIC_SUBSYS_FILE_STE,
   ldrPRIVATE_SUBSYS_FILE_STE,
   ldrPUBLIC_SUBSYS_FILE_FRAME,
   ldrPRIVATE_SUBSYS_FILE_FRAME,
   ldrPUBLIC_SUBSYS_FILE_MAKEFILE,
   ldrPRIVATE_SUBSYS_FILE_MAKEFILE
};     //ldrMODULE_SOURCE = 90

// ldrRole shows what is the role of the node in its parent's
// "family". Example: an "else" clause of an "if" statement
// always has role "ldrELSE", while it itself can be
// an "if" statement (ldrIF symbol type), a loop (ldrLOOP symbol type)
// or block, or just about anything.

enum ldrRole {
   ldrUNCLEAR,
   ldrIF_TEST,
   ldrTHEN,
   ldrELSE,

   ldrLOOP_COND,
   ldrLOOP_INIT,
   ldrLOOP_STEP,
   ldrLOOP_BODY,

   ldrENTRY,  
   ldrEXIT,

   ldrCONTAINMENT,

   ldrMEMBER,
   ldrINCOMING_RELATION,
   ldrOUTGOING_RELATION,

   ldrLOOP_TITLE,
   ldrLOOP_FOOTER,

   ldrFDECL,
   ldrSIMPLE,
   ldrFLOW_IF_TEST,
   ldrFLOW_ELSE,
   ldrLAST_ELSE,
   ldrLABEL,
   ldrGOTO,
   ldrFAKE_LABEL,
   ldrIF_YES,
   ldrSWITCH,
   ldrCASE,  
   ldrDEFAULT,
   ldrLOOP_DO,
   ldrBREAK, 
   ldrCLASS,
   ldrCLASS_MEMBER_TYPE 
};                    //        ldrCLASS_MEMBER_TYPE  = 31

// ldrConnectorType specifies the type of ldr connector
// nodes. It is used by View to pick the graphic representation
// of the connector.

enum ldrConnectorType {
   ldrcHIERARCHY,
   ldrcTRUE_BRANCH,
   ldrcFALSE_BRANCH,
   ldrcENDIF,
   ldrcFOR_BODY,
   ldrcSEQUENCE,
   ldrcSUPERCLASS_LIST,
   ldrcSUBCLASS_LIST,
   ldrcMEMBER_LIST,
   ldrcCONTAINER,
   ldrcREL_LIST_OUT,
   ldrcREL_LIST_IN,
   ldrcINVARIANT,
   ldrcENDELSE,
   ldrcIF_ELSE,
   ldrcLOOP_DO,
   ldrcLEFT_HORIZONTAL,
   ldrcCASE_EXIT,
   ldrcTREE,
   ldrcREL_OUT_1_OPT,
   ldrcREL_OUT_N_OPT,
   ldrcREL_OUT_1_REQ,
   ldrcREL_OUT_N_REQ,
   ldrcREL_IN_1_OPT,
   ldrcREL_IN_N_OPT,
   ldrcREL_IN_1_REQ,
   ldrcREL_IN_N_REQ,
   ldrcERD_INHERITANCE,
   ldrcERD_RELATION,
   ldrcERD_IDENTITY,
   ldrcUP_TREE,
   ldrcSM_CLIENT_SERVER,
   ldrcSM_PEER_PEER

};      //    ldrcUP_TREE = 30

#endif

/*
   START-LOG-------------------------------------------

   $Log: ldrTypes.h  $
   Revision 1.7 1998/09/07 10:33:25EDT Scott Turner (sturner) 
   adding support for Java "package"-level accessibility
 * Revision 1.2.1.4  1994/08/02  17:07:51  kws
 * Bug track: 7665
 * Make root of call tree graphically identifiable
 *
 * Revision 1.2.1.3  1993/02/03  23:46:27  wmm
 * Use different graphics for symbols with and without def DD elements.
 *
 * Revision 1.2.1.2  1992/10/09  19:50:39  jon
 * RCS History Marker Fixup
 *


   END-LOG---------------------------------------------

*/
