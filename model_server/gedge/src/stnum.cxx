/*-------------------------------------------------------------------------------------------

    ST_NUM.C

    Autor       : Bernd Rumscheid
    Datum       : 14.12.88
    nderungen  : 01.01.89


    COMPUTE_ST_NUMBERING erzeugt einen ST_GRAPHEN, dessen Knoten den unten beschriebenen 
    Bedingungen gengen. Der zugrunde liegende Algorithmus steht in S. Evens S.180ff. 

 -------------------------------------------------------------------------------------------*/


#include <psetmem.h>

#include "const.def"
#include "addcross.stc"
#include "adttree.stc"
#include "stnum.tst"    

extern int debug;


/*-----------------------------------------------------------------------------------------*
 *   USED gibt an, ob eine Kante markiert ist.                                             *
 *-----------------------------------------------------------------------------------------*/    
    
void  SET_EDGE_USED( ST_GRAPH *SG, ST_NODE *V, ST_NODE *W )
{
    SG->used[ (V->id - 1)*SG->V + W->id - 1 ] = true;
    SG->used[ (W->id - 1)*SG->V + V->id - 1 ] = true;

} /* SET_EDGE_USED */


/*------------------------------------------------------------*/

void  SET_ARROW_USED( ST_GRAPH *SG, ST_NODE *V, ST_NODE *W )
{
    SG->used[ (V->id - 1)*SG->V + W->id - 1 ] = true;

} /* SET_ARROW_USED */


/*------------------------------------------------------------*/

int   GET_USED( ST_GRAPH *SG, ST_NODE *V, ST_NODE *W )
{
    return( SG->used[ (V->id - 1)*SG->V + W->id - 1 ] );

} /* GET_USED */


/*------------------------------------------------------------*/

void CLEAR_USED( ST_GRAPH *SG )
{
    int     i;


    for ( i = 0; i < SG->V*SG->V; i++ )
        SG->used[ i ] = false;

} /* CLEAR_USED */
    

/*------------------------------------------------------------*/

void CLEAR_MARKS( ST_GRAPH *SG )
{
    ST_NODE     *V;


    for ( V = SG->first; V != nil; V = V->next )
        V->mark = false; 

} /* CLEAR_MARKS */



/*-----------------------------------------------------------------------------------------*
 *   SEGMENTS ist ein zweidimensionales ARRAY in dem die Segmentlisten der Kanten stehen.  *
 *-----------------------------------------------------------------------------------------*/    
 
void  CLEAR_SEGMENTS( ST_GRAPH *SG )
{
    int     i;


    for ( i = 0; i < SG->V*SG->V; i++ )
        SG->edges[ i ] = nil;
        
} /* CLEAR_SEGMENTS */


/*------------------------------------------------------------*/

void  SET_SEGMENTS( ST_GRAPH *SG, int V, int W, SEGMENT *SEG )
{
    SG->edges[ (V - 1) * SG->V + W - 1 ] = SEG;
    
} /* SET_SEGMENTS */


/*------------------------------------------------------------*/

SEGMENT*  GET_SEGMENTS( ST_GRAPH *SG, int V, int W )
{
    SEGMENT     *SEG;
     
    
    SEG = SG->edges[ (V - 1) * SG->V + W - 1 ];
    return( SEG );
    
} /* GET_SEGMENTS */


/*------------------------------------------------------------*/

void  ADD_SEGMENT( ST_GRAPH *SG, int V, int W, int x, int y )
{
    SEGMENT     *SEG;
    

    // if (debug) printf( "ADD_SEGMENT( %d, %d ) to %d->%d\n", x, y, V, W );
    
    SEG = (SEGMENT*) psetmalloc( sizeof( SEGMENT ) );
    SEG->x = x;
    SEG->y = y;
    SEG->next = GET_SEGMENTS( SG, V, W );
    SET_SEGMENTS( SG, V, W, SEG );
    
} /* ADD_SEGMENT */




/*-----------------------------------------------------------------------------------------*
 *   DG_TO_ST_GRAPH kopiert den Dynamischen-Graph DG in einen ST_GRAPH                     *
 *-----------------------------------------------------------------------------------------*/    
    
ST_GRAPH*  DG_TO_ST_GRAPH( DYN_GRAPH *DG )
{
    AL          *Al;                                                 /* eine Adjazenzliste */
    DYN_NODE    *DN;
    ST_NODE     *SN;
    ST_GRAPH    *SG;
    AL_NODE     *AN;    
    ST_NODE     *node;
    int         i;


    SG        = (ST_GRAPH*) psetmalloc( sizeof( ST_GRAPH ) );              /* Der ST_GRAPH und */
    SG->V     = DG->new_V;
    SG->first = (ST_NODE*) psetmalloc( sizeof( ST_NODE ) );
    SG->used  = (int*) psetmalloc( SG->V*SG->V * sizeof( int ) );
    SG->edges = (SEGMENT**) psetmalloc( SG->V*SG->V * sizeof( SEGMENT* ) );
    SG->S     = SG->T = nil;
    SG->st_inserted = false;
    SG->dummies = nil;
    CLEAR_USED( SG );
    CLEAR_SEGMENTS( SG );
    
    i = 1;
    SN = SG->first;                                         /* seine Knoten werden erzeugt */
    Al = DG->first_AL;                                              
    while ( Al != nil )
        {
          SN->name  = Al->name;
          SN->id    = i;
          SN->from  = nil;
          SN->to    = nil;
          SN->adj   = nil;
          SN->mark  = false;
          SN->dummy = ( SN->name > DG->old_V );
          SN->dfnum = SN->stnum = SN->low = 0;
          if ( Al->next_AL != nil) 
              SN->next = (ST_NODE*) psetmalloc( sizeof( ST_NODE ) );
          else
              SN->next = nil;

          SN = SN->next;    
          Al = Al->next_AL;
          i++;
        }
        
    SN = SG->first;    
    Al = DG->first_AL;                                  /* und die Adjazenzlisten kopiert. */
    while ( Al != nil )   
        { 
          SN->adj = (AL_NODE*) psetmalloc( sizeof( AL_NODE ) );
          AN      = SN->adj;
          DN      = Al->first;
          while ( DN != nil )
              {
                for ( node = SG->first; node->name != DN->name->name; node = node->next ) ;
                AN->pnode = node;
                 
                if ( DN->next != nil )
                    AN->next = (AL_NODE*) psetmalloc( sizeof( AL_NODE ) );
                else
                    AN->next = nil;
                
                AN = AN->next;
                DN = DN->next;
              }
          SN = SN->next;    
          Al = Al->next_AL;
        }
        
    return( SG );    
        
} /* DG_TO_ST_GRAPH */   
    
    
        
/*------------------------------------------------------------------------------------------*
 *   Funktionen zur Bearbeitung von Adjazenzlisten.                                         *
 *   - FIND_NODE liefert true, wenn der Knoten V in der Liste L enthalten ist.              * 
 *   - ADD_NODE hngt den Knoten v an die Liste l an, wenn er in dieser noch nicht vorkommt *
 *   - INSERT_NODE fgt den Knoten new vor dem Knoten next in L ein.                        *
 *   - SET_FIRST_NODE macht den Knoten first zum ersten Knoten der Liste L. first mua in    *
 *     L enthalten sein.                                                                    *
 *------------------------------------------------------------------------------------------*/
 
ST_NODE*  GET_NODE( ST_NODE *L, int name )
{
    ST_NODE     *SN;


    // if (debug) printf( "Suchen nach Knoten %d\n", name );
    SN = L;
    while ( SN != nil  &&  SN->name != name )
        SN = SN->next;

    return( SN );

} /* GET_NODE */




int FIND_NODE( AL_NODE *L, ST_NODE *V )
{
    AL_NODE     *AN;


    AN = L;
    while ( AN != nil  &&  AN->pnode != V )
        AN = AN->next;

    return( AN != nil );

} /* FIND_NODE */


/*------------------------------------------------------------*/

AL_NODE*  ADD_NODE( AL_NODE *L, ST_NODE *V )
{
    AL_NODE     *AN;
          

    if ( L == nil ) { 
        L = (AL_NODE*) psetmalloc( sizeof( AL_NODE ) );
        L->pnode = V;
        L->next = nil;
    } else {
        AN = L;
        while ( AN->next != nil  &&  AN->pnode != V ) 
            AN = AN->next;
        if ( AN->pnode != V ) {
            AN->next = (AL_NODE*) psetmalloc( sizeof( AL_NODE ) );
            AN       = AN->next;
            AN->pnode = V;
            AN->next = nil;
        }
    }         
    
    return( L );

} /* ADD_NODE */   
                

/*------------------------------------------------------------*/

AL_NODE*  REMOVE_NODE( AL_NODE *L, ST_NODE *V )
{
    AL_NODE     *prevAN, *AN;
    

    AN = L;
    prevAN = nil;
    while ( AN->pnode != V ) {
        prevAN = AN;
        AN = AN->next;
    }
    if ( prevAN == nil ) L = L->next;
    else prevAN->next = AN->next;
    /* psetfree( AN ); */
    return( L );
    
} /* REMOVE_NODE */


/*------------------------------------------------------------*/

AL_NODE* INSERT_NODE( AL_NODE *L, ST_NODE *New, ST_NODE *Next )
{
    AL_NODE     *AN, *prevAN, *newAN;
    
    
    newAN = (AL_NODE*) psetmalloc( sizeof( AL_NODE ) );
    newAN->pnode = New;
    newAN->next = nil;
        
    prevAN = nil;
    AN = L;
    while ( AN->pnode != Next ) {
        prevAN = AN;
        AN = AN->next;
    }
    if ( prevAN == nil ) {
        newAN->next = L;
        L = newAN;
    } else {
        newAN->next = AN;
        prevAN->next = newAN;
    }
    
    return( L );
    
} /* INSERT_NODE */


/*------------------------------------------------------------*/

AL_NODE*  SET_FIRST_NODE( AL_NODE *L, ST_NODE *first )
{
    AL_NODE     *AN;


    for ( AN = L; AN->next != nil; AN = AN->next ) ;                    /* L wird zirkular */
    AN->next = L;                                                
    while ( AN->pnode != first ) AN = AN->next;                 /* first als erstes Element */
    L = AN;                                                         
    while ( AN->next != L ) AN = AN->next;                          /* Zirkularitt wieder */
    AN->next = nil;                                                          /* aufbrechen */
    
    return( L );
    
} /* SET_FIRST_NODE */    



/*-----------------------------------------------------------------------------------------*
 *   CHOOSE_ST bestimmt die grate Flche des ST_GRAPHen und, sofern diese mehr als        *    
 *   drei Kanten hat, zwei Knoten auf dem Rand dieser Flche deren Verbindung die Flche   *
 *   in zwei gleichgroae Hlften teilt. S und T werden diese Knoten.                       *
 *   Als grate Flche wird die bevorzugt, die die wenigsten dummy-Knoten enthlt. Es      *
 *   wird ebenfalls versucht fr S und T keine dummy-Knoten zu whlen.                     *
 *   COUNT_EDGES wird von CHOOSE_ST benutzt um die Anzahl der Kanten der Flche, die mit   *
 *   V->W beginnt zu bestimmen. Die nchste Kante von W weg ist immer die nchste gegen    *
 *   den Uhrzeigersinn von V->W.                                                           *
 *-----------------------------------------------------------------------------------------*/    
 
int   COUNT_EDGES( ST_GRAPH *SG, ST_NODE *V, ST_NODE *W, AL_NODE **FACE, int *ndummies )
{
    AL_NODE     *AN;
    int         n;
    
    
    // if (debug) printf( "COUNT_EDGES: " );
    
    n = 0;
    (*ndummies) = 0;
    *FACE = nil;
    *FACE = ADD_NODE( *FACE, V );
    if ( V->dummy ) 
        (*ndummies)++;
    do {
        n++;
        SET_ARROW_USED( SG, V, W );
        *FACE = ADD_NODE( *FACE, W );
        if ( W->dummy )
            (*ndummies)++;
        
        // if (debug) printf( " %d->%d", V->name, W->name );
        
        for ( AN = W->adj; AN->pnode != V; AN = AN->next ) ;
        V = W;
        if ( AN ->next != nil )
            W = AN->next->pnode;
        else 
            W = W->adj->pnode;
    } while ( ! GET_USED( SG, V, W ) );
    
    // if (debug) printf( "    n: %d     ndummies: %d\n", n, *ndummies );     

    return( n );
    
 } /* COUNT_EDGES */
 
 
/*------------------------------------------------------------*/ 

void  CHOOSE_ST( ST_GRAPH *SG, int bisection )
{
    ST_NODE     *V, *W;
    ST_NODE     *prev_of_T, *next_of_S;
    AL_NODE     *AN, *FACE, *MAX_FACE;
    int         counted, dummies_counted, ndummies, n, i;

    

    // if (debug) printf( "CHOOSE_ST:\n" );
    
    CLEAR_USED( SG );
    counted = 0;
    dummies_counted = MAXINT;


    if ( ! bisection ) {
        SG->S = SG->first;
        SG->T = SG->S->adj->pnode;
        SG->st_inserted = false;
    } else {
        for ( V = SG->first; V != nil; V = V->next )                /* grate Flche bestimmen */
            for ( AN = V->adj; AN != nil; AN = AN->next ) {
                W = AN->pnode;
                if ( ! GET_USED( SG, V, W )  &&  
                     (n = COUNT_EDGES( SG, V, W, &FACE, &ndummies )) >= counted  &&
                     ndummies <= dummies_counted )
                {
                    if ( counted != n  ||  ndummies != dummies_counted ) {
                        counted = n;
                        dummies_counted = ndummies;
                        MAX_FACE = FACE;
                    }   
                }
            }
            
        if ( counted > 3 ) {                                             /* st-Kante eintragen */
            for ( AN = MAX_FACE; AN->next != nil; AN = AN->next ) ;
            AN->next = MAX_FACE;
        
            AN = MAX_FACE;                                /* mglichst kein dummy als S oder T */
            do {
                SG->S     = AN->pnode;
                next_of_S = AN->next->pnode;
                for ( i = 1; i < (counted / 2); i++ )
                    AN = AN->next;
                prev_of_T = AN->pnode;   
                SG->T     = AN->next->pnode;
                AN        = AN->next;
            } while ( (SG->S->dummy  ||  SG->T->dummy)  &&  AN != MAX_FACE );

            // if (debug) printf( "next_of_S: %d      prev_of_T: %d \n", next_of_S->name, prev_of_T->name );
            
            if ( ! FIND_NODE( SG->S->adj, SG->T ) ) {
                SG->S->adj = INSERT_NODE( SG->S->adj, SG->T, next_of_S );
                SG->T->adj = INSERT_NODE( SG->T->adj, SG->S, prev_of_T );
                SG->st_inserted = true;
	    } else 
                SG->st_inserted = false;
        } else {
            SG->S = MAX_FACE->pnode;
            SG->T = MAX_FACE->next->pnode;
            SG->st_inserted = false;
        }
    }

    // if (debug) printf( " counted: %d,   S = %d    T = %d\n", counted, SG->S->name, SG->T->name );

} /* CHOOSE_ST */

              
              
/*------------------------------------------------------------------------------------------*
 *   DEPTH_FIRST_SEARCH nummeriert die Knoten in der Reihenfolge wie sie besucht werden     *
 *   (dfnum) und bestimmt den kleinsten erreichbaren Knoten (low). Auaerdem werden die      *
 *   Kanten gerichtet und auf from und to aufgeteilt.                                       *
 *   T erhlt die dfnum 1.                                                                  *
 *------------------------------------------------------------------------------------------*/

void DFS( ST_NODE *V, ST_NODE *father, int *dfnum, ST_GRAPH *SG )
{
    ST_NODE    *W;
    AL_NODE    *AN;
     

    V->mark  = true;
    V->dfnum = *dfnum;
    V->low   = *dfnum;
            
    AN = V->adj;
    while ( AN != nil ) 
        {     
          W = AN->pnode;

          if ( ! GET_USED( SG, V, W ) ) {
              V->to   = ADD_NODE( V->to, W );   
              W->from = ADD_NODE( W->from, V );   
              SET_EDGE_USED( SG, V, W );
          }    

          if ( ! W->mark ) {
              (*dfnum)++;
              DFS( W, V, dfnum, SG );
              if ( W->low < V->low )
                  V->low = W->low;
          } else {
              if ( W != father  &&  W->dfnum < V->low ) 
                  V->low = W->dfnum;
          }        
                  
          AN = AN->next;    
        }
        
} /* DFS */     
      
      
/*------------------------------------------------------------*/        

void  DEPTH_FIRST_SEARCH( ST_GRAPH *SG )
{
    int     dfnum;
    

    CLEAR_USED( SG );
    
    SG->T->adj = SET_FIRST_NODE( SG->T->adj, SG->S );
    SG->S->adj = SET_FIRST_NODE( SG->S->adj, SG->T );
    //if (debug) auswertung_SG( SG );
    
    dfnum = 1;
    DFS( SG->T, nil, &dfnum, SG );
    
} /* DEPTH_FIRST_SEARCH */
 

              
/*------------------------------------------------------------------------------------------*
 *   Bestimmt eine st-Nummerierung fr die Knoten von SG.                                   *
 *                                                                                          * 
 *   Fr eine st-Nummerierung gilt:                                                         *
 *                                                                                          *   
 *      1.  st_num( s ) = 1                                                                 *
 *      2.  st_num( t ) = |V| = n                                                           *
 *      3.  Fr jeden Knoten v aus V - { s, t } gibt es zwei adjazente Knoten u und w so,   *
 *          daa  st_num( u ) < st_num( v ) < st_num( w )                                    *   
 *                                                                                          *
 *------------------------------------------------------------------------------------------*/

AL_NODE     *Stack;


/*------------------------------------------------------------*/

void PUSH( ST_NODE *V )
{
    AL_NODE     *AN;
    
    
    AN       = (AL_NODE*) psetmalloc( sizeof( AL_NODE ) );
    AN->pnode = V;
    AN->next = Stack;
    Stack    = AN;
    
} /* PUSH */


/*------------------------------------------------------------*/

ST_NODE*  POP()
{
    ST_NODE     *SN;
    AL_NODE     *AN;


    SN = nil;
    if ( Stack != nil ) {
        AN     = Stack;
        Stack  = Stack->next;
        SN     = AN->pnode;
        /* psetfree( AN ); */
    }

    return( SN );

} /* POP */ 


/*------------------------------------------------------------*/

void  FIND_FORWARD_PATH( ST_GRAPH *SG, ST_NODE *V )
{
    ST_NODE     *W;
    AL_NODE     *AN;


    AN = V->to;
    while ( AN != nil ) {
    
        W = AN->pnode;
        if ( GET_USED( SG, V, W ) == false  &&  
             ( W->dfnum == V->low  ||  ( W->dfnum > V->dfnum  &&  W->low == V->low ) ) ) {

            SET_EDGE_USED( SG, V, W );
            W->mark = true;
            if ( W->dfnum != V->low ) 
                FIND_FORWARD_PATH( SG, W );
            PUSH( V );
            return;
        }

        AN = AN->next;
    }
} /* FIND_FORWARD_PATH */


/*------------------------------------------------------------*/

void  FIND_BACKWARD_PATH( ST_GRAPH *SG, ST_NODE *V )
{
    ST_NODE     *W;
    AL_NODE     *AN;


    AN = V->from;
    while ( AN != nil ) {
    
        W = AN->pnode;
        if ( ! GET_USED( SG, V, W )  &&   V->dfnum > W->dfnum ) {
            SET_EDGE_USED( SG, V, W );
            if ( ! W->mark ) {
                W->mark = true;
                FIND_BACKWARD_PATH( SG, W );
            }
            PUSH( V );
            return;
        }

        AN = AN->next;
    }
} /* FIND_BACKWARD_PATH */


/*------------------------------------------------------------*/

int  FIND_PATH( ST_GRAPH *SG, ST_NODE *V )
{
    ST_NODE     *W;
    AL_NODE     *AN;

    
    AN = V->adj;
    while ( AN != nil ) {
    
        W = AN->pnode;
        if ( ! GET_USED( SG, V, W ) ) {

            SET_EDGE_USED( SG, V, W );
            W->mark = true;

            if ( W->dfnum > V->dfnum ) {               
                if ( FIND_NODE( V->to, W ) )           /* treeedge v -> w */
                    FIND_FORWARD_PATH( SG, W );
                else                                   /* backedge w -> v */
                    FIND_BACKWARD_PATH( SG, W );           
            }                                   /* else : backedge v -> w */

            PUSH( V ); 
            return( true );
        }   
        AN = AN->next;
    }
            
    return( false );
    
} /* FIND_PATH */                  


/*------------------------------------------------------------*/

void  ST_NUMBERING( ST_GRAPH* SG )
{  
    ST_NODE     *V;
    int         i;


    CLEAR_USED( SG );
    CLEAR_MARKS( SG );
    SET_EDGE_USED( SG, SG->S, SG->T );

    Stack = nil;
    PUSH( SG->T );      SG->T->stnum = SG->V;       SG->T->mark = true;    
    PUSH( SG->S );      SG->S->stnum = 1;           SG->S->mark = true;
    
    i = 1;
    while ( (V = POP()) != SG->T ) {
        if ( FIND_PATH( SG, V ) == false ) { 
            V->stnum = i;
            i++;
        }
    }
} /* ST_NUMBERING */


/*------------------------------------------------------------*/

ST_GRAPH*  COMPUTE_ST_NUMBERING( DYN_GRAPH *DG, int bisection )
{
    ST_GRAPH    *SG;


    SG = DG_TO_ST_GRAPH( DG );    
    CHOOSE_ST( SG, bisection );
    DEPTH_FIRST_SEARCH( SG );    
    ST_NUMBERING( SG );        
    return( SG );
} /* COMPUTE_ST_NUMBERING */



/*------------------------------------------------------------------------------------------*
 * TREE_ST_NUMBERING bestimmt eine ST-Nummerierung fuer einen Baum. Die Wurzel des Baumes   *
 * wird als S gewaehlt und ein neuer Dummy-Knoten wird T. Alle Blaetter des Baumes werden   *
 * mit einer Dummy-Kante mit dem neuen Knoten T verbunden. Jetzt ist der Graph zweifach     *
 * zusammenhaengend. Eine ST-Nummerierung wird durch Breitensuche von der Wurzel ausgehend  *
 * vorgenommen.                                                                             *
 *------------------------------------------------------------------------------------------*/

void  PRINT_ST_EDGES( ST_EDGE *SE )
{
    ST_EDGE      *E;


    printf( "ST_EDGES:" );
    for ( E = SE; E != nil; E = E->next )
        printf( " [%d,%d]", E->source, E->dest );
    printf( "\n" );

} /* PRINT_ST_EDGES */


/*------------------------------------------------------------*/

ST_EDGE*  ADD_ST_EDGE( ST_EDGE *SE, int V, int W )
{
    ST_EDGE       *newE;
  

    //if (debug) printf( "   Add ST-EDGE %d, %d\n", V, W );

    newE = (ST_EDGE*) psetmalloc( sizeof( ST_EDGE ) );
    newE->source = V;
    newE->dest   = W;
    newE->next   = SE;
    SE           = newE;

    return( SE );

} /* ADD_ST_EDGE */



/*------------------------------------------------------------*/

void  TREE_TO_ST_GRAPH( ST_NODE *father, TREE *T, ST_GRAPH *SG )
{
    ST_NODE       *SN;


    if ( T == nil ) return;

/*
    if (debug) {
        printf( "Tree to ST-Graph\n" );
        auswertung_SG( SG );
    }
*/

    SG->V++;
    SN = (ST_NODE*) psetmalloc( sizeof( ST_NODE ) );
    SN->name  = T->name;
    SN->id    = SG->V;
    SN->x     = SN->y     = 0;
    SN->dfnum = SN->low   = SN->stnum = SG->V;
    SN->mark  = SN->dummy = false;
    SN->from  = SN->to    = SN->adj   = nil;
    SN->next  = SG->first;
    SG->first = SN;

    SN->adj     = ADD_NODE( SN->adj, father );
    father->adj = ADD_NODE( father->adj, SN );
    if ( T->succ == nil ) {                                         /* leaf */
        SN->adj     = ADD_NODE( SN->adj, SG->T );
        SG->dummies = ADD_ST_EDGE( SG->dummies, SN->name, SG->T->name );
        SG->T->adj  = ADD_NODE( SG->T->adj, SN );
    }
    
    TREE_TO_ST_GRAPH( father, T->next, SG );
    TREE_TO_ST_GRAPH( SN, T->succ, SG );

} /* TREE_TO_ST_GRAPH */



/*------------------------------------------------------------*/

ST_GRAPH*  TREE_ST_NUMBERING( TREE *Tree )
{
    ST_GRAPH      *SG;
    ST_NODE       *S, *T;


    if ( Tree == nil ) return( nil );

    SG = (ST_GRAPH*) psetmalloc( sizeof( ST_GRAPH ) );
    
    S = (ST_NODE*) psetmalloc( sizeof( ST_NODE ) );            /* Wurzel des Baumes wird S */
    S->name  = Tree->name;
    S->id    = 1;
    S->x     = S->y     = 0;
    S->dfnum = S->low   = S->stnum = 1;
    S->mark  = S->dummy = false;
    S->from  = S->to    = S->adj   = nil;
    S->next  = nil;

    T = (ST_NODE*) psetmalloc( sizeof( ST_NODE ) );       /* Ein neuer Dummy-Knoten wird T */
    T->name  = MAXINT;
    T->id    = 0;
    T->x     = T->y     = 0;
    T->dfnum = T->low   = T->stnum = 0;
    T->mark  = T->dummy = false;
    T->from  = T->to    = T->adj   = nil;
    T->next  = nil;

    S->adj = ADD_NODE( S->adj, T );                        /* Dummy ST-Kante einfuegen */
    T->adj = ADD_NODE( T->adj, S );
    SG->S = S;
    SG->T = T;
    SG->dummies = nil;
    SG->st_inserted = true;
    SG->first = S;
    S->next = T;
    SG->V = 1;

    TREE_TO_ST_GRAPH( S, Tree->succ, SG );

    SG->V++;
    T->id = T->dfnum = T->low = T->stnum = SG->V;
    SG->used  = (int*) psetmalloc( SG->V*SG->V * sizeof( int ) );
    SG->edges = (SEGMENT**) psetmalloc( SG->V*SG->V * sizeof( SEGMENT* ) );
    CLEAR_USED( SG );
    CLEAR_SEGMENTS( SG );

/*
    if (debug) {
        printf( "TREE_TO_ST_GRAPH\n" );
        auswertung_SG( SG );
        PRINT_ST_EDGES( SG->dummies );
    }
*/

    return( SG );

} /* TREE_ST_NUMBERING */



/* END ST_NUM.C */

