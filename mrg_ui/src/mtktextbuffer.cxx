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
#include "mtktextbuffer.h"
#include "difffile.h"
#include "tcl.h"
#include "framework.h"
#include <stdlib.h>

struct Marker {
	char *tag_name;
	textBuffer *owner;
};

//============================================================================
// Construct a region with the given bounds, measured in character offset
// from the beginning of an implicit text buffer.
//
Region::Region(int start, int end) :
	offset(start),
	length(end - start),
	marker(NULL)
{
}

//============================================================================
// Stop tracking and remove highlights/underlines associated with this region.
//
Region::~Region()
{
	if(marker != NULL){
		textBuffer *tbuf = marker->owner;
		if (marker->tag_name != NULL) {
			tbuf->framework.Tcl("%s tag delete %s",
				             tbuf->tk_name.str(), marker->tag_name);
			delete [] marker->tag_name;
		}
		delete marker;
	}
}

//============================================================================
// This region is valid unless it has been tracking and its tag has been
// deleted.
int
Region::IsValid()
{
	if(marker != NULL){
		return marker->owner->tagAlive(marker->tag_name);
	} else {
		return 1;
	}
}

//============================================================================
// Return the character offset of the beginning of this region.  This is
// easy if tracking has not been enabled on the region.
//
int Region::GetOffset()
{
	if(marker == NULL){
		return offset;
	} else {
		textBuffer *tbuf = marker->owner;
		Tcl_Interp *itrp = tbuf->framework.interp;
		int index;
		int start = offset;
		if (tbuf->framework.Tcl("indexToOffset %s %s.first",
				   tbuf->tk_name.str(), marker->tag_name)
			== TCL_OK
		    && Tcl_GetInt(itrp, itrp->result, &index) == TCL_OK) {

		    start = index;
		}
		return start;
	}
}

//============================================================================
// Return the length of this region.  This is easy if tracking has not been
// enabled on the region.
//
int Region::GetLength(void)
{
	if(marker == NULL){
		return length;
	} else {
		int start, length;
		marker->owner->GetRegionRange(this, start, length);
		return length;
	}
}

//============================================================================
// Construct a text buffer which acts as a part of the given framework.
//
textBuffer::textBuffer(FrameWork &fw)
    : framework(fw)
{
}

//============================================================================
// Load this textBuffer with the contents of the named file.
//
void textBuffer::LoadFile(char *fname)
{
	FILE *fl = fopen(fname, "r");
	if(fl == NULL)
		return;
	fseek(fl, 0, SEEK_END);
	long size = ftell(fl);
	fseek(fl, 0, SEEK_SET);
	char *ptr = (char *)malloc(size + 1);
	if(ptr == NULL){
		fclose(fl);
		return;
	}
	int actual_size = fread(ptr, 1, size, fl);
	if(actual_size == 0){
		fclose(fl);
		free(ptr);
		return;
	}
	ptr[actual_size] = 0;
	fclose(fl);

	framework.Tcl("%s cget -state", tk_name.str());
	boolean read_only = (strcmp(framework.interp->result, "disabled") == 0);
	if (read_only) {
	    framework.Tcl("%s configure -state normal", tk_name.str());
	}
	SetText(ptr);
	if (read_only) {
	    framework.Tcl("%s configure -state disabled", tk_name.str());
	}
	free(ptr);
}

//============================================================================
// Store the text from here into the named file.
//
void textBuffer::SaveText(char *fname)
{
	if (framework.Tcl("%s get 1.0 end", tk_name.str()) != TCL_OK) {
		return;
	}
	char *ptr = framework.interp->result;
	if(ptr == NULL){
		return;
	}
	int size = strlen(ptr);
	if (size > 0) { 	// could be Assert(size): TK is reliably bad.
		--size;		// Bug 20121: TK text widget adds CR(s) to end text!
	}
	FILE *fl = fopen(fname, "w");
	if(fl == NULL){
		return;
	}
	fwrite(ptr, 1, size, fl);
	fclose(fl);
	return;
}

//============================================================================
// Associate this textBuffer object with the named Tk text widget.
//
void textBuffer::AttachDriver(genString &driver)
{
    tk_name = driver;
}

//============================================================================
// Set this widget's text as indicated.
//
void textBuffer::SetText(char *text)
{
    framework.Tcl("%s delete 1.0 end", tk_name.str());
    framework.Tcl("%s insert end %s",
                   tk_name.str(),
                   framework.quoteForTclString(text).str());
}

//============================================================================
// Set GUI focus to the given region, including keyboard focus, setting the
// selection to the start of the region, and scrolling to put it in view.
//
void textBuffer::FocusOnRegion(Region *region)
{
    int offset = region->GetOffset();
    framework.Tcl("focus %s", tk_name.str());
    framework.Tcl("%s mark set insert \"1.0 + %d chars\"",
		   tk_name.str(), offset);
    framework.Tcl("%s see \"1.0 + %d chars\"",
		   tk_name.str(), offset);
}

//============================================================================
// Underline the given region of this text GUI object.  Start tracking
// the region with the range of text.
//
void textBuffer::UnderlineRegion(Region *region)
{
	int start, length;

	char *m = GetMarker(region);
	GetRegionRange(region, start, length);
	framework.Tcl("%s tag configure %s -underline 1",
		       tk_name.str(), m);
}

//============================================================================
// Highlight the given region of this text GUI object.  Start tracking
// the region with the range of text.
//
void textBuffer::HighliteRegion(Region *region)
{
	int start, length;

	char *m = GetMarker(region);
	GetRegionRange(region, start, length);
	framework.Tcl("%s tag configure %s -background blue",
		       tk_name.str(), m);
	framework.Tcl("%s tag configure %s -foreground white",
		       tk_name.str(), m);
}

//============================================================================
// Remove highlight from the given region of this text GUI object.
//
void textBuffer::UnHighliteRegion(Region *region)
{
	int start, length;

	char *m = GetMarker(region);
	GetRegionRange(region, start, length);
	framework.Tcl("%s tag configure %s -background {}",
		       tk_name.str(), m);
	framework.Tcl("%s tag configure %s -foreground {}",
		       tk_name.str(), m);
}

//============================================================================
// Apply the given change to this text GUI object. If undo_change is not null,
// save the inverse change there, which can be applied later if needed.
// The offset_diff of the affected text relative to the change record
// must be provided.  Sets original_start and orginal_end according to
// the change record.  Sets new_start and new_end to the location of the
// changed region in this text GUI object.
//
void textBuffer::ApplyChange(Change *change, Change *undo_change, int &new_start, 
			 int &new_end, int &original_start, int &original_end,
			 int offset_diff)
{
    if(change->operation == DELETE){
	if(undo_change){
	    char *saved_text       = GetText(  change->offset
							  + offset_diff,
                                                          change->length);
	    undo_change->operation = INSERT;
	    undo_change->offset    = change->offset;
	    undo_change->length    = change->length;
	    undo_change->new_text  = saved_text;
	}

	DeleteText(change->offset + offset_diff, change->length);
	original_start = change->offset;
	original_end   = change->offset + change->length;
	new_start      = change->offset + offset_diff;
	new_end        = new_start;
    } else if(change->operation == INSERT){
	if(undo_change){
	    undo_change->operation = DELETE;
	    undo_change->offset    = change->offset;
	    undo_change->length    = change->new_text.length();
	    undo_change->new_text  = "";
	}

	InsertText(change->offset + offset_diff,
                                (char *)change->new_text);
	original_start = change->offset;
	original_end   = original_start;
	new_start      = change->offset + offset_diff;
	new_end        = new_start + change->new_text.length();
    } else if(change->operation == REPLACE){
	if(undo_change){
	    char *saved_text       = GetText(  change->offset
                                                          + offset_diff,
                                                          change->length);
	    undo_change->operation = REPLACE;
	    undo_change->offset    = change->offset;
	    undo_change->length    = change->new_text.length();
	    undo_change->new_text  = saved_text;
	}

	DeleteText(change->offset + offset_diff, change->length);
	InsertText(change->offset + offset_diff,
                                (char *)change->new_text);
	original_start = change->offset;
	original_end   = change->offset + change->length;
	new_start      = change->offset + offset_diff;
	new_end        = new_start + change->new_text.length();
    }
}

//============================================================================
// Add to the region an association with tags in this GUI object's text
// (if not yet associated).
//
void textBuffer::TrackRegion(Region *region)
{
	int start, length;

	GetRegionRange(region, start, length);
}

//============================================================================
// Return the tag name of the region's marker.  If the region doesn't
// have one yet, then construct one with a unique, new tag name.
//
char *textBuffer::GetMarker(Region *region)
{
	static int marker_count = 0;
	if (region->marker == NULL) {
		genString tagstr;
		tagstr.printf("marker%d", ++marker_count);
		char *mname = new char[tagstr.length() + 1];
		strcpy(mname, tagstr.str());
		Marker *m = new Marker;
		region->marker = m;
		m->tag_name = mname;
		m->owner = this;
		return mname;
	}
	else {
		return (region->marker)->tag_name;
	}
}

//============================================================================
// Return the current starting offset and length of the given region.
// If the region is being tracked, these values are brought up to date.
// If not, tracking is begun.
//
void textBuffer::GetRegionRange(Region *region, int &start, int &length)
{
    Tcl_Interp *itrp = framework.interp;

    char *m = GetMarker(region);
    if (!tagAlive(m)) {
	// Tag does not yet exist.
	framework.Tcl("makeTag %s %s %d %d",
		       tk_name.str(),
		       m,
		       region->offset,
		       region->offset + region->length);
	start = region->offset;
	length = region->length;
    }
    else {
	int index;
	start = region->offset;
	length = region->length;
	if (framework.Tcl("indexToOffset %s %s.first", tk_name.str(), m)
                == TCL_OK
	    && Tcl_GetInt(itrp, itrp->result, &index) == TCL_OK) {

	    start = index;
	    if (framework.Tcl("indexToOffset %s %s.last", tk_name.str(), m)
                    == TCL_OK 
		&& Tcl_GetInt(itrp, itrp->result, &index) == TCL_OK) {

		length = index - start;
	    }
	}
    }
}

//============================================================================
// Low-level GUI helper, returns whether the Tk tag exists in this
// textBuffer's Tk widget.
//
boolean textBuffer::tagAlive(char *tag_name)
{
    Tcl_Interp *itrp = framework.interp;

    int exists;
    if (framework.Tcl("tagExists %s %s", tk_name.str(), tag_name) == TCL_OK
	 && Tcl_GetBoolean(itrp, itrp->result, &exists) == TCL_OK) {
	// exists is set properly.
    }
    else {
	exists = 0;
    }
    return !!exists;
}

//============================================================================
// Updates a char buffer to contain the same text as this textBuffer's
// contents at the given start and length.  Returns a pointer to the
// updated buffer.
//
char *textBuffer::GetText(int start, int length)
{
	static char *tmp_buf = NULL;
	static int  size     = 0;

	if(length >= size || tmp_buf == NULL){
		if(tmp_buf)
			free(tmp_buf);
		tmp_buf = (char *)malloc((size = length + 1));
	}
	if (framework.Tcl("%s get \"1.0 + %d chars\" \"1.0 + %d chars\"",
			   tk_name.str(), start, start + length) != TCL_OK) {
		return "";
	}
	char *ptr = framework.interp->result;
	if(ptr == NULL){
		return "";
	}
	memcpy(tmp_buf, ptr, length);
	tmp_buf[length] = 0;
	return tmp_buf;
}

//============================================================================
// Deletes 'length' characters from this textBuffer beginning with offset
// 'start'.
//
void textBuffer::DeleteText(int start, int length)
{
	framework.Tcl("%s delete \"1.0 + %d chars\" \"1.0 + %d chars\"",
		       tk_name.str(), start, start + length);
}

//============================================================================
// Inserts the 'text' to this textBuffer at the location, offset 'start'
// characters from the beginning.
//
void textBuffer::InsertText(int start, char *text)
{
	framework.Tcl("%s insert \"1.0 + %d chars\" %s",
		       tk_name.str(),
		       start,
		       framework.quoteForTclString(text).str());
}
