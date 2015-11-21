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
/*
*
*
*
*
*/

#ifdef SOFTWARE_EMANCIPATION_PARSER

/* ++CCcc */
#define STDLIB_H_INCLUDED
#include "basic_hdrs.h"
#include "fe_common.h"
#include "SET_file_table.h"
#include "SET_additions.h"
/* --CCcc */

/* ++C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* --C */


/*
*
*
*
*
*/

long int file_table_hash_char(char c, int pos)
{
  return 0L;
}

int file_table_hash_string(const char *filename)
{
  int      retval = -1;

  if ( filename )
    {
      register int r = 0;
      register const char *s = filename;

      while ( *s )
	r = r * 113 + *s++;

      /* IMPORTANT: SAME NOTE AS IN HEADER FILE! 
       * THE MACRO FILE_TABLE_HASH_TABLE_SIZE *MUST*
       * HAVE THE FORM "2^N"  SO THAT THIS WILL
       * WORK OUT CORRECTLY!!!!! */
      r &= (FILE_TABLE_HASH_TABLE_SIZE-1);
      
      retval = r;
    }

  return retval;
}

int init_file_table(file_table **table)
{
  int retval = -1;

  if ( table )
    {
      *table = (file_table *) malloc(sizeof(file_table));
      if ( *table )
	if ( init_file_table_hash_buckets(*table) >= 0 )
	    retval = 1;
    }

  return retval;
}

int init_file_table_hash_buckets(file_table *table)
{
  int retval = -1;
  int i;

  if ( table )
    for ( i = 0, retval = 1; i < FILE_TABLE_HASH_TABLE_SIZE && retval >= 0; i++ )
      retval = init_one_file_table_hash_bucket(FILE_TABLE_HASH_BUCKET_PTR(*table, i));

  return retval;
}

int init_one_file_table_hash_bucket(file_table_hash_bucket *bucket)
{
  int retval = -1;

  if ( bucket )
    {
      FILE_TABLE_HASH_BUCKET_FILELIST(bucket) = 0;
      FILE_TABLE_HASH_BUCKET_SIZE(bucket)     = 0;
      FILE_TABLE_HASH_BUCKET_MAX_SIZE(bucket) = 0;
      retval = 1;
    }

  return retval;
}

int grow_file_table_hash_bucket(file_table_hash_bucket *bucket)
{
  int retval = -1;

  if ( bucket )
    {
      int old_size = FILE_TABLE_HASH_BUCKET_SIZE(bucket) * sizeof(void *);
      int new_size = old_size + ( FILE_TABLE_HASH_BUCKET_BLOCKSIZE * sizeof(void *));

      if ( FILE_TABLE_HASH_BUCKET_FILELIST(bucket) == 0 ) {
	FILE_TABLE_HASH_BUCKET_FILELIST(bucket) = malloc(new_size);
      }
      else 
	FILE_TABLE_HASH_BUCKET_FILELIST(bucket) = realloc(FILE_TABLE_HASH_BUCKET_FILELIST(bucket), new_size);

      if ( FILE_TABLE_HASH_BUCKET_FILELIST(bucket) )
	{
	  FILE_TABLE_HASH_BUCKET_MAX_SIZE(bucket) += FILE_TABLE_HASH_BUCKET_BLOCKSIZE;  
	  retval = 1;
	}
    }
  
  return retval;
}

int add_to_file_table(file_table *table, void *intrinfo)
{
  int retval = -1;

  if ( table && intrinfo )
    {
      int bucket_number = file_table_hash_string(get_filename_from_hash_bucket(intrinfo));
      if ( bucket_number >= 0 && bucket_number < FILE_TABLE_HASH_TABLE_SIZE )
	retval = add_to_file_table_hash_bucket(FILE_TABLE_HASH_BUCKET_PTR(*table, bucket_number), intrinfo);
    }
  
  return retval;
}

int add_to_file_table_hash_bucket(file_table_hash_bucket *bucket, void *intrinfo)
{
  int   retval = 0;

  if ( bucket && intrinfo )
    {
      /* check to see if we need to expand the bucket to make room for the new entry */
      if ( FILE_TABLE_HASH_BUCKET_SIZE(bucket) == FILE_TABLE_HASH_BUCKET_MAX_SIZE(bucket) )
	if ( grow_file_table_hash_bucket(bucket) < 0 )
	  retval = -1;

      /* check to see if the bucket is still valid (it could have been destroyed above, during a grow) */
      if ( retval >= 0 )
	{
	  int bucket_list_position = FILE_TABLE_HASH_BUCKET_SIZE(bucket);
	  FILE_TABLE_HASH_BUCKET_FILE(bucket, bucket_list_position) = intrinfo;
	  FILE_TABLE_HASH_BUCKET_SIZE(bucket)++;
	  retval = 1;
	}
    }
  
  return retval;
}

void* in_file_table(file_table *table, const char *filename)
{
  void *retval = NULL;

  if ( table && filename && *filename )
    {
      int bucket_number = file_table_hash_string(filename);
      if ( bucket_number >= 0 && bucket_number < FILE_TABLE_HASH_TABLE_SIZE )
	retval = in_file_table_hash_bucket(FILE_TABLE_HASH_BUCKET_PTR(*table, bucket_number), filename);
    }

  return retval;
}

void* in_file_table_hash_bucket(file_table_hash_bucket *bucket, const char *filename)
{
  void *retval = NULL;
  int i;

  if ( bucket && filename ) {
    void *info;
    for ( i = 0; i < FILE_TABLE_HASH_BUCKET_SIZE(bucket) && ! retval; i++ ) {
      info = FILE_TABLE_HASH_BUCKET_FILE(bucket, i);
      if ( info && strcmp(get_filename_from_hash_bucket(info), filename) == 0 ) {
	retval = info;
      }
    }
  }

  return retval;
}

int destroy_file_table(file_table **table)
{
  int retval = -1;

  if ( table && *table )
    {
      int i = 0;
      for ( i = 0; i < FILE_TABLE_HASH_TABLE_SIZE && retval >= 0; i++ )
	retval = destroy_file_table_hash_bucket(FILE_TABLE_HASH_BUCKET_PTR(**table, i));
      if ( retval >= 0 )
	free(*table);
    }
  
  return retval;
}

int destroy_file_table_hash_bucket(file_table_hash_bucket *bucket)
{
  int retval = -1;

  if ( bucket )
    {
      if ( FILE_TABLE_HASH_BUCKET_FILELIST(bucket) )
	free(FILE_TABLE_HASH_BUCKET_FILELIST(bucket));
      FILE_TABLE_HASH_BUCKET_SIZE(bucket)     = -1;
      FILE_TABLE_HASH_BUCKET_MAX_SIZE(bucket) = -1;
      retval = 1;
    }

  return retval;
}

int file_table_dump_hash_bucket(file_table_hash_bucket *bucket, FILE *outfile)
{
  int retval = -1;
  int i;
  void *info;

  if ( bucket && outfile )
    {
      fprintf(outfile, "\tBucket size: %d\n", FILE_TABLE_HASH_BUCKET_SIZE(bucket));
      fprintf(outfile, "\tBucket values:\n");
      for ( i = 0; i < FILE_TABLE_HASH_BUCKET_SIZE(bucket); i++ )
	info =  FILE_TABLE_HASH_BUCKET_FILE(bucket, i);
	fprintf(outfile, "\t\t%03d) %s\n", i, get_filename_from_hash_bucket(info));
      retval = 1;
    }

  return retval;
}

int file_table_dump_hash_table(file_table *table, FILE *outfile)
{
  int retval = -1;
  int i;

  if ( table && outfile )
    {
      retval = 0;
      for ( i = 0; i < FILE_TABLE_HASH_TABLE_SIZE && retval >= 0; i++ )
	{ 
	  fprintf(outfile, "Hash bucket #%d:\n", i);
	  retval = file_table_dump_hash_bucket(FILE_TABLE_HASH_BUCKET_PTR(*table, i), outfile);
	}
    }

  return retval;
}

#endif /* SOFTWARE_EMANCIPATION_PARSER */

