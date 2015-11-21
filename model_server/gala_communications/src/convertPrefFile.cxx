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
// **********
//
// convertedPrefFile.cxx - Preference File Subclass Class to handle conversion
//                         from text to galaxy format.
//
// **********

// cLibraryFunctions must be first to define file i/o stuff for NT.
// Unfortunately, this result in required vbool to be used instead of bool.
#include <cLibraryFunctions.h>
#include <vport.h>
#include <vbtree.h>
#include <convertPrefFile.h>
#include <galaxy_undefs.h>
#include <machdep.h>
#include <messages.h>
#include <genString.h>


// Table of old and new key names.
////////// THIS TABLE IS NOT CURRENTLY USED.
//    Types: 0=String, 1=Int, 2=Multi-Line-String
TranslateInfo convertPrefFile::keyTable[] = {
//         Old Key Name                           New Key Name                Type
//  ------------------------------      --------------------------------      -----
    {"*psetPrefs.Configurator",		"DIS_cm.CmExecutable",			0 },
    {"*psetPrefs.Dependency_Output",	"PSET.Dependency_Output",		0 },
    {"*psetPrefs.DIS_misc_BuildMetrics","DIS_model.SaveMetrics",		1 },
    {"*psetPrefs.DIS_misc_StoreMetrics","DIS_model.SaveMetrics",		1 },
    {"*psetPrefs.PutTrackBugs",		"DIS_cm.CmPutTrackBugs",		1 },
    {"*psetPrefs.VerifyPDF",		"DIS_misc.VerifyPDF",			1 },
    {"*psetPrefs.acceptSyntaxErrors",	"DIS_misc.Forgive",			1 },
    {"*psetPrefs.after-class-decl",	"DIS_bp.after-class-decl",		0 },
    {"*psetPrefs.after-class-def",	"DIS_bp.after-class-def",		0 },
    {"*psetPrefs.after-func-decl",	"DIS_bp.after-func-decl",		0 },
    {"*psetPrefs.after-func-def",	"DIS_bp.after-func-def",		0 },
    {"*psetPrefs.after-member-decl",	"DIS_bp.after-member-decl",		0 },
    {"*psetPrefs.after-member-def",	"DIS_bp.after-member-def",		0 },
    {"*psetPrefs.after-struct",		"DIS_bp.after-struct",			0 },
    {"*psetPrefs.before-class-decl",	"DIS_bp.before-class-decl",		0 },
    {"*psetPrefs.before-class-def",	"DIS_bp.before-class-def",		0 },
    {"*psetPrefs.before-func-decl",	"DIS_bp.before-func-decl",		0 },
    {"*psetPrefs.before-func-def",	"DIS_bp.before-func-def",		0 },
    {"*psetPrefs.before-member-decl",	"DIS_bp.before-member-decl",		0 },
    {"*psetPrefs.before-member-def",	"DIS_bp.before-member-def",		0 },
    {"*psetPrefs.before-struct",	"DIS_bp.before-struct",			0 },
    {"*psetPrefs.buildMetrics",		"DIS_model.SaveMetrics",		1 },
    {"*psetPrefs.buttonbarClass",	"DIS_buttonbar.ButtonbarClass",		0 },
    {"*psetPrefs.buttonbarERD",		"DIS_buttonbar.ButtonbarERD",		0 },
    {"*psetPrefs.buttonbarSSBrowser",	"DIS_buttonbar.ButtonbarSSBrowser",	0 },
    {"*psetPrefs.buttonbarSSMap",	"DIS_buttonbar.ButtonbarSSMap",		0 },
    {"*psetPrefs.buttonbarSTE",		"DIS_buttonbar.ButtonbarSTE",		0 },
    {"*psetPrefs.buttonbarSMT",		"DIS_buttonbar.ButtonbarSMT",		0 },
    {"*psetPrefs.buttonbarCalltree",	"DIS_buttonbar.ButtonbarCalltree",	0 },
    {"*psetPrefs.buttonbarDatachart",	"DIS_buttonbar.ButtonbarDatachart",	0 },
    {"*psetPrefs.buttonbarVisible",	"DIS_buttonbar.ButtonbarVisible",	1 },
    {"*psetPrefs.buttonbar_bar_visible", "DIS_buttonbar.ButtonbarVisible",	1 },
    {"*psetPrefs.c-hdr-file-bottom",	"DIS_bp.c-hdr-file-bottom",		0 },
    {"*psetPrefs.c-hdr-file-top",	"DIS_bp.c-hdr-file-top",		0 },
    {"*psetPrefs.c-src-file-bottom",	"DIS_bp.c-src-file-bottom",		0 },
    {"*psetPrefs.c-src-file-top",	"DIS_bp.c-src-file-top",		0 },
    {"*psetPrefs.cCompiler",		"DIS_c.CCompiler",			0 },
    {"*psetPrefs.cDefines",		"DIS_c.CDefines",			0 },
    {"*psetPrefs.cDfltHdrSuffix",	"DIS_c.CDefaultHeaderSuffix",		0 },
    {"*psetPrefs.cDfltSrcSuffix",	"DIS_c.CDefaultSourceSuffix",		0 },
    {"*psetPrefs.cFlags",		"DIS_c.CFlags",				0 },
    {"*psetPrefs.cHdrSuffix",		"DIS_c.CHeaderSuffix",			0 },
    {"*psetPrefs.cIncludes",		"DIS_c.CIncludes",			0 },
    {"*psetPrefs.cPlusCompiler",	"DIS_cpp.CPlusCompiler",		0 },
    {"*psetPrefs.cPlusDefines",		"DIS_cpp.CPlusDefines",			0 },
    {"*psetPrefs.cPlusDfltHdrSuffix",	"DIS_cpp.CPlusDefaultHeaderSuffix",	0 },
    {"*psetPrefs.cPlusDfltSrcSuffix",	"DIS_cpp.CPlusDefaultSourceSuffix",	0 },
    {"*psetPrefs.cPlusFlags",		"DIS_cpp.CPlusFlags",			0 },
    {"*psetPrefs.cPlusHdrSuffix",	"DIS_cpp.CPlusHeaderSuffix",		0 },
    {"*psetPrefs.cPlusIncludes",	"DIS_cpp.CPlusIncludes",		0 },
    {"*psetPrefs.cPlusSuffix",		"DIS_cpp.CPlusSuffix",			0 },
    {"*psetPrefs.cStyle",		"DIS_c.CStyle",				1 },
    {"*psetPrefs.cSuffix",		"DIS_c.CSuffix",			0 },
    {"*psetPrefs.c_compiler",		"DIS_c.CCompiler",			0 },
    {"*psetPrefs.c_defines",		"DIS_c.CDefines",			0 },
    {"*psetPrefs.c_dft_hdr_suffix",	"DIS_c.CDefaultHeaderSuffix",		0 },
    {"*psetPrefs.c_dft_src_suffix",	"DIS_c.CDefaultSourceSuffix",		0 },
    {"*psetPrefs.c_flags",		"DIS_c.CFlags",				0 },
    {"*psetPrefs.c_includes",		"DIS_c.CIncludes",			0 },
    {"*psetPrefs.c_style",		"DIS_c.CStyle",				1 },
    {"*psetPrefs.c_suffix",		"DIS_c.CSuffix",			0 },
    {"*psetPrefs.calltree_buttonbar",	"DIS_buttonbar.ButtonbarCalltree",	0 },
    {"*psetPrefs.class_buttonbar",	"DIS_buttonbar.ButtonbarClass",		0 },
    {"*psetPrefs.clearcaseGetModel",	"DIS_cm.GetModel",			0 },
    {"*psetPrefs.clearcasePutModel",	"DIS_cm.PutModel",			0 },
    {"*psetPrefs.configurator",		"DIS_cm.CmExecutable",			0 },
    {"*psetPrefs.configuratorAfterGetScript", "DIS_cm.CmAfterGetScript",	0 },
    {"*psetPrefs.configuratorAfterPutScript", "DIS_cm.CmAfterPutScript",	0 },
    {"*psetPrefs.configuratorBeforeGetScript", "DIS_cm.CmBeforeGetScript",	0 },
    {"*psetPrefs.configuratorBeforePutScript", "DIS_cm.CmBeforePutScript",	0 },
    {"*psetPrefs.configuratorCopy",	"DIS_cm.CmCopyOptions",			0 },
    {"*psetPrefs.configuratorCreate",	"DIS_cm.CmCreateOptions",		0 },
    {"*psetPrefs.configuratorCustomDirectory", "DIS_cm.CmCustomScriptsPath",	0 },
    {"*psetPrefs.configuratorDelete",	"DIS_cm.CmDeleteOptions",		0 },
    {"*psetPrefs.configuratorDiff",	"DIS_cm.CmDiffOptions",			0 },
    {"*psetPrefs.configuratorDoBackups", "DIS_cm.CmBackupMerges",		1 },
    {"*psetPrefs.configuratorFastPutMerge", "DIS_cm.CmFastPutMergeOptions",     1 },
    {"*psetPrefs.configuratorGet",	"DIS_cm.CmGetOptions",			0 },
    {"*psetPrefs.configuratorGetWithComments", "DIS_cm.GetWithComments",	1 },
    {"*psetPrefs.configuratorLock",	"DIS_cm.CmLockOptions",			0 },
    {"*psetPrefs.configuratorMergeBackups", "DIS_cm.CmBackupMerges",		1 },
    {"*psetPrefs.configuratorObsolete", "DIS_cm.CmObsoleteOptions",		0 },
    {"*psetPrefs.configuratorOptionalScriptsDir", "DIS_cm.OptionalScriptsDir",	0 },
    {"*psetPrefs.configuratorPendingFilter", "DIS_cm.CmPendingFilter",		0 },
    {"*psetPrefs.configuratorPut",	"DIS_cm.CmPutOptions",			0 },
    {"*psetPrefs.configuratorPutLocks",	"DIS_cm.PutLocks",			1 },
    {"*psetPrefs.configuratorRemovePsets", "DIS_cm.RemovePsets",		1 },
    {"*psetPrefs.configuratorStat",	"DIS_cm.CmStat",			0 },
    {"*psetPrefs.configuratorSystem",	"DIS_cm.CmSystem",			0 },
    {"*psetPrefs.configuratorTerse",	"DIS_cm.Terse",				1 },
    {"*psetPrefs.configuratorUnlock",	"DIS_cm.CmUnlockOptions",		0 },
    {"*psetPrefs.configuratorUseLocks",	"DIS_cm.UseLocks",			1 },
    {"*psetPrefs.configuratorUseSoftLinks", "DIS_cm.UseSoftLinks",		1 },
    {"*psetPrefs.configurator_create",	"DIS_cm.CmCreateOptions",		0 },
    {"*psetPrefs.configurator_get",	"DIS_cm.CmGetOptions",			0 },
    {"*psetPrefs.configurator_lock",	"DIS_cm.CmLockOptions",			0 },
    {"*psetPrefs.configurator_obsolete", "DIS_cm.CmObsoleteOptions",		0 },
    {"*psetPrefs.configurator_put",	"DIS_cm.CmPutOptions",			0 },
    {"*psetPrefs.configurator_stat",	"DIS_cm.Stat",				0 },
    {"*psetPrefs.configurator_unget",	"DIS_cm.CmUngetOptions",		0 },
    {"*psetPrefs.configurator_unlock",	"DIS_cm.CmUnlockOptions",		0 },
    {"*psetPrefs.cplusplus_compiler",	"DIS_OBSOLETE.CPlusCompiler",		0 },
    {"*psetPrefs.cplusplus_defines",	"DIS_OBSOLETE.CPlusDefines",		0 },
    {"*psetPrefs.cplusplus_dft_hdr_suffix", "DIS_OBSOLETE.CPlusDefaultHeaderSuffix", 0 },
    {"*psetPrefs.cplusplus_dft_src_suffix", "DIS_OBSOLETE.CPlusDefaultSourceSuffix", 0 },
    {"*psetPrefs.cplusplus_flags",	"DIS_OBSOLETE.CPlusFlags",		0 },
    {"*psetPrefs.cplusplus_includes",	"DIS_OBSOLETE.CPlusIncludes",		0 },
    {"*psetPrefs.cplusplus_suffix",	"DIS_OBSOLETE.CPlusSuffix",		0 },
    {"*psetPrefs.cpp-hdr-file-bottom",	"DIS_bp.cpp-hdr-file-bottom",		0 },
    {"*psetPrefs.cpp-hdr-file-top",	"DIS_bp.cpp-hdr-file-top",		0 },
    {"*psetPrefs.cpp-src-file-bottom",	"DIS_bp.cpp-src-file-bottom",		0 },
    {"*psetPrefs.cpp-src-file-top",	"DIS_bp.cpp-src-file-top",		0 },
    {"*psetPrefs.dc_buttonbar",		"DIS_buttonbar.ButtonbarDataChart",	0 },
    {"*psetPrefs.debugCPPOutputFile",	"DIS_c.Cpp_Logfile",		        0 },
    {"*psetPrefs.debugCppOutputFile",	"DIS_c.Cpp_Logfile",		        0 },
    {"*psetPrefs.debugDiscoverExecPath", "DIS_support.DiscoverExecPath",	0 },
    {"*psetPrefs.debugEnvSave",		"DIS_support.EnvSave",			0 },
    {"*psetPrefs.debugNewUi",		"DIS_support.NewUI	",		1 },
    {"*psetPrefs.debugShowMakeRuleOutput", "DIS_debug.ShowMakeRuleOutput",	0 },
    {"*psetPrefs.debugVariablesList",	"DIS_support.VariablesList",		0 },
    {"*psetPrefs.defaultPDF",		"DIS_pdf.DefaultPDF",			0 },
    {"*psetPrefs.doPassDependency",	"DIS_model.DoPassDependency",		1 },
    {"*psetPrefs.doPassHeaders",	"DIS_model.DoPassHeaders",		1 },
    {"*psetPrefs.doPassSaveProj",	"DIS_model.DoPassSaveProj",		1 },
    {"*psetPrefs.doPassSaveRoot",	"DIS_model.DoPassSaveRoot",		1 },
    {"*psetPrefs.envVariablesList",	"DIS_support.EnvVariablesList",		0 },
    {"*psetPrefs.erd_buttonbar",	"DIS_buttonbar.ButtonbarERD",		0 },
    {"*psetPrefs.excludedDirectories", 	"DIS_misc.ParserExcludePaths",		2 },
    {"*psetPrefs.excluded_directories",	"DIS_misc.ParserExcludePaths",		2 },
    {"*psetPrefs.executableTimer",	"DIS_misc.TimerExecutable",		0 },
    {"*psetPrefs.forgive",		"DIS_misc.Forgive",			1 },
    {"*psetPrefs.func-final",		"DIS_bp.func-final",			0 },
    {"*psetPrefs.func-init",		"DIS_bp.func-init",			0 },
    {"*psetPrefs.gdb",			"DIS_debug.DebuggerPath",		0 },
    {"*psetPrefs.gdbPath",		"DIS_debug.DebuggerPath",		0 },
    {"*psetPrefs.getModel",		"DIS_cm.GetModel",			0 },
    {"*psetPrefs.installRoot",		"DIS_misc.InstallRoot",			0 },
    {"*psetPrefs.install_root",		"DIS_misc.InstallRoot",			0 },
    {"*psetPrefs.logFileBuildCompress",	"DIS_support.LogfileBuildCompress",	0 },
    {"*psetPrefs.logFileCompress",	"DIS_support.LogFileCompress",		0 },
    {"*psetPrefs.logFileCreate",	"DIS_support.LogFileCreate",		0 },
    {"*psetPrefs.logFileDir",		"DIS_support.LogFilePath",		0 },
    {"*psetPrefs.logFileMonitor",	"DIS_support.LogFileMonitor",		0 },
    {"*psetPrefs.logFileNameFixed",	"DIS_support.LogFileNameFixed",		0 },
    {"*psetPrefs.logFileUserDelete",	"DIS_support.LogFileUserDelete",	0 },
    {"*psetPrefs.logPrefValues",	"DIS_support.LogPrefValues",		0 },
    {"*psetPrefs.makePath",		"DIS_build.MakeExecutable",		0 },
    {"*psetPrefs.makefileDir",		"DIS_build.MakefileDir",		0 },
    {"*psetPrefs.makefileTargets", 	"DIS_build.MakefileTargets",		2 },
    {"*psetPrefs.makefile_dir",		"DIS_build.MakefileDir",		0 },
    {"*psetPrefs.member-def-final",	"DIS_bp.member-def-final",		0 },
    {"*psetPrefs.member-def-init",	"DIS_bp.member-def-init",		0 },
    {"*psetPrefs.miniBrowserHistory",	"DIS_misc.MiniBrowserHistory",		1 },
    {"*psetPrefs.miniBrowserTitles",	"DIS_misc.MiniBrowserTitles",		1 },
    {"*psetPrefs.msgLogFile",           "DIS_msg.MsgLogFile",                  0 },
    {"*psetPrefs.msgLogger",		"DIS_msg.UseMsgLogger",			1 },
    {"*psetPrefs.msg_logger",		"DIS_msg.UseMsgLogger",			1 },
    {"*psetPrefs.nameCompany",		"DIS_auth.CompanyName",			0 },
    {"*psetPrefs.nameDiscoverAdministrator", "DIS_auth.AdministratorName",	0 },
    {"*psetPrefs.noPDFWarning",		"PSET.No_PDF_Warning",			1 },
    {"*psetPrefs.paraDocCommandLine",	"DIS_misc.DOCsetCommandLine",		0 },
    {"*psetPrefs.paraDocName",		"DIS_misc.DOCsetName",			0 },
    {"*psetPrefs.paraMakerPath",	"DIS_misc.DOCsetCommandLine",		0 },
    {"*psetPrefs.pdfFileBuild",		"DIS_pdf.BuildPDF",			0 },
    {"*psetPrefs.pdfFileUser",		"DIS_pdf.UserPDF",			0 },
    {"*psetPrefs.preRshEnv",		"DIS_model.PreRshEnv",			0 },
    {"*psetPrefs.priv-decl",		"DIS_bp.priv-decl",			0 },
    {"*psetPrefs.privateModelRoot",	"DIS_pdf.PrivateModelRoot",		0 },
    {"*psetPrefs.privateSrcRoot",	"DIS_pdf.PrivateSourceRoot",		0 },
    {"*psetPrefs.projectHome",		"DIS_pdf.HomeProject",			0 },
    {"*psetPrefs.projectList",		"DIS_pdf.ProjectList",			0 },
    {"*psetPrefs.projectParallel",	"DIS_pdf.ParallelProjects",		0 },
    {"*psetPrefs.prot-decl",		"DIS_bp.prot-decl",			0 },
    {"*psetPrefs.pub-decl",		"DIS_bp.pub-decl",			0 },
    {"*psetPrefs.putModel",		"DIS_cm.PutModel",			0 },
    {"*psetPrefs.relation-src-definition", "DIS_bp.relation-src-definition",	0 },
    {"*psetPrefs.relation-src-header",	"DIS_bp.relation-src-header",		0 },
    {"*psetPrefs.relation-src-member",	"DIS_bp.relation-src-member",		0 },
    {"*psetPrefs.relation-trg-definition", "DIS_bp.relation-trg-definition",	0 },
    {"*psetPrefs.relation-trg-header",	"DIS_bp.relation-trg-header",		0 },
    {"*psetPrefs.relation-trg-member",	"DIS_bp.relation-trg-member",		0 },
    {"*psetPrefs.rtlFileStat",		"DIS_misc.BrowserShowPermissions",	1 },
    {"*psetPrefs.rshProjInvoker",	"DIS_model.RshProjInvoker",		0 },
    {"*psetPrefs.saveMetrics",		"DIS_model.SaveMetrics",		1 },
    {"*psetPrefs.shadow_root",		"DIS_OBSOLETE.shadow_root",		0 },
    {"*psetPrefs.sharedModelRoot",	"DIS_pdf.SharedModelRoot",		0 },
    {"*psetPrefs.sharedSrcRoot",	"DIS_pdf.SharedSourceRoot",		0 },
    {"*psetPrefs.shell_config",		"DIS_OBSOLETE.shell_config",		0 },
    {"*psetPrefs.smod_size",		"DIS_OBSOLETE.smod_size",		1 },
    {"*psetPrefs.smt_buttonbar",	"DIS_buttonbar.ButtonbarSMT",		0 },
    {"*psetPrefs.steEditMode",		"DIS_misc.EditorMode",			1 },
    {"*psetPrefs.steSuffix",		"DIS_OBSOLETE.steSuffix",		0 },
    {"*psetPrefs.ste_suffix",		"DIS_OBSOLETE.steSuffix",		0 },
    {"*psetPrefs.storeMetrics",		"DIS_model.SaveMetrics",		1 },
    {"*psetPrefs.subsysBrowser_buttonbar", "DIS_buttonbar.ButtonbarSSBrowser",	0 },
    {"*psetPrefs.subsysMap_buttonbar",	"DIS_buttonbar.ButtonbarSSMap",	0 },
    {"*psetPrefs.subsys_setting_file",	"DIS_misc.SubsysSettingFile",		0 },
    {"*psetPrefs.txt_size",		"DIS_OBSOLETE.txt_size",		1 },
    {"*psetPrefs.version",		"DIS_OBSOLETE.version",			1 }
};

const int convertPrefFile::KEY_TABLE_SIZE = sizeof(convertPrefFile::keyTable)/sizeof(TranslateInfo);


// Constructor.
convertPrefFile::convertPrefFile() { }



// Destructor.
convertPrefFile::~convertPrefFile()
{
    // Make sure any changes are saved.
    Close();

    if (baseName_) vstrDestroy(baseName_);
    if (fullName_) vstrDestroy(fullName_);
    if (prefPath_) delete prefPath_;
    if (defaultPath_) delete defaultPath_;
    if (prefText_) {
        if (prefSelection_) prefText_->DestroySelection(prefSelection_);
        delete prefText_;
    }
}



//---------------------- Private convertPrefFile methods -----------------------



// InitBaseName - Initialize the base file name.
vbool convertPrefFile::InitBaseName(const vchar* name)
{
    vbool status = vTRUE;

    // Although never implemented, this function should test if the
    // given file exists but is of a different format. It should then
    // use the loadOldFormat function to convert the file into the new
    // format.

    if (!fileExists(name) ) {
        // File doesn't exist, create it.
        OSapi_fprintf(stderr, "WARNING - The file '%s' was not found!!!\n", name);
        baseName_ = vstrClone(name);
        baseName_ = vstrAppendScribed(baseName_, vcharScribeLiteral(".pr") );
        if (fileExists(baseName_) )
            OSapi_fprintf(stderr, "WARNING - Switching to '%s'.\n", baseName_);
        else
            baseName_ = vstrClone(name);

    } else if (fileExists(name) && fileIsGalaxyFormat(name) ) {
        // The file exists, but it is a galaxy file.
        // Use the file name with a '.pr' suffix.
        OSapi_fprintf(stderr, "WARNING - The file '%s' is a Galaxy file!!!\n",
                      name);
        OSapi_fprintf(stderr, "WARNING - You should specify a non-Galaxy file.\n");
        baseName_ = vstrClone(name);
        baseName_ = vstrAppendScribed(baseName_, vcharScribeLiteral(".pr") );
        OSapi_fprintf(stderr, "WARNING - Trying '%s'.\n",
                      baseName_);
    } else {
        // File exists and is OK.
        baseName_ = vstrClone(name);
    }

    return (status);
}




//////////     Preference Conversion Routines.     //////////


// Load old prefs. Return true if successfull.
vbool convertPrefFile::loadOldPrefFile(const char* pref_file)
{
    const int MAX_PREF_LEN = 2048;

    vbool status = vFALSE;

    FILE* ff = OSapi_fopen(pref_file, "r");
    if (ff) {
        status = vTRUE;
        char line[MAX_PREF_LEN];
        char oldKey[MAX_PREF_LEN];
        char oldValue[MAX_PREF_LEN];
	char new_line[MAX_PREF_LEN];
        int value;
        char* newKey;
	// reading a line; \<char> = <char>, \ at end of line = <CR>
	do {
	    int len = 0;
	    vbool prev_was_bs = vFALSE;
	    vbool line_continues;
	    vbool skip = vFALSE;
	    do {
		line_continues = vFALSE;
		if (OSapi_fgets(new_line, MAX_PREF_LEN, ff) == NULL) break;

		int i = 0;
		for ( ; isspace(new_line[i]); i++);
		for ( ; new_line[i] && new_line[i]!='\n'; i++) {
		    if (prev_was_bs) prev_was_bs = vFALSE;
		    else if (new_line[i]=='\\') prev_was_bs = vTRUE;
		    if (len<MAX_PREF_LEN) line[len++] = new_line[i];
		}
				
		if (len>=MAX_PREF_LEN || i+1 >= MAX_PREF_LEN) skip = vTRUE;
		line_continues = (i+1>=MAX_PREF_LEN || prev_was_bs);
		if (prev_was_bs && i+1<MAX_PREF_LEN) {
		    line[len-1] = '\n';
		    prev_was_bs = vFALSE;
		}
		
	    } while (line_continues);

	    if (len<MAX_PREF_LEN) line[len] = '\0';
	    
	    if (skip) {
                // Show user first 50 chars so they can find the line.
		line[50] = 0;
		dis_message(NULL, MSG_ERROR,
                            "*** Pref line '%s ...' is longer than %d characters and is ignored.\n",
			     line, MAX_PREF_LEN);
		continue;
	    }

            // Skip any white spaces.
            char* s = line;
            while (*s && isspace(*s) ) s++;
	    
	    // comment or empty line 
	    if (*s == '#' || *s == '\n' || *s == '\0') continue;

            // Copy line into oldKey until EOS or ':'.
            char* k = oldKey;
            while (*s && (*s != ':') ) *k++ = *s++;
            *k = '\0';
                    
            // Skip colon and any white spaces.
            if (*s == ':') s++;
            while (*s && isspace(*s) ) s++;
                    
            // Copy any remainder into oldValue.
            char* v = oldValue;
            while (*s ) *v++ = *s++;
            *v-- = '\0';

            // Replace trailing '\n' and white-space with NULLs.
            while ((v>=oldValue) && (isspace(*v) || *v == '\n') )
                *v-- = '\0';
                    
            int index = convertPrefFile::findOldKey((const char*)oldKey);
            if ( (index > -1) && (strlen(oldKey)>0) ) {
                // String preference.
                if (convertPrefFile::keyTable[index].type == 0) {
		    // We'll ignore the separation into lines, 
		    // thinking everything is just one line.
		    prev_was_bs = vFALSE;
		    char *s, *new_s;
                    for (s=new_s=oldValue; *s; s++) {
			if (!prev_was_bs && *s=='\\') prev_was_bs = vTRUE;
			else {
			    if (*s == '\n') *new_s++ = ' ';
			    else *new_s++ = *s;
			    prev_was_bs = vFALSE;
			}
		    }
		    *new_s = '\0';
		    
                    // Only update if a value was given.
                    if (strlen(oldValue) > 0)
                        Write(convertPrefFile::keyTable[index].newKey,
                                oldValue);
                // Integer preference.
                } else if (convertPrefFile::keyTable[index].type == 1) {
		    // Check if it's yes/no or true/false 
		    genString val_str = (char*)oldValue;
		    val_str.trim();
		    for (char * ch = (char*)val_str; *ch; ch++) *ch = toupper(*ch);
		    if ( !strcmp(val_str, "TRUE") || !strcmp(val_str, "YES"))
		      value = 1;
		    else if (!strcmp(val_str, "NO") || !strcmp(val_str, "FALSE"))
		      value = 0;
  		    else if (*oldValue && isdigit(*oldValue) )
		      value = OSapi_atoi(oldValue);
                    else {
                        value = 0;
			dis_message(NULL, MSG_WARN,
                                    "Old pref '%s' 0'ed because no value given.\n",
				    oldKey);
                    }
                    Write(convertPrefFile::keyTable[index].newKey, value);

                // Multi-line string preference.
                } else if (convertPrefFile::keyTable[index].type == 2) {
		    prev_was_bs = vFALSE;
		    char *s, *new_s;
                    for (s=new_s=oldValue; *s; s++) {
			if (!prev_was_bs && *s=='\\') prev_was_bs = true;
			else {
			    if (!prev_was_bs && *s == ':') *new_s++ = '\n';
			    else *new_s++ = *s;
			    prev_was_bs = vFALSE;
			}
		    }
		    *new_s = '\0';
                    Write(convertPrefFile::keyTable[index].newKey,
                            oldValue);
                }
            } else {
               dis_message(NULL, MSG_ERROR,
                           "*** Unable to find matching new pref for old pref '%s'\n",
                           oldKey);
	   }
        } while (!feof(ff));
	OSapi_fclose (ff);
    }
    
    return (status);
}

// Returns -1 on index into the static keyTable.
int convertPrefFile::findOldKey(const char* oldKey)
{
    int index = -1;
    for (int i=0; (i<convertPrefFile::KEY_TABLE_SIZE) && (index==-1); i++) {
        if (OSapi_strcmp((char*)oldKey, (char*)convertPrefFile::keyTable[i].oldKey) == 0)
            index = i;
    }

    return(index);
}





/**********     end of convertPrefFile.cxx     **********/

