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
// ----------------------------------------------------------------------
//
// path.C - contains the source code of the following classes necessary
// for file and directory browsing :
//
//  class File		- abstracts a file
//  class Directory	- abstratcs a directory (which is a file)
//  class HandleManager - Manages handles to Files used for integration to external systems
//  CLASS FilterList    - abstracts a list of file filters
//  class Filter 	- abstratcs a single file filter
//
// ----------------------------------------------------------------------

#include <vport.h>
#include vstartupHEADER
#include vcharHEADER
#include vstrHEADER
#include vfsHEADER
#include vexHEADER
#include vmemHEADER
#include vstdlibHEADER

#include <gpath.h> 

HandleManager File::handleManager;
FilterList    *Directory::globalFilterList;
vstr*         File::globalBeforeFlags = NULL;
vstr*         File::globalAfterFlags  = NULL;
vstr*         File::CplusExtensions   = NULL;
vstr*         File::CplusFlags        = NULL;
vstr*         File::CExtensions       = NULL;
vstr*         File::CFlags            = NULL;
static vstr*  resolvedFlags           = NULL;

const int INDENT_SIZE = 3;

// --------------------------------------------------------------------
//  This helper function will output an indented character string to
//  the given file.  Assumes that all arguments are valid.
// --------------------------------------------------------------------

static void WriteIndented (vfsFile *outputFile, int indent, vscribe *theString) 
{

    for (int i = 0; i < indent; i++) {
        outputFile->WriteStringScribed (vcharScribeFormatLiteral (" "));
    }

    outputFile->WriteStringScribed (theString);

}

// --------------------------------------------------------------------
// Class HandleManager
// --------------------------------------------------------------------

HandleManager::HandleManager() 
{
    free_entries = 0;
    count        = 0;
    size         = 25;
    filelist     = (File **) vmemAlloc(sizeof (File *) * 25);
}

// --------------------------------------------------------------------

HandleManager::~HandleManager ()
{
// 01/22/96 - tlw
// Purify complains when about the memory deleted here.
// Appearently Galaxy deletes this twice. Once when "exit" is
//    called (The debug ShutDown Manager seems to know it was 
//    created with vmemAlloc and frees it) and again when the
//    __STATIC_DESTRUCTOR (from Galaxy) is called.
// If non-static version of this class are used, we should
//    probably uncomment this and let Purify complain.
//    if (filelist) {
//        vmemFree(filelist);
//        filelist = NULL;
//    }
}

// --------------------------------------------------------------------

void HandleManager::UnregisterName(const vchar *name)
{
    for(int handle = 0; handle < count; handle++){
	File *f = filelist[handle];
	if(f && vcharCompare(f->FullName(), name) == 0){
	    filelist[handle] = NULL;
	    free_entries++;
	    return;
	}
    }
}

// --------------------------------------------------------------------

long int HandleManager::RegisterFile (File *file)
{
    for(int handle = 0; handle < count; handle++){
	File *f = filelist[handle];
	if(f && vcharCompare(f->FullName(), file->FullName()) == 0){
	    filelist[handle] = file;
	    return handle;
	}
    }
    if(free_entries){
	for(handle = 0; handle < count; handle++){
	    if(filelist[handle] == NULL){
		filelist[handle] = file;
		free_entries--;
		return handle;
	    }
	}
    }
    if (count == size) {
        size = size + 25;
        filelist = (File **) vmemRealloc(filelist, sizeof(File*) * size);
    }
    filelist[count] = file;
    return count++;
}

// --------------------------------------------------------------------

File *HandleManager::LookupFile (long int handle)
{
    if (handle >= 0 && handle <count) {
        return filelist[handle];
    }
    else {
        return NULL;
    }
}

// --------------------------------------------------------------------

File *HandleManager::LookupFile (const vchar *full_name)
{
    for(int handle = 0; handle < count; handle++){
	File *file = filelist[handle];
	if(vcharCompare(file->FullName(), full_name) == 0)
	    return file;
    }
    return NULL;
}

// --------------------------------------------------------------------

void HandleManager::Purge ()
{

    if (filelist) vmemFree(filelist);
    filelist = (File **) vmemAlloc(sizeof(File *) * 25);
    free_entries = 0;
    count        = 0;
    size         = 25;
}

// --------------------------------------------------------------------
// Class File
// --------------------------------------------------------------------

File::File (const vchar* name)
{

    vfsPath vpath;
    
    vpath.Set (name);
    vpath.MakeCanonical ();
    
    excluded = FALSE;
    fullname  = vpath.CloneString ();
    directory = NULL;
    next      = NULL;
    flags     = NULL;
    is_dir    = 0;
    handle    = handleManager.RegisterFile (this);
}

// --------------------------------------------------------------------

File::~File()
{
    if (fullname) vstrDestroy (fullname);
    if (flags) vstrDestroy (flags); 
#if 0
    if (globalBeforeFlags) { vstrDestroy (globalBeforeFlags); globalBeforeFlags = NULL; }
    if (globalAfterFlags) { vstrDestroy (globalAfterFlags); globalAfterFlags = NULL; }
    if (CplusExtensions) { vstrDestroy (CplusExtensions); CplusExtensions = NULL; }
    if (CplusFlags) { vstrDestroy (CplusFlags); CplusFlags = NULL; }
    if (CExtensions) { vstrDestroy (CExtensions); CExtensions = NULL; }
    if (CFlags) { vstrDestroy (CFlags); CFlags = NULL; }
#endif
}

// --------------------------------------------------------------------

vbool File::Save (vfsFile *vfile, int level)
{

    /*
     *  If there are flags set on this file then write it out to the
     *  File and return TRUE; otherwise retorn FALSE.
     */

    if ((flags && vcharLength(flags) != 0) || excluded) {
        for (int i = 0; i < level; i++) {
            vfile->WriteStringScribed (vcharScribeFormatLiteral ("+"));
        }

        vfile->WriteStringScribed (vcharScribeFormatLiteral (" "));
 
        vfile->WriteStringScribed (vcharScribeFormatLiteral ("%d %d %s %c", 
                                                             is_dir,
							     excluded,
                                                             Name(), 
                                                             vcharEOL));
	if(flags && vcharLength(flags) != 0)
	    vfile->WriteStringScribed (vcharScribeFormatLiteral ("%s%c", 
								 flags, 
								 vcharEOL));
	else
	    vfile->WriteStringScribed (vcharScribeFormatLiteral ("%c", vcharEOL)); 
	
        return (TRUE);
    }

    else {
        return (FALSE);
    }
}

// --------------------------------------------------------------------

File* File::Next()
{
    return next;
}

// --------------------------------------------------------------------

void File::SetDirectory (Directory * dir)
{
    this->directory = dir;
}

// --------------------------------------------------------------------

Directory* File::GetDirectory()
{
     return directory;
}

// --------------------------------------------------------------------

int File::IsDirectory()
{
    return is_dir;
}

// --------------------------------------------------------------------

const vchar* File::GetGlobalBeforeFlags()
{
    return globalBeforeFlags;
}

// --------------------------------------------------------------------

const vchar* File::GetGlobalAfterFlags()
{
    return globalAfterFlags;
}

// --------------------------------------------------------------------

void File::SetGlobalBeforeFlags (const vchar* flgs)
{
    if (globalBeforeFlags)  vstrDestroy (globalBeforeFlags);
    if (flgs) globalBeforeFlags = vstrClone (flgs);
    else globalBeforeFlags = (vchar *)NULL;
}

// --------------------------------------------------------------------

void File::SetGlobalAfterFlags (const vchar* flgs)
{
    if (globalAfterFlags)  vstrDestroy (globalAfterFlags);
    if (flgs) globalAfterFlags = vstrClone (flgs);
    else globalAfterFlags = (vchar *)NULL;
}

// --------------------------------------------------------------------

void File::SetCplusExtensions (const vchar* exts)
{
    if (CplusExtensions)  vstrDestroy (CplusExtensions);
    if (exts) CplusExtensions = vstrClone (exts);
    else CplusExtensions = (vchar *)NULL;
}

// --------------------------------------------------------------------

void File::SetCExtensions (const vchar* exts)
{
    if (CExtensions)  vstrDestroy (CExtensions);
    if (exts) CExtensions = vstrClone (exts);
    else CExtensions = (vchar *)NULL;
}

// --------------------------------------------------------------------

void File::SetCplusFlags (const vchar* flgs)
{
    if (CplusFlags)  vstrDestroy (CplusFlags);
    if (flgs) CplusFlags = vstrClone (flgs);
    else CplusFlags = (vchar *)NULL;
}

// --------------------------------------------------------------------

void File::SetCFlags (const vchar* flgs)
{
    if (CFlags)  vstrDestroy (CFlags);
    if (flgs) CFlags = vstrClone (flgs);
    else CFlags = (vchar *)NULL;
}

// --------------------------------------------------------------------

const vchar* File::GetFlags()
{
    return flags;
}

// --------------------------------------------------------------------

void File::SetFlags (const vchar* flgs)
{
    if (this->flags)  vstrDestroy (this->flags);
    if (flgs) this->flags = vstrClone (flgs);
    else this->flags = (vchar *)NULL;
}

// --------------------------------------------------------------------

const vchar *File::SetExcludeFlag(int flag)
{
    excluded = flag;
    return (const vchar *)"";
}

// --------------------------------------------------------------------

const vchar *File::GetExcludeFlag(void)
{
    static char buffer[10];
    sprintf(buffer, "%d", excluded);
    return (const vchar *)buffer;
}

// --------------------------------------------------------------------

const vchar* File::DetermineFlags()
{
    File* dir         = GetDirectory();
    const vchar *flgs = flags;

    while ((!flgs || vcharLength(flgs) == 0) && dir) {
	flgs = dir->GetFlags();
        dir  = dir->GetDirectory();
    }
    if(flgs && flgs[0] == '*')   // Make command
	flgs = (vchar *)"";
    vchar *gBefore       = (globalBeforeFlags) ? globalBeforeFlags : (vchar *)"";
    vchar *gAfter        = (globalAfterFlags) ? globalAfterFlags : (vchar *)"";
    vchar *CompilerFlags = (CplusFlags) ? CplusFlags : (vchar *)"";
    
    if (flgs)
	resolvedFlags = vstrCopyMulti (gBefore, (vchar *)" ", CompilerFlags, (vchar*)" ", flgs, (vchar *)" ", 
				       gAfter, NULL, resolvedFlags);
    else
	resolvedFlags = vstrCopyMulti (gBefore, (vchar *)" ", CompilerFlags, (vchar*)" ", gAfter, NULL, resolvedFlags);

    return (vchar*) resolvedFlags;
}

// --------------------------------------------------------------------

static char *parse_make_string(char *string, char *file_name)
{
#ifndef _WIN32
    char *stripped_file_name = strdup(file_name);
#else
    char *stripped_file_name = _strdup(file_name);
#endif
    if(!stripped_file_name)
	return NULL;
    char *point = strrchr(stripped_file_name, '.');
    if(point)
	*point = 0;

    int  out_len       = 0;
    int  file_name_len = strlen(stripped_file_name);
    char *ret_val      = NULL;
    
    for(int i = 0; i < strlen(string); i++)
	if(string[i] == '$')
	    out_len += file_name_len;
	else
	    out_len++;
    ret_val = (char *)malloc(out_len + 1);
    if(ret_val == NULL){
	free(stripped_file_name);
	return NULL;
    }
    char *p = ret_val;
    for(i = 0; i < strlen(string); i++)
	if(string[i] == '$'){
	    strncpy(p, stripped_file_name, file_name_len);
	    p += file_name_len;
	} else
	    *p++ = string[i];
    *p = 0;
    free(stripped_file_name);
    return ret_val;
}

// --------------------------------------------------------------------

void File::GenerateRules(vfsFile* output)
{
    if (IsDirectory()) {
	Directory* dir = (Directory*) this;
	for (File* file = dir->GetFirstFile(); file; file = file->next)
	    file->GenerateRules (output);

	if (flags && (vcharLength(flags) > 0))
	    if(flags[0] != '*')
		output->WriteStringScribed (vcharScribeFormatLiteral (
		    "  \"\" : %s/**/%%/.make => \"echo %s\"%c", 
                    FullName(),
                    flags,
                    vcharEOL));
	    else
		output->WriteStringScribed (vcharScribeFormatLiteral (
		    "  \"\" : %s/**/%%/.make => \"%s\"%c", 
                    FullName(),
                    &flags[1],
                    vcharEOL));
    } else
	if (flags && (vcharLength(flags) > 0))
	    if(flags[0] != '*'){
		output->WriteStringScribed (vcharScribeFormatLiteral (
		    "  \"\" : %s/%%/.make => \"echo %s\"%c", 
		    FullName(),
		    flags,
		    vcharEOL));
	    } else {
		char *make_string = parse_make_string((char *)&flags[1], (char *)Name());
		if(make_string != NULL){
		    output->WriteStringScribed (vcharScribeFormatLiteral (
			"  \"\" : %s/%%/.make => \"%s\"%c", 
			FullName(),
			make_string,
			vcharEOL));
		    free(make_string);
		}
	    }
		
}

// --------------------------------------------------------------------

const vchar* File::Name()
{
  vchar *separatorString = vstrClone (vfsPath::GetSeparatorString());
  size_t separatorStringLen = vcharLength (separatorString);
  const vchar* ptr = vcharSearchBackwards (fullname, separatorString);
 
    vstrDestroy (separatorString);

    if (ptr) 
	return ptr + separatorStringLen;
    else
	return fullname;
}

// --------------------------------------------------------------------

const vchar* File::FullName()
{
    return fullname;
}

// --------------------------------------------------------------------

void File::Print()
{
  vfsPath vpath;

    vpath.Set (fullname);
    vpath.Dump(stdout);
    vcharDumpF (stdout, "Handle: %d\n", handle);
}

// --------------------------------------------------------------------
//  Class Directory
// --------------------------------------------------------------------

Directory::Directory (const vchar* name) : File (name)
{
    first = NULL;
    is_dir = 1;
    filterList = NULL;
}

// --------------------------------------------------------------------

Directory::~Directory()
{
    if (first) RemoveFiles();
    if (this->filterList) delete (this->filterList); 

    /*
     *  If this is the root directory, then purge the handle manager
     */

    if (!this->directory) handleManager.Purge(); 
}

// --------------------------------------------------------------------

void Directory::Save (vchar *filename)
{

  vfsPath vpath;
  vfsFile *vfile;
  int returnEarly = 0;

    /*
     *  Open the output file.
     */

    vpath.Set (filename);

    vexWITH_HANDLING {
    vfile = vfsFile::Open (&vpath, vfsOPEN_WRITE_TRUNCATE);
    } vexON_EXCEPTION {
        returnEarly = 1;
    } vexEND_HANDLING;

    if (returnEarly) return;

    Save (vfile, 1);
    vfile->PutChar (vcharFromLiteral ('x'));
    vfile->Close();
}

// --------------------------------------------------------------------

vbool Directory::Save (vfsFile *vfile, int level)
{

    int position = vfile->GetPos();
    vbool dirContainsSomething = FALSE;

    /*
     *  If there is a filter or flags set on this file then write it out to the
     *  File.
     */

    for (int i = 0; i < level; i++) {
        vfile->WriteStringScribed (vcharScribeFormatLiteral ("+"));
    }

    vfile->WriteStringScribed (vcharScribeFormatLiteral (" "));
 
    vfile->WriteStringScribed (vcharScribeFormatLiteral ("%d %d %s %c", 
                                                         is_dir,
							 excluded,
                                                         level == 1 ? FullName() : Name(), 
                                                         vcharEOL));

    if (flags && vcharLength(flags) != 0) {
        dirContainsSomething = TRUE;
        vfile->WriteStringScribed (vcharScribeFormatLiteral ("%s%c", 
                                                             flags, 
                                                             vcharEOL));
    }
    else {
        vfile->WriteStringScribed (vcharScribeFormatLiteral ("%c", 
                                                             vcharEOL));
    }

    if ((Directory *)this->filterList) {
        dirContainsSomething = TRUE;
        vfile->WriteStringScribed (vcharScribeFormatLiteral ("%s%c", 
                                                             filterList->GetFilter(), 
                                                             vcharEOL));
    }
    else {
        vfile->WriteStringScribed (vcharScribeFormatLiteral ("%c", 
                                                             vcharEOL));
    }

    for (File *fposition = first; fposition; fposition = fposition->Next()) {
        if (fposition->IsDirectory()) {
            dirContainsSomething = ((Directory *)fposition)->Save (vfile, level + 1) || 
                                   dirContainsSomething;
        }
        else {
            dirContainsSomething = fposition->Save (vfile, level + 1) ||
                                   dirContainsSomething;
        }
    }

    /*
     *  If we went through the entire directory and its children and did not find a single
     *  significant thing, then rewind the file to the position at the beginning
     *  of the directory.
     */

    if (!dirContainsSomething && level != 1) {
        vfile->Seek (position, vfsSEEK_FROM_START);
    }

    return (dirContainsSomething);

}

// --------------------------------------------------------------------

void Directory::Restore (vfsFile *vfile, int level)
{

  File *theList = (File *)NULL;
  int errorStatus = 0;
  vchar buffer[1024];
  long int position = vfile->GetPos();

    while (vfile->ReadString (buffer, sizeof (buffer)) && buffer[0] != (vchar)'x') {

        vchar *levelStr = vcharGetFirstToken (buffer, (const vchar *)" ");

        /*
         *  If this file is from the previous directory, then put it back
         *  and return.
         */

        if (vcharLength (levelStr) <= level) {
            vfile->Seek (position, vfsSEEK_FROM_START);
            break;
        } 

        vchar *is_dirStr   = vcharGetNextToken ((const vchar *)" ");
        vchar *is_excluded = vcharGetNextToken ((const vchar *)" ");
        vchar *fileSpec    = vcharGetNextToken ((const vchar *)" ");
        
        if (atoi((char *)is_dirStr)) {
            errorStatus = AddFile (fileSpec, &theList);
            vfile->ReadString (buffer, sizeof (buffer));
            if (errorStatus == 0 && vcharLength (buffer) > 0 && buffer[0] != vcharEOL) {
                buffer[vcharLength(buffer) - 1] = vcharNULL;
                theList->SetFlags (buffer);
            }
	    theList->SetExcludeFlag(atoi((char *)is_excluded));
            vfile->ReadString (buffer, sizeof (buffer));
            if (errorStatus == 0 && vcharLength (buffer) > 0 && buffer[0] != vcharEOL) {
                buffer[vcharLength(buffer) - 1] = vcharNULL;
                ((Directory *)theList)->SetFilter (buffer);
            }

            if (errorStatus == 0) ((Directory *)theList)->Restore (vfile, level + 1);
        }

        else {
            errorStatus = AddFile (fileSpec, &theList);
            vfile->ReadString (buffer, sizeof (buffer));
            if (errorStatus == 0 && vcharLength (buffer) > 0 && buffer[0] != vcharEOL) {
                buffer[vcharLength(buffer) - 1] = vcharNULL;
                theList->SetFlags (buffer);
            }
	    theList->SetExcludeFlag(atoi((char *)is_excluded));
        }

        position = vfile->GetPos();
    }

    if (buffer[0] == (vchar)'x') {
        vfile->Seek (position, vfsSEEK_FROM_START);
    }

    first = InsertionSort (theList);

}

// --------------------------------------------------------------------

File *Directory::Restore (vchar *filename)
{

  vfsPath vpath;
  vfsFile *vfile;
  int returnEarly = 0;
  vchar buffer[1024];

    /*
     *  Open the output file.
     */

    vpath.Set (filename);

    vexWITH_HANDLING {
    vfile = vfsFile::Open (&vpath, vfsOPEN_READ_ONLY);
    } vexON_EXCEPTION {
        returnEarly = 1;
    } vexEND_HANDLING;

    if (returnEarly) return (NULL);

    /*
     *  Read in the Root
     */

    vfile->ReadString (buffer, sizeof (buffer));
    vchar *level         = vcharGetFirstToken (buffer, (const vchar *)" ");
    vchar *is_dirStr     = vcharGetNextToken ((const vchar *)" ");
    vchar *is_excluded   = vcharGetNextToken ((const vchar *)" ");
    vchar *directorySpec = vcharGetNextToken ((const vchar *)" ");
    Directory *rootDir   = new Directory (directorySpec);

    vfile->ReadString (buffer, sizeof (buffer));
    if (vcharLength (buffer) > 0 && buffer[0] != vcharEOL) {
        buffer[vcharLength(buffer) - 1] = vcharNULL;
        rootDir->SetFlags (buffer);
	rootDir->SetExcludeFlag(atoi((char *)is_excluded));
    }
    vfile->ReadString (buffer, sizeof (buffer));
    if (vcharLength (buffer) > 0 && buffer[0] != vcharEOL) {
        buffer[vcharLength(buffer) - 1] = vcharNULL;
        rootDir->SetFilter (buffer);
    }

    rootDir->Restore (vfile, 1);

    vfile->Close();

    return (rootDir);
}

// --------------------------------------------------------------------

vchar *Directory::GetGlobalFilter () 
{
    if (globalFilterList)
	return (globalFilterList->GetFilter());
    else
	return (vchar*) NULL;
}

// --------------------------------------------------------------------

void Directory::SetGlobalFilter (const vchar *filter) 
{

    if (globalFilterList) delete (globalFilterList); 
    if (filter) globalFilterList = new FilterList (filter);
    else globalFilterList = NULL;
}

// --------------------------------------------------------------------

vchar *Directory::GetFilter () 
{
    if (filterList)
	return (filterList->GetFilter());
    else return (vchar*)NULL;
}

// --------------------------------------------------------------------

void Directory::SetFilter (const vchar *filter) 
{
    if (this->filterList) {
        delete (this->filterList); 
        this->filterList = NULL;
    }

    if (filter && *filter) {    // Non-null filter given.
        for (const vchar* ptr = filter; *ptr; ptr++)
            if (!vcharIsWhiteSpace(*ptr)) { // Non-blank filter given.
                filterList = new FilterList (filter);
                break;
            }
    }
}

// --------------------------------------------------------------------

vchar *Directory::DetermineFilter() 
{
    FilterList *flist = DetermineLocalFilter();
    if (flist) return flist->GetFilter();
    else if (globalFilterList) return globalFilterList->GetFilter();
    else return (vchar *)NULL;
}

// --------------------------------------------------------------------

FilterList *Directory::DetermineLocalFilter () 
{

    if (!filterList && directory) {
        return (directory->DetermineLocalFilter());
    }

    else {
        return filterList;
    }
}

// --------------------------------------------------------------------

unsigned int Directory::CountPDFfiles(vchar *sourceRoot)
{
    unsigned int number = 0;
    
    /* 
     *  Count top level files
     */
    for (File *file = first; file; file = file->Next()) {
	if (!file->IsDirectory() && !atoi((char *)file->GetExcludeFlag())) {
	    number++;
	} 
    }

    /*
     *  Go through the sub directories
     */
    for (File *dir = first; dir; dir = dir->Next()) {
        if (dir->IsDirectory()) {
            number += ((Directory *)dir)->CountPDFfiles(sourceRoot);
        }
    }

    return number;
}

// --------------------------------------------------------------------

void Directory::WritePDFFile (vchar *outputFile,
                              vchar *projectName,
                              vchar *sourceRoot,
                              vchar *modelRoot)
{

    vfsPath vpath;
    vfsFile *vfile;
    vchar logicalPath[1024];
    vchar physicalPath[1024];
    int returnEarly = 0;
    
    /*
     *  Open the output file.
     */
    
    vpath.Set (outputFile);

    vexWITH_HANDLING {
    vfile = vfsFile::Open (&vpath, vfsOPEN_WRITE_TRUNCATE);
    } vexON_EXCEPTION {
        returnEarly = 1;
    } vexEND_HANDLING;

    if (returnEarly) return;

    /*
     *  Write out the beginning of the project.
     */

    vfile->WriteStringScribed (
        vcharScribeFormatLiteral ("%s: %s <-> /%s%c{%c", 
                                  projectName, 
//                                  this->FullName(),
				  "$PROJECT_ROOT",
                                  projectName,
                                  vcharEOL,
                                  vcharEOL)
    );
  
    /*
     * Create maps for DISCOVER files
     */

    vfile->WriteStringScribed (
	vcharScribeFormatLiteral ("  .rtl : .rtl <-> .rtl { *.rtl }%c", vcharEOL));
    vfile->WriteStringScribed (
	vcharScribeFormatLiteral ("  .boilerplate : boil <-> /Boiler { * }%c", vcharEOL));
    vfile->WriteStringScribed (
	vcharScribeFormatLiteral ("  .AssociationGroups : .assoc <-> /AssociationGroups { *.txt }%c", vcharEOL));
    vfile->WriteStringScribed (
	vcharScribeFormatLiteral ("  subsystems : discover-subsystems <-> /DISCOVER-Subsystems { *.subsys }%c", vcharEOL));
    vfile->WriteStringScribed (
	vcharScribeFormatLiteral ("%c", vcharEOL));

    /* 
     *  Write out excluded files
     */
    
    for (File * file=first; file; file=file->Next()) {
	if (!file->IsDirectory() && atoi((char *)file->GetExcludeFlag())) { 
	    vfile->WriteStringScribed(
		vcharScribeFormatLiteral("  ^%s%c", file->Name(), vcharEOL));
	} 
    }
    
   /*
    * Print out filters if any files are found.
    */
  
    for (file = first; file; file = file->Next()) {
	if (!file->IsDirectory()) {    // Found a flat file.
             vchar * filterString = DetermineFilter();
	     if (filterString) {
		 FilterList * newFilter = new FilterList(filterString);
		 newFilter->GeneratePDF(vfile, INDENT_SIZE);
		 delete newFilter;
	     }
	     
	     break;	// Only need to generate once.
        }
    }



    /*
     *  Go through the sub directories and write them out.
     */

    physicalPath[0] = (vchar) NULL;
    vcharCopyFast(projectName, logicalPath);

    for (File *dir = first; dir; dir = dir->Next()) {
        if (dir->IsDirectory() && !dir->excluded) {
            ((Directory *)dir)->GeneratePDF(vfile, INDENT_SIZE,
                filterList, physicalPath, logicalPath);
        }
    }

    vfile->WriteStringScribed (vcharScribeFormatLiteral ("}%c", vcharEOL));

    /*
     * Write out special pset rules for C & C++ externsions
     */
    vchar *checked_model_root;
    if(!modelRoot || ((char *)modelRoot)[0] == 0)
	checked_model_root = vstrClone(sourceRoot);
    else
	checked_model_root = vstrClone(modelRoot);

    vfile->WriteStringScribed (vcharScribeFormatLiteral (
        "__rule : / [[ R ]] <-> /__rule001%c{%c", 
	vcharEOL,
        vcharEOL));

    char *ext = strtok((char *)CplusExtensions, " ");
    while(ext){
	vfile->WriteStringScribed (vcharScribeFormatLiteral (
	    "  %s/(1).o : %s/(**)%s/%%/.pset => %s/(1)%s.pset%c%c",
	    sourceRoot,
	    sourceRoot,
	    ext,
	    checked_model_root,
	    ext,
	    vcharEOL,
	    vcharEOL));
	ext = strtok(NULL, " ");
    }
    ext = strtok((char *)CExtensions, " ");
    while(ext){
	vfile->WriteStringScribed (vcharScribeFormatLiteral (
	    "  %s/(1).o : %s/(**)%s/%%/.pset => %s/(1)%s.pset%c%c",
	    sourceRoot,
	    sourceRoot,
	    ext,
	    checked_model_root,
	    ext,
	    vcharEOL,
	    vcharEOL));
	ext = strtok(NULL, " ");
    }

    /*
     * Write out general rules
     */
    
    vfile->WriteStringScribed (vcharScribeFormatLiteral (
        "  .pmod : %s/(**).pmod/%%/.pmoddir => %s/(1).pmod%c",
	sourceRoot,
	checked_model_root,
        vcharEOL));
    vfile->WriteStringScribed (vcharScribeFormatLiteral (
        "  \"\" : %s/(**)/%%/.pset => %s/(1).pset%c%c",
	sourceRoot,
	checked_model_root,
        vcharEOL,
        vcharEOL));

    GenerateRules(vfile);

    vstrDestroy(checked_model_root);

    vfile->WriteStringScribed (vcharScribeFormatLiteral ("}%c", vcharEOL));
  
    vfile->Close();
}

// --------------------------------------------------------------------

void Directory::GeneratePDF (vfsFile *outputFile, 
                             unsigned int indent,
                             FilterList *filters,
                             vchar * physicalPath,
                             vchar * logicalPath)
{
    int hasFlatFiles = 0;	// non-zero if any non-dir files are found.
    vchar lPath[1024];
    vchar pPath[1024];

   /*
    * Update the logical path.
    */

#if 0
    if (vcharLength(logicalPath)) {
        vcharCopyFast(logicalPath, lPath);
        vcharAppendFromLiteral(lPath, "/"); // KLUDGE, use seperator.
        vcharAppendFast(lPath, this->Name());
    } else {
	vcharCopyFast(this->Name(), lPath);
    }
#endif
    vcharCopyFast(this->Name(), lPath);


    /*
     *  Make sure that the directory is expanded.
     */

    ReadDirectory (FALSE);
    
    /*
     * See if any files are found.
     */

    for (File * file=first; file; file=file->Next()) {
        if (!atoi((char *)file->GetExcludeFlag())) { // Found a 'real' file.
            hasFlatFiles = 1;

            break;
        }
    }

#if 0
    // Reset the physical path to this dir.
    vcharCopyFast(this->Name(), pPath);
    WriteIndented(outputFile, indent, vcharScribeFormatLiteral(
	"%s: %s <-> %s%c", this->Name(), pPath,
	lPath, vcharEOL) );
    indent += INDENT_SIZE;
    WriteIndented(outputFile, indent, vcharScribeFormatLiteral(
	"{%c", vcharEOL) );
#endif

    /*
     * Write out file info.
     */

    if (hasFlatFiles) {
        // Reset the physical path to this dir.
        vcharCopyFast(this->Name(), pPath);

        WriteIndented(outputFile, indent, vcharScribeFormatLiteral(
	    "%s: %s <-> %s%c", this->Name(), pPath,
            lPath, vcharEOL) );
        indent += INDENT_SIZE;
        WriteIndented(outputFile, indent, vcharScribeFormatLiteral(
            "{%c", vcharEOL) );

        /* 
         *  Write out excluded files
         */

	for (File * file=first; file; file=file->Next()) {
	    if (!file->IsDirectory() && atoi((char *)file->GetExcludeFlag())) { 
		WriteIndented(outputFile, indent, vcharScribeFormatLiteral(
		    "^%s%c", file->Name(), vcharEOL)); 
	    }
	}
	
        /*
         *  Write out the filters.
         */
        vchar * filterString = DetermineFilter();
        if (filterString) {
            FilterList * newFilter = new FilterList(filterString);
            newFilter->GeneratePDF(outputFile, indent);
	    delete newFilter;
         }
    } else {
        // Append this dir to the physical path.
        if (vcharLength(physicalPath)) {
           vcharCopyFast(physicalPath, pPath);
           vcharAppendFromLiteral(pPath, "/"); // KLUDGE, use seperator.
           vcharAppendFast(pPath, this->Name());
        } else {
           vcharCopyFast(this->Name(), pPath);
        }
    }

    /*
     *  Write out the sub directories.
     */

    for (File *position = first; position; position = position->Next()) {
        if (position->IsDirectory() && !position->excluded) {
            ((Directory *)position)->GeneratePDF (outputFile, indent,
                filterList, pPath, lPath);
        }
    }

    /*
     * Close out this level.
     */

#if 0
    WriteIndented (outputFile, indent,
		   vcharScribeFormatLiteral ("}%c", vcharEOL));
#endif
    if (hasFlatFiles) {
        WriteIndented (outputFile, indent,
            vcharScribeFormatLiteral ("}%c", vcharEOL)
        );
    }
}

// --------------------------------------------------------------------

void Directory::ReadDirectory(vbool displayAll)
{

  vstr    *dirEntry;
  vfsDir  *openDir;
  vfsPath vpath;

  File    *newList = (File *)NULL;
  Directory *dirsToBeProcessed = (Directory *)NULL;

    displayAll = FALSE;

    vpath.Set (fullname);

    vexWITH_HANDLING {
    openDir = vfsDir::Open (&vpath);
    } vexON_EXCEPTION {
        openDir = NULL;
    } vexEND_HANDLING;

    if (openDir == (vfsDir *) NULL) {
        return;
    }

    vstr *thisDir = vstrCloneScribed (vcharScribeLiteral ("."));
    vstr *parentDir = vstrCloneScribed (vcharScribeLiteral (".."));

    while ((dirEntry = openDir->Read()) != (vchar *) NULL) {


        if (vcharCompare (dirEntry, thisDir) == 0) {
            vstrDestroy (dirEntry);
            continue;
        }

        if (vcharCompare (dirEntry, parentDir) == 0) {
            vstrDestroy (dirEntry);
            continue;
        }

        if (AddFile (dirEntry, &newList) != 0) {
            vstrDestroy (dirEntry);
            continue;
        }

        /*
         *  Process the directories by caching them to be processed later.
         *  This is to prevent too many files from being open at once.
         */

        if (newList && newList->IsDirectory() == TRUE) {
            Directory *temp = (Directory *)newList;
            newList = newList->Next();
            temp->next = dirsToBeProcessed;
            dirsToBeProcessed = temp;
        }

        vstrDestroy (dirEntry);
    }

    openDir->Close();

    /*
     *  Process the directories last so we won't have too many open files.
     */

    while (dirsToBeProcessed) {

        Directory *nextDir;

        if (displayAll && (vcharCompare (dirsToBeProcessed->Name(), parentDir) != 0)) {
            dirsToBeProcessed->ReadDirectory(displayAll);
        }

        nextDir = (Directory *)dirsToBeProcessed->next;
        dirsToBeProcessed->next = newList;
        newList = dirsToBeProcessed;
        dirsToBeProcessed = nextDir;
    }

    vstrDestroy (parentDir);
    vstrDestroy (thisDir);

    RemoveFiles ();
    first = InsertionSort (newList);
}


// --------------------------------------------------------------------

File * Directory::FindFileFromPath (const vchar* name) {
    vfsPath vpath;
  
    vpath.Set (name);
    vpath.MakeCanonical ();
    vstr* str = vpath.CloneString();


    if (vcharCompare (this->FullName(), str) == 0)
	return this;

    File *position = first;
    
    while (position) {
        if (vcharCompare (position->FullName(), str) == 0) {
            return position;
        }
        if (position->IsDirectory()) {
	    File* subfile = ((Directory*)position)->FindFileFromPath (name);
	    if (subfile) return subfile;
	}
        position = position->next;
    }
    return NULL;
}

File * Directory::GetFirstFile () {
    return first;
}

// --------------------------------------------------------------------

void  Directory::RemoveFiles () 
{
 
    File *position = first;
    File *nxt;

    while (position) {
        nxt = position->next;
        delete (position);
        position = nxt;
    }

    first = (File *) NULL;
}

// --------------------------------------------------------------------

void Directory::Print()
{
  File *position = first;

    File::Print();

    while (position) {
        position->Print();
        position = position->next;
    }
}

// --------------------------------------------------------------------

File* Directory::FindFile (vchar *name)
{

  File *position = first;

    while (position) {
        if (vcharCompare (position->Name(), name) == 0) {
            return position;
        }
        position = position->next;
    }

    return (File *) NULL;
}

// --------------------------------------------------------------------

int Directory::AddFile (vchar *fileName, File **newList) {

  vfsInfo fileinfo;
  vfsPath vpath;
  int retValue = 0;

  vpath.Set (fullname);
  vpath.Append (fileName, NULL);

//    filePath->MakeCanonical ();

    vexWITH_HANDLING {
        vpath.GetFileInfo (&fileinfo);
    } vexON_EXCEPTION {
        retValue = 1;
    } vexEND_HANDLING;

    if (retValue != 0) {
        return (retValue);
    }

    if (fileinfo.IsDirectory() == TRUE) {
        vstr *fullPath = vpath.CloneString ();
        Directory *newDir = new Directory(fullPath);
        vstrDestroy (fullPath);
        newDir->SetDirectory (this);

        Directory *previousDir;

        if ((previousDir = (Directory *)FindFile (fileName)) != NULL) {
            newDir->SetFlags (previousDir->GetFlags());
            newDir->SetExcludeFlag (atoi((char *)previousDir->GetExcludeFlag()));
            newDir->SetFilter (previousDir->GetFilter());
            newDir->first = previousDir->GetFirstFile();
            previousDir->first = NULL;
        }

        newDir->next = *newList;
        *newList = newDir;
    }

    else {

        FilterList *myFilterList = DetermineLocalFilter ();

        vbool keep = TRUE;
        if (myFilterList) keep = myFilterList->IsFiltered (fileName);
        else if (globalFilterList) keep = globalFilterList->IsFiltered (fileName);

        if (keep) {
            vstr *fullPath = vpath.CloneString ();
            File *newFile = new File(fullPath);
            vstrDestroy (fullPath);
            newFile->SetDirectory (this);

	    File* previousFile;
	    if ((previousFile = FindFile (fileName)) != NULL) {
                newFile->SetFlags (previousFile->GetFlags());
		newFile->SetExcludeFlag(atoi((char *)previousFile->GetExcludeFlag()));
            }

            newFile->next = *newList;
            *newList      = newFile;
        } else {
            vstr *fullPath = vpath.CloneString();
	    handleManager.UnregisterName((const vchar *)fullPath);
            vstrDestroy (fullPath);
            return 1;
        }
    }

    return (0);
}

// --------------------------------------------------------------------

File* Directory::InsertionSort (File *fileList) 
{

  File* myFirst = (File *)NULL;
 
    if (!fileList) return (File *)NULL;
 
    myFirst = fileList;
    fileList = fileList->next;
    myFirst->next = (File *)NULL;

    while (fileList) {

        File* position;
        File* nextFile;
        File* temp;

        /*
         *  Insertion at the beginning of the list.
         */

        if (vcharCompare (fileList->Name(), myFirst->Name()) < 0) {
            temp = fileList;
            fileList = fileList->next;
            temp->next = myFirst;
            myFirst = temp;
        }

        else {

            /*
             *  Find the correct position for the file.
             */

            for (position = myFirst, nextFile = position->next;
                 (nextFile && (vcharCompare (fileList->Name(), nextFile->Name()) > 0));
                 position = position->next, nextFile = nextFile->next);

            /*
             *  Insertion in the middle or end of the list.
             */

            temp = fileList;
            fileList = fileList->next;
            temp->next = nextFile;
            position->next = temp;
        }

    }
 
    return myFirst;

}

// --------------------------------------------------------------------
//  Class Filter
// --------------------------------------------------------------------

Filter::Filter (vchar *newFilter) {

    this->filter = newFilter;
    if (newFilter[0] == vcharFromLiteral('*')) {
        while (*(this->filter) != vcharNULL && 
               *(this->filter) != vcharFromLiteral('.')) {
            (this->filter)++;
        }
        if (*(this->filter) != vcharNULL) (this->filter)++;
        wild = TRUE;
    }
    else {
        wild = FALSE;
    }

}

// --------------------------------------------------------------------

Filter::~Filter () {

}

// --------------------------------------------------------------------

vbool Filter::IsFiltered (vchar *fileName) {

    vbool returnValue = FALSE;
    vchar *extension;

    if (wild) {

        //
        // if the filter is empty then the wildcard was simply '*'.
        //

        if (*(this->filter) == vcharNULL) returnValue = TRUE;

        else {
            vfsPath vPath;
            vPath.Set (fileName);
            extension = vPath.CloneExtension ();

            if (extension) {
                if (vcharCompare (extension, filter) == 0) returnValue = TRUE;

                vstrDestroy (extension);
	    }
        }
    }

    else if (vcharCompare (filter, fileName) == 0) returnValue = TRUE;

    return returnValue;

}

// --------------------------------------------------------------------

void Filter::GeneratePDF (vfsFile *outputFile, 
                          unsigned int indent) {


    if (wild && *(this->filter) == vcharNULL) {
        WriteIndented (outputFile, indent,
            vcharScribeFormatLiteral ("*%c", vcharEOL)
        );
    }

    else if (wild) {
        WriteIndented (outputFile, indent,
            vcharScribeFormatLiteral ("*.%s%c", this->filter, vcharEOL)
        );
    }

    else {
        WriteIndented (outputFile, indent,
            vcharScribeFormatLiteral ("%s%c", this->filter, vcharEOL)
        );
    }
}

// --------------------------------------------------------------------
//  Class FilterList
// --------------------------------------------------------------------

FilterList::FilterList (const vchar *filters) {

    vchar *separators = vstrCloneScribed (vcharScribeLiteral (", "));
    vchar *token;

    filterList = NULL;

    if (!filters) return;

    filterstring = vstrClone (filters);
    copyFilterstring = vstrClone (filters);

    token = vcharGetFirstToken (filterstring, separators);
    size = 0;

    while (token) {
        if (!filterList) {
            size = 1;
            filterList = (Filter **) vmemAlloc(sizeof(Filter *));
        }

        else {
            size++;
            filterList = (Filter **) vmemRealloc(filterList, sizeof(Filter *) * size);
        }

        filterList[size - 1] = new Filter (token);
        token = vcharGetNextToken (separators);
    }

    vstrDestroy (separators); 

}

// --------------------------------------------------------------------

FilterList::~FilterList () {

    for (int i = 0; i < size; i++) delete (filterList[i]);
    if (filterstring) vstrDestroy (filterstring);
    if (copyFilterstring) vstrDestroy (copyFilterstring);
    if (filterList) {
        vmemFree(filterList);
        filterList = NULL;
    }
}

// --------------------------------------------------------------------

vchar *FilterList::GetFilter () {
    return copyFilterstring;
}

// --------------------------------------------------------------------

vbool FilterList::IsFiltered (vchar *fileName) {

    for (int i = 0; i < size; i++) {
        if (filterList[i]->IsFiltered(fileName)) return TRUE;
    }

    return FALSE;
}

// --------------------------------------------------------------------

void FilterList::GeneratePDF (vfsFile *outputFile, 
                              unsigned int indent) {

    for (int i = 0; i < size; i++) {
        filterList[i]->GeneratePDF (outputFile, indent);
    }

}

// --------------------------------------------------------------------
// --------------------------------------------------------------------

//
//  The following Main procedure is used for testing this feature in a 
//  stand-alone environment.
//

#if 0
main(int argc, char **argv)
{
    Directory* d;
    vstr *dirSpec;

    vstartup (argc, argv);
    dirSpec = vstrCloneScribed (vcharScribeLiteral ("/users/jnp/test"));
    d = new Directory (dirSpec);
    d->ReadDirectory(TRUE);
    d->Print();

    vstrDestroy (dirSpec);

    delete (d);

}
#endif

