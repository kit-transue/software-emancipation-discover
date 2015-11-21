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
#ifndef SET_FILE_TABLE_H
#define SET_FILE_TABLE_H

#ifndef _STDIO_H
#include <stdio.h>
#endif

/* IMPORTANT: 
 * THE MACRO FILE_TABLE_HASH_TABLE_SIZE *MUST*
 * HAVE THE FORM "2^N"  SO THAT SOME
 * CALCULATIONS (SPECIFICALLY FILENAME HASHING)
 * WORK OUT CORRECTLY!!!!! */
#define FILE_TABLE_HASH_TABLE_SIZE 1024

struct file_table_hash_bucket_str
{
  void **filelist;
  int    size;
  int    maxsize;
};

typedef struct file_table_hash_bucket_str file_table_hash_bucket;
typedef file_table_hash_bucket file_table[FILE_TABLE_HASH_TABLE_SIZE];

#define FILE_TABLE_HASH_BUCKET_PTR(T,I)    (&((T)[I]))
#define FILE_TABLE_HASH_BUCKET_FILELIST(B) ((B)->filelist)
#define FILE_TABLE_HASH_BUCKET_FILE(B,I)   ((B)->filelist[I])
#define FILE_TABLE_HASH_BUCKET_SIZE(B)     ((B)->size)
#define FILE_TABLE_HASH_BUCKET_MAX_SIZE(B) ((B)->maxsize)
#define FILE_TABLE_HASH_BUCKET_BLOCKSIZE   16

int      add_to_file_table(file_table *, void *);
int      add_to_file_table_hash_bucket(file_table_hash_bucket *, void *);
int      destroy_file_table(file_table **);
int      destroy_file_table_hash_bucket(file_table_hash_bucket *);
int      file_table_dump_hash_table(file_table *, FILE *);
int      file_table_dump_hash_(file_table_hash_bucket *, FILE *);
int      file_table_hash_string(const char *);
long int file_table_hash_char(char, int);
int      grow_file_table_hash_bucket(file_table_hash_bucket *);
void*    in_file_table(file_table *, const char *);
void*    in_file_table_hash_bucket(file_table_hash_bucket *, const char *);
int      init_file_table(file_table **);
int      init_file_table_hash_buckets(file_table *);
int      init_one_file_table_hash_bucket(file_table_hash_bucket *);

#endif /* SET_FILE_TABLE_H */
