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
// --------------------------------------------------------------------
//
//  gpath.h - contains the definitions of the following classes necessary
//  for file and directory browsing :
//
//  class File		- abstracts a file
//  class Directory	- abstracts a directory (which is a file)
//  class HandleManager - Manages handles to Files for interation to external systems
//  class FilterList	- abstracts a list of file filters
//  class Filter- abstracts a single file filter
//
// --------------------------------------------------------------------

#ifndef _path_h
#define _path_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vcharINCLUDED
#include vcharHEADER
#endif

#ifndef vfsINCLUDED
#include vfsHEADER
#endif

class File;
class Directory;
class FilterList;

// --------------------------------------------------------------------
// Class HandleManager - manages handles to files used for integration
//                       to external systems.
// --------------------------------------------------------------------

class HandleManager
{

  public:
    HandleManager ();
    ~HandleManager ();
    
    void     UnregisterName(const vchar *name);
    long int RegisterFile (File *file);
    File     *LookupFile (long int handle);
    File     *LookupFile (const vchar *full_name);
    void     Purge ();
    
  private:
    File  ** filelist;
    unsigned int size;
    unsigned int count;
    unsigned int free_entries;
};


// --------------------------------------------------------------------
// Class File - abstracts a file.
// --------------------------------------------------------------------

class File 
{
friend class Directory;

  public:
    File(const vchar* name);
    virtual ~File ();
    
    vbool Save (vfsFile *vfile, int level);
    
    File* Next ();
    
    void SetDirectory (Directory*);
    Directory* GetDirectory ();
    int IsDirectory ();
    
    static const vchar* GetGlobalBeforeFlags ();
    static const vchar* GetGlobalAfterFlags ();
    static void  SetGlobalBeforeFlags (const vchar*);
    static void  SetGlobalAfterFlags (const vchar*);
    static void  SetCplusExtensions (const vchar* exts);
    static void  SetCExtensions (const vchar* exts);
    static void  SetCplusFlags (const vchar* flgs);
    static void  SetCFlags (const vchar* flgs);

    
    const  vchar* SetExcludeFlag(int flag);
    const  vchar* GetExcludeFlag();
    const  vchar* GetFlags ();
    void   SetFlags (const vchar*);
    const  vchar* DetermineFlags ();
    
    void GenerateRules (vfsFile *outputFile);		      
    
    const vchar* Name ();
    const vchar* FullName ();
    
    virtual void Print();
    
  public:
    static HandleManager handleManager;
    static vchar*        globalBeforeFlags;
    static vchar*        globalAfterFlags;
    static vchar*        CplusFlags;
    static vchar*        CFlags;
    static vchar*        CplusExtensions;
    static vchar*        CExtensions;
    long   int           handle;
    
  protected:
    Directory *directory;
    File  *next;
    vchar *fullname;
    vchar *flags;
    int   is_dir;
    int   excluded;
};

// --------------------------------------------------------------------
// Class Directory - abstracts a directory which is a subclass of File.
// --------------------------------------------------------------------

class Directory : public File
{

public:
  Directory (const vchar* name);
  virtual ~Directory ();
 
  void Save (vchar *filename);
  vbool Save (vfsFile *vfile, int level);
  static File *Restore (vchar *filename);
  void Restore (vfsFile *vfile, int level);
 
  static vchar* GetGlobalFilter ();
  static void SetGlobalFilter (const vchar*);
  vchar* GetFilter ();
  void SetFilter (const vchar*);
  vchar* DetermineFilter ();
  FilterList* DetermineLocalFilter ();

  unsigned int  CountPDFfiles(vchar *sourceRoot);

  void WritePDFFile (vchar *outputFile, 
                     vchar *projectName, 
                     vchar *sourceRoot, 
                     vchar *modelRoot);

  void GeneratePDF (vfsFile *outputFile,
                    unsigned int level,
                    FilterList *filters,
                    vchar * physicalPath,
                    vchar * logicalPath);

  void ReadDirectory (vbool displayAll);
  void  RemoveFiles ();
  virtual void Print ();
 
  File *GetFirstFile();

  File* FindFileFromPath (const vchar* path);

private:
  int AddFile (vchar *fileName, File **newList);
  File* FindFile (vchar *name);
  File* InsertionSort (File* fileList);

private:
  File* first;
  FilterList *filterList;
  static FilterList *globalFilterList;
};

// --------------------------------------------------------------------
// Class Filter - abstracts a single File Filter.
// --------------------------------------------------------------------

class Filter
{

public:
  Filter (vchar *filter);
  ~Filter ();
  vbool IsFiltered (vchar *fileName);

  void GeneratePDF (vfsFile *outputFile,
                    unsigned int indent);

private:
  vchar *filter;
  vbool wild;
};

// --------------------------------------------------------------------
// Class FilterList - abstracts a list of File Filters.
// --------------------------------------------------------------------

class FilterList
{

public:
  FilterList (const vchar *filterString);
  ~FilterList ();
  vbool IsFiltered (vchar *fileName);
  vchar *GetFilter ();

  void GeneratePDF (vfsFile *outputFile,
                    unsigned int indent);

private:
  vchar *filterstring;
  vchar *copyFilterstring;
  Filter **filterList;
  unsigned int size;
};

#endif



