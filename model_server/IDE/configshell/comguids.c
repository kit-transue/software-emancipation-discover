/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.00.15 */
/* at Tue Jul 08 18:10:20 1997
 */
/* Compiler settings for comserver.idl:
    Os, W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef IID_DEFINED
#define IID_DEFINED

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // IID_DEFINED

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IConfigShell = {0x2786c390,0xf18a,0x11d0,{0x96,0x99,0x00,0xa0,0x24,0x58,0x36,0xd0}};


const IID LIBID_ServerLib = {0x2786c391,0xf18a,0x11d0,{0x96,0x99,0x00,0xa0,0x24,0x58,0x36,0xd0}};


const CLSID CLSID_ConfigShell = {0x2786c392,0xf18a,0x11d0,{0x96,0x99,0x00,0xa0,0x24,0x58,0x36,0xd0}};


#ifdef __cplusplus
}
#endif

