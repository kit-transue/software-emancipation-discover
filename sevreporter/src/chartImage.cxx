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
////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

//++C
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
//--C

//++C++
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
#endif /* ISO_CPP_HEADERS */
//--C++

//++OSPORT
#include <pdufile.h>
#include <pdumem.h>
#include <pduio.h>
#include <pduproc.h>
#include <pdupath.h>
#include <pdustring.h>
//--OSPORT

//++LIBGD
#include <gdc.h>
#include <gdchart.h>
//--LIBGD

//++SEV
#include <chartImage.h>
#include <reporterArgs.h>
#include <qualityItem.h>
#include <queryFolder.h>
//--SEV

//
//
// Construction/Destruction
//
//

chartImage::chartImage(void)
{
}

chartImage::~chartImage(void)
{
}

//
//
// Methods
//
//

//
// Public interface to create the image
//

int chartImage::plot_data(qualityItem *qi, queryFolder *qf, char const *filename, imagetype type)
{
  int retval = -1;

  if ( qi && qf && filename )
    {
      switch ( type )
	{
	case gif:
	  retval = generateGIF(qi, qf, filename);
	  break;

	case png:
	  retval = generatePNG(qi, qf, filename);
	  break;

	default:
	  break;
	}
    }

  return retval;
}

//
// Private interface to create image in GIF format using Perl
//

int chartImage::generateGIF(qualityItem *qi, queryFolder *qf, char const *filename)
{
  int retval = -1;

  char  **titles = 0;
  float *numbers = 0;

  if ( qi && qf && filename )
    {
      // seperate the titles out from the data points
      if ( getDataToArrays(qi, qf, &titles, &numbers) >= 0 )
	{
	  // use the GD library to graph the data
	  if ( graphWithGD(titles, numbers, 1 + qf->numItems(), (char *)filename) >= 0 )
	    {
	      retval = 0;
	    }
	}
    }

  FREE_MEMORY(titles);
  FREE_MEMORY(numbers);

  return retval;
}

//
// Private interface to create image in PNG format
//
int chartImage::generatePNG(qualityItem *qi, queryFolder *qf, char const *filename)
{
  int retval = -1;

  char const *tempfile = 0;
  char const *command  = 0;

  if ( qi && qf && filename )
    {
      // get a temp file to hold a GIF image
      if ( tempfile = pdtempname(0) )
	{
	  // generate a GIF of the data
	  if ( generateGIF(qi, qf, tempfile) >= 0 )
	    {
	      // convert GIF to PNG
	      if ( convertGIFtoPNG(tempfile, filename) >= 0 )
		{
		  retval = 0;
		}
	      pdrmfile(tempfile);
	    }
	}
    }

  return retval;
}

//
// Function to convert GIF file to PNG file
//
int chartImage::convertGIFtoPNG(char const *giffile, char const *pngfile)
{
  int   retval  = -1;

  char const *command            = 0;
  char const *converter_exec     = 0;
  char const *converter_exec_res = 0;

  if ( giffile && pngfile )
    {
      // get the gif to png converter program
      if ( converter_exec = rptgenArg_value(rptgenArg_GIF_TO_PNG_EXEC) )
	{
	  // resolve the converter exec
	  if ( converter_exec_res = pdexecpath(converter_exec) )
	    {
	      // create command line for conversion program
	      if ( command = pdmpack("sss", "%s %s %s", converter_exec_res, giffile, pngfile) )
		{
		  // execute command
		  pdsystem(command);
		  retval = 0;
		}
	    }
	}
    }

  FREE_MEMORY(converter_exec_res);
  FREE_MEMORY(command);

  return retval;
}

int chartImage::getDataToArrays(qualityItem *qi, queryFolder *qf, char ***titles, float **numbers)
{
  int retval = -1;

  if ( titles && numbers )
    {
      // get space
      *titles  = GET_MEMORY(char *, 1 + qf->numItems());
      *numbers = GET_MEMORY(float, 1 + qf->numItems());
      
      if ( *titles && *numbers )
	{
	  for ( int i = 0; i <= qf->numItems(); i++ )
	    {
	      if ( i == 0 )
		{
		  (*titles)[0]  = pdstrdup("Avg.");
		  (*numbers)[0] = (float) pdatodbl(qi->score());
		}
	      else
		{
		  qualityItem *q = (qualityItem *) qf->getItem(i-1);
		  if ( q )
		    {
		      (*titles)[i]  = pdinttoa(i);
		      (*numbers)[i] = (float) pdatodbl(q->score());
		    }
		}
	    }
	  
	  retval = 0;
	}
    }

  return retval;
}

int chartImage::graphWithGD(char **titles, float *numbers, int numpoints, char *filename)
{
  int retval = -1;

  int   width   = pdatoint(rptgenArg_value(rptgenArg_GRAPH_WIDTH));
  int   height  = pdatoint(rptgenArg_value(rptgenArg_GRAPH_HEIGHT));
  FILE *giffile = 0;

  if ( titles && numbers )
    {
      // make background transparent
      GDC_transparent_bg = TRUE;

      // set the colors
      unsigned long bar_color = pdatolong(rptgenArg_value(rptgenArg_GRAPH_BAR_COLOR));
      unsigned long fg_color  = pdatolong(rptgenArg_value(rptgenArg_GRAPH_FG_COLOR));
      GDC_LineColor           = fg_color;
      GDC_SetColor            = &bar_color;
      GDC_GridColor           = fg_color;
      // set the title
      GDC_title      = (char *) rptgenArg_value(rptgenArg_GRAPH_TITLE);
      GDC_TitleColor = pdatolong(rptgenArg_value(rptgenArg_GRAPH_TEXT_COLOR));
      
      // open the file
      if ( giffile = fopen(filename, "wb") )
	{
	  GDC_out_graph(width, height,  // width and height of graph
			giffile,        // output file of gif
			GDC_3DBAR,      // type of chart
			numpoints,      // numbers of bars in chart
			titles,         // title data
			1,              // number of data sets
			numbers);       // the data set
	  
	  retval = 0;
	}
    }
  
  if ( giffile )
    fclose(giffile);

  return retval;
}
