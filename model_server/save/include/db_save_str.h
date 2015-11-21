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
#include "general.h"
#include <sys/types.h>
#ifndef _WIN32
#include <netinet/in.h>
#else
#include <winsock.h>
#endif
#include <machdep.h>

#ifdef DECLARE_STRUCT
#undef DECLARE_STRUCT
#endif
#ifdef DECLARE_FIELD_INT
#undef DECLARE_FIELD_INT
#endif
#ifdef DECLARE_FIELD_CHARARR
#undef DECLARE_FIELD_CHARARR
#endif
#ifdef DECLARE_FIELD_BITS
#undef DECLARE_FIELD_BITS
#endif
#ifdef DECLARE_END_BITFIELD
#undef DECLARE_END_BITFIELD
#endif
#ifdef DECLARE_FIELD_LONG
#undef DECLARE_FIELD_LONG
#endif
#ifdef DECLARE_FIELD_UNSIGNED
#undef DECLARE_FIELD_UNSIGNED
#endif
#ifdef DECLARE_FIELD_BYTE
#undef DECLARE_FIELD_BYTE
#endif
#ifdef DECLARE_FIELD_USHORT
#undef DECLARE_FIELD_USHORT
#endif
#ifdef DECLARE_END_STRUCT
#undef DECLARE_END_STRUCT
#endif
#ifdef DECLARE_FIELD_USHORTBITS
#undef DECLARE_FIELD_USHORTBITS
#endif 

#define DECLARE_STRUCT(name) inline void paste3(struct_,name,_save) (void *x, char *buf){ \
                             struct name *str = (struct name *)x;\
	                     int pos          = 0;\
			     unsigned b       = 0;\
			     int bits_left    = 32;
#define DECLARE_FIELD_CHARARR(name,size) struct_save_char_arr(str->name, size, buf, pos);
#define DECLARE_FIELD_INT(name) struct_save_int(str->name, buf, pos);
#define DECLARE_FIELD_BITS(name,size) struct_save_bits(str->name, size, b, bits_left, buf, pos);
#define DECLARE_FIELD_USHORTBITS(name,size) DECLARE_FIELD_BITS(name,size)
#define DECLARE_END_BITFIELD struct_flush_bits(b, bits_left, buf, pos);
#define DECLARE_FIELD_LONG(name) struct_save_long(str->name, buf, pos);
#define DECLARE_FIELD_UNSIGNED(name) struct_save_unsigned(str->name, buf, pos);
#define DECLARE_FIELD_BYTE(name) struct_save_unsigned_char((unsigned char)str->name, buf, pos);
#define DECLARE_FIELD_USHORT(name) struct_save_unsigned_short(str->name, buf, pos);
#define DECLARE_END_STRUCT };
#define STRUCT_SAVE_FUNCTION(name) paste3(struct_,name,_save)

#define STRUCT_SAVE_PARAMS(name) STRUCT_SAVE_FUNCTION(name), sizeof(SAVEABLE_STRUCT_NAME(name))

typedef void (*str_handler)(void *s, char *buf);

class StructSaver {
  public:
    StructSaver(void *str, str_handler save_func, int buffer_size){
	buffer    = new char[buffer_size + 1];
	do_delete = 1;
	save_func(str, buffer);
    }
    StructSaver(void *str, str_handler save_func, int, char *buf){
	buffer    = buf;
	do_delete = 0;
	save_func(str, buffer);
    }
    ~StructSaver() { if(do_delete)
			 delete [] buffer; 
		 };

    char *get_buffer() { return buffer; };
  private:
    char *buffer;
    int  do_delete;
};

#if 0
void struct_save_int(int, char *buf, int& pos);
void struct_save_char_arr(char *, int, char *buf, int& pos);
void struct_save_long(long, char *buf, int& pos);
void struct_save_bits(unsigned, int size, unsigned &b, int &bits_left, char *buf, int& pos);
void struct_save_char(char, char *buf, int& pos);
void struct_save_unsigned_char(unsigned char, char *buf, int& pos);
void struct_save_unsigned(unsigned, char *buf, int& pos);
void struct_save_unsigned_short(unsigned short, char *buf, int& pos);
void struct_save_short(short, char *buf, int& pos);
void struct_flush_bits(unsigned &b, int &bits_left, char *buf, int& pos);

#else
inline void struct_save_char_arr(char *arr, int length, char *buf, int& pos)
{
    OSapi_bcopy(arr, (buf + pos), length);
    pos += length;
}

inline void struct_save_int(int i, char *buf, int& pos)
{
    int tmp             = htonl(i);
    *(int *)(buf + pos) = tmp;
    pos                += sizeof(int);
}

inline void struct_save_long(long l, char *buf, int& pos)
{
    long tmp             = htonl(l);
    *(long *)(buf + pos) = tmp;
    pos                 += sizeof(long);
}

inline void struct_save_unsigned(unsigned u, char *buf, int& pos)
{
    unsigned tmp         = htonl(u);
    *(long *)(buf + pos) = tmp;
    pos                 += sizeof(unsigned);
}

inline void struct_save_unsigned_short(unsigned short x, char *buf, int& pos)
{
    unsigned short  tmp            = htons(x);
    *(unsigned short *)(buf + pos) = tmp;
    pos                           += sizeof(unsigned short);
}

inline void struct_save_unsigned_char(unsigned char c, char *buf, int& pos)
{
    *(unsigned char *)(buf + pos) = c;
    pos++;
}

inline void write_8bits(unsigned result, char *buf, int &pos)
{
    union {
	char     store[4];
	unsigned value;
    } val;
    
    // This is alignement correct function
    val.value = htonl(result);
    char *p   = (buf + pos);
    *p        = val.store[0];
    pos++;
}

inline void write_16bits(unsigned result, char *buf, int &pos)
{
    union {
	char     store[4];
	unsigned value;
    } val;
    
    // This is alignement correct function
    val.value = htonl(result);
    char *p   = (buf + pos);
    *p++      = val.store[0];
    *p        = val.store[1];
    pos      += 2;
}

inline void write_24bits(unsigned result, char *buf, int &pos)
{
    union {
	char     store[4];
	unsigned value;
    } val;
    
    // This is alignement correct function
    val.value = htonl(result);
    char *p   = (buf + pos);
    *p++      = val.store[0];
    *p++      = val.store[1];
    *p        = val.store[2];
    pos      += 3;
}

inline void write_bits(unsigned result, char *buf, int &pos)
{
    union {
	char     store[4];
	unsigned value;
    } val;
    
    // This is alignement correct function
    val.value = htonl(result);
    char *p   = (buf + pos);
    *p++      = val.store[0];
    *p++      = val.store[1];
    *p++      = val.store[2];
    *p        = val.store[3];
    pos      += 4;
}

inline void struct_flush_bits(unsigned &b, int &bits_left, char *buf, int& pos)
{
    if(bits_left >= 24)
	write_8bits(b, buf, pos);
    else if(bits_left >= 16)
	write_16bits(b, buf, pos);
    else if(bits_left >= 8)
	write_24bits(b, buf, pos);
    else
	write_bits(b, buf, pos);
    bits_left = 32;
}

inline void struct_save_bits(unsigned val, int length, unsigned& bits_val, 
			     int &bits_left, char *buf, int &pos)
{
    if(length > bits_left){
	write_bits(bits_val, buf, pos);
	bits_val  = 0;
	bits_left = 32;
    }
    bits_left   -= length;
    unsigned res = val << bits_left;
    bits_val    |= res;
}

#endif
