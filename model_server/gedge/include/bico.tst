#ifndef _BICO_TST
#define _BICO_TST

/*------------------------------------------------------------------------------------------
  
    BICO.TST
     
    Autor      : Bernd Rumscheid
    Date       : 14.01.89
    nderungen : 

    Testausgabe fr BICO.C

 ------------------------------------------------------------------------------------------*/
 
#include <stdio.h>


/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void AUSWERTUNG_UNDIR_GRAPH( UNDIR_GRAPH* );

#ifdef __cplusplus
}
#endif /* __cplusplus */



void  print_BICO( BICO_COMP *BC )
{
    ARTI_LIST       *AL;
    int             i;
        
    
    if ( BC == nil ) return;

    AUSWERTUNG_UNDIR_GRAPH( BC->UG );

    if ( BC->name_table != nil ) 
        for ( i = 1; i <= BC->UG->n; i++ )
            printf( "name[ %d ] = %d\n", i, BC->name_table[i] );
    else printf( "Keine Namentabelle !\n" );

    printf( "Artikulationspunkte : " );
    for ( AL = BC->AL; AL != nil; AL = AL->next ) 
        printf( "%d ", AL->AP->name );
    printf( "\n" );

}       


extern void auswertung_BICO_COMP( BICO_COMP*, ARTI_POINT*, int* );
	



void  auswertung_ARTI_POINT( ARTI_POINT *AP, BICO_COMP *father, int *no )
{
    BICO_LIST       *BL;


    if ( AP == nil ) return;

    printf( "ARTI_POINT: %d, flag %d\n", AP->name, AP->flag );

    for ( BL = AP->BL; BL != nil; BL = BL->next ) 
        if (BL->BC != father) auswertung_BICO_COMP( BL->BC, AP, no );

}




void  auswertung_BICO_COMP( BICO_COMP *BC, ARTI_POINT *father, int *no )
{
    ARTI_LIST       *AL;
    
    
    if ( BC == nil ) return;
    BC->flag = false;
    
    printf( "\nNummer %d********************\n", *no );
    (*no)++;
    print_BICO( BC );
    for ( AL = BC->AL; AL != nil; AL = AL->next ) 
        if (AL->AP != father) auswertung_ARTI_POINT( AL->AP, BC, no );
    
} /* auswertung_BICO_COMP */





void  auswertung_BICO( BICO_COMP *BC, int *no )
{

    auswertung_BICO_COMP( BC, nil, no );

}     

#endif

        
        






