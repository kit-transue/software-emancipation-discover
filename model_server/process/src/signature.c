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
#include <string.h>
#ifdef _WIN32
#include "../../../gcc/src/tree.h"
#include "../../../gcc/src/cp/cp-tree.h"
#else
#include "tree.h"
#include "cp/cp-tree.h"
#endif

extern void *psetmalloc();

extern char *make_signature();
extern char *make_type();

static char *signature ();

int constructor_with_in_charge;

char *make_signature (fndecl)
     tree fndecl;
{
  char *f_name; 
  if (!fndecl)
    return NULL;
#ifndef GCC2
  f_name = IDENTIFIER_POINTER (DECL_NAME (fndecl));
#else
  f_name = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (fndecl));
#endif
  if (!DECL_LANG_SPECIFIC (fndecl)) 
    return NULL;
  switch (DECL_LANGUAGE (fndecl))
    {
      case lang_c:    
	return NULL;
      case lang_cplusplus:
      default:	
	break;
    }
  {
    char buf[1024];
    char *bb;
    char *p;

    if (DECL_CONSTRUCTOR_FOR_VBASE_P (fndecl))
    constructor_with_in_charge = 1;

    bb = signature (TREE_TYPE (fndecl), buf);
    if (!bb)
      return NULL;
    if (DECL_STATIC_FUNCTION_P (fndecl))
      {
	p = (char *) psetmalloc ((bb - buf) + 2);
	*p = 'S';
	*(p + 1) = '\0';
	strcat (p, buf);
      }
    else
      {
	p = (char *) psetmalloc ((bb - buf) + 1);
	strcpy (p, buf);
      }
    return p;
  }
}

char *make_type (fndecl)
  tree fndecl;
{
  char* f_name;
  constructor_with_in_charge = 0;
  if (!fndecl)
    return NULL;
  if (!DECL_LANG_SPECIFIC (fndecl))
    return 0;
  switch (DECL_LANGUAGE (fndecl))
    {
      case lang_c:    
	return NULL;
      case lang_cplusplus:
      default:	
	break;
    }
  {
    char buf[1024];
    char *bb;
    char *p;

    bb = signature (TREE_TYPE (TREE_TYPE (fndecl)), buf);
    if (!bb)
      return NULL;
    p = (char *) psetmalloc ((bb - buf) + 1);
    strcpy (p, buf);
    return p;
  }
}

static char *signature(parmtype, p)
  tree parmtype;
  char *p;
/*      
 *	take a signature suitable for argument types for overloaded
 *	function names
 */
{
  tree t = parmtype;
  int pp = 0;	       /* counter of iterations */

xx:

/*   
 *   first unroll typedefs and handle derived types:
 */

  switch (TREE_CODE(t))
    {
/* ????
 *    case TYPE:
 *	if (TREE_READONLY(t))
 *	  *p++ = 'C';
 *	t = TREE_TYPE(t);
 *	goto xx;
 */
      case ARRAY_TYPE:                  /*  A<size>_  */
        {   
	  int len;
	  tree index_type = TYPE_DOMAIN (t);
	  tree min_val = TYPE_MIN_VALUE (index_type);
	  tree max_val = TYPE_MAX_VALUE (index_type);
	  
	  if (pp
	      &&
	      TREE_CODE(TREE_TYPE(t)) != ARRAY_TYPE
	      &&
	      TREE_CODE(min_val) == INTEGER_CST
	      &&
	      TREE_CODE(max_val) == INTEGER_CST
	      &&
	      (len = TREE_INT_CST_LOW (max_val) - TREE_INT_CST_LOW (min_val)))
	    {
	      *p++ = 'A';
	      sprintf(p,"%d\0", len + 1);      /* don't trust             */
		                               /* sprintf return value    */
	      while (*++p);
	      *p++ = '_';
	    }
          else
	    *p++ = 'P';
	  t = TREE_TYPE(t);
	  pp = 1;
	  goto xx;
        }

      case POINTER_TYPE:
	if (TREE_READONLY(t))
	  *p++ = 'C';			               /*  *const            */
	if (TREE_CODE(TREE_TYPE(t)) == OFFSET_TYPE)    /* M<size><classname> */
          {
	    tree cl;
	    tree name;
	    int d;
	    char *s;
	    t = TREE_TYPE (t);
	    cl = TYPE_OFFSET_BASETYPE (t);
	    *p++ = 'M';
	    name = TYPE_NAME (cl);
	    if (TREE_CODE (name) == TYPE_DECL)
	      name = DECL_NAME (name);
	    if (TREE_CODE (name) != IDENTIFIER_NODE)
	      return NULL;
	    d = IDENTIFIER_LENGTH (name);
	    s = IDENTIFIER_POINTER (name);
	    if (d/10)
	      *p++ = '0' + d / 10;
	    *p++ = '0' + d % 10;             /*  assume  < 100 char  */
	    while (*p++ = *s++);
	    --p;                             /*  not the '\0'        */
          }
	else
	  *p++ = 'P';
	t = TREE_TYPE(t);
	pp = 1;
	goto xx;

      case REFERENCE_TYPE:
	*p++ = 'R';
	t = TREE_TYPE(t);
	pp = 1;
	goto xx;

      case FUNCTION_TYPE:
      case METHOD_TYPE:
	{
          tree f = t;
	  tree n = TYPE_ARG_TYPES(t); 

	  if (TREE_CODE(t) == METHOD_TYPE)
	    {
	      if (TREE_READONLY (TREE_TYPE (TREE_VALUE (n))))
		*p++ = 'C';	      /* constant member function  */
/*		if (f->f_static)			*p++ = 'S';	// static member function */
	      n = TREE_CHAIN (n);     /* skip "this" argument        */
	      if (constructor_with_in_charge)
		n = TREE_CHAIN (n);   /* skip "__in$charge" argument */
	    }
	  *p++ = 'F';

	  if (!n || n == void_list_node)
	    *p++ = 'v';	      /* VOID, that is f() == f(void) */
	                                       /* foo(...) encoded as */
	                                       /* _foo__Fve  ???      */
                                               /* but any way ...     */
	  else
	    for ( ; n != 0 && (n != void_list_node); n = TREE_CHAIN(n))
	      { 	                 /* 
                                          *  print argument encoding
		                          *  check if argtype is the same
		                          *  as previously seen argtype
                                          */
		 int i = 0;
		 tree nn;
		 for (nn = TYPE_ARG_TYPES(t); n != nn; nn = TREE_CHAIN(nn))
		   {
		     i++;
		     if (TREE_VALUE(nn) == TREE_VALUE(n)
	             /*  ||                         */
		     /*	 nn->tp->check(n->tp,0)==0  */ )
	               { 
				       	/* typeof (n) == typeof(arg i) */
		         int x = 1;     /* try for a run after n       */
			 tree nnn = n;
			 while ((nnn = TREE_CHAIN(nnn))
				 &&
				 nnn != void_list_node
				 &&
				 x < 9)
			   {
			     if (TREE_VALUE (nnn) == TREE_VALUE (n)
			/*	 ||                           */
		        /*     	 nnn->tp->check(n->tp,0)== 0  */ )
			       {
				 x++;
				 n = nnn;
			       }
			     else
			       break;
                           }		
			 if (x == 1)		/*  Ti  */
			   *p++ = 'T';
			 else
			   {			/*  Nxi */
			     *p++ = 'N';
			     *p++ = '0'+x;
			   }
			
						/*  assume < 100 arguments  */
			 if (9 < i)
			   *p++ = '0'+i/10;
			 *p++ = '0'+i%10;
		         goto zk;
		       }
		   }
		
		 /* ``normal'' case print argument type signature */
		 p = signature(TREE_VALUE(n), p);
		 if (!p)
		   return NULL;
		 zk:;
	       }
	  if (!n)
	    *p++ = 'e'; 
                        
	  if (pp)
	    {	         	                /*  '_' result type  */
	      *p++ = '_';
	      p = signature(TREE_TYPE (f) ,p);
	      if (!p)
		return NULL;
	    }

	  *p = 0;
	  return p;
	}
      }
/*
 *     base type modifiers:
 */

  if (TREE_READONLY(t))
    *p++ = 'C';

/*
 *	if ( Pbase(t)->b_signed )	*p++ = 'S';   AT&T
 *
 *	if ( Pbase(t)->b_volatile )	*p++ = 'V';   AT&T 
 */

/*
 *    now base types:
 */

  switch (TREE_CODE(t))
    {
      case VOID_TYPE:
	*p++ ='v';
        break;

      case INTEGER_TYPE:
	if (TREE_UNSIGNED(t))
	  *p++ = 'U'; 
        t = TYPE_MAIN_VARIANT (t);
        switch (TYPE_MODE(t))
	  {
            case TImode:
	      if (t == long_integer_type_node
	          ||
                  t == long_unsigned_type_node)
	        *p++ = 'l';
	      else
	        return NULL;
	      break;

	    case DImode:
	      if (t == long_integer_type_node
		  || 
		  t == long_unsigned_type_node)
	        *p++ = 'l';
	      else if (t == integer_type_node
		       ||
		       t == unsigned_type_node)
	        *p++ = 'i';
	      else if (t == short_integer_type_node
		       ||
		       t == short_unsigned_type_node)
	        *p++ = 's';
	      else 
	        return NULL;
	      break;

	    case SImode:
	      if (t == long_integer_type_node
		  ||
		  t == long_unsigned_type_node)
	        *p++ = 'l';
	      else if (t == short_integer_type_node
		       ||
		       t == short_unsigned_type_node)
	        *p++ = 's';
	      else
	        *p++ = 'i';
	      break;

	    case HImode:
	      if (t == integer_type_node
		  ||
		  t == unsigned_type_node)
	        *p++ = 'i';
	      else
	        *p++ = 's';
	      break;

	    case QImode:
	      *p++ = 'c';
	      break;

	    default:
	      return NULL;
	  }	  
        break;
	      	    
      case REAL_TYPE:
        t = TYPE_MAIN_VARIANT(t);
        if (t == long_double_type_node)
          *p++ = 'r';
        else if (t == double_type_node)
          *p++ = 'd';
        else if (t == float_type_node)
          *p++ = 'f';
        else
          return NULL;
        break;

      case ENUMERAL_TYPE:
      case UNION_TYPE:
      case RECORD_TYPE:
        {
          int d;
          char *s;
          tree name = TYPE_NAME(t);
          if (TREE_CODE(name) == TYPE_DECL)
            name = DECL_NAME(name);
          if (TREE_CODE(name) != IDENTIFIER_NODE)
            return NULL;
          d = IDENTIFIER_LENGTH (name);
          s = IDENTIFIER_POINTER (name);
          if (d/10)
            *p++ = '0'+d/10;
	  *p++ = '0'+ d%10;       /*  assume less that 100 characters  */
	  while (*p++ = *s++);
	  --p;                    /*  not the '\0'                     */
	}
	break;
      
      default:
	return NULL;
    }

  *p = 0;
  return p;
}

