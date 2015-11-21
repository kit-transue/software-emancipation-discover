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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "machdep.h"
#include "difffile.h"

//----------------------------------------------------------------------------------

Change::Change()
{
	applied = 0;
}

//----------------------------------------------------------------------------------
ChangeFileName::ChangeFileName(char const *str) :
	name(str)
{
}


//----------------------------------------------------------------------------------

DiffFile::DiffFile(char const *source) :
	source_name(source)
{
}

//----------------------------------------------------------------------------------

DiffFile::~DiffFile()
{
    int i;
    for(i = 0; i < changes.size(); i++){
	Change *ch = *(Change **)changes[i];
	delete ch;
    }
    for(i = 0; i < conflicts.size(); i++){
	Conflict *cf = *(Conflict **)conflicts[i];
	delete cf;
    }
    for(i = 0; i < file_components.size(); i++){
	ChangeFileName *fn = *(ChangeFileName **)file_components[i];
	delete fn;
    }
}

//----------------------------------------------------------------------------------

char
DiffFile::GetSeparator(char *str)
{
    int sep   = '/';
    int found;
    do {
	found   = 0;
	char *p = strchr(str, sep);
	if(p){
	    sep++;
	    found = 1;
	}
    } while(found == 1 && sep < 255);
    return sep;
}

//----------------------------------------------------------------------------------

void
DiffFile::SaveTextField(char const *txt, FILE *fl)
{
    const char *symbol = "> ";

    char const *p = txt;
    while(*txt != 0){
	while(*p != 0 && *p != '\n')
	    p++;
	if(*p == '\n'){
	    fwrite(txt, 1, p - txt + 1, fl); // write text including '\n'
	    fwrite(symbol, 1, 2, fl); // write '> '
	    p++;
	    txt = p;
	} else {
	    fwrite(txt, 1, p - txt, fl); // write text excluding '\0'
	    break;
	}
    }
}

//----------------------------------------------------------------------------------

int
DiffFile::SaveChangeFile(char const *name)
{
    FILE *fl = fopen(name, "wb");
    if(fl == NULL)
	return 0;
    for(int i = 0; i < changes.size(); i++){
	Change *ch = *(Change **)changes[i];
	genString tmp;
	if(ch->index != -1){
	}
	char separator = GetSeparator((char *)ch->new_text);
	tmp.printf("%d,%d,%c", ch->offset, ch->length, separator);
	fwrite((char *)tmp, 1, strlen((char *)tmp), fl);
	SaveTextField((char *)ch->new_text, fl);
	tmp.printf("%c\n", separator);
	fwrite((char *)tmp, 1, strlen((char *)tmp), fl);
/*
	tmp.sprintf((vchar *)"%d,%d,%c%s%c\n", ch->offset, ch->length, separator, (char *)ch->new_text, separator);
	fwrite((char *)tmp, 1, strlen((char *)tmp), fl);
*/
    }
    fclose(fl);
    return 1;
}

//----------------------------------------------------------------------------------

int
DiffFile::LoadHeader(FILE *fl)
{
	return fl != NULL;
}

//----------------------------------------------------------------------------------

static int WhiteSpace(int ch)
{
	return (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t');
}

//----------------------------------------------------------------------------------

int
DiffFile::ParseIndex(FILE *fl, int c, Change *ch)
{
	char buf[256];

	while(c != EOF && WhiteSpace(c))
		c = fgetc(fl);
	if(c == '['){
		int size = 0;
		c    = fgetc(fl);
		while(c != ']' && c != EOF && size < 255){
			buf[size++] = c;
			c           = fgetc(fl);
		}
		if(size == 0)
			return EOF; // ERROR ?
		buf[size] = 0;
		ch->index = atoi(buf);
		c = fgetc(fl);
	} else {
	    ch->index = -1;
	}
	return c;
}

//----------------------------------------------------------------------------------

int
DiffFile::ParseStartOffset(FILE *fl, int c, Change *ch)
{
	char buf[256];

	while(c != EOF && WhiteSpace(c))
		c = fgetc(fl);
	if(c == EOF)
		return c;
	int size = 0;
	while(c != EOF && isdigit(c)){
		buf[size++] = c;
		c           = fgetc(fl);
	}
	if(size == 0)
		return EOF; // ERROR ?
	buf[size]  = 0;
	ch->offset = atoi(buf);
	while(c != EOF && WhiteSpace(c))
		c = fgetc(fl);
	if(c == ',')
		c = fgetc(fl);
	else
		return EOF; // ERROR ?
	return c;
}

//----------------------------------------------------------------------------------

int
DiffFile::ParseLength(FILE *fl, int c, Change *ch)
{
	char buf[256];

	while(c != EOF && WhiteSpace(c))
		c = fgetc(fl);
	if(c == EOF)
		return c;
	int size = 0;
	while(c != EOF && isdigit(c)){
		buf[size++] = c;
		c           = fgetc(fl);
	}
	if(size == 0)
		return EOF; // ERROR ?
	buf[size]  = 0;
	ch->length = atoi(buf);
	while(c != EOF && WhiteSpace(c))
		c = fgetc(fl);
	if(c == ',')
		c = fgetc(fl);
	else
		return EOF; // ERROR ?
	return c;
}

//----------------------------------------------------------------------------------

int
DiffFile::ParseText(FILE *fl, int c, Change *ch)
{
    while(c != EOF && WhiteSpace(c))
	c = fgetc(fl);
    if(c == EOF)
	return c;
    int separator = c;
    c             = fgetc(fl);
    while(c != EOF && c != separator){
	if(c == '\n'){
	    fgetc(fl);  // Skip '> ' after newline
	    fgetc(fl);  
	}
	ch->new_text += (char)c;
	c             = fgetc(fl);
    }
    if(c == EOF)
	return EOF; // ERROR ?
    return c;
}

//----------------------------------------------------------------------------------

int
DiffFile::ParseLine(FILE *fl)
{
	int c = fgetc(fl);
	do {
	    while(c != EOF && WhiteSpace(c))
		c = fgetc(fl);
	    if(c == EOF)
		return 0;
	    if(c == '#'){
		while(c != EOF && c != '\n')
		    c = fgetc(fl);
	    } else 
		break;
	} while(1);		
	Change *ch = new Change;
        c = ParseIndex(fl, c, ch);
	if(c == EOF) return 0;
	c = ParseStartOffset(fl, c, ch);
	if(c == EOF) return 0;
	c = ParseLength(fl, c, ch);
	if(c == EOF) return 0;
	c = ParseText(fl, c, ch);
	if(c == EOF) return 0;
	if(ch->length == 0)
		ch->operation = INSERT;
	else
		if(ch->length != 0 && ch->new_text.length() == 0)
			ch->operation = DELETE;
		else 
			ch->operation = REPLACE;
	changes.append(&ch);
	return 1;
}

//----------------------------------------------------------------------------------

int
DiffFile::LoadChanges(FILE *fl)
{
	while(!feof(fl)){
		if(ParseLine(fl) == 0)
		{
			// ERROR;
		} else {

		}
	}
	return 1;
}

//----------------------------------------------------------------------------------

void
DiffFile::LoadChangeFile(char const *fname)
{
	FILE *fl = fopen(fname, "r");
	if(LoadHeader(fl)){
		LoadChanges(fl);
	}
	fclose(fl);
	ChangeFileName *fn = new ChangeFileName(fname);
	file_components.append(&fn);
}

//----------------------------------------------------------------------------------

int
DiffFile::ChangeCompare(const void *elem1, const void *elem2)
{
	Change *change1 = *(Change **)elem1;
	Change *change2 = *(Change **)elem2;

	if(change1->offset < change2->offset)
		return -1;
	else if(change1->offset > change2->offset)
		return 1;
	else return 0;
}

//----------------------------------------------------------------------------------

void
DiffFile::Sort()
{
#if defined(ISO_CPP_HEADERS) && defined(sun5)
	std::qsort(changes[0], changes.size(), sizeof(ChangePtr), ChangeCompare);
#else
	qsort(changes[0], changes.size(), sizeof(ChangePtr), ChangeCompare);
#endif
}

//----------------------------------------------------------------------------------

int
DiffFile::TestConflicts()
{
	int end_offset = -1;
	for(int i = 0; i < changes.size(); i++){
		Change *ch = *(Change **)changes[i];
		if(ch->offset < end_offset)
			return 1;
		if(ch->offset + ch->length > end_offset)
			end_offset = ch->offset + ch->length;
	}
	return 0;
}

//----------------------------------------------------------------------------------

void
DiffFile::PurgeConflictsList()
{
	for(int i = 0; i < conflicts.size(); i++){
		Conflict *cf = *(Conflict **)conflicts[i];
		delete cf;
	}
	conflicts.reset();
}

//----------------------------------------------------------------------------------

void
DiffFile::RemoveSimilarChanges()
{
    // Assumes that changes array is sorted already.
    int i = 0;
    while(i < changes.size()){
	Change *ch = *(Change **)changes[i];
	int      j = i + 1;
	while(j < changes.size()){
	    Change *ch1 = *(Change **)changes[j];
	    if(ch1->offset == ch->offset && ch1->length == ch->length &&
		(ch1->length == 0 && ch->length == 0 ||
		strcmp((char *)ch1->new_text, (char *)ch->new_text) == 0))
		changes.remove(j);
	    else
		break;
	}
	i++;
    }       
}

//----------------------------------------------------------------------------------

void
DiffFile::BuildConflictsList()
{
	PurgeConflictsList();
	int end_offset   = -1;
	int first_change = 0;
	int i = 0;
	while(i < changes.size()){
		Change *ch = *(Change **)changes[i];
		if(ch->offset < end_offset){
			Conflict *cf     = new Conflict;
			cf->first_change = first_change;
			cf->no_changes   = 2;
			if(ch->offset + ch->length > end_offset)
				end_offset = ch->offset + ch->length;
			i++;
			while(i < changes.size()){
				ch = *(Change **)changes[i];
				if(ch->offset < end_offset)
					cf->no_changes++;
				else
					break;
				if(ch->offset + ch->length > end_offset)
					end_offset = ch->offset + ch->length;
				i++;
			}
			conflicts.append(&cf);
		} else {
			end_offset   = ch->offset + ch->length;
			first_change = i;
			i++;
		}
	}
}

//----------------------------------------------------------------------------------

Conflict *
DiffFile::FindConflict(Change *change)
{
	for(int i = 0; i < conflicts.size(); i++){
		Conflict *cf = *(Conflict **)conflicts[i];
		for(int j = cf->first_change; j < cf->first_change + cf->no_changes; j++){
			Change *ch = *(Change **)changes[j];
			if(ch == change)
				return cf;
		}
	}
	return NULL;
}

//----------------------------------------------------------------------------------

void
DiffFile::GetConflictRange(Conflict *cf, int &start, int &end)
{
	Change *ch = *(Change **)changes[cf->first_change];
	start      = ch->offset;
	end        = ch->offset + ch->length;
	for(int j = cf->first_change + 1; j < cf->first_change + cf->no_changes; j++){
		ch = *(Change **)changes[j];
		if(ch->offset + ch->length > end)
			end = ch->offset + ch->length;
	}
}

//----------------------------------------------------------------------------------

Change *
DiffFile::GetChange(int num)
{
	Change *ch = *(Change **)changes[num];
	return ch;
}

//----------------------------------------------------------------------------------

int
DiffFile::GetNumOfChanges()
{
	return changes.size();
}

//----------------------------------------------------------------------------------

int
DiffFile::GetNumOfConflicts()
{
	return conflicts.size();
}

//----------------------------------------------------------------------------------

void
DiffFile::RemoveDeltaFiles()
{
    for(int i = 0; i < file_components.size(); i++){
	ChangeFileName *fn = *(ChangeFileName **)file_components[i];
	OSapi_unlink((char *)fn->name);
    }
}

//----------------------------------------------------------------------------------

int
DiffFile::GetDeltaFileCount()
{
    return file_components.size();
}

//----------------------------------------------------------------------------------

char *
DiffFile::GetDeltaFileName(int index)
{
    ChangeFileName *fn = *(ChangeFileName **)file_components[index];
    return (char *)fn->name;
}

//----------------------------------------------------------------------------------

void
DiffFile::ReadComment(char *file_name, gString& result)
{
    char buffer[512];

    result   = "";
    FILE *fl = fopen(file_name, "r");
    if(fl == NULL)
	return;
    while(fgets(buffer, sizeof(buffer), fl)){
	if(buffer[0] == '#'){
	    result += &buffer[1];
	} else
	    break;
    }
    fclose(fl);
}
