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
#include "mrg_update.h"

const char* mrgHash::name(const Object* ob) const
{
	const char *ret = NULL;
    if (ob) {
		namedObject *mob = (namedObject *)ob;
 		ret = mob->get_name();
    }
    return ret;
}

mrgHash name_hash;

int hash_file(char* fname, genArrOf(pObject)  &objPtr_arr, FILE* flog)    
{
// read <file_info> and store in buf
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

// process each BEGIN ... END block and put mrg_object to the hash table  
	char* ptr = buf;	
	ptr = strtok( ptr, "\n" );		/* ptr point to  BEGIN */
	while(ptr != NULL) {
		ptr = strtok( NULL, "\n" );		/* ptr point to lname */
		mrg_object** ppObj = objPtr_arr.grow();
		*ppObj = new mrg_object(ptr);
		
		ptr = strtok( NULL, "\n" );		/* ptr point to .tga file name */

		STAT(ptr, &st);
		FILE* ftga = fopen(ptr, "r");

		/* read .tga file to buf1 */ 
		if(!ftga) {
			//cerr << "can not open " << ptr << '\n';
			fprintf(flog, "can not open %s\n", ptr);
			return -1;
		}
#ifdef _WIN32
		_setmode( _fileno(ftga), _O_TEXT );
#endif
		fsize = (int) st.st_size;
		char* buf1 = new char[fsize + 1];
		sz = fread(buf1, 1, fsize, ftga);
		buf1[sz] = '\0';
		fclose(ftga);

		(*ppObj)->tga_buf = buf1;
		
		name_hash.add(*((Object*)(*ppObj)));
		strtok( NULL, "\n" );  /* skip END */
		ptr = strtok( NULL, "\n" );  /* ptr to BEGIN if not finshed */
	}

	delete[] buf;
	fclose(fl);

	return 0;
}


