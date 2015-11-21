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
/* discrypt.c
 * 
 * Simple backwards and forwards encryption
 * scheme based on xor'ing.
 *
 * [abover, 04/27/97]
 *
 */

#include <stdio.h>
#include <string.h>

void usage(char *exe)
{
  fprintf(stderr, "Usage: %s <password>\n", exe);
}

int xor(char *k)
{
  int retval = 1;

  char *cp = k;
  int c, i, len, sum, mul, nli, seq, val;
  static const int mag = 0xbeef;

  if (k) {

    retval = 0;

    /* Gather characteristics about the key;
     * used later to spice up the xor'ing:
     */

    len = strlen(k);
    for (i = 0, sum = 0, mul = 1, nli = 1; i < len; i++, 
         sum += k[i],
         mul *= k[i] ? k[i] : k[i - 1],
         nli *= sum * mul)
          ;

    for (i = 0, seq = 0; (c = getc(stdin)) != EOF; ++cp, ++i) {

      if (!(*cp)) { cp = k; ++seq; }

      /* Heart of the encryption: */

      val = *cp + nli * (len ^ i ^ seq * sum ^ mul) % mag;

      c ^= val;

      putc(c, stdout);

    }

  }

  return retval;
}

main(int argc, char **argv)
{
  int retval = 1;

  if (argc != 2)
    usage(argv[0]);
  else
    retval = xor(argv[1]);

  return retval;
}
