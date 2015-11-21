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
#include "msg.h"
#include "mrg_update.h"

struct Update_info {
	char *lname;
	int pos;
	int line_num;
	int attr;		//only used for ATTR
};

extern mrgHash name_hash;

int GAT_list_update(char* fname, genArrOf(pObject)  &objPtr_arr, FILE* flog)   
{
// read <GAT_list> to buf
	struct STAT st;
	STAT(fname, &st);
	FILE* fl = fopen(fname, "r");
	if(!fl) {
		//cerr << "can not open " << fname << '\n';
		fprintf(flog, "can not open %s\n", fname);
		return -1;
	}
#ifdef _WIN32
	_setmode( _fileno(fl), _O_TEXT );
#endif
	int  fsize = (int) st.st_size;
	char* buf = new char[fsize + 1];
	int sz = fread(buf, 1, fsize, fl);
	buf[sz] = '\0';

// process each .grp  or .attr file in the <GAT_list>
	int suc_num = 0;
	int fail_num = 0;
	char* ptr = buf;
	ptr = strtok( ptr, " \t\n" );	/* ptr point to "GROUP" or "ATTR" */
	while(ptr != NULL) {
		if(!strcmp(ptr, "GROUP")) { /* group */
			ptr = strtok(NULL, " \t\n" );	/* ptr point to .grp file name */
			if(update_group(ptr, objPtr_arr, flog)) {
				fprintf(flog, "update GROUP %s failed\n", ptr); 
				fail_num++;
			}
			else {
				fprintf(flog, "GROUP %s updated\n", ptr);
				suc_num++;
			}
		} else if(!strcmp(ptr, "ATTR")) {  /* attribute */
			ptr = strtok(NULL, " \t\n" );	/* ptr point to .attr file name */
			if(update_attribute(ptr, objPtr_arr, flog)) {
				fprintf(flog, "update ATTR %s failed\n", ptr); 
				fail_num++;
			}
			else {
				fprintf(flog, "ATTR %s updated\n", ptr);
				suc_num++;
			}
		} else { /* UNKNOWN */
			ptr = strtok(NULL, " \t\n" );
			fprintf(flog, "UNKNOWN %s\n", ptr);
		}

		ptr = strtok(NULL, " \t\n" );	
	}

// clean up the position array inside objPtr_arr
	int size = objPtr_arr.size();
	for(int i=0; i< size; i++) 
		delete *objPtr_arr[i];
	
	delete[] buf;
	fclose(fl);
	return (fail_num == 0) ? 0: suc_num;
}

int update_group(char *fgroup, genArrOf(pObject)  &objPtr_arr, FILE* flog)
{
// read .grp file to buf
	struct STAT st;
	STAT(fgroup, &st);
	FILE* fl = fopen(fgroup, "r");
	if(!fl) {
		//cerr << "can not open " << fgroup << '\n';
		fprintf(flog, "can not open %s\n", fgroup);
		return 1;
	}
#ifdef _WIN32
	_setmode( _fileno(fl), _O_TEXT );
#endif
	int fsize = (int) st.st_size;
	char* buf = new char[fsize + 1];
	int sz = fread(buf, 1, fsize, fl);
	buf[sz] = '\0';
	fclose(fl);

// process instances in the group file
	int pos, line_num=0;
	char lname[_MAX_PATH];
	char str[_MAX_PATH+16];
	int ret_value, idx;
	Object *cur = 0;
	mrg_object *mrg_cur;
	position *pPosition;
	char *pStart, *pEnd;
	pStart = pEnd = buf;	
	
	while(pStart != NULL) {
		pEnd = strchr(pStart, '\n');
		if(!pEnd) break;

		strncpy(str, pStart, pEnd-pStart);
		str[pEnd-pStart] = '\0';

		ret_value = sscanf(str, "%d:%s", &pos, lname);
		line_num++;
		if(ret_value == 2) {	/* it must be instance */	
			int found = name_hash.find(lname, idx, cur); 
			if(found) {
				mrg_cur = (mrg_object*)cur;
				pPosition = mrg_cur->pos_arr.grow();
				pPosition->pos = pos;
				pPosition->line_num = line_num;
				pPosition->attr = 0;
			}
		}
		pStart = pEnd + 1;	/* skip '\n' and point to next line */
	}

	int total_len = 0;	/* total_line will store numbers of lines need be updated in this group file */
	int obj_size = objPtr_arr.size(), pos_size;

	int i;
	for(i=0; i<obj_size; i++) {
		pos_size = (*objPtr_arr[i])->pos_arr.size();
		if( pos_size > 0) {
			if(pos_size > 1)	
				qsort( (*objPtr_arr[i])->pos_arr[0], pos_size, sizeof(position), sort_pos );
			GetNewPositionArray((*objPtr_arr[i])->pos_arr[0], pos_size, (*objPtr_arr[i])->tga_buf);
			total_len += pos_size; 
		}
	}

// write all the update info to array of structure update_info
	int count = 0;
	Update_info *pUpdate_info = new Update_info[total_len];
	for(i=0; i<obj_size; i++) {
		pos_size = (*objPtr_arr[i])->pos_arr.size();
		for(int j=0; j< pos_size; j++) {
			pPosition = (*objPtr_arr[i])->pos_arr[j];
			pUpdate_info[count].lname = (*objPtr_arr[i])->get_name();
			pUpdate_info[count].pos = pPosition->pos;
			pUpdate_info[count].line_num = pPosition->line_num;
			count++;
		}
		(*objPtr_arr[i])->pos_arr.reset();
	}

	if(total_len > 1)
		qsort( pUpdate_info, total_len, sizeof( Update_info ), sort_linenum );
	
	strcpy(lname, fgroup);
	strcat(lname, ".bak");
#ifdef _WIN32
	sprintf(str, "copy %s %s", fgroup, lname);
#else
	sprintf(str, "cp %s %s", fgroup, lname);
#endif
	system(str);

	FILE *fnew = fopen(fgroup, "w+");
	if(!fnew) {
		//cerr << "can not open " << fname << '\n';
		fprintf(flog, "can not write %s\n", fgroup);
		return 1;
	}

	pStart = pEnd = buf;
	count = 1;
	for(i=0 ; i<total_len; i++) {
		for(;count < pUpdate_info[i].line_num; count++) {
			pEnd = strchr(pEnd, '\n');
			pEnd++;
		}

		print_string(pStart, pEnd, fnew);
		//cout << pUpdate_info[i].pos << ":" << pUpdate_info[i].lname << '\n';
		fprintf(fnew, "%d:%s\n", pUpdate_info[i].pos, pUpdate_info[i].lname);

		pEnd = strchr(pEnd, '\n');
		count++;
		pStart = ++pEnd;
	}
	print_string(pStart, NULL, fnew);
	fclose(fnew);
	
	delete[] buf;
	delete[] pUpdate_info;

	return 0;
}

int update_attribute(char *fattr, genArrOf(pObject)  &objPtr_arr, FILE* flog)
{
// read .attr file to buf
	struct STAT st;
	STAT(fattr, &st);
	FILE* fl = fopen(fattr, "r");
	if(!fl) {
		msg("Cannot open $1", error_sev) << fattr << eom;
		return 1;
	}
#ifdef _WIN32
	_setmode( _fileno(fl), _O_TEXT );
#endif
	int fsize = (int) st.st_size;
	char* buf = new char[fsize + 1];
	int sz = fread(buf, 1, fsize, fl);
	buf[sz] = '\0';
	fclose(fl);

// process instances in the attr file
	int pos, line_num=0, attr;
	char lname[_MAX_PATH];
	char str[_MAX_PATH+16];
	int ret_value, idx;
	Object *cur = 0;
	mrg_object *mrg_cur;
	position* pPosition;
	char *pStart, *pEnd, *pAttr;
	pStart = pEnd = buf;	
	
	while(pStart != NULL) {
		pEnd = strchr(pStart, '\n');
		if(!pEnd) break;

		line_num++;
		strncpy(str, pStart, pEnd-pStart);
		str[pEnd-pStart] = '\0';

		pAttr = strchr(str, '#');
		if(pAttr) {
			*pAttr = '\0';
			ret_value = sscanf(str, "%d:%s", &pos, lname);		
			if(ret_value == 2) {
				attr = atoi(pAttr+1);			
				int found = name_hash.find(lname, idx, cur); 
				if(found) {
					mrg_cur = (mrg_object*)cur;
					pPosition = mrg_cur->pos_arr.grow();
					pPosition->pos = pos;
					pPosition->line_num = line_num;
					pPosition->attr = attr;
				}
			}
		}
		pStart = pEnd + 1;	/* skip '\n' and point to next line */
	}

	int total_len = 0;	/* total_line will store numbers of lines need be updated in this attr file */
	int obj_size = objPtr_arr.size(), pos_size;

	int i;
	for(i=0; i<obj_size; i++) {
		pos_size = (*objPtr_arr[i])->pos_arr.size();
		if( pos_size > 0) {
			if(pos_size > 1)	
				qsort( (*objPtr_arr[i])->pos_arr[0], pos_size, sizeof(position), sort_pos );
			GetNewPositionArray((*objPtr_arr[i])->pos_arr[0], pos_size, (*objPtr_arr[i])->tga_buf);
			total_len += pos_size; 
		}
	}

// write all the update info to array of structure update_info
	int count = 0;
	Update_info *pUpdate_info = new Update_info[total_len];
	for(i=0; i<obj_size; i++) {
		pos_size = (*objPtr_arr[i])->pos_arr.size();
		for(int j=0; j< pos_size; j++) {
			pUpdate_info[count].lname = (*objPtr_arr[i])->get_name();
			pPosition = (*objPtr_arr[i])->pos_arr[j];
			pUpdate_info[count].pos = pPosition->pos;
			pUpdate_info[count].line_num = pPosition->line_num;
			pUpdate_info[count].attr = pPosition->attr;
			count++;
		}
		(*objPtr_arr[i])->pos_arr.reset();
	}

	if(total_len > 1)
		qsort( pUpdate_info, total_len, sizeof( Update_info ), sort_linenum );


	strcpy(lname, fattr);
	strcat(lname, ".bak");
#ifdef _WIN32
	sprintf(str, "copy %s %s", fattr, lname);
#else
	sprintf(str, "cp %s %s", fattr, lname);
#endif
	system(str);

	FILE *fnew = fopen(fattr, "w+");
	if(!fnew) {
		//cerr << "can not open " << fname << '\n';
		fprintf(flog, "can not write %s\n", fattr);
		return 1;
	}

	pStart = pEnd = buf;
	count = 1;
	for(i=0 ; i<total_len; i++) {
		for(;count < pUpdate_info[i].line_num; count++) {
			pEnd = strchr(pEnd, '\n');
			pEnd++;
		}

		print_string(pStart, pEnd, fnew);
		//cout << pUpdate_info[i].pos << ":" << pUpdate_info[i].lname << '\n';
		fprintf(fnew, "%d:%s#%d\n", pUpdate_info[i].pos, pUpdate_info[i].lname, pUpdate_info[i].attr);

		pEnd = strchr(pEnd, '\n');
		count++;
		pStart = ++pEnd;
	}
	print_string(pStart, NULL, fnew);
	fclose(fnew);
	
	delete[] buf;
	delete[] pUpdate_info;

	return 0;
}

int sort_pos( const void *arg1, const void *arg2 )
{
   return ((position *)arg1)->pos - ((position *)arg2)->pos;
}

int sort_linenum( const void *arg1, const void *arg2 )
{
   return ((Update_info *)arg1)->line_num - ((Update_info *)arg2)->line_num;
}

//used for -merge
int sort_line( const void *arg1, const void *arg2 )
{
   return ((position *)arg1)->line_num - ((position *)arg2)->line_num;
}


