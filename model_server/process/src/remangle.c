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
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef _WIN32
#include "../../../gcc/src/tree.h"
#else
#include "tree.h"
#endif
#if !defined( sun5 ) && !defined( _WIN32 ) && !defined( irix6 )
#define memcpy(s1, s2, n) bcopy ((s2), (s1), (n))
#define memcmp(s1, s2, n) bcmp ((s2), (s1), (n))
#define strchr index 
#endif

static int munge_function_name();
static void inv_prt();

char *remangle();

extern char *make_signature();
extern char *make_type();

extern char *psetmalloc ();

static struct {
   char *in;
   char *out;
} optable[] = {
  "new", "__nw",
  "delete", "__dl",
  "ne", "__ne",
  "eq", "__eq",
  "ge", "__ge",
  "gt", "__gt",
  "le", "__le",
  "lt", "__lt",
  "plus", "__pl",
  "minus", "__mi",
  "mult", "__ml",
  "convert", NULL,	/* unary + */ /* no analog ???? */
  "negate", NULL,  	/* unary - */ /* no analog ???? */
  "trunc_mod", "__md",    
  "trunc_div", "__dv",
  "truth_andif", "__aa",
  "truth_orif", "__oo",
  "truth_not", "__nt",
  "postincrement", "__pp",
  "postdecrement", "__mm",
  "bit_ior", "__or",
  "bit_xor", "__er",
  "bit_and", "__ad",
  "bit_not", "__co",
  "call", "__cl",
  "cond", NULL,                         /* no analog ???? */
  "alshift", "__ls",
  "arshift", "__rs",
  "component", "__rf",
  "indirect", "__ml",
  "method_call", NULL,                  /* no analog ???? */
  "addr", "__ad",	       
  "array", "__vc",
  "assign_nop", "__as",		     
  "assign_plus", "__apl",	
  "assign_mult", "__amu",
  "assign_trunc_mod", "__amd",
  "assign_arshift", "__ars",
  "assign_bit_ior", "__aor",	
  "assign_minus", "__ami",	
  "assign_trunc_div", "__adv",		  
  "assign_alshift", "__als",		  
  "assign_bit_and", "__aad",		  
  "assign_bit_xor", "__aer",		  
};

/**********************************************************************
**********************************************************************/

char *remangle (fndecl)
     tree fndecl;
{
#ifndef GCC2
  char *type = IDENTIFIER_POINTER (DECL_NAME (fndecl));
#else
  char *type = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (fndecl));
#endif

  int n;
  int success = 0;
  int constructor = 0;
  int const_flag = 0;
  int i;
  char *p;
  char *p_old;
  char my_buf[1024];
  char mini_buf[100];
  char *tem;
  char *name;
  char *f_signature;

  if (type == NULL || *type == '\0')
       return NULL;

  p = type;
  while (*p != '\0' && !(*p == '_' && p[1] == '_'))
    p++;
  if (*p == '\0')
    {
      /* destructor */
      if (type[0] == '_' && type[1] == '$' && type[2] == '_')
	{
	  sprintf(my_buf, "_dt__%d%sFv\0", strlen(type + 3), type + 3);
	  tem = (char *) psetmalloc(strlen(my_buf) + 1);
	  strcpy(tem, my_buf);
	  return tem;
	}
      /* static data member */
      if (*type != '_' && (p = strchr(type, '$')) != NULL)
	{
	  memcpy(mini_buf, type, p - type);
	  mini_buf[p - type] = 0;
	  sprintf(my_buf, "%s__%d%s\0", p + 1, p - type, mini_buf);
	  tem = (char *) psetmalloc(strlen(my_buf) + 1);
	  strcpy(tem, my_buf);
	  return tem;
	}
      /* virtual table */
      if (type[0] == '_' && type[1] == 'v' && type[2] == 't' && type[3] == '$')
	{
	  sprintf(my_buf, "__vtbl\0");
	  inv_prt(my_buf + 6, type + 4);
	  tem = (char *) psetmalloc(strlen(my_buf) + 1);
	  strcpy(tem, my_buf);
	  return tem;
	}
      return NULL;
    }

  if (p == type)
    {
      if (!isdigit (p[2]))
	{
	  return NULL;
	}
      /* constructor */
      strcpy (my_buf, "__ct__");
    }
  else
    {
      if (munge_function_name (fndecl, type, p, my_buf))
	return NULL;
    }
  p += 2;

  switch (*p)
    {
    case 'C':
      /* const member function */
      if (!isdigit (p[1]))
	return NULL;
      p++;
              /*  fall through  */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      /*  read size of classname  */
      n = 0;
      p_old = p;
      do
	{
	  n *= 10;
	  n += *p - '0';
	  p++;
	}
      while (isdigit (*p));
      if (strlen (p) < n)
	  return NULL;
      /*  write classname  with size   */
      memcpy (mini_buf, p_old, (p - p_old) + n);
      mini_buf[(p - p_old) + n] = '\0';
      strcat (my_buf, mini_buf);

               /*  fall through  */ 
    case 'F':
      /* all functions    */
      break;

    default:
      return NULL;
    }

  /* now signature of function */
  if (f_signature = make_signature (fndecl))
    {
      strcat (my_buf, f_signature);
      free (f_signature);
      tem = (char *) psetmalloc( strlen(my_buf) +1);
      strcpy (tem, my_buf);
      return tem;
    }
  return NULL;
}

/***********************************************************************
***********************************************************************/

static void inv_prt(to, from)
  char *to;
  char *from;
  {
    char *tmp;
    if (tmp = strchr(from, '$'))
      {
	*tmp = 0;
	inv_prt(to, tmp + 1);
        sprintf(to + strlen(to), "__%d%s\0", strlen(from), from);
        *tmp = '$';
      }
    else
      {
	sprintf(to + strlen(to), "__%d%s\0", strlen(from), from);
      }
    return;
  }

/************************************************************************
************************************************************************/

static int munge_function_name (fndecl, type, p, my_buf)
  tree fndecl;
  char *type;
  char *p;
  char *my_buf;
{
  /* operator overloading */
  if (type[0] == 'o' && type[1] == 'p' && type[2] == '$')
    {
      int len = (p - type) - 3;
      int i;
      for (i = 0; i < sizeof (optable)/sizeof (optable[0]); i++)
        {
	  if (strlen (optable[i].in) == len 
	      &&
	      memcmp (optable[i].in, type + 3, len) == 0)
	    {
	      if (optable[i].out == 0) return NULL;
	      strcpy (my_buf, optable[i].out);
	      strcat (my_buf, "__");
	      return 0;
       	    }
	}
      return -1;
    }
  else if (p -type >= 5 && memcmp (type, "type$", 5) == 0)
    {
      /* type conversion operator */
      char *op_type;
      strcpy (my_buf, "__op");
      if (op_type = make_type (fndecl))
	{
	  strcat (my_buf, op_type);
	  strcat (my_buf, "__");
	  free (op_type);
	  return 0;
	}
      else
	return -1;
    }
  else
    {
      /* other functions   */
      memcpy (my_buf, type, p - type);
      my_buf[p - type] = '\0';
      strcat (my_buf, "__");
      return 0;
    }
}
















