
/******************************************************************************/
/*									      */
/* Bestimmung der ARTIKULATIONSPUNKTE eines Graphen ( nach Hopcroft )         */
/*									      */
/*									      */
/* Autor		Martin Schmidt, Martin-Luther-Str. 1, 7630 Lahr       */
/*			Tel.: 07821 / 41748				      */
/*									      */
/* Letzte Aenderung     07.01.88					      */
/*									      */
/*									      */
/* Literatur            Alfred v. AHO, John E. HOPCROFT, Jeffrey D. Ullmann   */
/*			Data Structures and Algorithms.                       */
/*                      Addison-Wesley Publishing Company 1983                */
/*									      */
/*									      */
/* Vorgehen :  1. APinfo = CREATE_ARTI_HELP( G )                              */
/*                ( Struktur von APinfo : siehe HOPCROFT.STC )                */
/*                Erzeugt 3 Tabellen   - Tiefennummern von G                  */
/*				       - LOW-Funktionswerte                   */
/*				       - Entscheidungstabelle fuer Arti.Pu.   */
/*									      */
/*	       Es stehen dann folgenden Funktionen zur Verfuegung             */
/*									      */
/*             DFS_no = GET_DFS( APinfo, x )     			      */
/*                      Bestimme die Tiefennummer des Knoten x                */
/*									      */
/*             LOW_no = GET_LOW( APinfo, x )				      */
/*			Bestimme die LOW-Nummer des Knoten mit der Tiefennr.x */
/*									      */
/*             test   = IS_ARTI_POINT( APinfo, x )                            */
/*			Ist der Knoten mit der Tiefennummer x ein Arti.Pu. ?  */
/*									      */
/*             void     DELETE_ARTI_HELP( APinfo )                            */
/*			Gebe den Speicherplatz der Tabellen wieder frei.      */
/*									      */
/******************************************************************************/



#include  <stdio.h>  
#include  <psetmem.h>

#include  "const.def"             	/* Definition von Konstanten wie
                                     	   true, false und nil.
                                  	*/
#include  "adtugraph.stc"         	/* Strukturvereinbarung
                                     	   UNDIR_GRAPH.
                                  	*/
#include  "hopcroft.stc"          	/* Strukturvereinbarung
					   ARTI_HELP.
                                  	*/




/* Bestimme alle Artikulationspunkte eines Graphen.
   bzw. Bestimme die Tiefennummerierung und die Low-Funktion des Graphen.
   Es gilt dann :
   a) Die Wurzel ist ein Artikulationspunkt, wenn sie mehr als einen
      gerichteten Nachfolger hat.
   b) Andere Knoten v sind Artikulationspunkte, wenn sie mindestens
      ein gerichteten Nachfolger w haben mit  LOW(W)>=DFS(v).
*/

ARTI_HELP *CREATE_ARTI_HELP( G  )

  UNDIR_GRAPH    *G;                 	/* Zeiger auf einen Graphen           */
{
                                     	/* Modul  ADTUGRAPH                   */
  extern void RESET_UNODE_FLAGS();   	/* Setzte Knotenmarkierungen auf Null */

  void REKURSION_ARTI_HELP();          	/* Rekursiver Aufruf                  */

  ARTI_HELP     *APinfo;
  int           marke;
  int		i;			/* Laufvariable                       */

  APinfo      = (ARTI_HELP *)psetmalloc( sizeof( ARTI_HELP ) );
  APinfo->DFS = (int (*)[])psetmalloc( G->n * sizeof( int ) );
  APinfo->LOW = (int (*)[])psetmalloc( G->n * sizeof( int ) );
  APinfo->AP  = (int (*)[])psetmalloc( G->n * sizeof( int ) );

  for ( i=1; i<=G->n; i++ ) (*APinfo->AP)[i-1] = 0;

  RESET_UNODE_FLAGS( G );            	/* Alle Knoten noch unbearbeitet     */

  marke = 1;
  REKURSION_ARTI_HELP( G, APinfo, 1, 0, &marke );

  return( APinfo );

} /* end of CREATE_ARTI_HELP */ 




/* Forstsetzung von CREATE_ARTI_HELP mit Hilfe einer rekursiven Funktion.
*/

void REKURSION_ARTI_HELP( G, APinfo, x, fatherx, marke )

  UNDIR_GRAPH 	*G;         		/* Zeiger auf einen Graphen           */
  ARTI_HELP     *APinfo;		/* Funktionstabellen                  */
  int  		x;                     	/* Knoten wird als naechstes markiert */
  int  		fatherx;               	/* Vater des Knoten x                 */
  int  		*marke;                	/* Marke die vergeben wird.           */
{
                                   	/* Modul  ADTUGRAPH.C                 */
  extern void SET_UNODE_FLAG();    	/* Setze eine Knotenmarkierung        */
  extern int  GET_UNODE_FLAG();    	/* Lese eine Knotenmarkierung         */
  extern int  GET_FIRST_UNODE(); 	/* Bestimme ersten Nachbarknoten      */
  extern int  GET_NEXT_UNODE();      	/* Bestimme naechsten Nachbarknoten   */

  int  		low;                    /* Zwischenergebnis von LOW           */
  int  		i;                      /* Laufvariable                       */
  int           s;                      /* Zaehle die gerichteten Nachfolger  */
  NODE          *help;			/* Hilfszeiger                        */

  low = *marke;

  SET_UNODE_FLAG( G, x, 1 );       	/* Markiere Knoten x als besucht      */
  (*APinfo->DFS)[ x-1 ] = *marke;       /* Tiefennummer fuer Knoten x setzen  */
  s = 0;

  i = GET_FIRST_UNODE( G, x, &help );
  while ( i!=0 )
        {
          if ( GET_UNODE_FLAG( G, i ) == 0 )
             { /* Knoten i noch nicht markiert */
               s++;			/* Zaehle die gerichteten Nachfolger  */
               (*marke)++;            
               REKURSION_ARTI_HELP( G, APinfo, i, x, marke );
               if ( GET_LOW( APinfo, i ) < low ) low = GET_LOW( APinfo, i ); 
               
               /* Pruefe ob es sich um einen Artikulationspunkt handelt */
               if (  ( x==1  &&  s>1 )    ||
                     ( x!=1  && GET_LOW( APinfo, i ) >= GET_DFS( APinfo, x ) ))
                  (*APinfo->AP)[ x-1 ] = 1;
             }
          else
             { /* Knoten i schon besucht */
               if ( i!=fatherx && GET_DFS( APinfo,i ) < GET_DFS( APinfo,x ) &&
                    GET_DFS( APinfo, i )<low )
                  low = GET_DFS( APinfo, i );
             }
          i = GET_NEXT_UNODE( &help );
        }

  (*APinfo->LOW)[x-1] = low;           /* Entgueltiger LOWwert setzen         */

} /* end of REKURSION_ARTI_HELP */





/* Gebe den Speicherplatz der Artikulationspunkte-Hilfsstruktur frei.
*/

void DELETE_ARTI_HELP( APinfo )

  ARTI_HELP	*APinfo;
{
  free( APinfo->DFS );
  free( APinfo->LOW );
  free( APinfo->AP  );

  free( APinfo );

} /* end of DELETE_ARTI_HELP */





/* Bestimme die Tiefennummer des Knoten x.
*/

int GET_DFS( APinfo, x )

  ARTI_HELP	*APinfo;
  int  		x;                      /* Nummer eines Knoten                */
{
  return( (*APinfo->DFS)[x-1] );

} /* end of GET_DFS */






/* Lese den LOW-Funktionswert von Knoten x.
*/

int GET_LOW( APinfo, x )

  ARTI_HELP	*APinfo;
  int  		x;                      /* Nummer eines Knoten               */
{
  return( (*APinfo->LOW)[x-1] );

} /* end of GET_LOW */




/* Teste ob ein Punkt ein Artikulationspunkt ist.
   ( Aufgrund seiner Funktionstabelle )
*/

int IS_ARTI_POINT( APinfo, x )

  ARTI_HELP	*APinfo;
  int		x;
{
  if ( (*APinfo->AP)[x-1] == 1 ) return( true ); else return( false );

} /* end of IS_ARTI_POINT */




/* end of HOPCROFT.C */
