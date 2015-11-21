/**-------------------------------------------------------------------------------------------

     PLANARLAYOUT.C
     
     Autor      : Bernd Rumscheid
     Date       : 17.01.89
     nderungen :
     
 ------------------------------------------------------------------------------------------*/
 
#ifndef ISO_CPP_HEADERS
#include  <stdlib.h>                                                             
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
                 
#include "all.h"
 

#include "const.def"
#include "bico.stc"


extern int debug;                                                                                

extern BICO_COMP*    GET_BICO_COMPONENTS( UNDIR_GRAPH* );
extern int           PLANAR_BICO_COMPONENTS( BICO_COMP*, int* );
extern void          BUILD_TREE_NAME_TABLES( TREE_LIST* );
extern ST_NODE*      GET_NODE( ST_NODE*, int );

extern SEGMENT*      GET_SEGMENTS( ST_GRAPH*, int, int );


extern auswertung_Al();

extern void MakeImproper(graph*);
extern void MakeProper(graph*);

/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void          SET_UGRAPH_TRANS_NO( UNDIR_GRAPH*, int, int );
extern UNDIR_GRAPH*  CREATE_UGRAPH( int ); 
extern int           IS_UEDGE( UNDIR_GRAPH*, int, int );
extern void          SET_UEDGE( UNDIR_GRAPH*, int, int );

#ifdef __cplusplus
}
#endif /* __cplusplus */


int SetDFNums( graph* g )
{
   int i = 1;

   for (node* n = g->getheadnode(); n != 0; n = n->getnextnode(), i++)
     n->setdfnum( i );

   return i-1;
}

 

UNDIR_GRAPH* NewUndirGraph( graph* dg )
{
    UNDIR_GRAPH *G;

    int n;       /* Anzahl Knoten des Graphs */  
    int sourcenr, targetnr;

    n = SetDFNums(dg); 
    G = CREATE_UGRAPH( n );

    /* Querverweistabelle erzeugen */
    for(int i = 1; i <= G->n; i++) SET_UGRAPH_TRANS_NO( G, i, i );

    /* Kanten in den ungerichteten Graphen eintragen */
    for (edge* e = dg->getheadedge(); e != 0; e = e->getnextedge()) {
       sourcenr = (e->getsource())->getdfnum();
       targetnr = (e->gettarget())->getdfnum();
       if (!IS_UEDGE( G, sourcenr, targetnr ))
          SET_UEDGE( G, sourcenr, targetnr );

    }

    return G;
}



void  GRAPH_TO_EDGE( graph *g, ST_GRAPH *SG, node *, node *, 
                     int, int , int ,
                     int )
{
    SEGMENT     *SEG;
    ST_NODE     *V;
    node        *newS, *newT;
    edge        *newE;
    int         i, j;



    for ( i = 1; i <= SG->V; i++ )
        for ( j = 1; j <= SG->V; j++ ) {

            SEG = GET_SEGMENTS( SG, i, j );
            if ( SEG != nil ) {
                // if (debug) printf( "Printout [ %d, %d ]\n", i, j );
                while ( SEG->next != nil ) {
                    // if (debug) printf( "Create Segment: [ %d, %d ] - [ %d, %d ]\n",
                                       // SEG->x, SEG->y, SEG->next->x, SEG->next->y );
                    newS = g->add_node(  1, 0, 0, 0);
                    newT = g->add_node( 1, 0, 0, 0);
                    newS->setx( SEG->x);
                    newS->sety( SEG->y);
                    newT->setx( SEG->next->x);
                    newT->sety( SEG->next->y);
                    newS->setdummy(1);
                    newT->setdummy(1);
                    newE = g->add_edge( newS, newT, 0, 1, 0, 0 );
                    newE->setdummy(1);
                    SEG = SEG->next;
		}
             }

	  }

    for (node* n = g->getheadnode(); n != 0; n = n->getnextnode()) {
       for ( V = SG->first; V != nil  &&  V->name != n->getdfnum(); V = V->next );
       if ( V != nil ) n->movenode( V->x, V->y );
    }


} /* GRAPH_TO_EDGE */




/* END MAIN */














