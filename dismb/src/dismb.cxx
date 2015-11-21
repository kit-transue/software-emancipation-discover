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
#include <pduio.h>
#include <pdumem.h>
#include <pdupath.h>
#include <pduproc.h>
#include <pdustring.h>

//Add $PSETHOME/bin to the begining of $PATH if it is not present.
void set_PATH_environment_variable(char const* psethome)
{
    char const *old_path = pdgetenv("PATH");
#ifdef _WIN32
    char const *psethome_bin = pdmpack("s", "%s\\bin;", psethome);
#else
    char const *psethome_bin = pdmpack("s", "%s/bin:", psethome);
#endif
    if (pdstrncmp(old_path, psethome_bin, pdstrlen(psethome_bin), 0) != 0) {
        // will add this string to environment -- should not be freed
        char const *path_env = pdmpack("ss", "PATH=%s%s", psethome_bin, old_path);
	pdputenv((char *)path_env);
    }
    FREE_MEMORY(psethome_bin);
}

int main ( int argc, char **argv )
{
  int retval = -1;

  char const *psethome = pdgetenv("PSETHOME");

  if ( psethome )
    {
      set_PATH_environment_variable(psethome);

      char const *dismbdir = pdgetenv("DISMBDIR");
      int freedis = 0;
      if ( ! dismbdir )
        {
          dismbdir = pdmpack("s", "%s/mb", psethome);
	  freedis = 1;
	  // add to environment -- should not be freed
          char const *dismbdir_env = pdmpack("s", "DISMBDIR=%s", dismbdir);
          pdputenv((char *)dismbdir_env);
        }

      char const *cmd = pdmpack("s", "disperl \"%s/dismb_driver.pl\" ", dismbdir);
      if ( cmd )
	{
	  // add arguments
	  for ( int i = 1; i < argc; i++ )
	    {
	      if ( argv[i] )
		{
		  char const *tmp = pdmpack("ss", "%s %s", cmd, argv[i]);
		  if ( tmp )
		    {
		      FREE_MEMORY(cmd);
		      cmd = tmp;
		    }
		}
	    }

	  retval = pdsystem(cmd);
	}
      FREE_MEMORY(cmd);
      if (freedis)
	{
	  FREE_MEMORY(dismbdir);
        }
    } 

  return retval;
}
