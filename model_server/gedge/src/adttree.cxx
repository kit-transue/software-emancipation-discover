/*------------------------------------------------------------------------------------------

    ADTTREE.C

    Autor      : Bernd Rumscheid
    Date       : 18.01.89
    nderungen :

 ------------------------------------------------------------------------------------------*/

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std{};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <psetmem.h>
#include "const.def"
#include "adttree.stc"



TREE*  CREATE_TREE( int root, int leaf )
{
    TREE        *t1, *t2;


    t1 = (TREE*) psetmalloc( sizeof( TREE ) );
    t1->name = root;
    t1->x = t1->y = 0;
    t1->next = nil;
    t2 = (TREE*) psetmalloc( sizeof( TREE ) );
    t1->succ = t2;
    t2->name = leaf;
    t2->x = t2->y = 0;
    t2->next = nil;
    t2->succ = nil;
    return( t1 );

} /* CREATE_TREE */



TREE*  CONCAT_TREES( TREE *t1, TREE *t2 )
{
    if ( t1 == nil ) return( t2 );
    if ( t2 == nil ) return( t1 );
    t1->succ->succ = t2->succ;
    return( t1 );

} /* CONCAT_TREES */



TREE*  CONNECT_TREES( TREE *t1, TREE *t2 )
{
    TREE        *t;


    if ( t1 == nil ) return( t2 );
    if ( t2 == nil ) return( t1 );
    for ( t = t1->succ; t->next != nil; t = t->next ) ;
    t->next = t2->succ;
    return( t1 );

} /* CONNECT_TREES */



TREE_LIST*  ADD_TREE( TREE_LIST *TL, TREE* t )
{
    TREE_LIST       *newTL;


    if ( t == nil ) return( TL );
    newTL = (TREE_LIST*) psetmalloc( sizeof( TREE_LIST ) );
    newTL->t = t;
    newTL->next = TL;
    return( newTL );

} /* ADD_TREE */



TREE_LIST*  APPEND_TREE_LISTS( TREE_LIST *TL1, TREE_LIST *TL2 )
{
    TREE_LIST       *TL;


    if ( TL2 == nil ) return( TL1 );
    if ( TL1 == nil ) return( TL2 );
    for ( TL = TL1; TL->next != nil; TL = TL->next ) ;
    TL->next = TL2;
    return( TL1 );

} /* APPEND_TREE_LISTS */
        


TREE_LIST*  FIND_TREE( TREE_LIST *TL, int rootName )
{
    TREE_LIST    *t;
 
    
    for ( t = TL; t != nil  &&  t->t->name != rootName; t = t->next );
    return( t );

} /* FIND_TREE */



void  auswertung_TREE( TREE *t, int level )
{
    if ( t == nil ) return;
    printf( "%d: %d\n", level, t->name );
    auswertung_TREE( t->succ, level + 1 );
    auswertung_TREE( t->next, level ); 

} /* auswertung_TREE */




void  auswertung_TREE_LIST( TREE_LIST *TL )
{
    printf( "TREES:\n" );
    while ( TL != nil ) {
        auswertung_TREE( TL->t, 1 );
        TL = TL->next;
    }

} /* auswertung_TREE */




/* END ADTTREE.C */
