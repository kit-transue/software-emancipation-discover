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
#include vcharHEADER
#include vstrHEADER
#include vstdlibHEADER

#include <ginterp.h>
#include <gpath.h>
#include <gString.h>

// ---------------------------------------------------------------------------

int dis_DirExpandVariables (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if (argc != 2) {
        return TCL_ERROR;
    }
    if(argv[1][0] == 0){
	Tcl_SetResult (interp, "", TCL_VOLATILE);
    } else {
	vfsPath vpath;
	vpath.Set((vchar *)argv[1]);
	vpath.ExpandVariables();
	vpath.MakeCanonical();
	vstr *result_path = vpath.CloneString();
	Tcl_SetResult (interp, (char *)result_path, TCL_VOLATILE);
	vstrDestroy(result_path);
    }
    return TCL_OK;
}

// ---------------------------------------------------------------------------

int dis_DirCreate (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    gString tmp;

    if (argc != 2) {
        return TCL_ERROR;
    }
    else if (argv[1] && *argv[1]) {
        Directory *dir;
        vstr *dirName = vstrCloneScribed (vcharScribeSystem (argv[1]));
        dir = new Directory(dirName);
        vstrDestroy (dirName);
        if (dir) {
	    tmp.sprintf((vchar *)"%d", dir->handle);
	    Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
            return TCL_OK;
        }
        else {
	    Tcl_SetResult(interp, "0", TCL_STATIC);
            return TCL_ERROR;
        }
    }
    else {
	Tcl_SetResult(interp, "0", TCL_STATIC);
        return TCL_ERROR;
    }
}

// ---------------------------------------------------------------------------

int dis_DirFindFileFromPath (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        File *file     = File::handleManager.LookupFile ((const vchar *)argv[1]);
	Directory *dir = NULL;
	if (file && file->IsDirectory())
	    dir = (Directory *)file;
        if (!dir) return TCL_ERROR;
        vstr *dirName = vstrCloneScribed (vcharScribeSystem (argv[1]));

/*        File* file    = dir->FindFileFromPath (dirName); */

        vstrDestroy (dirName);
        int h = -1;
	if (file) h = file->handle;
	gString tmp;
	tmp.sprintf ((vchar *)"%d", h);
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirSize (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

        File *position = dir->GetFirstFile();
        int count = 0;

        while (position) {
            count++;
            position = position->Next();
        }
	gString tmp;
        tmp.sprintf((vchar *)"%d", count + 1); /* directory name itself */
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirItemName (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    if (argc == 2) {
        File *position = (File*)File::handleManager.LookupFile(atoi (argv[1]));
        if (!position) return TCL_ERROR;

	if (position->IsDirectory())
	    Tcl_AppendResult (interp, (char *)position->Name(), "/", NULL);
	else
	    Tcl_SetResult (interp, (char *)position->Name(), TCL_VOLATILE);
	return TCL_OK;
    }

    else if (argc == 3) {
        File *file = (File *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!file) return TCL_ERROR;

	if(!file->IsDirectory()) return TCL_ERROR;
	Directory *dir = (Directory *)file;
        File *position = dir->GetFirstFile();
        int index      = atoi (argv[2]);
        int count      = 0;

	if(index == 0) {
	    Tcl_SetResult(interp, ".", TCL_VOLATILE);
	    return TCL_OK;
	} else {
	    index--;
	    while (position && count != index) {
		count++;
		position = position->Next();
	    }
	    
	    if (position) {
		if (position->IsDirectory())
		    Tcl_AppendResult (interp, (char *)position->Name(), "/", NULL);
		else
		    Tcl_SetResult (interp, (char *)position->Name(), TCL_VOLATILE);
		return TCL_OK;
	    }
	    else {
		return TCL_ERROR;
	    }
	}
    }

    else {
        return TCL_ERROR;
    }
}

// ---------------------------------------------------------------------------

int dis_DirItemFullname (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc == 2) {
        File *position = (File*)File::handleManager.LookupFile (atoi (argv[1]));
        if (!position) return TCL_ERROR;

        Tcl_SetResult (interp, (char *)position->FullName(), TCL_VOLATILE);
        return TCL_OK;
    }

    else if (argc == 3) {
        File *file     = (File *)File::handleManager.LookupFile (atoi (argv[1]));
        Directory *dir = NULL;
	if (file->IsDirectory())
	    dir = (Directory*) file;
        if (!dir) return TCL_ERROR;

        File *position = dir->GetFirstFile();
        int index = atoi (argv[2]);
        int count = 0;
	if(index == 0){
	    Tcl_SetResult (interp, (char *)dir->FullName(), TCL_VOLATILE);
	    return TCL_OK;
	} else {
	    index--;
	    while (position && count != index) {
		count++;
		position = position->Next();
	    }

	    if (position) {
		Tcl_SetResult (interp, (char *)position->FullName(), TCL_VOLATILE);
		return TCL_OK;
	    }
	    else {
		return TCL_ERROR;
	    }
	}
    }
    else {
        return TCL_ERROR;
    }
}

// ---------------------------------------------------------------------------

int dis_DirItem (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 3) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

        File *position = dir->GetFirstFile();
        int index      = atoi (argv[2]);
        int count      = 0;

	if(index == 0){
	    gString tmp;
	    tmp.sprintf ((vchar *)"%d", dir->handle);
	    Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
	    return TCL_OK;
	} else {
	    index--;
	    while (position && count != index) {
		count++;
		position = position->Next();
	    }
	    
	    if (position) {
		gString tmp;
		tmp.sprintf ((vchar *)"%d", position->handle);
		Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
		return TCL_OK;
	    }
	    else {
		return TCL_ERROR;
	    }
	}
    }
}

// ---------------------------------------------------------------------------

int dis_DirRefresh (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        File *file = (File *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!file) return TCL_ERROR;

	if(!file->IsDirectory())
	    return TCL_ERROR;
	Directory *dir = (Directory *)file;
        File *position;
        int count = 0;

        dir->ReadDirectory (FALSE);
        position = dir->GetFirstFile();

        while (position) {
            count++;
            position = position->Next();
        }

	gString tmp;
        tmp.sprintf ((vchar *)"%d", count + 1); /*  + 1 - Directory name ('.') itself  */
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirIsDirectory (ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        File *dir = (File *) File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

	gString tmp;
        tmp.sprintf ((vchar *)"%d", dir->IsDirectory());
	Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirGetGlobalBeforeFlags (ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    const vchar* flags = File::GetGlobalBeforeFlags();
    if (flags)
	Tcl_SetResult (interp, (char*)flags, TCL_VOLATILE);

    return TCL_OK;
}

// ---------------------------------------------------------------------------

int dis_DirGetGlobalAfterFlags (ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    const vchar* flags = File::GetGlobalAfterFlags();
    if (flags)
	Tcl_SetResult (interp, (char*)flags, TCL_VOLATILE);

    return TCL_OK;
}

// ---------------------------------------------------------------------------

int dis_DirSetGlobalBeforeFlags (ClientData, Tcl_Interp *, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
	File::SetGlobalBeforeFlags ((vchar*)argv[1]);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirSetGlobalAfterFlags (ClientData, Tcl_Interp *, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
	File::SetGlobalAfterFlags ((vchar*)argv[1]);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirGetFlags (ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    if (argc != 2) {
        return TCL_ERROR;
    }
    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

	char *flgs = (char *)dir->GetFlags();
	if(!flgs || flgs[0] != '*')
	    Tcl_SetResult(interp, flgs, TCL_VOLATILE);
	else
	    Tcl_SetResult(interp, &flgs[1], TCL_VOLATILE);
	return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirIsFlags(ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    if (argc != 2) {
        return TCL_ERROR;
    }
    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

	char *flgs = (char *)dir->GetFlags();
	if(!flgs || flgs[0] != '*')
	    Tcl_SetResult(interp, "1", TCL_VOLATILE);
	else
	    Tcl_SetResult(interp, "0", TCL_VOLATILE);
	return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirSetFlags (ClientData, Tcl_Interp *, int argc, char *argv[])
{

    if (argc != 3) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

        vstr *newFlags = vstrCloneScribed (vcharScribeSystem (argv[2])); 
        dir->SetFlags (newFlags);
        vstrDestroy (newFlags);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirSetCompilerFlags(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{
    
    // Arguments : 1 - C++ extensions, 2 - C++ flags, 3 - C extensions, 4 - C flags

    if(argc != 5){
	return TCL_ERROR;
    }
    File::SetCplusExtensions ((vchar*)argv[1]);
    File::SetCplusFlags ((vchar*)argv[2]);
    File::SetCExtensions ((vchar*)argv[3]);
    File::SetCFlags ((vchar*)argv[4]);
    return TCL_OK;
}

// ---------------------------------------------------------------------------

int dis_DirDetermineFlags (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc == 2) {
	
        File *file = (File *)File::handleManager.LookupFile (atoi (argv[1]));
        if (file) {
	    Tcl_SetResult (interp, (char *)file->DetermineFlags(), TCL_VOLATILE);
	    return TCL_OK;
	}
    }
    return TCL_ERROR;
}

// ---------------------------------------------------------------------------

int dis_DirSetExcludeFlag(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc == 3) {
	
	File *file = (File *)File::handleManager.LookupFile (atoi (argv[1]));
        if (file) {
	    Tcl_SetResult (interp, (char *)file->SetExcludeFlag(atoi(argv[2])), TCL_VOLATILE);
	    return TCL_OK;
	}
    }
    return TCL_ERROR;
}


// ---------------------------------------------------------------------------

int dis_DirGetExcludeFlag(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc == 2) {
        File *file = (File *)File::handleManager.LookupFile (atoi (argv[1]));

        if (file) {
	    const vchar *flag = file->GetExcludeFlag();
	    Tcl_SetResult (interp, (char *)flag, TCL_VOLATILE);
	    return TCL_OK;
	}
    }
    return TCL_ERROR;
}


// ---------------------------------------------------------------------------

int dis_DirRemove (ClientData, Tcl_Interp, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;
        dir->RemoveFiles ();
	return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirGetGlobalFilter (ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    const vchar* filter = Directory::GetGlobalFilter();
    if (filter)
	Tcl_SetResult (interp, (char*)filter, TCL_VOLATILE);

    return TCL_OK;
}

// ---------------------------------------------------------------------------

int dis_DirSetGlobalFilter (ClientData, Tcl_Interp *, int argc, char *argv[])
{
    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
	Directory::SetGlobalFilter ((vchar*)argv[1]);
        return TCL_OK;
    }

}
// ---------------------------------------------------------------------------

int dis_DirGetFilter (ClientData, Tcl_Interp *interp, int argc, char *argv[]) 
{

    if (argc = 2) {
        File *file = File::handleManager.LookupFile (atoi (argv[1]));
	if (file && file->IsDirectory()) {
	    Directory *dir = (Directory *)file;
	    
	    Tcl_SetResult (interp, (char *)dir->GetFilter(), TCL_VOLATILE);
	    return TCL_OK;
        }
    }
    Tcl_SetResult (interp, "", TCL_STATIC);
    return TCL_OK;
}

// ---------------------------------------------------------------------------

int dis_DirSetFilter (ClientData, Tcl_Interp *, int argc, char *argv[])
{

    if (argc == 3) {
        File *file = File::handleManager.LookupFile (atoi (argv[1]));
	if (file && file->IsDirectory()) {
	    Directory *dir = (Directory *)file;
	    
	    vstr *newFilter = vstrCloneScribed (vcharScribeSystem (argv[2])); 
	    dir->SetFilter (newFilter);
	    vstrDestroy (newFilter);
	    return TCL_OK;
	} 
    }
    return TCL_ERROR;
}

// ---------------------------------------------------------------------------

int dis_GeneratePDF (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 6) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

        vstr *outputFile = vstrCloneScribed (vcharScribeSystem (argv[2]));
        vstr *projectName = vstrCloneScribed (vcharScribeSystem (argv[3]));
        vstr *sourceRoot = vstrCloneScribed (vcharScribeSystem (argv[4]));
        vstr *modelRoot = vstrCloneScribed (vcharScribeSystem (argv[5]));

        dir->WritePDFFile (outputFile, projectName, sourceRoot, modelRoot);

        vstrDestroy (outputFile);
        vstrDestroy (projectName);
        vstrDestroy (sourceRoot);
        vstrDestroy (modelRoot);
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_CountPDFfiles(ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 3) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;
        vstr *sourceRoot = vstrCloneScribed (vcharScribeSystem (argv[2]));

        unsigned int number = dir->CountPDFfiles (sourceRoot);
	char result[20];
	sprintf(result, "%u", number);
        Tcl_SetResult (interp, result, TCL_VOLATILE);

        vstrDestroy (sourceRoot);
	
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirSave (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 3) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;

        vstr *outputFile = vstrCloneScribed (vcharScribeSystem (argv[2]));
        dir->Save (outputFile);
        vstrDestroy (outputFile);

        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirRestore (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        vstr *outputFile = vstrCloneScribed (vcharScribeSystem (argv[1]));
        Directory *dir = (Directory *)Directory::Restore (outputFile);
        vstrDestroy (outputFile);

        if (dir) {
	    gString tmp;
            tmp.sprintf ((vchar *)"%d", dir->handle);
	    Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
            return TCL_OK;
        }
        else {
	    Tcl_SetResult(interp, "0", TCL_STATIC);
            return TCL_ERROR;
        }
    }
}

// ---------------------------------------------------------------------------

int dis_DirDestroy (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc != 2) {
        return TCL_ERROR;
    }

    else {
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));
        if (!dir) return TCL_ERROR;
 
        delete (dir);       
        return TCL_OK;
    }
}

// ---------------------------------------------------------------------------

int dis_DirDetermineFilter (ClientData, Tcl_Interp *interp, int argc, char *argv[])
{

    if (argc == 2) {
	
        Directory *dir = (Directory *)File::handleManager.LookupFile (atoi (argv[1]));

        if (dir) {
            vchar *flist = dir->DetermineFilter();
            if (flist) {
	        Tcl_SetResult (interp, (char *)flist, TCL_VOLATILE);
            }
	    return TCL_OK;
	}
    }
    return TCL_ERROR;
}
