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
met_reader.C  
Ofer Mazor

NOTE: This is a stand-alone tool.  
      It is not part of DISCOVER or any executable.
      It is for IN HOUSE USE ONLY, and does not need to be 
      installed in the release area.

  This executable accepts the name of a *.pmod.met file as an argument.
It then prints out the contants of that file in text format.
The tree structure of the file is represented by the indenting of the lines.
Each line corresponds to one MLink (4 bytes) in the file.
Each line shows the type of the MLink and its value.

This file compiled for me on sun5. 
(I don't know how well it works on other platforms)


ALSO:
Make sure the information in this file corresponds with the enumerated type 
"metricType" in smt/include/metric.h.

*/


#include <stdio.h>

main(int argc, char** argv) 
{
  int tabs[100];
  for (int i=0; i<100; i++)
    tabs[i] = 0;
  int curtab = 0;
  
  FILE * file = NULL;
  unsigned long val[1];
  file = fopen(argv[1], "rb");
  //rewind(file);
  if (!file) printf ("No File\n");
  fseek(file, 0, SEEK_SET);
  while (fread (val,4,1,file)) {

    for (int i=0; i<curtab; i++) printf(" ");

    unsigned long block = val[0] & 0x01000000;
    if (block != 0) {
      unsigned long l = val[0] & 0x00ffffff;
      tabs[curtab] = l;
      curtab++;
    }

    unsigned long j = val[0] >> 25;
    switch (j) {
    case 0:
      printf("MET_normal_CC");
      break;
    case 1:
      printf("MET_normal_LOC");
      break;
    case 2:
      printf("MET_normal_MYERS");
      break;
    case 3:
      printf("MET_normal_FANIN");
      break;
    case 4:
      printf("MET_normal_FANOUT");
      break;
    case 5:
      printf("MET_normal_BYTELEN");
      break;
    case 6:
      printf("MET_normal_DEPTH");
      break;
    case 7:
      printf("MET_normal_DANG_EI");
      break;
    case 8:
      printf("MET_normal_DANG_SW");
      break;
    case 9:
      printf("MET_normal_STRT_LINE");
      break;
    case 10:
      printf("MET_normal_RETURNS");
      break;
    case 11:
      printf("MET_normal_ULOC");
      break;
    case 12:
      printf("MET_normal_XLOC");
      break;
    case 13:
      printf("MET_normal_IFS");
      break;
    case 14:
      printf("MET_normal_SWITCHES");
      break;
    case 15:
      printf("MET_normal_LOOPS");
      break;
    case 16:
      printf("MET_normal_DEFAULTS");
      break;

    case 111:
      printf("MET_LOCAL_DEC_BLOCK");
      break;
    case 112:
      printf("MET_UNDEFINED_ELEMENT");
      break;
    case 113:
      printf("MET_INSTNACE_LENGTH");
      break;
    case 114:
      printf("MET_INSTANCE_SYMBOL");
      break;
    case 115:
      printf("MET_INSTANCE_SCOPE");
      break;
    case 116:
      printf("MET_INSTANCE_LN_OFFSET");
      break;
    case 117:
      printf("MET_INSTANCE_LINE");
      break;
    case 118:
      printf("MET_INSTANCE_OFFSET");
      break;
    case 119:
      printf("MET_INSTANCE");
      break;
    case 120:
      printf("MET_INSTANCE_BLOCK");
      break;
    case 121:
      printf("MET_MET_lmdhigh");
      break;
    case 122:
      printf("MET_MET_lmd");
      break;
    case 123:
      printf("MET_CHILDREN");
      break;
    case 124:
      printf("MET_MOVED");
      break;
    case 125:
      printf("MET_ELEMENT");
      break;
    case 126:
      printf("MET_OBJECT");
      break;
    case 127:
      printf("MET_FREE");
      break;
    default:
      printf("%d", j);
      break;
    }
    unsigned long l = val[0] & 0x00ffffff;
    //unsigned long l = val[0] << 8;
    //l = l >> 8;
    printf(" : %d\n", l);
    //fseek(file, 4, SEEK_CUR);

    for (int g=0;g<curtab;g++)
      tabs[g]--;
    while (tabs[curtab-1] <= 0)
      curtab--;

  }
}

