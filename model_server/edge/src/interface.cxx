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
/*	Template for application specific changes
 *	Customizer can fill in code in the following procedures 
 */

#include "all.h"

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */
#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * APPL_ReadGraph
 */
int APPL_ReadGraph(char* )
{
    return 1;
}

/*
 * APPL_WriteGraph
 */
int APPL_WriteGraph(char*)
{
    return 1;
}

/*
 * APPL_ShowGraph
 */
int APPL_ShowGraph(graph*)
{
    return 1;
}

/*
 * APPL_Redraw
 */
int APPL_Redraw()
{
    return 1;
}

/*
 * APPL_AddNode
 */
node* APPL_AddNode(node*)
{
    return NULL;
}

/*
 * APPL_DeleteNode
 */
int APPL_DeleteNode(node*)
{
    return 1;
}


/*
 * APPL_DrawNode
*/
int APPL_DrawNode (node*)
{
	return 1;
}

/*
 * APPL_EditNode
 */
int APPL_EditNode(node*, char*)
{
    return 1;
}

/*
 * APPL_ShowNode
 */
int APPL_ShowNode(node*)
{
    return 1;
}

/*
 * APPL_ActionNode
 */
int APPL_ActionNode(node*)
{
    return 1;
}

/*
 * APPL_AddEdge
 */
edge* APPL_AddEdge(edge*, node*, node*, char*)
{
    return NULL;
}

/*
 * APPL_DeleteEdge
 */
int APPL_DeleteEdge(edge*)
{
    return 1;
}

/*
 * APPL_DrawEdge
 */
int APPL_DrawEdge(edge*)
{
    return 1;
}

/*
 * APPL_EditEdge
 */
int APPL_EditEdge(edge*, char*)
{
    return 1;
}

/*
 * APPL_ShowEdge
 */
int APPL_ShowEdge(edge*)
{
    return 1;
}

/*
 * APPL_ActionEdge
 */
int APPL_ActionEdge(edge*)
{
    return 1;
}

/*
 * APPL_Layout
 */
int APPL_Layout()
{
    return 1;
}

/*
 * APPL_Constraints
 */
int APPL_Constraints()
{
    return 1;
}
/*
   START-LOG-------------------------------------------

   $Log: interface.cxx  $
   Revision 1.1 1992/11/21 16:27:05EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:42  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
