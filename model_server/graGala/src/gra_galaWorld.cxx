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
#include <vport.h>
#include vdialogHEADER
#include vstdioHEADER
#include vapplicationHEADER
#include vdomainitemHEADER
#include vimageHEADER
#include <gra_galaWorld.h>
#include <gra_galaSymbol.h>
#define  xxxPrintf printf
#undef   printf

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <messages.h>
#include <ddict.h>
#include <view.h>
#include <graSymbol.h>
#include <genError.h>
#include <machdep.h>
#include <driver.h>
#include <ldrTypes.h>
#include <ldrHeader.h>
#include <ldrERDHierarchy.h>
#include <ldrDCHierarchy.h>
#include <ldrSubsysMapHierarchy.h>
#include <ldrSubsysHierarchy.h>
#include <genString.h>
#include <_Xref.h>
#include <xrefSymbol.h>
#include <viewTypes.h>
#include <appDGraph.h>
#include <disUIHandler.h>
#include <disUIDG_extern.h>

#define printf xxxPrintf

#define TAG(t) vnameInternGlobalLiteral(t)

smtTreePtr dd_get_smt(ddElementPtr);
int dis_create_view_of_def( symbolPtr );
extern "C" void push_busy_cursor();
extern "C" void pop_cursor();

vfont *ui_font;

class DomainItem : public vdomainitem {
  public:
    vloadableINLINE_CONSTRUCTORS(DomainItem, vdomainitem);
    vkindTYPED_FULL_DECLARATION(DomainItem);

    vbool HandleButtonDownEvent(vdomainviewButtonDownEvent *event);
    
    void DrawObject(vdomainObject *object);
    void DrawSelection(vdomainObject *object);
};

class DomainShell : public vdialog {
  public:
    vloadableINLINE_CONSTRUCTORS(DomainShell, vdialog);
    
    vkindTYPED_FULL_DECLARATION(DomainShell);
//    void LoadInit(vresource resource);
    
    static void OpenDomain(void);
    static vdomainitem *GetDomainItem(void);
    
    vdomainitem *domain_item;
};

vkindTYPED_FULL_DEFINITION(DomainShell, vdialog, NULL);
vkindTYPED_FULL_DEFINITION(DomainItem, vdomainitem, NULL);


DomainShell *domain_shell;

static const int GRA_SYMBOL_ARRAY_SIZE = 150;

struct graSymbolData{
    graAttachableSymbol* pictel[GRA_MAX_MODES];
    short                text_mode;
    char                 *text;
};

struct ImageData {
    vimage *image;
    char   *name;
};

struct ColorData {
    int  r;
    int  g;
    int  b;
    char *name;
};

ColorData*     graColorsArray;
static int     num_colors;

ImageData*     graImageArray;
static int     num_images;

graSymbolData* graSymbolArray;
int            num_gra_symbol_types;

/* ========================================================================== */

static int sym_index = -1;
static int sym_layer = -1;

extern "C" void sym_define_new(char *sym_name)
{
    sym_index = -1;

    if(strcmp(sym_name, "ldrSOMETHING") == 0){
	sym_index = ldrSOMETHING;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrIF") == 0){
	sym_index = ldrIF;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrLOOP") == 0){
	sym_index = ldrLOOP;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrBLOCK") == 0){
	sym_index = ldrBLOCK;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrENTRY_SYMBOL") == 0){
	sym_index = ldrENTRY_SYMBOL;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrEXIT_SYMBOL") == 0){
	sym_index = ldrEXIT_SYMBOL;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrTEXT") == 0){
	sym_index = ldrTEXT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCONTAINER") == 0){
	sym_index = ldrCONTAINER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrLIST") == 0){
	sym_index = ldrLIST;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_METHOD") == 0){
	sym_index = ldrPUBLIC_METHOD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_METHOD") == 0){
	sym_index = ldrPROTECTED_METHOD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPACKAGE_PROT_METHOD") == 0){
	sym_index = ldrPACKAGE_PROT_METHOD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPACKAGE_PROT_CLASS") == 0) {
	sym_index = ldrPACKAGE_PROT_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPACKAGE_PROT_MEMBER") == 0) {
	sym_index = ldrPACKAGE_PROT_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPACKAGE_PROT_MEMBER_NOVICE") == 0) {
	sym_index = ldrPACKAGE_PROT_MEMBER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPACKAGE_PROT_METHOD_NOVICE") == 0) {
	sym_index = ldrPACKAGE_PROT_METHOD_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPACKAGE_PROT_METHOD_PARAMETER_NOVICE") == 0) {
	sym_index = ldrPACKAGE_PROT_METHOD_PARAMETER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_METHOD") == 0){
	sym_index = ldrPRIVATE_METHOD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_VIRTUAL_CLASS") == 0){
	sym_index = ldrPUBLIC_VIRTUAL_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_VIRTUAL_CLASS") == 0){
	sym_index = ldrPROTECTED_VIRTUAL_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_VIRTUAL_CLASS") == 0){
	sym_index = ldrPRIVATE_VIRTUAL_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCURRENT_CLASS") == 0){
	sym_index = ldrCURRENT_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_CLASS") == 0){
	sym_index = ldrPUBLIC_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_CLASS") == 0){
	sym_index = ldrPROTECTED_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_CLASS") == 0){
	sym_index = ldrPRIVATE_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_MEMBER") == 0){
	sym_index = ldrPUBLIC_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_MEMBER") == 0){
	sym_index = ldrPROTECTED_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_MEMBER") == 0){
	sym_index = ldrPRIVATE_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrNOTE") == 0){
	sym_index = ldrNOTE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCURRENT_SUBSYSTEM") == 0){
	sym_index = ldrCURRENT_SUBSYSTEM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYSTEM") == 0){
	sym_index = ldrPUBLIC_SUBSYSTEM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_SUBSYSTEM") == 0){
	sym_index = ldrPROTECTED_SUBSYSTEM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYSTEM") == 0){
	sym_index = ldrPRIVATE_SUBSYSTEM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrRELATION") == 0){
	sym_index = ldrRELATION;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_METHOD_PARAMETER") == 0){
	sym_index = ldrPUBLIC_METHOD_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_METHOD_PARAMETER") == 0){
	sym_index = ldrPROTECTED_METHOD_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_METHOD_PARAMETER") == 0){
	sym_index = ldrPRIVATE_METHOD_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_NESTED_TYPE") == 0){
	sym_index = ldrPUBLIC_NESTED_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_NESTED_TYPE") == 0){
	sym_index = ldrPROTECTED_NESTED_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_NESTED_TYPE") == 0){
	sym_index = ldrPRIVATE_NESTED_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_VIRTUAL_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrPUBLIC_VIRTUAL_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_VIRTUAL_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrPROTECTED_VIRTUAL_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_VIRTUAL_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrPRIVATE_VIRTUAL_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCURRENT_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrCURRENT_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrPUBLIC_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrPROTECTED_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_TEMPLATE_PARAMETER") == 0){
	sym_index = ldrPRIVATE_TEMPLATE_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrBULLET") == 0){
	sym_index = ldrBULLET;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrIF_FLOW") == 0){
	sym_index = ldrIF_FLOW;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrLOOP_BOX") == 0){
	sym_index = ldrLOOP_BOX;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrOVAL") == 0){
	sym_index = ldrOVAL;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_METHOD") == 0){
	sym_index = ldrHIDDEN_METHOD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_METHOD_PARAMETER") == 0){
	sym_index = ldrHIDDEN_METHOD_PARAMETER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_MEMBER") == 0){
	sym_index = ldrHIDDEN_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_NESTED_TYPE") == 0){
	sym_index = ldrHIDDEN_NESTED_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_EMPTY") == 0){
	sym_index = ldrFLOW_EMPTY;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_LABEL") == 0){
	sym_index = ldrFLOW_LABEL;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_YES") == 0){
	sym_index = ldrFLOW_YES;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_SWITCH") == 0){
	sym_index = ldrFLOW_SWITCH;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_CASE") == 0){
	sym_index = ldrFLOW_CASE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_DEFAULT") == 0){
	sym_index = ldrFLOW_DEFAULT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFLOW_LOOP") == 0){
	sym_index = ldrFLOW_LOOP;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFUNC_CALL") == 0){
	sym_index = ldrFUNC_CALL;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrERD_CLASS") == 0){
	sym_index = ldrERD_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrERD_MEMBER") == 0){
	sym_index = ldrERD_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrERD_ELLIPSIS") == 0){
	sym_index = ldrERD_ELLIPSIS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROJECT") == 0){
	sym_index = ldrPROJECT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMODULE") == 0){
	sym_index = ldrMODULE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrSTATEMENT") == 0){
	sym_index = ldrSTATEMENT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFUNCTION") == 0){
	sym_index = ldrFUNCTION;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrEXPRESSION") == 0){
	sym_index = ldrEXPRESSION;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMODULE_STD") == 0){
	sym_index = ldrMODULE_STD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMODULE_SMT") == 0){
	sym_index = ldrMODULE_SMT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMODULE_SEARCH") == 0){
	sym_index = ldrMODULE_SEARCH;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMODULE_PROJECT") == 0){
	sym_index = ldrMODULE_PROJECT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrSM_SUBSYSTEM") == 0){
	sym_index = ldrSM_SUBSYSTEM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_METHOD_NOVICE") == 0){
	sym_index = ldrPUBLIC_METHOD_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_METHOD_NOVICE") == 0){
	sym_index = ldrPROTECTED_METHOD_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_METHOD_NOVICE") == 0){
	sym_index = ldrPRIVATE_METHOD_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_MEMBER_NOVICE") == 0){
	sym_index = ldrPUBLIC_MEMBER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_MEMBER_NOVICE") == 0){
	sym_index = ldrPROTECTED_MEMBER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_MEMBER_NOVICE") == 0){
	sym_index = ldrPRIVATE_MEMBER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_METHOD_PARAMETER_NOVICE") == 0){
	sym_index = ldrPUBLIC_METHOD_PARAMETER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_METHOD_PARAMETER_NOVICE") == 0){
	sym_index = ldrPROTECTED_METHOD_PARAMETER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_METHOD_PARAMETER_NOVICE") == 0){
	sym_index = ldrPRIVATE_METHOD_PARAMETER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_NESTED_TYPE_NOVICE") == 0){
	sym_index = ldrPUBLIC_NESTED_TYPE_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPROTECTED_NESTED_TYPE_NOVICE") == 0){
	sym_index = ldrPROTECTED_NESTED_TYPE_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_NESTED_TYPE_NOVICE") == 0){
	sym_index = ldrPRIVATE_NESTED_TYPE_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_METHOD_NOVICE") == 0){
	sym_index = ldrHIDDEN_METHOD_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_METHOD_PARAMETER_NOVICE") == 0){
	sym_index = ldrHIDDEN_METHOD_PARAMETER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_MEMBER_NOVICE") == 0){
	sym_index = ldrHIDDEN_MEMBER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrHIDDEN_NESTED_TYPE_NOVICE") == 0){
	sym_index = ldrHIDDEN_NESTED_TYPE_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FUNC") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FUNC;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FUNC") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FUNC;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_CLASS") == 0){
	sym_index = ldrPUBLIC_SUBSYS_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_CLASS") == 0){
	sym_index = ldrPRIVATE_SUBSYS_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_VAR") == 0){
	sym_index = ldrPUBLIC_SUBSYS_VAR;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_VAR") == 0){
	sym_index = ldrPRIVATE_SUBSYS_VAR;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_ENUM") == 0){
	sym_index = ldrPUBLIC_SUBSYS_ENUM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrSUBSYS_TEST") == 0){
	sym_index = ldrSUBSYS_TEST;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_METHOD") == 0){
	sym_index = ldrXREF_METHOD;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_CLASS") == 0){
	sym_index = ldrXREF_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_MEMBER") == 0){
	sym_index = ldrXREF_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_NESTED_TYPE") == 0){
	sym_index = ldrXREF_NESTED_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_ERD_CLASS") == 0){
	sym_index = ldrXREF_ERD_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_ERD_MEMBER") == 0){
	sym_index = ldrXREF_ERD_MEMBER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_METHOD_NOVICE") == 0){
	sym_index = ldrXREF_METHOD_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_MEMBER_NOVICE") == 0){
	sym_index = ldrXREF_MEMBER_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_NESTED_TYPE_NOVICE") == 0){
	sym_index = ldrXREF_NESTED_TYPE_NOVICE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_UNKNOWN") == 0){
	sym_index = ldrPUBLIC_SUBSYS_UNKNOWN;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_UNKNOWN") == 0){
	sym_index = ldrPRIVATE_SUBSYS_UNKNOWN;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_ENUM") == 0){
	sym_index = ldrPRIVATE_SUBSYS_ENUM;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrXREF_CURRENT_CLASS") == 0){
	sym_index = ldrXREF_CURRENT_CLASS;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFUNC_CALL_ROOT") == 0){
	sym_index = ldrFUNC_CALL_ROOT;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMULTIPLE_IF") == 0){
	sym_index = ldrMULTIPLE_IF;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFUNCTION_TYPE") == 0){
	sym_index = ldrFUNCTION_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCLASS_TYPE") == 0){
	sym_index = ldrCLASS_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrVARIABLE_TYPE") == 0){
	sym_index = ldrVARIABLE_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrENUM_TYPE") == 0){
	sym_index = ldrENUM_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrUNKNOWN_TYPE") == 0){
	sym_index = ldrUNKNOWN_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrSUBSYSTEM_TYPE") == 0){
	sym_index = ldrSUBSYSTEM_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMACRO_TYPE") == 0){
	sym_index = ldrMACRO_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrUNION_TYPE") == 0){
	sym_index = ldrUNION_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrENUMVALUE_TYPE") == 0){
	sym_index = ldrENUMVALUE_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrTYPEDEF_TYPE") == 0){
	sym_index = ldrTYPEDEF_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrBLANK1") == 0){
	sym_index = ldrBLANK1;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFIELD_TYPE") == 0){
	sym_index = ldrFIELD_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrSEM_TYPE") == 0){
	sym_index = ldrSEM_TYPE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrLIST1") == 0){
	sym_index = ldrLIST1;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrLIST2") == 0){
	sym_index = ldrLIST2;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrOPEN_FOLDER") == 0){
	sym_index = ldrOPEN_FOLDER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCLOSED_FOLDER") == 0){
	sym_index = ldrCLOSED_FOLDER;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFILE") == 0){
	sym_index = ldrFILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrCPP_FILE") == 0){
	sym_index = ldrCPP_FILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrC_FILE") == 0){
	sym_index = ldrC_FILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrSTE_FILE") == 0){
	sym_index = ldrSTE_FILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrFRAME_FILE") == 0){
	sym_index = ldrFRAME_FILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrMAKEFILE") == 0){
	sym_index = ldrMAKEFILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_MACRO") == 0){
	sym_index = ldrPUBLIC_SUBSYS_MACRO;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_MACRO") == 0){
	sym_index = ldrPRIVATE_SUBSYS_MACRO;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_UNION") == 0){
	sym_index = ldrPUBLIC_SUBSYS_UNION;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_UNION") == 0){
	sym_index = ldrPRIVATE_SUBSYS_UNION;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_TYPEDEF") == 0){
	sym_index = ldrPUBLIC_SUBSYS_TYPEDEF;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_TYPEDEF") == 0){
	sym_index = ldrPRIVATE_SUBSYS_TYPEDEF;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FILE_UNKNOWN") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FILE_UNKNOWN;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FILE_UNKNOWN") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FILE_UNKNOWN;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FILE_CPP") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FILE_CPP;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FILE_CPP") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FILE_CPP;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FILE_C") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FILE_C;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FILE_C") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FILE_C;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FILE_STE") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FILE_STE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FILE_STE") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FILE_STE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FILE_FRAME") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FILE_FRAME;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FILE_FRAME") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FILE_FRAME;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPUBLIC_SUBSYS_FILE_MAKEFILE") == 0){
	sym_index = ldrPUBLIC_SUBSYS_FILE_MAKEFILE;
	goto index_found;
    }
    if(strcmp(sym_name, "ldrPRIVATE_SUBSYS_FILE_MAKEFIL") == 0){
	sym_index = ldrPRIVATE_SUBSYS_FILE_MAKEFILE;
	goto index_found;
    }
    printf("GraWorld: Incorrect symbol name.\n");
  index_found:;
}

extern "C" void sym_set_text_mode(char *mode)
{
    if(sym_index < 0)
	return;
    if(strcmp(mode, "textinside") == 0)
	graSymbolArray[sym_index].text_mode = graTextModeInside;
    else
	if(strcmp(mode, "textpositioned") == 0)
	    graSymbolArray[sym_index].text_mode = graTextModePositioned;
    else
	if(strcmp(mode, "textright") == 0)
	    graSymbolArray[sym_index].text_mode = graTextModeRight;
    else
	printf("GraWorld: Incorrect symbol text mode.\n");
}

extern "C" void sym_define_layer(char *layer_name)
{
    sym_layer = -1;

    if(strcmp(layer_name, "graLayerNoBase") == 0)
	sym_layer = graLayerNoBase;
    else
	if(strcmp(layer_name, "graLayerNoNormal") == 0)
	    sym_layer = graLayerNoNormal;
    else
	if(strcmp(layer_name, "graLayerNoExpanded") == 0)
	    sym_layer = graLayerNoExpanded;
    else
	if(strcmp(layer_name, "graLayerNoCollapsed") == 0)
	    sym_layer = graLayerNoCollapsed;
    else
	if(strcmp(layer_name, "graLayerNoDecorationLL") == 0)
	    sym_layer = graLayerNoDecorationLL;
    else
	if(strcmp(layer_name, "graLayerNoDecorationCL") == 0)
	    sym_layer = graLayerNoDecorationCL;
    else
	if(strcmp(layer_name, "graLayerNoDecorationUL") == 0)
	    sym_layer = graLayerNoDecorationUL;
    else
	if(strcmp(layer_name, "graLayerNoDecorationLC") == 0)
	    sym_layer = graLayerNoDecorationLC;
    else
	if(strcmp(layer_name, "graLayerNoDecorationUC") == 0)
	    sym_layer = graLayerNoDecorationUC;
    else
	if(strcmp(layer_name, "graLayerNoDecorationLR") == 0)
	    sym_layer = graLayerNoDecorationLR;
    else
	if(strcmp(layer_name, "graLayerNoDecorationCR") == 0)
	    sym_layer = graLayerNoDecorationCR;
    else
	if(strcmp(layer_name, "graLayerNoDecorationUR") == 0)
	    sym_layer = graLayerNoDecorationUR;
    else
	if(strcmp(layer_name, "graLayerNoHilited") == 0)
	    sym_layer = graLayerNoHilited;
    else
	if(strcmp(layer_name, "graLayerNoSelected") == 0)
	    sym_layer = graLayerNoSelected;
    else
	if(strcmp(layer_name, "graLayerNoTextHilight") == 0)
	    sym_layer = graLayerNoTextHilight;
    else
	if(strcmp(layer_name, "graLayerNoBreakMarker") == 0)
	    sym_layer = graLayerNoBreakMarker;
    else
	if(strcmp(layer_name, "graLayerNoDisabledBreakMarker") == 0)
	    sym_layer = graLayerNoDisabledBreakMarker;
    else
	if(strcmp(layer_name, "graLayerNoHardAssocMarker") == 0)
	    sym_layer = graLayerNoHardAssocMarker;
    else
	if(strcmp(layer_name, "graLayerNoExecutionMarker") == 0)
	    sym_layer = graLayerNoExecutionMarker;
    else
	if(strcmp(layer_name, "graLayerNoErrorMarker") == 0)
	    sym_layer = graLayerNoErrorMarker;
    else
	if(strcmp(layer_name, "graLayerNoSearchMarker") == 0)
	    sym_layer = graLayerNoSearchMarker;
    else
	if(strcmp(layer_name, "graLayerNoNote") == 0)
	    sym_layer = graLayerNoNote;
    else
	if(strcmp(layer_name, "graLayerNoHyperMarker") == 0)
	    sym_layer = graLayerNoHyperMarker;
    else
	if(strcmp(layer_name, "graLayerNoCursor") == 0)
	    sym_layer = graLayerNoCursor;
    else
	printf("GraWorld: unknown graLayer specified.\n");
}

extern "C" void sym_set_text(char *txt)
{
    if(sym_index < 0)
	return;
    graSymbolArray[sym_index].text = strdup(txt);
}

extern "C" void sym_moveto(int x, int y)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    graSymbolArray[sym_index].pictel[sym_layer]->AddMoveto(x, y);
}

extern "C" void sym_lineto(int x, int y)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    graSymbolArray[sym_index].pictel[sym_layer]->AddLineto(x, y);
}

extern "C" void sym_linewidth(int lw)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    graSymbolArray[sym_index].pictel[sym_layer]->AddLineWidth(lw);
}

extern "C" void sym_ellipse(int x, int y, int w, int h)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    graSymbolArray[sym_index].pictel[sym_layer]->AddEllipse(x, y, w, h);
}

extern "C" void sym_color(int r, int g, int b) 
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    r = 0.01 * vcolorCOMPONENT_MAX * r;
    g = 0.01 * vcolorCOMPONENT_MAX * g;
    b = 0.01 * vcolorCOMPONENT_MAX * b;
    graSymbolArray[sym_index].pictel[sym_layer]->SetColors(r, g, b);
}

extern "C" void sym_stroke(void)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    graSymbolArray[sym_index].pictel[sym_layer]->AddStroke();
}

extern "C" void sym_fill(void)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    graSymbolArray[sym_index].pictel[sym_layer]->AddFill();
}

extern "C" void sym_image(char *image_name)
{
    if(sym_index < 0 || sym_layer < 0)
	return;
    for(int i = 0; i < num_images; i++){
	if(strcmp(graImageArray[i].name, image_name) == 0){
	    graSymbolArray[sym_index].pictel[sym_layer]->AddImage(i);
	    return;
	}
    }
    printf("graWorld: Image %s not found.\n", image_name);
}

extern "C" void sym_attachment(char *attach_point_name, int x, int y)
{
    int attach_index = -1;

    if(sym_index < 0)
	return;
    if(strcmp(attach_point_name, "lc") == 0)
	attach_index = graAttachLC;
    else
	if(strcmp(attach_point_name, "cr") == 0)
	    attach_index = graAttachCR;
    else
	if(strcmp(attach_point_name, "cl") == 0)
	    attach_index = graAttachCL;
    else
	if(strcmp(attach_point_name, "uc") == 0)
	    attach_index = graAttachUC;
    else
	if(strcmp(attach_point_name, "lc") == 0)
	    attach_index = graAttachLC;
    graSymbolArray[sym_index].pictel[graLayerNoNormal]->x[attach_index] = x;
    graSymbolArray[sym_index].pictel[graLayerNoNormal]->y[attach_index] = y;
}

/* ========================================================================== */

int decode_red(vstr *str)
{
    char buf[20];
    int  i = 0;

    while(*str != ' ' && *str != 0){
	buf[i++] = *str;
	str++;
    }
    buf[i] = 0;
    return atoi(buf);
}

int decode_green(vstr *str)
{
    char buf[20];
    int  i = 0;

    while(*str != ' ' && *str != 0)
	str++;
    if(*str == 0)
	return -1;
    str++;
    while(*str != ' ' && *str != 0){
	buf[i++] = *str;
	str++;
    }
    buf[i] = 0;
    return atoi(buf);
}

int decode_blue(vstr *str)
{
    char buf[20];
    int  i = 0;

    while(*str != ' ' && *str != 0)
	str++;
    if(*str == 0)
	return -1;
    str++;
    while(*str != ' ' && *str != 0)
	str++;
    if(*str == 0)
	return -1;
    str++;
    while(*str != ' ' && *str != 0){
	buf[i++] = *str;
	str++;
    }
    buf[i] = 0;
    return atoi(buf);
}

int gra_lookup_color(char *name, int& r, int& g, int& b)
{
    int ret_code = 0;
    r = 0;                    
    g = 0;
    b = 0;                    // default to black

    if (name)
    for(int i = 0; i < num_colors; i++){
	if(strcmp(graColorsArray[i].name, name) == 0){
	    r = graColorsArray[i].r;
	    g = graColorsArray[i].g;
	    b = graColorsArray[i].b;
	    ret_code = 1;
          break;
	}
    }
    if (ret_code == 0)
       printf("graWorld: Color %s not found.\n", name);
    return ret_code;
}
 
graWorld::graWorld( int argc, char *argv[] )
{
//    printf("Constructing graWorld.\n");
    load_images();
    load_colors();
    load_symbols();
}

void graWorld::run(const int& not_done)
{
}

void graWorld::load_colors()
{
    vapplication *application = vapplication::GetCurrent();
    
    vresource colors_dictionary = vresourceGet(application->GetResources(), TAG("ConnectionColors"));
    num_colors                  = vresourceCountComponents(colors_dictionary);
    graColorsArray              = new ColorData[num_colors];
    for(int i = 0; i < num_colors; i++){
	const vname *tag       = vresourceGetNthTag(colors_dictionary, i);
	vstr *str              = vresourceGetString(colors_dictionary, tag);
	graColorsArray[i].r    = decode_red(str);
	graColorsArray[i].g    = decode_green(str);
	graColorsArray[i].b    = decode_blue(str);
	graColorsArray[i].name = strdup((char *)tag);
	vstrDestroy(str);
    }
}

void graWorld::load_images(void)
{
    vapplication *application = vapplication::GetCurrent();
    
    vresource image_dictionary;
    image_dictionary = vresourceGet(application->GetResources(), TAG("Images"));
    num_images       = vresourceCountComponents(image_dictionary);
    graImageArray    = new ImageData[num_images];
    for(int i = 0; i < num_images; i++){
	const vname *tag    = vresourceGetNthTag(image_dictionary, i);
	vresource image_res = vresourceGet(image_dictionary, tag);
	vimage *image       = new vimage(vloadableDONT_LOAD);
	image->Load(image_res);
	graImageArray[i].image = image;
	graImageArray[i].name  = strdup((char *)tag);
    }
}

extern "C" void grasym_load_defs(FILE *fl);

void graWorld::load_symbols()
{
    Initialize(graWorld::load_symbols);

    graSymbolArray = new graSymbolData[GRA_SYMBOL_ARRAY_SIZE];
    OS_dependent::bzero (graSymbolArray, GRA_SYMBOL_ARRAY_SIZE * sizeof(graSymbolData));
    num_gra_symbol_types = 0;
    for(int index = 0; index < GRA_SYMBOL_ARRAY_SIZE; index++){
	for(int i = 0; i < GRA_MAX_MODES; i++){
	    graSymbolArray[index].pictel[i] = new graAttachableSymbol();
	}
	graSymbolArray[index].text_mode = graTextModeInside;
	graSymbolArray[index].text      = NULL;
    }
    genString file = customize::getStrPref("DIS_graphics.Symbols_DataFile");
    if (!file.length()){
	;
//        throw_error(GRAERR_NO_SYMBOL_VAR); 
    }
    char *p = strrchr(file, '.');
    if(p)
	*p = (char)0;
    genString new_file;
    new_file.printf("%s.new", (char *)file);
    FILE *fl = OSapi_fopen((char *)new_file, "r");
    if(fl == NULL){
	printf("%s file not found.\n", (char *)new_file);
    } else
	grasym_load_defs(fl);
}
 
void graWorld::process_pending_events()
{
}
 
graAttachableSymbol *graWorld::get_pictel( int symbol, int pic )
{
    return ::graSymbolArray[symbol].pictel[pic];
}

void graWorld::get_symbol_size(int symbol,
			       float &cx, float &cy, float &w, float &h )
{
}
 
int graWorld::get_text_mode( int symbol )
{
    return ::graSymbolArray[symbol].text_mode;
}

char *graWorld::get_text( int symbol )
{
    return ::graSymbolArray[symbol].text;
}

void graWorld::flush()
{
}


extern "C" void ui_get_window_size(void *, int *w, int *h)
{
    *w = 500;
    *h = 400;
}

vbool DomainItem::HandleButtonDownEvent(vdomainviewButtonDownEvent *event)
{
    vdomainObjectSet  *connection_objects;
    vdomainObjectSet  *objectSet;

    vdomain *theDomain   = GetDomain();
    vpoint pos           = event->position;
    connection_objects   = vdomainCreateObjectSet(NULL);
    vdomainSelection sel = domain->CreateSelection();
    objectSet            = domain->GetObjectSet(sel, vdomainALL, NULL);
    domain->DestroySelection(sel);
    vdomainObjectSetIterator iter;
    iter.Start(objectSet);
    while(iter.Next()){
	vdomainObject *obj       = iter.GetObject();
	graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
	if(sym->SymbolType() == graConnectionSymbolType || sym->SymbolType() == graFlexConnType)
	    vdomainAddToObjectSet(&connection_objects, obj);
    }

    int x = pos.GetX();
    int y = pos.GetY();
    iter.Start(connection_objects);
    while(iter.Next()){
	vdomainObject *obj       = iter.GetObject();
	graParametricSymbol *sym = (graParametricSymbol *)domain->GetObjectData(obj);
	if(sym->PointNear(x, y, 5)){
	    printf("selection detected.\n");

	    viewSymbolNodePtr viewnode = sym->get_viewnode();
	    genString status_line;
	    viewnode->description(status_line);
	    const char *pc = status_line.str();
	    if (pc == NULL)
		pc = "";
	    printf("%s\n", pc);

	    return vFALSE;
	}
    }

    printf("Clicked at : %d %d\n", pos.GetX(), pos.GetY());
    return vTRUE; // process further events
}

void DomainItem::DrawObject(vdomainObject *object)
{
    int cx, cy, w, h;

    vdomain *theDomain       = GetDomain();
    graParametricSymbol *sym = (graParametricSymbol *)theDomain->GetObjectData(object);
    sym->Draw(cx, cy, w, h, TRUE);
}

void DomainItem::DrawSelection(vdomainObject *object)
{
    vrect	*bounds;
    
    vdomain *theDomain = GetDomain();
    graParametricSymbol *sym = (graParametricSymbol *)theDomain->GetObjectData(object);
    if(sym->SymbolType() == graConnectionSymbolType || sym->SymbolType() == graFlexConnType)
	return;

    vdomainSelection sel = GetSelection();
    if(theDomain->IsSelected(sel, object))
	printf("--Selected\n");
    else
	printf("++Not selected\n");

    vdrawGSave();
    
    vdrawSetLineWidth(1);
    vdrawSetFunction(vdrawFUNC_INVERT);
    
    bounds = theDomain->GetObjectBounds(object);
    
    vdrawRectsClip(bounds, 1);
    
    vdrawMoveTo(bounds->x, bounds->y + 1);
    vdrawLineTo(bounds->x + bounds->w, bounds->y + bounds->h + 1);
    vdrawMoveTo(bounds->x, bounds->y + bounds->h);
    vdrawLineTo(bounds->x + bounds->w, bounds->y);
    vdrawStroke();
    
    vdrawGRestore();
}

vimage *gra_get_image(int index)
{
    return graImageArray[index].image;
}

void DomainShell::OpenDomain(void)
{
    vresource resource;

    vapplication *application = vapplication::GetCurrent();
    resource                  = vresourceGet(application->GetResources(), TAG("Viewer"));
    domain_shell              = new DomainShell(vloadableDONT_LOAD);
    domain_shell->domain_item = (vdomainitem *)DomainItem::RequestSubLoad(domain_shell, TAG("view_domain"));
    domain_shell->Load(resource);
    domain_shell->Open();
    ui_font = domain_shell->domain_item->DetermineFont();
}

vdomainitem *DomainShell::GetDomainItem(void)
{
    return domain_shell->domain_item;
}

vdomainitem *ui_get_domain_item(void)
{
    return DomainShell::GetDomainItem();
}

vfont *ui_get_font(void)
{
    return ui_font;
}

ldrHeader* dialog_layout(symbolPtr, repType);

static appTreePtr dd_get_smt_or_smt_root(ddElement *dd)
{
    if (!dd) return NULL;
    appTreePtr at = dd_get_smt(dd);
    if (!at) {
	at = (appTreePtr)get_relation(ref_file_of_dd, dd);
	if (!at) {
	    appPtr ah = dd->get_main_header();
	    if (ah && is_smtHeader(ah))
		at = (appTreePtr)ah->get_root();
	}
    }
    return at;
}

extern viewPtr ct_viewGraHeader(ldrPtr, repType);
extern bool app_is_not_viewable(appPtr app_head);

char *create_gra_view_name(char *type_name, symbolArr& syms, int num)
{
    static genString name;

    name  = type_name;
    name += ": ";
    for(int i = 0; i < num - 1; i++){
	name += syms[i].get_name();
	if(strlen((char *)name) >  120){
	    name += "...";
	    return (char *)name;
	}
	name += ", ";
    }
    i     = num - 1;
    name += syms[i].get_name();
    return (char *)name;
}

view *gra_create_call_tree_view(symbolArr& symbols)
{
    symbolPtr sym       = symbols[0];
    ldrHeader *ldr_head = dialog_layout(sym, Rep_Tree);
    viewPtr view_head   = NULL;
    if(ldr_head)
    {
//	view_head = vsh->find_view_for_viewer(ldr_head, view_target_viewer());
	view_head = dialog::open_view((ldr *)ldr_head, Rep_Tree, 0);
	if(view_head){
	    char *nm = create_gra_view_name("Tree Diagram", symbols, 1);
	    view_head->set_name(nm);
	}
    }
    return view_head;
}

view *gra_create_flowchart_view(symbolArr& symbols)
{
    ldrHeader *ldr_head = NULL;
    symbolPtr sym       = symbols[0];
    viewPtr view_head = NULL;
    if (sym.get_kind() != DD_FUNC_DECL) {
      dis_message(NULL, MSG_WARN, "Only functions can be displayed in flowcharts.");
      return view_head;
    } 
    ddElement *dd       = sym->get_def_dd();
    if(dd){
		appTreePtr app_node = dd_get_smt_or_smt_root(dd);
		if (app_node)
		{
			appPtr app_head= (appPtr)app_node->get_header();
    	    	    	if (!app_is_not_viewable(app_head))
				ldr_head=dialog::layout(app_head, app_node, Rep_FlowChart, 0, 0, rtlDisplayUnknown);
		}
    } else {
		RelationalPtr obj = sym;
		if(obj != NULL){
			if(is_appTree(obj)){
				appTreePtr app_node = appTreePtr(obj);
				appPtr app_head     = (appPtr)app_node->get_header();
    	        	    	if (!app_is_not_viewable(app_head))
					ldr_head            =
						dialog::layout(app_head, app_node, Rep_FlowChart, 0, 0, rtlDisplayUnknown);
			}
		}
    }
    if(ldr_head)
    {
		//	view_head = vsh->find_view_for_viewer(ldr_head, view_target_viewer());
		view_head = dialog::open_view((ldr *)ldr_head, Rep_FlowChart, 0);
		if(view_head){
			char *nm = create_gra_view_name("Flow Chart", symbols, 1);
			view_head->set_name(nm);
		}
    }
    return view_head;
}

view *gra_create_outline_view(symbolArr& symbols)
{
    ldrHeader *ldr_head = NULL;
	
    symbolPtr sym       = symbols[0];
    ddElement *dd       = sym->get_def_dd();
    if(dd){
		appTreePtr app_node = dd_get_smt_or_smt_root(dd);
		if(app_node){
			appPtr app_head     = (appPtr)app_node->get_header();
    	    	    	if (!app_is_not_viewable(app_head))
				ldr_head            = dialog::layout(app_head, app_node, Rep_VertHierarchy, 0, 0, rtlDisplayUnknown);
		}
	} 
	else 
	{
		RelationalPtr obj = sym;
		if(obj != NULL){
			if(is_appTree(obj))
			{
				appTreePtr app_node = appTreePtr(obj);
				appPtr app_head     = (appPtr)app_node->get_header();
    		    	    	if (!app_is_not_viewable(app_head))
					ldr_head            = dialog::layout(app_head, app_node, Rep_VertHierarchy, 0, 0, rtlDisplayUnknown);
			}
		}
    }
    viewPtr view_head   = NULL;
    if(ldr_head)
	{
		//	view_head = vsh->find_view_for_viewer(ldr_head, view_target_viewer());
		view_head = dialog::open_view((ldr *)ldr_head, Rep_VertHierarchy, 0);
		if(view_head)
		{
			char *nm = create_gra_view_name("Outline", symbols, 1);
			view_head->set_name(nm);
		}
    }
    return view_head;
}

view *gra_create_inheritance_view(symbolArr& symbols)
{
    view * view_head = (view *)0;

    symbolPtr sym = symbols[0];
    ddKind sym_kind = sym.get_kind();
    if (sym_kind == DD_CLASS || sym_kind == DD_INTERFACE) {
	ldrHeader *ldr_head = dialog_layout(sym, Rep_OODT_Inheritance);
	if(ldr_head){
	    view_head = dialog::open_view((ldr *)ldr_head, Rep_OODT_Inheritance, 0);
	    if(view_head){
		char *nm = create_gra_view_name("Inheritance", symbols, 1);
	        view_head->set_name(nm);
	    }
	}
    } else if (sym.get_name()) {
	dis_message(C_VIEW, MSG_ERROR, "M_CANTOPENINHERITANCEV", sym.get_name());
    }

    return view_head;
}

view *gra_create_datachart_view(symbolArr& symbols)
{
    symbolArr view_contents;

    view_contents.insert_last(symbols);
    ldrHeader *ldr_head = new ldrDCHierarchy(view_contents);
    viewPtr view_head   = ct_viewGraHeader(ldr_head, Rep_DataChart);
    if(view_head){
	put_relation(view_of_ldr, ldr_head, view_head);
	char *nm = create_gra_view_name("Data Chart", symbols, symbols.size());
	view_head->set_name(nm);
    }
    return view_head;
}

view *gra_create_graph_view(symbolArr& symbols)
{
    return NULL;
}

view *gra_create_erd_view(symbolArr& symbols)
{
    symbolArr view_contents;

    view_contents.insert_last(symbols);
    ldrHeader *ldr_head = new ldrERDHierarchy(view_contents);
    viewPtr view_head   = ct_viewGraHeader(ldr_head, Rep_ERD);
    if(view_head){
	put_relation(view_of_ldr, ldr_head, view_head);
	char *nm = create_gra_view_name("ERD", symbols, symbols.size());
	view_head->set_name(nm);
    }
    return view_head;
}


view *gra_create_view(symbolArr& symbols, int view_type, void *view_layer)
{
    view *v = NULL;
    if (symbols.size() > 0) {
    switch(view_type) {
      case viewCallTree:
	v = gra_create_call_tree_view(symbols);
	break;
      case viewFlowchart:
	v = gra_create_flowchart_view(symbols);
	break;
      case viewOutline:
	v = gra_create_outline_view(symbols);
	break;
      case viewInheritance:
	v = gra_create_inheritance_view(symbols);
	break;
      case viewDatachart:
	v = gra_create_datachart_view(symbols);
	break;
      case viewGraph:
	v = gra_create_graph_view(symbols);
	break;
      case viewERD:
	v = gra_create_erd_view(symbols);
	break;
     default:
	printf("Wrong view type selected.\n");
    }
    if(v)
	v->open_window((long)view_layer);
    }
    return v;
}

view *gra_create_view(symbolPtr sym, repType rep_type)
{
    if (sym.isnotnull() && sym.relationalp()) {
	Relational * rel = sym;
	if (rel) {
	    if (is_appDGraph(rel) && rep_type==Rep_DGraph) {
		appDGraph * gh        = (appDGraph *)rel;
		viewPtr view_head     = NULL;
		appTreePtr app_node   = (appTree *)(gh->get_root()); 
		appTreePtr focus_node = app_node;
		app_node              = app_node->get_world();
		ldrPtr ldr_head       = dialog::layout(gh, app_node, rep_type, 0, 0, rtlDisplayUnknown);
		view_head             = dialog::open_view(ldr_head, rep_type, 0);
		return view_head;
	    }
	}
    }
    
    view *v = NULL;

    symbolPtr xsym = sym.get_xrefSymbol();
    if(xsym.isnotnull()){
	symbolArr syms;
	syms.insert_last(xsym);
	switch(rep_type){
	  case Rep_Tree:
	    v = gra_create_call_tree_view(syms);
	    break;
	  case Rep_FlowChart:
	    v = gra_create_flowchart_view(syms);
	    break;
	  case Rep_VertHierarchy:
	    v = gra_create_outline_view(syms);
	    break;
	  case Rep_OODT_Inheritance:
	    v = gra_create_inheritance_view(syms);
	    break;
	  case Rep_UNKNOWN:
	    push_busy_cursor();	
	    dis_create_view_of_def( xsym );
	    pop_cursor();
	    break;
	}
    }
    return v;
}
    
void domain_view_create(symbolPtr sym1, symbolPtr sym2, repType rep_type)
{
    symbolArr view_contents;

    view_contents.insert_last(sym1);
    view_contents.insert_last(sym2);
    ldrHeader *ldr_head = new ldrERDHierarchy(view_contents);
    viewPtr view_head   = ct_viewGraHeader(ldr_head, Rep_ERD);
    view_head->open_window(0);
}

void OpenDomainShell()
{
    DomainShell::OpenDomain();

    symbolPtr sym1 =  lookup_xrefSymbol(DD_CLASS, "Alex");
    symbolPtr sym2 =  lookup_xrefSymbol(DD_CLASS, "Chen");
    if(sym1.xrisnotnull()){
	domain_view_create(sym1, sym2, Rep_ERD);
    }
}

//
// returns: -1: error, 0: if the graph was built
//

//This function should return non-zero value in case of success
//If this function returns 0 (failure), GreateGraph dialog will not be close
//even if OK was pressed 	
int build_graph(symbolArr &src, symbolArr &tar, char *device_name)
{
    disUIHandler *handler = (disUIHandler *)0;
    int result = generate_call_graph(src, tar, &handler);
    if( result >= 0 && handler && device_name )
      handler->output( device_name );
    if( handler )
      delete handler;
    return(result >= 0);
}
