
/******************************************************************************/
/*                                                                            */
/* Abstrakter Datentyp fuer UNGERICHTETE GRAPHEN                              */
/*                                                                            */
/* Autor               Martin Schmidt, Martin-Luther-Str.1, 7630 Lahr         */
/*                     Tel.: 07821 / 41748                                    */
/*                                                                            */
/* Letzte Aenderung    21.01.88                                               */
/*                                                                            */
/*                                                                            */
/* Datentyp            UNDIR_GRAPH   ( Beschreibung siehe ADTUGRAPH.STC )     */
/*                                                                            */
/* Operatoren                                                                 */
/*                                                                            */
/*   G = CREATE_UGRAPH( N )           Erzeuge einen ungerichteten Graphen mit */
/*                                    N Knoten. Es gibt noch keine Kanten.    */
/*                                                                            */
/*   SG = CREATE_SUB_UGRAPH( G,flag ) Erzeuge aus den mit flag markierten     */
/*                                    Knoten eines Graphen einen Untergraphen.*/
/*                                                                            */
/*   void DELETE_UGRAPH( G )          Loesche den ungerichteten Graphen.      */
/*                                                                            */
/*   void DELETE_UEGE( G, x, y )      Loesche die Kante zwischen den Knoten   */
/*                                    x und y.                                */
/*                                                                            */
/*   void SET_UEGE( G, x, y )         Setze eine Kante zwischen die Knoten    */
/*                                    x und y.                                */
/*                                                                            */
/*   test = IS_UEGE(  G, x, y )       Befindet sich eine Kante zwischen       */
/*                                    Knoten x und y ? ( true oder false )    */
/*                                                                            */
/*   y = GET_FIRST_NODE( G,x,&help )  Bestimme den ersten Knoten in der       */
/*                                    Adjazenzliste des Knoten x.             */
/*                                                                            */
/*   y = GET_NEXT_NODE( &help )       Bestimme den naechsten Knoten in der    */
/*                                    aktuellen Adjazenzliste.                */
/*                                                                            */
/*   void SET_UNODE_FLAG( G,x,wert )  Setze ein Flag mit Wert wert fuer den   */
/*                                    Knoten x.                               */
/*                                                                            */
/*   void RESET_UNODE_FLAGS(  G )     Setze alle Knotenflags auf 0.           */
/*                                                                            */
/*   flag = GET_UNODE_FLAG(  G, x )   Welchen Wert hat das Flag des Knoten x? */
/*                                                                            */
/*   no = GET_UGRAPH_TRANS_NO( G, i ) Lese einen Wert in der Umsetzungstab.   */
/*                                                                            */
/*   void SET_UGRAPH_TRANS_NO( G,i,w) Setze einen Wert in der Umsetzungstab.  */
/*                                                                            */
/*   G = READ_UGRAPH( )               Lese einen ungerichteten Graphen ein.   */
/*                                                                            */
/******************************************************************************/


#include  <stdio.h>  
#include  <psetmem.h>

#include  "const.def"             	/* Definition von Konstanten wie
                                     	   true, false und nil.
                                  	*/
#include  "adtugraph.stc"         	/* Strukturdefinition
                                     	   UNDIR_GRAPH.
                                  	*/

#include  "adtugraph.tst"


/* Erzeuge einen Graphen G mit N Knoten und 0 Kanten.
*/

UNDIR_GRAPH *CREATE_UGRAPH ( N )
 
  int    	N;                      /* Anzahl der Knoten des Graphen      */
{
  UNDIR_GRAPH 	*G;                 	/* Pointer auf ungerichteten Graph    */
  int         	i;               	/* Laufvariable                       */

  if ( N==0 ) G = nil;
  else 
    {
     G = (UNDIR_GRAPH *)psetmalloc( sizeof( UNDIR_GRAPH ) );
     /* Knotenzahl */
     G->n = N;  
     /* Adjazenzlisten erzeugen */

     G->first = (NODE ***)psetmalloc( G->n * sizeof(NODE*) );
     for ( i=0; i < G->n; i++ ) (*G->first)[i] = nil;    
     /* Transformationstabelle */
     G->trans  = (int  (*)[])psetmalloc( G->n * sizeof( int ) );
     /* Flags fuer Algorithmen */
     G->flags  = (int  (*)[])psetmalloc( G->n * sizeof( int ) ); 
    }

  return( G );

} /* end of CREATE_UGRAPH */




/* Erzeuge aus einen gegeben Graphen einen Untergraphen.
   Untergraph = alle Knoten, die mit flag markiert sind.
*/

UNDIR_GRAPH *CREATE_SUB_UGRAPH( G, flag )

  UNDIR_GRAPH  	*G;              	/* Zeiger auf einen Graphen           */
  int         	flag;            	/* Knoten mit Flagwert flag werden    */
                                   	/* zusammengefasst.                   */
{
  int 	GET_UNODE_FLAG();         	/* Lese das Flag eines Knoten         */
  void	SET_UEDGE();              	/* Setze eine Kante zwischen 2 Knoten */
  int  	GET_UGRAPH_TRANS_NO();          /* Lese in der trans-Tabelle          */
  void 	SET_UGRAPH_TRANS_NO();          /* Schreibe in die trans-Tabelle      */
  int  	GET_FIRST_UNODE();              /* erste Kante von x aus              */
  int  	GET_NEXT_UNODE();               /* naechste Kante von x aus           */
  UNDIR_GRAPH *CREATE_UGRAPH();		/* Erzeuge einen leeren Graphen       */

  UNDIR_GRAPH 	*SG;               	/* Zeiger auf einen Untergraphen      */
  NODE          *help;                  /* intern                             */
  int  		i,j, x,y;               /* Laufvariable                       */
  int  		m;                      /* Dimension des Untergraphen         */
  int           (*tab)[];               /* Umsetzungstabelle                  */

  /* Bestimme die Dimension des Untergraphen */
  m = 0;
  for ( i=1; i<=G->n; i++ ) if ( GET_UNODE_FLAG( G, i ) == flag ) m++; 

  /* Erzeuge eine Umsetzungstabelle */
  tab = (int (*)[]) psetmalloc( G->n * sizeof( int ) );

  x = 0;
  for ( i=1; i<=G->n; i++ ) if ( GET_UNODE_FLAG( G, i ) == flag )
      { x++; (*tab)[i-1] = x; }

  /* Erzeuge einen leeren Untergraphen der Dimension m ohne Kanten */
  SG = CREATE_UGRAPH( m );

  /* Trage alle Kanten des Untergraphen ein */
  x = 1;
  /* Fuer jede Adjazenzliste */
  for( j=1; j<=G->n; j++ )          	
     if ( GET_UNODE_FLAG( G, j ) == flag )
        {
           /* Transformationsinformation umspeichern */
           SET_UGRAPH_TRANS_NO( SG, x, GET_UGRAPH_TRANS_NO( G, j ) ); 

           y = GET_FIRST_UNODE( G, j, &help );
           while( y != 0 )
                {
                  if ( GET_UNODE_FLAG( G, y ) == flag  &&  x < y )
                       SET_UEDGE( SG, x, (*tab)[y-1] );
                  y = GET_NEXT_UNODE( &help );
                }

           x++;
         }

  free( tab ); 		               /* Umsetzungstabelle wieder freigeben  */

  return( SG );

} /* end of CREATE_SUB_UGRAPH */





/* Gebe den Speicherplatz des ungerichteten Graphen G wieder frei.
*/

void DELETE_UGRAPH ( G )

  UNDIR_GRAPH  	*G;                     /* Zeiger auf einen Graphen           */
{
  int    	i;              	/* Laufvariable                       */
  NODE          *help1, *help2;

  /* Adjazenzlisten freigeben */
  for ( i=0; i<G->n; i++ )
      { help1 = (*G->first)[ i ];
        while ( help1 != nil )
              { help2 = help1->next;
                free( help1 );
                help1 = help2;
              }
      }

  free( G->first );
  free( G->flags );        		/* Flagtabelle freigeben              */
  free( G        );                     /* Graph freigeben                    */

} /* end of DELETE_UGRAPH */




/* Loesche die Kante zwischen Knoten x und y im Graphen G.
   ( Vorbedingung : Die Kante muss existieren. )
*/

void DELETE_UEDGE ( G, x, y )

  UNDIR_GRAPH  	*G;        		/* Zeiger auf einen Graphen           */
  int          	x, y;                   /* Knotennummern                      */
{
  NODE          *help1, *help2;

  help1 = (*G->first)[x-1];
  help2 = nil;

  while ( help1 != nil && help1->name != y ) 
        { help2 = help1; help1 = help1->next; }

  if ( help2 == nil ) (*G->first)[ x-1 ] = help1->next;
  else                help2->next = help1->next;

  free( help1 );
 
} /* end of DELETE_UEDGE */



/* Setze eine Kante zwischen Knoten x und y im Graphen G.
   ( Vorbedingung : Die Kante (x,y) darf vorher nicht existieren. )
*/

void SET_UEDGE ( G, x, y )

  UNDIR_GRAPH  	*G;             	/* Zeiger auf einen Graphen          */ 
  int          	x, y;                   /* Knotennummern                     */
{
  NODE          *newNODE;

  /* Trage die Kante in beide Adjazenzlisten ein. */
  newNODE             = (NODE *) psetmalloc( sizeof( NODE ) );
  newNODE->name       = y;
  newNODE->next       = (*G->first)[ x-1 ];
  (*G->first)[ x-1 ]  = newNODE;
  newNODE             = (NODE *) psetmalloc( sizeof( NODE ) );
  newNODE->name       = x;
  newNODE->next       = (*G->first)[ y-1 ];
  (*G->first)[ y-1 ]  = newNODE;
  
} /* end of SET_UEDGE */




/* Begfindet sich eine Kante zwischen Knoten x und y im Graphen G ?
*/

int IS_UEDGE ( G, x, y )

  UNDIR_GRAPH  	*G;             	/* Zeiger auf einen Graphen           */
  int           x, y;                   /* Knotennummern                      */
{
  NODE          *help;

  help = (*G->first)[x-1];

  while ( help != nil && help->name != y ) help = help->next;

  if ( help != nil ) return( true  );
  else               return( false ); 

} /* end of IS_UEDGE */





/* Bestimme den ersten Nachbarn des Knoten x.
   Wenn von x keine Kante weggeht, dann wird 0 zurueckgegeben.
   ( Der helpPointer wird in der Funktion auf die erste Kante gesetzt.
     Wenn in der Prozedur GET_NEXT_UNODE die naechste Kante gesucht wird,
     dann wird dieser Pointer ausgenutzt. )
*/

int GET_FIRST_UNODE( G, x, helpPointer )

  UNDIR_GRAPH	*G;			/* Zeiger auf einen Graphen           */
  int           x;                      /* Knotennummer                       */
  NODE          **helpPointer;          /* Verwaltungsunterstuetzung          */
{
  *helpPointer = (*G->first)[x-1];

  if ( *helpPointer == nil ) return( 0 );
  else                       return( (*helpPointer)->name );

} /* end of GET_FIRST_UNODE */




/* Bestimme den naechsten Nachbarknoten des Knoten x.
   Wenn es keinen Nachbarn mehr gibt, dann wird 0 zurueckgegeben.
   ( helpPointer : siehe GET_FIRST_UEDGE. )
*/

int GET_NEXT_UNODE( helpPointer )

  NODE          **helpPointer;          /* Verwaltungsunterstuetzung          */
{
  *helpPointer = (*helpPointer)->next;

  if ( *helpPointer == nil ) return( 0 );
  else                       return( (*helpPointer)->name );

} /* end of GET_NEXT_UNODE */






/* Setze das Knotenflag fuer den Koten x auf den Wert wert.
*/

void SET_UNODE_FLAG ( G, x, wert )

  UNDIR_GRAPH  	*G;                 	/* Zeiger auf einen Graphen           */
  int          	x;                      /* Knotennummmer                      */
  int         	wert;                   /* Flagwert                           */
{
   (*G->flags)[ x-1 ] = wert;

} /* end of SET_UNODE_FLAG */





/* Setze alle Knotenflags des Graphen G auf 0 .
*/

void RESET_UNODE_FLAGS( G )

  UNDIR_GRAPH 	*G;    			/* Zeiger auf einen Graphen           */
{
  void SET_UNODE_FLAG();        	/* Setze ein Flag fuer einen Knoten   */

  int  i;                       	/* Laufvariable                       */

  for( i=1; i<=G->n; i++ ) SET_UNODE_FLAG( G, i, 0 );

} /* end of RESET_UNDOE_FLAGS */




/* Lese das Knotenflag fuer den Knoten x des Graphen G.
*/

int GET_UNODE_FLAG ( G, x )

  UNDIR_GRAPH  *G;                    	/* Zeiger auf einen Graphen           */
  int          x;                       /* Knotennummer                       */
{
  return( (*G->flags)[ x-1 ] );

} /* end of GET_UNODE_FLAG */




/* Bestimme den i-ten Eintrag in der Transformationstabelle .
*/

int GET_UGRAPH_TRANS_NO( G, i )

  UNDIR_GRAPH 	*G;             	/* Zeiger auf einen Graphen           */
  int         	i;                      /* Index in der Tabelle               */
{
  return( (*G->trans)[ i-1 ] );

} /* end of GET_UGRAPH_TRANS_NO */




/* Setze den i-ten Eintrag in der Transformationstabelle auf den Wert wert.
*/

void SET_UGRAPH_TRANS_NO( G, i, wert )

  UNDIR_GRAPH 	*G;  			/* Zeiger auf einen Graphen   	      */
  int         	i;                      /* Index in der Tabelle               */
  int         	wert; 			/* Wert des Eintrages     	      */
{
  (*G->trans)[ i-1 ] = wert;

} /* end of SET_UGRAPH_TRANS_NO */




/* Lese einen ungerichteten Graphen ein.
   Eingelesen wird vom Standardinput.
   Format : 1.Zeile  Anzahl der Knoten 
            2.Zeile  1, 2,3,4          * Knoten 1 ist mit 2,3,4 verbunden *
            ...usw 
            n.Zeile  00                * Ende der Datei                   *
*/

UNDIR_GRAPH *READ_UGRAPH ( )

{
  UNDIR_GRAPH *CREATE_UGRAPH();		/* Erzeuge einen leeren Graphen       */
  void SET_UEDGE();                     /* Setze die Kante (x,y)              */
  void SET_UGRAPH_TRANS_NO();           /* Transformationstabelle             */

  UNDIR_GRAPH  *G;                   	/* Zeiger auf einen Graphen           */

  int  n;                            	/* Anzahl der Knoten */
  int  i;
  int  a[24];
  
  /* Dimensiom des Graphen einlesen . */
  scanf( "%d", &n );

  G = CREATE_UGRAPH( n );

  /* Queerverweistabelle erzeugen */
  for( i=1; i<=G->n; i++ ) SET_UGRAPH_TRANS_NO( G, i, i ); 

  do {
      for( i=1; i<=11; a[i++]=0 );
    scanf( "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                 &a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7],&a[8],&a[9],&a[10],
                 &a[11],&a[12],&a[13],&a[14],&a[15],&a[16],&a[17],&a[18],
                 &a[19],&a[20],&a[21],&a[22] );

      i=2;
      if ( a[1] != 0 )
        while ( i <= 22  && a[i]!=0 )
            {
               if ( IS_UEDGE( G, a[1], a[i] ) == false )
                  SET_UEDGE( G, a[1], a[i] );
               i++;
            }

    } while ( a[1] != 0 );

  return( G );

} /* end READ_UGRAPH */





/* end of ADTUGRAPH.C */


