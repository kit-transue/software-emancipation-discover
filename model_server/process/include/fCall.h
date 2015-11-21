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
typedef enum {
    TBYTE,
    TSHORT,
    TLONG,
    TPTR,
    TFLOAT,
    TDOUBLE,
    TSTRUCT,
    TFNDECL
} DataType;

typedef struct {
    void *fn;
    va_list ap;
} ArgStruct;

#ifdef _STAND_ALONE
/*  NEW ROUTING */
#define __MAX_SR_HEADER              0xff
#define __ADDR_PACKET_SIZE           0x80
#define __MAX_FUNCTIONS_TO_INTERPRET 0x3fffff
#define __MAX_LAYOUT 6

#define __is_GDB(operation) ((__getFromStatusRegister(__operationInProgress)-operation)==0)

#define __is_GDB_STEP() __is_GDB(__GDB_STEP)
#define __is_GDB_NEXT() __is_GDB(__GDB_NEXT)
#define __is_GDB_FINISH() __is_GDB(__GDB_FINISH)
#define __is_GDB_STEPorNEXTorFINISH() (__getFromStatusRegister(__operationInProgress) < 4)

#define __GDB_SET_SO_CHANGED __gdb_statusRegister |= 0x40
#define __GDB_INTERP_COMMAND __gdb_statusRegister & 0x8

typedef struct {
    int code[3];
} __functionPatch__;

typedef struct {
    long ast_addr;
    long source_addr; 
    void* ast_segm;   /* for multiple ast */
    __functionPatch__ source_code;
} __routedFunctions__;

enum __gdb_Operation {
    __gdb_Operation__ = 0,
    __GDB_STEP, 
    __GDB_NEXT,
    __GDB_FINISH,
    __GDB_DONOTCARE
};

enum __statusRegisterLayout {
    __lastStatementInCompiled = 0,
    __stepInProgress,
    __finishInProgress,
    __commandFromINTERP,
    __operationInProgress,
    __registerHeader
};

extern __routedFunctions__ * __functionAddrArray__ ;
extern int __interpretedFunction;
extern int __gdb_statusRegister;
extern int __dbg_registerHeader;
extern __functionPatch__ __functionPatchInstance__;

#ifdef _fCall_h_CC
extern "C" int  __getFuncAddrIndex(int, int);
extern "C" int  __resizeFunctionAddrArray(int);
extern "C" int  __getFromARegister(int, int);
extern "C" int  __getFromStatusRegister(int);
extern "C" void __setARegister(int *, int, int);
extern "C" void __setStatusRegister(int, int);
extern "C" void __new_gdb_statusRegister(int, int);
#endif
#endif /* _STAND_ALONE */

enum userCompiler {
  GNU_C = 0,
  GNU_CC,
  ATT_C, 
  ATT_CC,
  SOLARIS_C,
  SOLARIS_CC
};

#ifdef MIXED_RETURN_DEFS

#define ROUTER_TYPE void *
#define C_STRUCT_GET_RET_INFO exec_get_return_type_size
	      
#define HANDLE_C_STRUCT_return \
            { \
	      memcpy((char*)ret_struct,(char*)ret_ptr,size); \
              asm("add %i7,4,%i7\n"); \
	    }
#define HANDLE_C_STRUCT_GNU_return \
            { \
	      ret_ptr = ret_struct; \
              asm("add %i7,4,%i7\n"); \
            }

/* -- main calling "routine" for data types for INTERP --> COMPILED -- */

#define GET_RET_STRUCT (((void**)ap)[-1])

#define HANDLE_RETURN_FROM_I2C \
            { \
	      int size = 0; \
	      int compiler_type = 0; \
	      DataType return_type; \
	      compiler_type = C_STRUCT_GET_RET_INFO(&args,&return_type,&size); \
	      swap_segments(oldms); \
	      if (return_type == TSTRUCT) { \
		  switch (compiler_type) { \
		    case GNU_CC: \
		    case GNU_C:  HANDLE_C_STRUCT_GNU_return; break; \
		    case ATT_CC : \
		    case ATT_C : HANDLE_C_STRUCT_return; break; \
		    case SOLARIS_CC : \
		    case SOLARIS_C : HANDLE_C_STRUCT_return; break; \
		    default : HANDLE_C_STRUCT_return; break; \
		  } \
	      } else if (return_type == TDOUBLE) { \
		  ret_double(*(double*)ret_ptr); \
	      } else if (return_type == TFLOAT) { \
		  ret_float(*(float*)ret_ptr); \
              } \
	      return ret_ptr; \
            }

extern int C_STRUCT_GET_RET_INFO();

#endif

/*
   START-LOG-------------------------------------------
 
   $Log: fCall.h  $
   Revision 1.12 2000/07/07 08:12:24EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.8  1994/07/19  11:25:56  aharlap
 * multiple ast
 *
 * Revision 1.2.1.7  1994/07/09  00:16:52  aharlap
 * multiple ast
 *
 * Revision 1.2.1.6  1994/04/21  13:15:42  pero
 * support for newly added TFLOAT return value
 *
 * Revision 1.2.1.5  1994/04/07  20:49:07  pero
 * support for return values other than ints and ptrs (double, structure)
 *
 
   END-LOG---------------------------------------------
 
*/
