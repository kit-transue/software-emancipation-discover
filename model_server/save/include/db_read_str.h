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

#define DECLARE_STRUCT(name) inline void paste3(struct_,name,_read) (void *x, char *buf){ \
                             struct name *str = (struct name *)x;\
	                     int pos          = 0;\
			     unsigned b       = 0;\
			     int bits_left    = 32;
#define DECLARE_FIELD_CHARARR(name,size) struct_read_char_arr(str->name, size, buf, pos);
#define DECLARE_FIELD_INT(name) str->name = struct_read_int(buf, pos);
#define DECLARE_FIELD_BITS(name,size) str->name = struct_read_bits(size, bits_left, buf, pos);
#define DECLARE_FIELD_USHORTBITS(name,size) DECLARE_FIELD_BITS(name,size)
#define DECLARE_END_BITFIELD struct_flush_read_bits(bits_left, pos);
#define DECLARE_FIELD_LONG(name) str->name = struct_read_long(buf, pos);
#define DECLARE_FIELD_UNSIGNED(name) str->name = struct_read_unsigned(buf, pos);
#define DECLARE_FIELD_BYTE(name) str->name = struct_read_unsigned_char(buf, pos);
#define DECLARE_FIELD_USHORT(name) str->name = struct_read_unsigned_short(buf, pos);
#define DECLARE_END_STRUCT };
#define STRUCT_READ_FUNCTION(name) paste3(struct_,name,_read)

#if 0

int            struct_read_int(char *buf, int& pos);
void           struct_read_char_arr(char *, int, char *buf, int& pos);
long           struct_read_long(char *buf, int& pos);
unsigned       struct_read_bits(int size, int &bits_left, char *buf, int& pos);
char           struct_read_char(char *buf, int& pos);
unsigned char  struct_read_unsigned_char(char *buf, int& pos);
unsigned       struct_read_unsigned(char *buf, int& pos);
unsigned short struct_read_unsigned_short(char *buf, int& pos);
short          struct_read_short(char *buf, int& pos);
void           struct_flush_read_bits(int& bits_left, int& pos);

#else

inline void struct_read_char_arr(char *arr, int length, char *buf, int& pos)
{
    OSapi_bcopy((buf + pos), arr, length);
    pos += length;
}

inline int struct_read_int(char *buf, int& pos)
{
    int tmp = *(int *)(buf + pos);
    pos    += sizeof(int);
    return((int)ntohl(tmp));
}

inline long struct_read_long(char *buf, int& pos)
{
    long tmp = *(long *)(buf + pos);
    pos     += sizeof(long);
    return((long)ntohl(tmp));
}

inline unsigned struct_read_unsigned(char *buf, int& pos)
{
    unsigned tmp = *(unsigned *)(buf + pos);
    pos         += sizeof(unsigned);
    return((unsigned)ntohl(tmp));
}

inline unsigned short struct_read_unsigned_short(char *buf, int& pos)
{
    unsigned short tmp = *(unsigned short *)(buf + pos);
    pos               += sizeof(unsigned short);
    return((unsigned short)ntohs(tmp));
}

inline unsigned char struct_read_unsigned_char(char *buf, int& pos)
{
    unsigned char res = *(unsigned char *)(buf + pos);
    pos++;
    return res;
}

inline void struct_flush_read_bits(int &bits_left, int& pos)
{
    if(bits_left >= 24)
	pos++;
    else if(bits_left >= 16)
	pos += 2;
    else if(bits_left >= 8)
	pos += 3;
    else 
	pos += 4;
    bits_left = 32;
}

inline unsigned struct_read_bits(int length,
				 int &bits_left, char *buf, int &pos)
{
    union {
	char     store[4];
	unsigned value;
    } mem_val;

    if(length > bits_left){
	pos      += sizeof(unsigned);
	bits_left = 32;
    }
    int to_read  = 4 - ((bits_left - length) >> 3);
    char *p      = buf + pos;
//    int  i       = 0;
//    while(to_read--)
//	mem_val.store[i++] = *p++;
    mem_val.store[0] = *p++;
    if(to_read > 1)
	mem_val.store[1] = *p++;
    if(to_read > 2)
	mem_val.store[2] = *p++;
    if(to_read > 3)
	mem_val.store[3] = *p;
    bits_left     -= length;
    unsigned res   = (ntohl(mem_val.value)) >> bits_left;
    res           &= (1 << length) - 1;
    return res;
}
#endif

typedef void (*str_handler)(void *s, char *buf);

class StructReader {
  public:
    static void Read(void *in_buffer, void *out_buffer, str_handler read_func){
	read_func(out_buffer, (char *)in_buffer);
    }
};

