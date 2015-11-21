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
#include <elsLanguage.h>
#include <xrefSymbol.h>
#include <symbolPtr.h>
#include <objCommon.h>
#include <symbolArr.h>

/* Given symbolPtr, try to find elsLanguage of this symbol.
   1. Call symbolPtr::get_language(). If it returns C or CPP,
      return dummy elsLanguage pointers returned by
      get_els_language_for_c() and get_els_language_for_cpp()
      respectively.
   2. If symbolPtr::get_language() returns smt_ELS and sym is
      xrefSymbol, try to get definition file and use
      els_get_language() on that file. If not definition file
      can be found, get files where referenced and use
      els_get_language() on the first file found.
   3. Return NULL if everything fails.
*/
const elsLanguage* elsLanguage::get_els_language(symbolPtr& sym)
{
  Initialize(elsLanuage::get_els_language);

  const elsLanguage* ret = 0;

  smtLanguage smtLang = smtLanguage(sym.get_language());

  if (is_CPLUSPLUS(smtLang))
    ret = get_els_language_for_cpp();
  else if (is_C(smtLang))
    ret = get_els_language_for_c();
  else if (smtLang == smt_ELS)
    {
      if (sym.get_kind() == DD_MODULE)
	{
	  ret = els_get_language(sym.get_name());
	}
      else if ( sym.is_xrefSymbol())
	{
	  symbolPtr deffile;
	  deffile = sym->get_def_file(); 
	  
	  if (deffile.xrisnotnull())
	    {
	      ret = (elsLanguage*)els_get_language(deffile.get_name());
	    }
	  else
	    {
	      /* could not find definition file, try files where referenced */
	      symbolArr files;
	      sym->get_ref_files(files);
	      if (files.size() > 0)
		{
		  ret = (elsLanguage*)els_get_language(files[0].get_name());
		}
	    }
	}
    }

  return ret;
}

const elsLanguage* elsLanguage::get_els_language_for_cpp()
{
  static elsLanguage els_cpp("cpp");
  return &els_cpp;
}

const elsLanguage* elsLanguage::get_els_language_for_c()
{
  static elsLanguage els_c("c");
  return &els_c;
}


  


