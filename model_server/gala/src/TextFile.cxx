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
#include <TextFile.h>

TextFile::TextFile (const vchar* filename)
{
    modified_ = 0;
    filename_ = filename;

    vfsPath vpath;
    vpath.Set(filename);
    vfsFile* vfile = 0;

    vexWITH_HANDLING {
        vfile = vfsFile::Open (&vpath, vfsOPEN_READ_ONLY);
    } vexON_EXCEPTION {
    } vexEND_HANDLING;
    if (vfile == NULL) return;

    vfile->Seek(0, vfsSEEK_FROM_END);
    long file_size = vfile->GetPos();
    char* file_buffer = new char[file_size + 1];
    vfile->Seek(0, vfsSEEK_FROM_START);
    vfile->RawRead((vbyte *)file_buffer, file_size);

    vtextSelection sel;
    InitSelection (&sel);
	InsertText(&sel, (const vchar *)file_buffer, file_size);
	//ScrollToStart(vtextviewSTS_VERTICAL);
    delete [] file_buffer;
}

TextFile::~TextFile()
{
}

TextFile* TextFile::Find (const vchar* filename)
{
    return 0;
}

void TextFile::OpenFile()
{
}

void TextFile::CloseFile()
{
}


void TextFile::RevertFile()
{
}

void TextFile::SaveFile()
{
}

void TextFile::SaveFileAs(const vchar* filename)
{
}

int TextFile::isModified()
{
    return modified_;
}

