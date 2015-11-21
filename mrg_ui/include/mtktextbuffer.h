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
#ifndef _MTKTEXTBUFFER_H
#define _MTKTEXTBUFFER_H

#include "genString.h"
#include "general.h"
class Change;
class FrameWork;
struct Marker;

class Region {
public:
	Region(int start, int end);
	~Region();

	int IsValid();
	int GetOffset();
	int GetLength();

	int  offset;
	int  length;
	Marker *marker;
};

class textBuffer {
public:
	textBuffer(FrameWork &fw);

	void LoadFile(char *fname);
	void SaveText(char *fname);
	void AttachDriver(genString &driver);
	void SetText(char *text);
	void FocusOnRegion(Region *region);
	void UnderlineRegion(Region *region);
	void HighliteRegion(Region *region);
	void UnHighliteRegion(Region *region);
	void ApplyChange(Change *change, Change *undo_change, int &new_start, int &new_end, 
					 int &original_start, int &original_end, int offset_diff = 0);
	void TrackRegion(Region *region);
	boolean tagAlive(char *tag_name);
	void GetRegionRange(Region *reg, int &start, int &length);

// DATA:
public:
	FrameWork &framework;
	genString tk_name;
private:
	char *GetMarker(Region *region);
	char *GetText(int start, int length);
	void DeleteText(int start, int length);
	void InsertText(int start, char *text);
};

#endif
