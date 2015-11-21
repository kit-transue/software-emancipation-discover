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
#include <sys/types.h>
#include "basics.h"
#include "host_envir.h"
#include "md5.h"
#include "SET_checksum.h"

typedef unsigned char a_checksum[16]; 

typedef struct checksumInfo {
    MD5_CTX       *MD5_context;
    a_checksum    *MD5_checksum;
} checksumInfo;

void initialize_checksum_info(void **csinfo)
{
    checksumInfo *csi = (checksumInfo *)malloc(sizeof(checksumInfo));

    if (csi != NULL) {
        csi->MD5_context = (MD5_CTX *) malloc(sizeof(MD5_CTX));
        MD5Init(csi->MD5_context);
        csi->MD5_checksum = NULL;
   
        *csinfo = (void *)csi;
    }
    return;
}

void finalize_checksum_info(void *csinfo)
{
    checksumInfo *csi = (checksumInfo *)csinfo;

    if (csi->MD5_context != NULL) {
        csi->MD5_checksum = (unsigned char (*)[16])malloc(16);
        MD5Final(*csi->MD5_checksum, csi->MD5_context);
        free(csi->MD5_context);
        csi->MD5_context = NULL; 
    }
}

unsigned char *get_final_checksum(void *csinfo)
{
    checksumInfo *csi = (checksumInfo *)csinfo;
    if (csi != NULL && csi->MD5_checksum != NULL) {
        return *csi->MD5_checksum;
    }
    return NULL;
}

void update_checksum(int ch, void *csinfo)
{
    checksumInfo *csi = (checksumInfo *)csinfo;
    if (csi != NULL && csi->MD5_context != NULL) {
        unsigned char uch = (unsigned char)ch;
        MD5Update(csi->MD5_context, &uch, 1);
    }
}

void update_checksum_with_buf(char *str, int len, void *csinfo)
{
    checksumInfo *csi = (checksumInfo *)csinfo;

    if (csi != NULL && csi->MD5_context != NULL) {
        unsigned char *ustr = (unsigned char *)str;
        MD5Update(csi->MD5_context, ustr, len);
    }
}

void checksum_of_archive(char *file_name, void **csip)
{
    FILE *arc_fp;
    arc_fp = open_input_file(file_name, TRUE);
    if (arc_fp) {
        char buf[1024];
        unsigned int readlen;
        unsigned int buf_sz = sizeof(buf);

        initialize_checksum_info(csip);        
        while((readlen = fread(buf, sizeof(char), buf_sz, arc_fp)) > 0) {
            update_checksum_with_buf(buf, readlen, *csip);
        }
        finalize_checksum_info(*csip);  

        fclose(arc_fp);
    }
}




