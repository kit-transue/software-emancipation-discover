#ifndef _STNUM_TST
#define _STNUM_TST

/*-------------------------------------------------------------------------------------------

    ST_NUM.TST

    Autor       : Bernd Rumscheid
    Datum       : 14.12.88
    nderungen  : 01.01.89

 -------------------------------------------------------------------------------------------*/

#include <stdio.h>

    
/*------------------------------------------------------------------------------------------*
 *   auswertung_SG  gibt den ST_GRAPHEN auf dem Bildschirm aus                              *
 *------------------------------------------------------------------------------------------*/
 
 void  auswertung_SG( ST_GRAPH *SG )
 {
    ST_NODE     *SN;
    AL_NODE     *AN;
    
    
    printf( "\n" );
    printf( "Ausgabe des ST_GRAPHS:\n" );
    printf( "\n" );
    printf( "Der Graph hat %d Knoten\n", SG->V );
    printf( "\n" );
    printf( "Adjazenzlisten:\n" );

    
    for ( SN = SG->first; SN != nil; SN = SN->next ) {

        printf( "%d: ", SN->name );
        printf( " dfnum: %d\n", SN->dfnum );
        printf( "    stnum: %d\n", SN->stnum );
        printf( "    low  : %d\n", SN->low   );

        printf( "    adj  :" );
        for ( AN = SN->adj; AN != nil; AN = AN->next )
            printf( " %d ", AN->pnode->name );
        printf( "\n" ); 

        printf( "    from :" );
        for ( AN = SN->from; AN != nil; AN = AN->next )
            printf( " %d ", AN->pnode->name );
        printf( "\n" ); 

        printf( "    to   :" );
        for ( AN = SN->to; AN != nil; AN = AN->next )
            printf( " %d ", AN->pnode->name );
        printf( "\n" );         
    }
    
    printf( "\n" );
    
} /* auswertung_SG */



void PRINT_USED( ST_GRAPH *SG )
{
    int     i, j;

    for ( i = 0; i < SG->V; i++ ) {
        for ( j = 0; j < SG->V; j++ )
            printf( "%d", SG->used[ i * SG->V + j ] );
        printf( "\n" );
    }
} /* PRINT_USED */



void  PRINT_STACK( AL_NODE *Stack )
{
    AL_NODE     *AN;


    printf( "Stack: " );
    AN = Stack;
    while ( AN != nil ) {
        printf( " %d", AN->pnode->name );
        AN = AN->next;
    }
    printf( "\n" );

} /* PRINT_STACK */



/* END ST_NUM.TST */


#endif
