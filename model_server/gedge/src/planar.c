
/******************************************************************************/
/*                                                                            */
/* PLANARISATIONSALGORITHMUS                                                  */
/*                                                                            */
/*   									      */
/* Autor		Martin Schmidt, Martin-Luther-Str. 1, 7630 Lahr       */
/*			Tel.: 07821 / 41748				      */
/*									      */
/* Letzte Aenderung     23.02.88					      */
/*									      */
/*									      */
/* Problem : Gegeben : Ein ungerichteter, zweifachzusammenhaengender Graph,   */
/*                     der keine Schlingen und Multikanten hat.               */
/*                     ( Der Graph wird durch Adjazenzlisten dargestellt. )   */
/*                                                                            */
/*           Gesucht : Der maximale planare Subgraph.                         */
/*                     D.h. man sucht einen ebenen Graphen, bei dessen Dar-   */
/*                     stellung keine Kante eine andere Kante schneidet.      */
/*                                                                            */
/*                                                                            */
/*           Ergebnis: 1. Die Menge der Kanten die geloescht werden muss.     */
/*                        ( Diese Menge sollte minimal sein ! )               */
/*                     2. Beschreibung der relativen Lage der Kanten und      */
/*                        Knoten.                                             */
/*                     (  Beschreibung der Ergebnisdatenstrukturen :          */
/*                        siehe PLANAR.STC                              )     */
/*                                                                            */
/*                                                                            */
/*    In dem Artikel JOHNSON D.S.                                             */
/*                   The NP-completeness column : an ongoing guide,           */
/*                   Journal of Algorithms 3 (1982)                           */
/*                                                                            */
/*    wird nachgewiesen, dass die Suche nach dem maximalen planaren Sub-      */
/*    graphen ( Maximal heisst hier, dass eine minimale Anzahl von Kanten     */
/*    geloescht wird. ) ein NP-vollstaendiges Problem ist.                    */
/*    Somit kann das Problem nur durch eine Approximation geloesst werden.    */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*    Loesungsansatz : Als Grundlage des folgenden Algorithmusses diente der  */
/*                     Artikel                                                */
/*									      */
/*                     CHIBA, NISHIOKA, SHIRAKAWA                             */
/*                     An Algorithm of maximal Planarization of Graphs.       */
/*                     Proceedings of 1979 ISCAS p. 649                       */
/*                                                                            */
/*                     Der Algorithmus erstellt fuer den Graphen einen span-  */
/*                     nenden Baum und die sogenannten Frond-Listen.Mit Hilfe */
/*                     dieser Listen werden nacheinander Pfade generiert.     */
/*                     Die Pfade werden nacheinander in der Ebene eingebettet.*/
/*                     Wenn ein Pfad nicht kreuzungsfrei in der Ebene einge-  */
/*                     bettet werden kann, so wird die letzte Kante des Pfades*/
/*                     ( dies ist immer eine Frond ) verworfen.               */
/*                     So wird nach und nach der urspruengliche Graph wieder  */
/*                     aufgebaut.                                             */
/*                                                                            */
/*                     Der Vorgang des Einbetten ist im obigen Artikel nicht  */
/*                     naeher beschrieben. Als Grundlage fuer dieses Teil-    */
/*                     algorithmus diente der Artikel                         */
/*                                                                            */
/*                     J. Hopcroft, R. Tarjan                                 */
/*                     Efficient Planarity Testing.                           */
/*                     Journal of ACM 1974                                    */
/*									      */
/*                     Die Datenstrukturen und Loesungsideen sind aus diesem  */
/*		       Artikel entnommen. Da der dort angegebene Algorithmus  */
/*                     rekursiv ist und bei nicht Planaritaet abbricht,       */
/*                     wurden einige Aenderungen vorgenommen.                 */
/*                                                                            */
/******************************************************************************/



#include  <stdio.h>  
#include  <psetmem.h>

#include  "const.def"           	/* Definition von Konstanten wie
                                   	   true, false und nil.
                                	*/
#include  "adtugraph.stc"       	/* Structurvereinbarung
                                   	   UNDIR_GRAPH.
                                	*/
#include  "planar.stc"       		/* Strukturvereinbarung 
                                           PLANAR_GRAPH.
                                	*/

#include  "planar.fl"           	/* Generierung und Manipulation
                                           der Frondlisten.
 					*/
#include  "planar.st"           	/* Generierung und Manipulation
					   der Stacks.
					*/
#include  "planar.pg"			/* Aufbau der planaren Graphen PG.
                                        */




/* Der Algorithmus PLANARIZATION liefert einen Pfad s->..->v- ->w nach
   dem anderen. Diese Pfade werden in einer gewissen Reihenfolge geliefert
   und haben gewisse Eigenschaften.
   1. Jede Kante des Graphen ist genau einmal in einem der Pfade enthalten.
   2. Jeder Pfad hat genau 2 Punkte mit dem bisher plazierten Graphen
      gemeinsam ( Anfangs- und Endpunkt ).
   3. ....

   Die Aufgabe von EMBED ist es zu entscheiden, ob ein gegebener Pfad ohne
   Ueberschneidungen zu erzeugen, in den alten Teilgraphen eingebettet werden
   kann und gegebenenfalls stoerende Pfade umzuordnen. 
*/

void EMBED( PG, st, s, w, result, inside, side, mode )

  PLANAR_GRAPH  *PG;			/* Beschreibung des planaren Graphen  */
  STACK 	*st;			/* Hilfsstruktur                      */
  int           s, w;			/* Pfad s->.. - ->w                   */
  int		*result;		/* Pfad plazierbar ?                  */
  int		*inside, *side;        	/* 0 = linke Seite,  1 = rechte Seite */
  int  		*mode;			/* 1 = Normaler Path                  */
		                        /* 2 = First Cycle                    */
                                        /* 3 = Special Path  (beide Seiten)   */
                                        /* 4 = Special Path  (nur eine Seite) */
{
  int  TOP_OF_LR();			/* Lese das erste Element eines Stacks*/
  void UPDATE_LR();			/* Loesche ueberfluessige Eintraege   */

  void TOP_OF_S();			/* Lese erstes S-Element              */
  void POP_OF_S();			/* Loesche erstes S-Element           */
 
  int  TAUSCHE();			/* Fuehre Vertauschung durch.         */

  int  fathers_s, father_v, fathers_w;	/* Vaterpfad                          */
  int  outside;	    			/* 0 = linke Seite,  1 = rechte Seite */

  /* Kann man ein altes Segment abschliessen ? */
  TOP_OF_S( st, &fathers_s, &father_v, &fathers_w, inside );
  while ( s <= fathers_s)  
        {
          POP_OF_S( st );	        	          /* Segment loeschen */
          TOP_OF_S( st, &fathers_s, &father_v, &fathers_w, inside);
        }  

  /* Loesche alle Informationen ueber Fronds, die oberhalb von s enden. */
  UPDATE_LR( st, s );

  /* Die Lage des Vatersegment bestimmt, was INNEN und was AUSSEN ist.  */
  outside = ( *inside + 1 ) % 2;

  /* Pruefe : Ist der Pfad ein special Path ?
              Ist der Graph noch biconnected ?
  */
  if ( w >= s )		                   *mode = 2;      /* First Cycle     */
  else
  if ( w == fathers_w  && fathers_s != 1 ) *mode = 3;      /* Special Path    */
  else                                     *mode = 1; 	   /* Normal Path     */

  if ( *mode == 2 )
     {
       /* Der Graph ist durch das Loeschen von Kanten nicht mehr biconnected. */
       /* Vorgehensweise : Behandle den Pfad wie ein First Cycle.             */
       /*                  ( Er wird immer INNEN eingebettet. )               */
       *side   = *inside;
       *result = true;
     }
  else
  if ( *mode == 3 )
     {
       /* Special Path gefunden. */
       if ( ( TOP_OF_LR( st, *inside ) <= w ||
              (*st->SPERRE)[ (*st->NEXT)[ *inside ] ] == 1 ) &&
            ( TOP_OF_LR( st, outside ) <= w || 
              (*st->SPERRE)[ (*st->NEXT)[ outside ] ] == 1 ) )
          { 
            /* INNEN und AUSSEN ist noch Platz. */
            *side   = *inside;
            *result = true;
          }
       else
       if ( TOP_OF_LR( st, *inside ) <= w                ||
            (*st->SPERRE)[ (*st->NEXT)[ *inside ] ] == 1  ||
            TAUSCHE( PG, st, *inside, s, w ) == true        )
          {
            /* Es ist nur noch INNEN Platz oder 
               Durch Tauschen wurde Innen Platz geschaffen. */
            *side   = *inside;
            *mode   = 4;
            *result = true;
          }
       else
       if ( TOP_OF_LR( st, outside ) <= w || 
            (*st->SPERRE)[ (*st->NEXT)[ outside ] ] == 1 ) 
          {
            /* Es ist nur noch AUSSEN Platz. */
            *side   = outside;
            *mode   = 4;
            *result = true;
          }
       else *result = false;
     }
  else /* *mode == 1 */
     {
       /* Ein normaler Pfad wurde gefunden. */
       if ( TOP_OF_LR( st, *inside ) <= w   ||
            TAUSCHE( PG, st, *inside, s, w )  )
          {
            /* INNEN ist noch Platz  oder
               Durch Tauschen wurde INNEN Platz geschaffen. */
            *side   = *inside;
            *result = true;
          }
       else
       if ( TOP_OF_LR( st, outside ) <= w  )
          {
            /* AUSSEN ist noch Platz. */
            *side   = outside;
            *result = true;
          }
       else *result = false;
     }

} /* end of EMBED */





/* Hauptalgorithmus 
   Planarisierung eines Graphen G.
   Uebergeben wird ein ungerichteter, einfacher, zweifachzusammenhaengender
   Graph.
   Zurueckgegeben wird die Beschreibung des maximalen planaren Teilgraphen.
*/

PLANAR_GRAPH *PLANARISATION( GB )

  UNDIR_GRAPH 	*GB;            	/* Zeiger auf einen biconnected Gr.   */
{
  PLANAR_GRAPH  *CREATE_PGRAPH();       /* Lege einen leeren planaren G. an   */
  FL_LISTE      FL_GENERATION();        /* Erzeuge die Frondlisten            */
  STACK    	*MAKE_EMBED_STACKS();   /* Anlegen einer Hilfsstruktur        */
  void          EMBED();                /* Versuche Pfad zu plazieren         */
  void		GET_PATH_FROM_FL();	/* Lese Pfad aus den Frond-Liste      */
  void		DELETE_TRIPLES_IN_FL(); /* Loesche Tripel (u,v,*) aus FL(u)   */
  void          INSERT_PATH_IN_PG();	/* Fuege Pfad in den planaren Graphen */
  void    	CORRECT_PALM_TREE();    /* Korrigiere PG                      */

  PLANAR_GRAPH  *PG;			/* Beschreibung des planaren Graphen  */
  STACK         *stack;                 /* EMBED-Hilfsstruktur                */
  FL_LISTE	FL;			/* Frond-Listen			      */
  int           s, v, w;		/* Pfad s->.. ->v- ->w		      */
  int     	u;			/* Knoten        		      */
  int           result;			/* Ist ein Pfad plazierbar ?          */
  int           inside, side;           /* Was ist INNEN, wo liegt der Pfad   */
  int  		mode;			/* Was fuer ein Pfad ist es ?         */
  DELETED_FROND *del_frond;		/* Eine geloeschte Frond-Edge         */

  PG     = CREATE_PGRAPH( GB );        
  FL     = FL_GENERATION( GB, PG );  
  stack  = MAKE_EMBED_STACKS( PG->E );

  /* Bestimme den First-Cycle :  s = 1->...->v- ->w=1 */
  /* Der FC wird auf der linken Seite eingebettet.    */
  s  = 1;
  GET_PATH_FROM_FL( FL, s, &v, &w );
  INSERT_PATH_IN_PG( PG, stack, 0, 0, 2, s, v, w );
 
  s = v;
  while ( s != 1 )
     if ( (*FL)[ s ] != nil )
        {
          /* Es existiert noch ein nicht plaziertes Segment S(s).*/
          /* Bestimme den First Path P(s) des Segment S(s).      */
          GET_PATH_FROM_FL( FL, s, &v, &w );
          /* Wenn P(s) plaziert werden kann, dann plaziere P(s). */
          EMBED( PG, stack, s, w, &result, &inside, &side, &mode );
          if ( result == true )
             {
	       /* Trage den Pfad s->..->v- ->w in PG ein. */
               INSERT_PATH_IN_PG( PG, stack, inside, side, mode, s, v, w );
               s = v;
             }
          else
             {
               /* Loesche die letzte Frond. */
               /* Achtung : Es kann sein das beim Loeschen einer Frond mehr */
               /*           als nur die Frond geloescht wird.               */
               del_frond = (DELETED_FROND *)psetmalloc( sizeof( DELETED_FROND ) );
  	       del_frond->x     = v;
               del_frond->y     = w;
               del_frond->next  = PG->DEL_FROND;
               PG->DEL_FROND    = del_frond;
               /* s bleibt unveraendert ! */
             }
        }
      else
        {
          /* Sei u->s die Tree-Edge, die in s endet.   */
          /* Loesche all Tripel ( u,s, * ) aus FL( u ).*/
          u = (*PG->AL)[ s ].father;
          DELETE_TRIPLES_IN_FL( FL, u, s );
          s = u;
        }

  CORRECT_PALM_TREE( PG, stack );	/* Korrigiere PG                      */
  DELETE_EMBED_STACKS( stack );		/* Gebe Speicherplatz frei.           */

  return( PG );

} /* end of PLANARISATION */


/* end of PLANAR.C */

