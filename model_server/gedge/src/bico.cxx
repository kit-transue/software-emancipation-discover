/*------------------------------------------------------------------------------------------
  
    BICO.C
     
    Autor      : Bernd Rumscheid
    Date       : 14.01.89
    Aenderungen : 

    Ein Graph kann in seine zweifach zusammenhngenden Komponenten (BICO) zerlegt
    werden. Der daraus entstehende Baum aus Artikulationspunkten und zweifach 
    zusammenhngenden Komponenten kann durchlaufen werden.

 ------------------------------------------------------------------------------------------*/
 
#include "const.def"
#include "bico.stc"
#include "bico.tst"
#include "hopcroft.stc"
#include <psetmem.h>

extern int debug;

/* aus ADTUGRAPH */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern int GET_UNODE_FLAG( UNDIR_GRAPH*, int );
extern void SET_UNODE_FLAG( UNDIR_GRAPH*, int, int );
extern UNDIR_GRAPH* CREATE_UGRAPH( int );
extern void SET_UEDGE( UNDIR_GRAPH*, int, int );
extern int IS_UEDGE( UNDIR_GRAPH*, int, int );
extern int GET_FIRST_UNODE( UNDIR_GRAPH*, int, NODE** );
extern int GET_NEXT_UNODE( NODE** );
extern void RESET_UNODE_FLAGS( UNDIR_GRAPH* );
extern void SET_UGRAPH_TRANS_NO( UNDIR_GRAPH*, int, int );

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* aus HOPCROFT */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern ARTI_HELP* CREATE_ARTI_HELP( UNDIR_GRAPH* );
extern int GET_DFS( ARTI_HELP*, int );
extern int GET_LOW( ARTI_HELP*, int );
extern int IS_ARTI_POINT( ARTI_HELP*, int );
extern void DELETE_ARTI_HELP( ARTI_HELP* );

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* aus ADTTREE */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */


#ifdef __cplusplus
}
#endif /* __cplusplus */

extern TREE* CREATE_TREE( int, int );
extern TREE* CONCAT_TREES( TREE*, TREE* );
extern TREE* CONNECT_TREES( TREE*, TREE* );
extern TREE_LIST* ADD_TREE( TREE_LIST*, TREE* );
extern TREE_LIST* APPEND_TREE_LISTS( TREE_LIST*, TREE_LIST* );
extern void auswertung_TREE( TREE*, int );
extern void auswertung_TREE_LIST( TREE_LIST* );

/* aus PLANAR */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern PLANAR_GRAPH *PLANARISATION( UNDIR_GRAPH* );        

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* aus ADDCROSSINGS */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern DYN_GRAPH *ADD_CROSSINGS( PLANAR_GRAPH* );           

#ifdef __cplusplus
}
#endif /* __cplusplus */

extern auswertung_Al( DYN_GRAPH* );


/*-----------------------------------------------------------------------------------------*
 *  CREATE_APOINT erzeugt und initialisiert einen Artikulationspunkt.                      *
 *-----------------------------------------------------------------------------------------*/

ARTI_POINT*  CREATE_APOINT( int name )
{
    ARTI_POINT  *AP;
     
    
    //if (debug) printf( "Create APoint %d\n", name );

    AP = (ARTI_POINT*) psetmalloc( sizeof( ARTI_POINT ) );
    AP->name = name;
    AP->n    = 0;
    AP->flag = true;
    AP->BL   = nil;
    return( AP );
    
} /* CREATE_APOINT */



/*-----------------------------------------------------------------------------------------*
 *  CREATE_BICO erzeugt und initialisiert eine zweifach zusammenhngende Komponente        *
 *-----------------------------------------------------------------------------------------*/

BICO_COMP*  CREATE_BICO()
{
    BICO_COMP   *BC;
     
     
    //if (debug) printf( "Create Bico\n" ); 
     
    BC = (BICO_COMP*) psetmalloc( sizeof( BICO_COMP ) );
    BC->flag  = false;
    BC->UG    = nil;
    BC->PG    = nil;
    BC->DG    = nil;
    BC->n     = 0;
    BC->AL    = nil;
    BC->first = nil;
    BC->dummies = nil;
    BC->trees = nil;
    return( BC );
    
} /* CREATE_BICO */



/*-----------------------------------------------------------------------------------------*
 *  ADD_APOINT fgt in die Liste von Artikulationspunkten einer BICO_COMP einen neuen      *
 *  Artikulationspunkt ein.                                                                *
 *-----------------------------------------------------------------------------------------*/

void  ADD_APOINT( BICO_COMP *BC, ARTI_POINT *AP )
{
    ARTI_LIST   *newAL;
    
    
    //if (debug) printf( "    Add APoint %d to Bico\n", AP->name );
    
    newAL = (ARTI_LIST*) psetmalloc( sizeof( ARTI_LIST ) );
    newAL->AP   = AP;
    newAL->next = BC->AL;
    BC->AL = newAL;
    
} /* ADD_APOINT */



/*-----------------------------------------------------------------------------------------*
 *  ADD_BICO fgt in die Liste von zweifach zusammenhngenden Komponenten eines ARTI_POINT *
 *  eine neue zweifach zusammenhngende Komponente ein.                                    *
 *-----------------------------------------------------------------------------------------*/
 
void  ADD_BICO( ARTI_POINT *AP, BICO_COMP *BC )
{
    BICO_LIST   *newBL;
    
    
    //if (debug) printf( "    Add Bico to APoint %d\n", AP->name );
    
    newBL = (BICO_LIST*) psetmalloc( sizeof( BICO_LIST ) );
    newBL->BC   = BC;
    newBL->next = AP->BL;
    AP->BL = newBL;
    (AP->n)++;

} /* ADD_BICO */



/*-----------------------------------------------------------------------------------------*
 *  ADD_EDGE fgt eine neue Kante in einer zweifach zusammenhngenden Komponente ein.      *
 *-----------------------------------------------------------------------------------------*/
 
void  ADD_EDGE( BICO_COMP *BC, int V, int W )
{
    BC_EDGE         *newE;
    
    
    //if (debug) printf( "    Add Edge %d, %d\n", V, W );
    
    newE = (BC_EDGE*) psetmalloc( sizeof( BC_EDGE ) );
    newE->source = V;
    newE->dest   = W;
    newE->next   = BC->first;
    BC->first    = newE;
    
} /* ADD_EDGE */



/*-----------------------------------------------------------------------------------------*
 *   IS_DUMMY prueft, ob die Kante zwischen V und W eine dummy-Kante ist.                  *
 *-----------------------------------------------------------------------------------------*/
 
int  IS_DUMMY( BC_EDGE* dummies, int V, int W )
{
    BC_EDGE         *E;
     
    
    E = dummies;
    while ( E != nil  &&
            (E->source != V || E->dest != W)  &&
            (E->dest != V || E->source != W)  )
        E = E->next;
    return( E != nil );
    
} /* IS_DUMMY */
 


/*-----------------------------------------------------------------------------------------*
 *   ADD_DUMMY setzt die Kantezwischen V und W auf eine Dummy-Kante.                       *
 *-----------------------------------------------------------------------------------------*/
 
BC_EDGE*  ADD_DUMMY( BC_EDGE* dummies, int V, int W )
{
    BC_EDGE         *E;
     
    
    E = (BC_EDGE*) psetmalloc( sizeof( BC_EDGE ) );
    E->source = V;
    E->dest   = W;
    E->next   = dummies;
    dummies   = E;

    return( dummies );
    
} /* ADD_DUMMY */
 
 
 
/*-----------------------------------------------------------------------------------------*
 *   USED gibt an, ob eine Kante markiert ist.                                             *
 *-----------------------------------------------------------------------------------------*/    
    
void  SET_USED( int *used, int N, int V, int W )
{
    used[ (V - 1)*N + W - 1 ] = true;
    used[ (W - 1)*N + V - 1 ] = true;

} /* SET_USED */



int  USED( int *used, int N, int V, int W )
{
    return( used[ (V - 1)*N + W - 1 ] );

} /* USED */



int*  CREATE_USED( int N )
{
    int     i;
    int     *used;


    used = (int*) psetmalloc( N*N * sizeof( int ) );
    for ( i = 0; i < N*N; i++ ) used[ i ] = false;
    return( used );

} /* CREATE_USED */
    


/*------------------------------------------------------------------------------------------*
 *   UNION_BICO_COMPONENTS vereinigt zwei zweifache Zusammenhangskomponenten, die ber den  *
 *   Artikulationspunkt ARTI verbunden sind. Da durch die Planarisierung neue Knoten ent-   *
 *   standen sein knnen werden ab newNO die Nummern der neuen Knoten vergeben.             *
 *   Zuerst werden die Knotenlisten zusammengehngt, dabei wird die Umbenennung rckgngig  *
 *   gemacht und es werden die beiden Adjazenzlisten fr den Artikulationspunkt gesucht.    *
 *   Die Adjazenzlisten des Artikulationspunktes werden zusammengehngt.                    *
 *   Um die Vereinigung 'biconnected' zu machen wird noch eine Dummy-Kante eingefgt, die   *
 *   einen Nachbarn des Artikulationspunktes aus der einen Komponente mit einem Nachbarn    *
 *   aus der anderen Komponente verbindet.                                                  *
 *   Um zu verhindern, daa die Umbenennung mehrfach stattfindet, wird nach der ersten Um-   *
 *   benennung die name_table gelscht.                                                     *
 *------------------------------------------------------------------------------------------*/
 

BICO_COMP*  UNION_BICO_COMPONENTS( BICO_COMP *BC1, BICO_COMP *BC2, int ARTI, int *newNO )
{
    struct AL              *Al, *arti1, *arti2, *prevAl, *prev_of_arti2, *node1, *node2;
    DYN_NODE        *DN, *prevDN, *newDN;
    BC_EDGE         *E;
    
    
    //if (debug) printf( "Find ARTI in BC1\n" );

    prevAl = nil;                                                          /* arti1 suchen */
    Al = BC1->DG->first_AL;
    while ( Al != nil ) {
        if ( BC1->name_table != nil ) {
            if ( Al->name <= BC1->DG->old_V ) 
                Al->name = BC1->name_table[ Al->name ];
            else {
                (*newNO)++;
                Al->name = (*newNO);
            }
        }
        if ( Al->name == ARTI ) arti1 = Al;
        prevAl = Al;
        Al = Al->next_AL;
    }

    //if( debug) printf( "Find ARTI in BC2\n" );

    prevAl->next_AL = BC2->DG->first_AL;                    /* Knotenlisten zusammenhngen */
    Al = BC2->DG->first_AL;
    while ( Al != nil ) {                                                  /* arti2 suchen */
        if ( BC2->name_table != nil ) {    
            if ( Al->name <= BC2->DG->old_V )
                Al->name = BC2->name_table[ Al->name ];
            else {
                (*newNO)++;
                Al->name = (*newNO);
            }
        }
        if ( Al->name == ARTI ) {
            arti2 = Al;
            prev_of_arti2 = prevAl;
        }
        prevAl = Al;
        Al = Al->next_AL;
    }
    
    prevDN = nil;                    /* Adjazenzlisten des Aritkulationspunktes vereinigen */
    DN = arti1->first;
    while ( DN != nil ) {
        prevDN = DN;
        DN = DN->next;
    }
    prevDN->next = arti2->first;
    
    
    node1 = prevDN->name;                 /* Dummy-Kante zwischen node1 und node2 einggen */
    node2 = arti2->first->name;
    //if (debug) printf( "Add Dummy-Edge [ %d, %d ] \n", node1->name, node2->name );    

    prevDN = nil;                   /* node2 vor ARTI in Adjazenzliste von node1 eintragen */
    DN = node1->first;
    while ( DN->name->name != ARTI ) {
        prevDN = DN;
        DN = DN->next;
    }
    newDN = (DYN_NODE*) psetmalloc( sizeof( DYN_NODE ) );
    newDN->name = node2;
    newDN->flag = false;
    newDN->next = DN;
    if ( prevDN == nil ) 
        node1->first = newDN;
    else 
        prevDN->next = newDN;
        
    DN = node2->first;             /* node1 nach ARTI in Adjazenzliste von node2 eintragen */
    while ( DN->name->name != ARTI )
        DN = DN->next;
    newDN = (DYN_NODE*) psetmalloc( sizeof( DYN_NODE ) );
    newDN->name = node1;
    newDN->flag = false;
    newDN->next = DN->next;
    DN->next = newDN;

    E = (BC_EDGE*) psetmalloc( sizeof( BC_EDGE ) );                   /* Dummy-Kante eintragen */
    E->source = node1->name;
    E->dest   = node2->name;
    E->next   = BC1->dummies;
    BC1->dummies  = E;  
            
    while ( E->next != nil ) E = E->next;        /* Listen der Dummy-Kanten zusammenhngen */
    E->next = BC2->dummies;
    
    prev_of_arti2->next_AL = arti2->next_AL;         /* Zweites Auftreten von ARTI lschen */
    
    BC1->DG->old_V += BC2->DG->old_V - 1;
    BC1->DG->new_V += BC2->DG->new_V - 1;
    
    if ( BC1->name_table != nil ) {
        /* psetfree( BC1->name_table ); */
        BC1->name_table = nil;
    }
    return( BC1 );

} /* UNION_BICO_COMPONENTS */




/*-----------------------------------------------------------------------------------------*
 *  BUILD_UGRAPH trgt die Kanten aus BC_EDGE in den ungerichteten Graphen ein.            *
 *-----------------------------------------------------------------------------------------*/

void  BUILD_UGRAPH( BICO_COMP *BC, int max )
{
    int         *used;
    int         *inv_table;
    int         i, n;
    BC_EDGE     *E;
    
    
    //if (debug) printf( "    Build UGraph\n" );
    
    used = (int *) psetmalloc( (max+1) * sizeof( int ) );               /* used initialisieren */
    for ( i = 1; i <= max; i++ ) used[i] = false;
    
    for ( E = BC->first; E != nil; E = E->next ) {        /* Knoten aller Kanten eintragen */
        used[ E->source ] = true;
        used[ E->dest   ] = true;
    }
    
    n = 0;                                                                /* Knoten zhlen */
    for ( i = 1; i <= max; i++ ) if ( used[i] ) n++;
    BC->n = n;

    n = 0;                                                 /* Umsetzungstabellen erstellen */
    BC->name_table = (int *) psetmalloc( (BC->n+1) * sizeof( int ) );
    inv_table      = (int *) psetmalloc( (max+1) * sizeof( int ) );
    for ( i = 1; i <= max; i++ ) 
        if ( used[i] ) { 
            n++; 
            BC->name_table[ n ] = i;
            inv_table[ i ] = n;
        }
        
    BC->UG = CREATE_UGRAPH( BC->n );                     /* Ungerichteten Graphen erzeugen */
    for ( i = 1; i <= BC->n; i++ ) SET_UGRAPH_TRANS_NO( BC->UG, i, i );
    for ( E = BC->first; E != nil; E = E->next )               /* und die Kanten eintragen */
        SET_UEDGE( BC->UG, inv_table[ E->source ], 
                           inv_table[ E->dest   ] );
        
    /* psetfree( used );
       psetfree( inv_table ); */
            
} /* BUILD_UGRAPH */ 



/*-----------------------------------------------------------------------------------------*
 *  BUILD_BICO erzeugt den Baum aus Artikulationspunkten und zweifach zusammenhngenden    *
 *  Komponenten. Vorher mua mit DFS fr jeden Knoten dfnum und low bestimmt worden sein.   *
 *-----------------------------------------------------------------------------------------*/
 
void  BUILD_BICO( UNDIR_GRAPH *UG, ARTI_HELP *APinfo, int *used, BICO_COMP *BC, int V )
{
    ARTI_POINT      *AP;
    NODE            *help;
    BICO_COMP       *newBC;
    int             W;
    
    
    if ( GET_UNODE_FLAG( UG, V ) ) return;
    SET_UNODE_FLAG( UG, V, true );
    
    if ( IS_ARTI_POINT( APinfo, V ) ) {
        AP = CREATE_APOINT( V );
        ADD_BICO( AP, BC );
        ADD_APOINT( BC, AP );
    } else
        AP = nil;

    for ( W = GET_FIRST_UNODE( UG, V, &help ); W != 0; W = GET_NEXT_UNODE( &help ) ) {
        
        if ( ! USED( used, UG->n, V, W ) ) {
            SET_USED( used, UG->n, V, W );
            if ( GET_DFS( APinfo, W ) < GET_DFS( APinfo, V )  ||  
                 GET_LOW( APinfo, W ) < GET_DFS( APinfo, V )  ||
                 AP == nil ) 
            {
                ADD_EDGE( BC, V, W );
                BUILD_BICO( UG, APinfo, used, BC, W );
            } else {                                           /* V ist Artikulationspunkt */
                newBC = CREATE_BICO();
                ADD_APOINT( newBC, AP );
                ADD_BICO( AP, newBC );
                ADD_EDGE( newBC, V, W );
                BUILD_BICO( UG, APinfo, used, newBC, W );
                BUILD_UGRAPH( newBC, UG->n );
            }
        }   
    }   
        
} /* BUILD_BICO */
            


/*-----------------------------------------------------------------------------------------*
 *  GET_BICO_COMPONENTS zerteilt einen ungerichteten Graphen in seine zweifach zusammen-   *
 *  hngenden Komponenten.                                                                 *
 *-----------------------------------------------------------------------------------------*/
 
BICO_COMP*  SEARCH_BICO( BICO_COMP *BC )
{
    ARTI_LIST        *AL;
    BICO_LIST        *BL;

  
    if ( BC->n == nil  ||  BC->flag ) return( nil );
    BC->flag = true;

    if ( BC->n > 2 ) return( BC );

    for ( AL = BC->AL; AL != nil; AL = AL->next )
        for ( BL = AL->AP->BL; BL != nil; BL = BL->next ) {
            BC = SEARCH_BICO( BL->BC );
            if ( BC != nil  &&  BC->n > 2 ) return( BC );
	}

    return( nil );

} /* SEARCH_BICO */




BICO_COMP*  GET_BICO_COMPONENTS( UNDIR_GRAPH *UG )
{
    BICO_COMP       *BC; 
    BICO_LIST       *BL;
    ARTI_HELP       *APinfo;
    int             *used;
        
    
    BC     = CREATE_BICO();
    APinfo = CREATE_ARTI_HELP( UG );
    used   = CREATE_USED( UG->n );
    
    RESET_UNODE_FLAGS( UG );
    BUILD_BICO( UG, APinfo, used, BC, 1 );
    BUILD_UGRAPH( BC, UG->n );    
    if ( IS_ARTI_POINT( APinfo, 1 ) ) {                             /* Entferne erste BICO */
        BL = BC->AL->AP->BL;
        while ( BL->next->BC != BC ) BL = BL->next;
        /* psetfree( BL->next ); */
        BL->next = nil;
        BC = BC->AL->AP->BL->BC;
    }
                  
    DELETE_ARTI_HELP( APinfo );
    /* psetfree( used ); */
    BC = SEARCH_BICO( BC );

/*
    if (debug) {
        printf( "\n\n*****************AUSWERTUNG*******************\n\n" );
        int i = 1; 
        auswertung_BICO( BC, &i );  
    }
*/
  
    return( BC );
        
} /* GET_BICO_COMPONENTS */



/*-----------------------------------------------------------------------------------------*
 *   PLANAR_BICO_COMPONENTS lat alle zweifachen Zusammenhangskomponenten planarisieren    *
 *   und vereinigt sie dann zu einer. BC zeigt als Eingabe auf eine Baumstruktur von       *
 *   zweifachen Zusammenhangskomponenten und Artikulationspunkten, als Ausgabe auf eine    *
 *   einzige zZ, deren Kreuzungsknoten ab newNO durchnummeriert werden.                    *
 *-----------------------------------------------------------------------------------------*/

DYN_GRAPH*  TWO_NODES_DYNGRAPH()
{
    struct AL   *Al1, *Al2;
    DYN_NODE    *DN;
    DYN_GRAPH   *DG;


    DG = (DYN_GRAPH*) psetmalloc( sizeof( DYN_GRAPH ) );
    DG->old_V = DG->new_V = 2;

    Al1 = (struct AL*) psetmalloc( sizeof(struct AL ) );
    Al2 = (struct AL*) psetmalloc( sizeof(struct AL ) );

    Al1->name = 1;
    Al1->flag = false;
    Al1->first = nil;
    Al1->next_AL = Al2;
    Al2->name = 2;
    Al2->flag = false;
    Al2->first = nil;
    Al2->next_AL = nil;
    DN = (DYN_NODE*) psetmalloc( sizeof( DYN_NODE ) );
    DN->name = Al2;
    DN->flag = false;
    DN->next = nil;
    Al1->first = DN;
    DN = (DYN_NODE*) psetmalloc( sizeof( DYN_NODE ) );
    DN->name = Al1;
    DN->flag = false;
    DN->next = nil;
    Al2->first = DN;
    DG->first_AL = Al1;

    return( DG );

} /* TWO_NODES_DYNGRAPH */
    



int  PLANAR_BICO_COMPONENTS( BICO_COMP *BC, int *newNO )
{
    ARTI_LIST       *AL;
    BICO_LIST       *BL;
    TREE            *APTree, *t;
    ARTI_POINT      *Arti;
    int             nBicos;



    if ( BC->UG->n > 2 ) {
        BC->PG = PLANARISATION( BC->UG );
        BC->DG = ADD_CROSSINGS( BC->PG );
    } else 
        BC->DG = TWO_NODES_DYNGRAPH();

    nBicos = 0;
    APTree = nil;
    Arti   = nil;
    for ( AL = BC->AL; AL != nil; AL = AL->next ) {
        if ( ! AL->AP->flag ) Arti = AL->AP;
        else {
            //if (debug) printf( "ARTICULATION POINT: %d\n", AL->AP->name );
            AL->AP->flag = false;
            APTree = nil;
            for ( BL = AL->AP->BL; BL != nil; BL = BL->next ) 
                if ( BL->BC != BC ) {
                    if ( PLANAR_BICO_COMPONENTS( BL->BC, newNO ) ) {
                        if ( BL->BC->trees != nil ) 
                           APTree = CONNECT_TREES( APTree, BL->BC->trees->t );
                    } else
                        nBicos++;
		}
            BC->trees = ADD_TREE( BC->trees, APTree );
	}
    }
    //if (debug) auswertung_TREE_LIST( BC->trees );

                     
    if ( BC->n == 2  &&  nBicos == 0 ) {
        
        if ( Arti->name == BC->name_table[1] )
             t = CREATE_TREE( BC->name_table[1], BC->name_table[2] );
        else t = CREATE_TREE( BC->name_table[2], BC->name_table[1] );

        if ( APTree == nil )
             BC->trees = ADD_TREE( BC->trees, t );
        else BC->trees->t = CONCAT_TREES( t, BC->trees->t );

        BC->DG = nil;

        //if (debug) auswertung_TREE_LIST( BC->trees );

        return( true );

    } else {
        
        for ( AL = BC->AL; AL != nil; AL = AL->next )
            if ( AL->AP != Arti )
                for ( BL = AL->AP->BL; BL != nil; BL = BL->next )
                    if ( BL->BC->DG != nil  &&  BL->BC != BC ) {
                        BC = UNION_BICO_COMPONENTS( BC, BL->BC, AL->AP->name, newNO );
                        BC->trees = APPEND_TREE_LISTS( BC->trees, BL->BC->trees );
		    }
                
/*
        if (debug) {
            auswertung_Al( BC->DG );
            auswertung_TREE_LIST( BC->trees );
	}
*/

        return( false );    
        
    }

} /* PLANAR_BICO_COMPONENTS */




/* END BICO.C */


