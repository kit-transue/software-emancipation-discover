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
#ifndef _GLOBDEF_H
#define _GLOBDEF_H

/*	global definitions (including menu commands)
 */

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

#define	STYLEFILE	".edge"
#define	DEFAULTNODE	0
#define	DEFAULTNODENAME	"node"
#define	DEFAULTEDGE	0
#define	DEFAULTEDGENAME	"edge"
#define	FONTNAME	"fixed"
#define	NODE_ICONFILE	"node.icon"
#define	ABS_ICONFILE	"abstraction.icon"
#define	EC_ICONFILE	"ec.icon"
#ifndef ICON_PATH
#define	ICON_PATH	"../icon"
#endif
#ifndef DATA_PATH
#define	DATA_PATH	"."
#endif
#define	MAXTEXTSIZE	100
#define STARTUPTYPE     0
#define INTERACTIVETYPE 1
#define SPECIFICTYPE    2
#define TYPENOTEXISTS   -1

/* pop-up menu pane and selection numbers */
#define GRAPH_EXIT 100
#define GRAPH_NEW 101
#define GRAPH_SAVE 102
#define GRAPH_PRINT 103
#define GRAPH_STYLE 105
#define GRAPH_SEPERATE 106
#define LAYOUT_SUGI 205
#define LAYOUT_ISI 206
#define LAYOUT_CIRCUIT 207
#define LAYOUT_TREE 208
#define LAYOUT_PLANAR 209
#define LAYOUT_SUGICONSTR 210
#define LAYOUT_NONE 211
#define LAYOUT_PARAMS 212
#define NODE_ADD 300
#define NODE_DELETE 301
#define NODE_EDIT 302
#define NODE_SHOW 303
#define NODE_ACTION 304
#define NODE_STYLE 305
#define EDGE_ADD 400 
#define EDGE_DELETE 401
#define EDGE_EDIT 402
#define EDGE_SHOW 403
#define EDGE_ACTION 404
#define EDGE_STYLE 405
#define ABS_MAKE 500
#define ABS_SOLVE 501
#define ABS_SAVE 502
#define ABS_READ 503
#define ABS_WHITE 504
#define ABS_GREY 505
#define ABS_BLACK 506
#define ABS_SAVE_SEP 507
#define GRAPH_SAVE_SEP 508
#define GRAPH_STYLE_SEP 509

#define GRAPH_CONSTRAINT 600

#define NO    0
#define YES   1


#define maxval(A,B) ((A) > (B) ? (A):(B))
#define minval(A,B) ((A) < (B) ? (A):(B))

/* function select numbers */
#define INPUT              0
#define OUTPUT             1
#define SUBGRAPH	   2
#define REDRAW             3
#define ABSCREATE          4
#define ABSDISSOLVE        5
#define ABSZOOMIN          6
#define ABSZOOMOUT         7

#define NODEADD            0
#define NODEDELETE         1
#define NODEEDIT           2
#define NODEDRAW           3
#define NODEACTION         4
#define NODESHOW           5

#define EDGECOMPOSITION    0
#define EDGEROUTING        1
#define EDGEADD            2
#define EDGEDELETE         3 
#define EDGEEDIT           4
#define EDGEDRAW           5
#define EDGEACTION         6
#define EDGESHOW           7

#endif
