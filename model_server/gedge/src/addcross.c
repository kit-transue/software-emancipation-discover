

/******************************************************************************/
/*                                                                            */
/* ADDCROSSING                                                                */
/*                                                                            */
/*   									      */
/* Autor		Martin Schmidt, Martin-Luther-Str. 1, 7630 Lahr       */
/*			Tel.: 07821 / 41748				      */
/*									      */
/* Letzte Aenderung     03.02.88					      */
/*									      */
/*									      */
/* Eingabe		Ein PLANAR_GRAPH PG, der einen maximalen planaren     */
/*			Graphen beschreibt, und eine Menge von geloeschten    */
/*			Kanten.						      */
/*									      */
/* Problem		Versuche die Kanten so in den Graphen hineinzulegen,  */
/*			dass so wenig wie moeglich anderer Kanten gekreuzt    */
/*			werden muessen.					      */
/*									      */
/* Ausgabe		DYN_GRAPH G, in dem zu den alten Knoten neue Knoten   */
/*			hinzugefuegt wurden (Schnittpunkte) und die Kanten    */
/* 			entsprechend modifiziert wurden.		      */
/*                                                                            */
/* Loesungsansatz       Aus dem gerichteten Graphen PG wird ein ungerichteter */
/*			Graph erzeugt, wobei die Reihenfolge der Kanten, die  */
/*			von einem Knoten weggehen, wichtig ist.               */
/*			Aus diesem ungerichteten Graphen G wird der Duale     */
/*			Graph DG erzeugt. ( Ein Dualer Graph wird dadurch     */
/*			erzeugt, dass in jede Flaeche des Graphen G ein Punkt */
/* 			gelegt wird und benachbarte Flaechen durch Kanten re- */
/*			praesentiert werden. )                                */
/*			Soll nun eine Kante vom Knoten v zum Knoten w in G    */
/* 			eingefuegt werden, so sucht man im Dualen Graphen den */
/*			kuerzesten Weg zwischen einer Flaeche, die am Rand    */
/* 			den Knoten v besitzt und einer Flaeche, die am Rand   */
/*			den Knoten w besitzt. ( Der Wechsel von einer Flaeche */
/*			in eine benachbarte Flaeche bedeutet gerade das Durch-*/
/*			kreuzen einer Kante. )                                */
/*			Anschliessend muss der gefundene Weg in Kanten und    */
/*			Schnittpunkte im Graphen G umgesetzt werden.          */
/*                                                                            */
/******************************************************************************/


#include  <stdio.h>	
#include  <psetmem.h>


#include  "const.def"			/* Konstantendefinitionen
 					   true, false, nil
					*/
#include  "planar.stc"			/* Strukturbeschreibung
					   PLANAR_GRAPH
					*/
#include  "addcross.stc"		/* Strukturbeschreibung
					   DYN_GRAPH
					*/


typedef
struct POINT_LIST			/* Liste von Punkten	              */
{
  struct AL		*Al;		/* Name des Knoten ( Verweis )        */
  struct POINT_LIST	*next;		/* Verkettung			      */
} POINT_LIST;


typedef
struct EDGE_LIST			/* Liste von Kanten und deren benach- */
{					/* barten Flaechen.		      */
  struct FACE		*face;		/* Flaeche			      */
  struct EDGE		*edge;		/* Kante			      */
  int			rot;		/* Orientierung der Kante 	      */
  struct EDGE_LIST	*next;		/* Verkettung			      */
} EDGE_LIST;


typedef
struct QUEUE_NODE			/* FIFO-Queue (First-In-First-Out)    */
{					/* fuer die Breitensuche              */
  struct FACE		*face;
  struct QUEUE_NODE	*next;		/* Vorwaertsverkettung		      */
  struct QUEUE_NODE	*last;		/* Rueckwaertsverkettung	      */
} QUEUE_NODE;



typedef
struct E_NODE				/* Eine Kante einer Flaeche           */
{
  struct EDGE		*edge;		/* Name der Kante ( Verweis )         */
  struct E_NODE		*next;		/* Verkettung			      */
} E_NODE;


typedef
struct EDGE				/* Eine Kante			      */
{
  struct AL		*x, *y;		/* Anfangs- und Endpunkt der Kante    */
  struct FACE		*face_a,*face_b;/* Zwei benachbarte Flaechen          */
  struct EDGE		*next;		/* Verkettung			      */
} EDGE;


typedef
struct FACE				/* Eine Flaeche			      */
{
  int			flag;
  E_NODE		*first;		/* Zugehoerige Kantenliste	      */
  struct FACE		*next;		/* Verkettung			      */
} FACE;


typedef
struct DUAL_GRAPH			/* Beschreibung des Dualen Graphen    */
{
  EDGE			*edge;		/* Liste aller Kanten des Graphen G   */
  FACE			*face;		/* Liste aller Flaechen von G         */
} DUAL_GRAPH;


typedef
struct HELP_COMP			/* Komponente der Zugriffstabelle     */
{
  AL    		*Al;
  DYN_NODE		*mark1;
  DYN_NODE		*mark2;
} HELP_COMP;


typedef
struct HELP_COMP (*HELP)[];		/* Zugriffstabelle		      */



#include  "addcross.tst"		/* Testausgabeprozeduren.
				    	*/



/* Erzeuge aus dem PALM-TREE einen ungerichteten Graphen.
   ( Dabei muss die Reihenfolge der Kanten beachtet werden. )
*/

DYN_GRAPH *CONVERT_PG_IN_G( PG, help )

  PLANAR_GRAPH	*PG;			/* Planarer Graph		      */
  HELP		help;			/* Zugriffstabelle		      */
{
  void RECURSION_CONVERT();		/* Rekursive Teilprozedure            */

  AL		*Al;
  DYN_NODE	*DN;
  DYN_GRAPH	*G;
  int		v;
  
  /* Anlegen des Dynamischen Graphen. */
  G           = ( DYN_GRAPH * ) psetmalloc ( sizeof( DYN_GRAPH ) );
  G->old_V    = PG->V;
  G->new_V    = PG->V;
  G->first_AL = nil;

  /* Erzeuge die leeren Adjazenzlisten */
  for ( v = 1; v <= G->old_V; v++ )
      {
        Al               = ( AL * ) psetmalloc ( sizeof( AL ) );
        Al->name         = (*PG->trans)[ v ];
        Al->first	 = nil;
        Al->next_AL      = G->first_AL;
        G->first_AL      = Al;
        (*help)[ v ].Al  = Al;
      }

  /* Bestimme die Kanten, die vom Knoten 1 ausgehen ( und ihre Reihenfolge ).
     Dabei werden rekursiv auch die anderen Knoten bearbeitet.
  */
  RECURSION_CONVERT( G, PG, help, 1, 0 );

  /* Kleine Korrektur an der Adjazenzliste AL(1)                 */
  /* d.h loesche das erste Element von AL(1). (Programmiertrick) */
  DN = (*help)[ 1 ].Al->first; 
  (*help)[ 1 ].Al->first = DN->next; 
  free( DN );

  return ( G );

} /* end of CONVERT_PG_IN_G */



/* Rekursive Fortsetzung vom CONVERT_PG_IN_G.
   Bestimme die Nachbarliste des Punkt v.
*/

void RECURSION_CONVERT( G, PG, help, v, father_v )

  DYN_GRAPH	*G;
  PLANAR_GRAPH	*PG;
  HELP		help;
  int		v, father_v;
{
  DYN_NODE	*DN;
  P_NODE	*hp;
  int		side;			/* 0 = linke Seite,  1 = rechte Seite */

  /* Der erste Nachbar ist immer der Vaterknoten. */
  /* Beachte, dass der Knoten 1 keinen Vater hat. */
  DN       = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
  DN->name = (*help)[ father_v ].Al;
  DN->next = nil;

  (*help)[ v ].Al->first = DN;
  (*help)[ v ].mark1     = DN;
  (*help)[ v ].mark2     = DN;

  for ( side = 0; side <= 1; side++ )      /* Fuer die linke und rechte Seite */
     {
       hp = (*PG->AL)[ v ].x[ side ];
       while ( hp != nil )
             {
               /* Tree-Edge oder Frond-Edge einfuegen. */
  	       DN       = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
  	       DN->name = (*help)[ hp->name ].Al;
  	       DN->next = nil;
                    
               while( (*help)[ v ].mark2 != nil ) 
                    { 
  		      (*help)[ v ].mark1 = (*help)[ v ].mark2;
  		      (*help)[ v ].mark2 = (*help)[ v ].mark2->next;
                    }
  	       (*help)[ v ].mark2       = DN;
  	       (*help)[ v ].mark1->next = DN;

               /* Die Tree-Edges muessen mit Tiefensuche weiter bearbeitet
                  werden.
                  Frond-Edges werden noch in die entsprechende Liste
                  eingetragen.
               */
               if ( hp->name > v )
                    /* Tree-Edge */
                    RECURSION_CONVERT( G, PG, help, hp->name, v );
               else
                  { /* Frond-Edge */
  		    DN       = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
                    DN->name = (*help)[ v ].Al;
                    DN->next = nil;

                    if ( side == 0 )
                      {
                        /* Wenn die Frond auf die linke Seite zeigt, dann .. */
                        DN->next = (*help)[ hp->name ].mark1->next;
			(*help)[ hp->name ].mark1->next = DN;
                      }
                    else
                      {
                        /* Wenn die Frond auf die rechte Seite zeigt, dann .. */
                        DN->next = (*help)[ hp->name ].mark2->next;
			(*help)[ hp->name ].mark2->next = DN;
                      }
                  } 

              hp = hp->next;
            }

     } /* for both sides */

} /* end of RECURSION_CONVERT */




/* Fuege eine Flaeche in den Dualen Graphen G ein.
   Die Flaeche beginnt an der Kante  x-->DN und wird
   in der vorgegebenen Richtung vervollstaendigt.
*/

void INSERT_FACE_IN_DG( DG, x, DN )

  DUAL_GRAPH	*DG;			/* Dualer Graph 		      */
  AL		*x;			/* Startpunkt			      */
  DYN_NODE	*DN;			/* Naechster Punkt		      */
{
  AL		*c, *y, *xx, *yy;
  FACE		*F;
  E_NODE	*EN;
  EDGE		*e_help;
  DYN_NODE	*d_help;
  int		first_point;

  first_point = x->name;                /* Merke den Startpunkt der Flaeche   */

  /* Neue Flaeche anlegen */
  F        = ( FACE * ) psetmalloc ( sizeof( FACE ) );
  F->first = nil;	       		     /* Noch keine Kanten eingetragen */
  F->next  = DG->face;
  DG->face = F;

  do {
       /* Kante x,y in die neue Flaeche einfuegen. */
       /* Setzte die Endpunkte immer so, dass x<=y */
       y  = DN->name;
       xx = x;
       yy = y;
       if ( xx->name > yy->name ) { c = xx; xx = yy; yy = c; }

       /* Pruefe, ob die Kante schon in der Kantenliste ist. */
       e_help = DG->edge;
       while ( e_help != nil && ( e_help->x != xx || e_help->y != yy ) )
             e_help = e_help->next;

       if ( e_help == nil )
          {
            /* Kante ist noch in keiner Flaeche enthalten. */
            /* D.h. die Kantenliste muss erweitert werden. */
            e_help         = ( EDGE * ) psetmalloc ( sizeof( EDGE ) );
            e_help->x      = xx;
            e_help->y      = yy;
            e_help->face_a = F;
	    e_help->face_b = nil;
            e_help->next   = DG->edge;
            DG->edge       = e_help;
          }
       else
            /* Die Kante ist schon in einer anderen Flaeche vorhanden. */
            e_help->face_b = F;

       /* Die Kantenliste der Flaeche ergaenzen. 
          Beachte, das die Reihenfolge der Kanten dem Umlaufsinn
          der Nachbarn eines Knoten entspricht, wenn
          die Kantenliste immer vorne ergaenzt wird.
       */
       EN       = ( E_NODE * ) psetmalloc ( sizeof( E_NODE ) );
       EN->edge = e_help;
       EN->next = F->first;
       F->first = EN;

       /* Bestimme die naechste Kante */
       DN = DN->name->first;
       d_help = DN;
       while ( d_help->name != x )  d_help = d_help->next;
       if ( d_help->next != nil ) DN = d_help->next;
       DN->flag = 1;
       x = y;

     } while ( y->name != first_point );

} /* end of INSERT_FACE_IN_DG */





/* Baue aus dem Graphen G den Dualen Graphen (Flaechengraphen) auf.
   Ein Punkt im Flaechengraphen steht fuer eine Flaeche, die von
   mindestens drei Kanten abgegrenzt ist.
   Eine Kante im Flaechengraph steht fuer die Nachbarschaft
   zweier Flaechen.
*/

DUAL_GRAPH *CREATE_DUAL_GRAPH( G )

  DYN_GRAPH	*G;
{
  void		INSERT_FACE_IN_DG();	/* Fuege eine Flaeche in DG ein.      */

  DUAL_GRAPH	*DG;			/* Dualer Graph			      */
  AL		*last_AL;		/* Lezte Adjazenzliste                */
  DYN_NODE	*DN;

  /* Loesche alle Flags im Graphen G */
  last_AL = G->first_AL;
  while ( last_AL != nil )
        {
          DN = last_AL->first;
          while ( DN != nil ) { DN->flag = 0;  DN = DN->next; }
          last_AL->flag = 0;
          last_AL = last_AL->next_AL;
        }

  /* Den Dualen Graphen anlegen. */
  DG       = ( DUAL_GRAPH * ) psetmalloc ( sizeof( DUAL_GRAPH ) );
  DG->edge = nil;
  DG->face = nil;

  last_AL = G->first_AL;
  if ( last_AL != nil ) DN = last_AL->first;
  while ( last_AL != nil )
        {
          /* Suche in der letzten Adjazenzliste die erste unmarkierte Kante.*/
          while ( DN != nil && DN->flag != 0 ) DN = DN->next;
   
          if ( DN != nil && DN->flag == 0 )
             {
               /* Bestimme die Flaeche, die bei last_AL->DN beginnt */
               DN->flag = 1;               /* Markiere die Kante als besucht. */
               INSERT_FACE_IN_DG( DG, last_AL, DN );
             }
          else
             {
               /* Keine Flaeche mehr von last_AL erreichbar */
               last_AL = last_AL->next_AL;
               if ( last_AL != nil ) DN = last_AL->first;
             }
        }

  return( DG );

} /* end of CREATE_DUAL_GRAPH */




/* Lege ein neues Element an den Anfang der FIFO-QUEUE.
*/

void PUT_IN_QUEUE( QUEUE, face )

  QUEUE_NODE	**QUEUE;
  FACE		*face;
{
  QUEUE_NODE	*QN;

  QN       = ( QUEUE_NODE * ) psetmalloc ( sizeof( QUEUE_NODE ) );
  QN->face = face;
  QN->next = *QUEUE;
  QN->last = (*QUEUE)->last;
  (*QUEUE)->last->next = QN;
  (*QUEUE)->last       = QN;
  (*QUEUE )            = QN;

} /* end of PUT_IN_QUEUE */




/* Hole ein Element vom Ende der FIFO-QUEUE.
   ( Wenn die QUEUE leer ist, dann wird nil zurueckgegeben. )
*/

FACE *GET_FROM_QUEUE( QUEUE )

  QUEUE_NODE	**QUEUE;
{
  FACE		*face;
  QUEUE_NODE	*QN;

  if ( (*QUEUE)->face == nil ) face = nil;
  else
     {
       QN = (*QUEUE)->last->last;
       if ( QN == *QUEUE ) *QUEUE = QN->next;
       face = QN->face;
       QN->last->next = QN->next;
       QN->next->last = QN->last;
       free( QN );
     }

  return( face );

} /* end of GET_FROM_QUEUE */





/*  Bestimme den kuerzesten Weg zwischen Flaechen, die an v angrenzen
    und den Flaechen, die an w angrenzen.
    Als Ergebnis liefert diese Funktion eine Liste von Kanten, die
    durch den Kantenzug v --> w im Graphen G geschnitten werden muessen.

       1.Flaeche 1.Kante  .... n. Flaeche n. Kante  n+1. Flaeche  
    		    y			       y
    v               |                          |                  w
		    x  			       x
*/

EDGE_LIST *GET_SHORTEST_PATH( DG, v, w )

  DUAL_GRAPH	*DG;			/* Dualer Graph			      */
  int		v,w;			/* Anfangs- und Endpunkt der Kante    */
{
  void		PUT_IN_QUEUE();		/* Verwaltung der FIFO-Queue          */
  FACE		*GET_FROM_QUEUE();

  QUEUE_NODE	*QUEUE, *QN;    
  EDGE		*E;
  EDGE_LIST     *CE, *cross_edges; 
  E_NODE	*EN;
  FACE		*F, *mark_F;
  int		ende;

  /* Eine leere FIFO-QUEUE mit einem Dummyknoten anlegen. */
  QN       = ( QUEUE_NODE * ) psetmalloc ( sizeof( QUEUE_NODE ) );
  QN->face = nil;
  QN->next = QN;
  QN->last = QN;
  QUEUE    = QN;

  /* Markiere die Flaechen, die an den Punkt v angrenzen mit  1  */
  /* und merke diese Flaechen in der FIFO-QUEUE.	         */
  /* Markiere alle anderen Flaechen mit 0.                       */
  F = DG->face;
  while ( F != nil ) { F->flag = 0; F = F->next; }

  E = DG->edge;
  while ( E != nil )
        {
          if ( E->x->name == v || E->y->name == v ) 
             {
               if ( E->face_a->flag == 0 )
                  {
		    E->face_a->flag = 1;
                    PUT_IN_QUEUE( &QUEUE, E->face_a );
                  }
               if ( E->face_b->flag == 0 )
                  {
                    E->face_b->flag = 1;
                    PUT_IN_QUEUE( &QUEUE, E->face_b );
                  }
             }
          E = E->next;
        }

  /* Fuehre ein Breitensuche durch. */
  ende = false;
  F = GET_FROM_QUEUE( &QUEUE );
  /* Solange die FIFO-QUEUE nicht leer ist und w nicht erreicht wurde */
  while ( F != nil && ende == false )
        {
          /* Markiere alle unmarkierten Nachbarflaechen. */
          EN = F->first;
          while ( EN != nil && ende == false )
                {
                  if ( EN->edge->x->name == w || EN->edge->y->name == w )
                     {
		       /* Der Punkt w liegt am Rand der aktuellen Flaeche. */
                       if ( EN->edge->face_a->flag != 0  && 
                           ( EN->edge->face_b->flag == 0 ||
                             EN->edge->face_a->flag < EN->edge->face_b->flag ) )
                            mark_F = EN->edge->face_a;
                       else mark_F = EN->edge->face_b;
                       ende = true;
                     }
                  else
                  if ( EN->edge->face_a != F  &&  EN->edge->face_a->flag == 0 )
                     {
                       EN->edge->face_a->flag = F->flag + 1;
                       PUT_IN_QUEUE( &QUEUE, EN->edge->face_a );
                     }
                  else
                  if ( EN->edge->face_b != F  && EN->edge->face_b->flag == 0 )
                     {
                       EN->edge->face_b->flag = F->flag + 1;
                       PUT_IN_QUEUE( &QUEUE, EN->edge->face_b );
                     }  

                  EN = EN->next;
                }

          F = GET_FROM_QUEUE( &QUEUE );
        }

  /* Loesche die FIFO-QUEUE */
  while ( GET_FROM_QUEUE( &QUEUE ) != nil );
  free( QUEUE );

  /* Bestimme nun rueckwaerts den Weg von w nach v . */
  cross_edges = nil;

  CE          = ( EDGE_LIST * ) psetmalloc ( sizeof( EDGE_LIST ) );
  CE->edge    = nil;	        	 /* Es gibt n+1 Flaechen und n Kanten */
  CE->face    = mark_F;
  CE->next    = cross_edges;
  cross_edges = CE;

  /* Suche so lange, bis eine Flaeche mit dem Wert 1 gefunden wird. */
  while ( mark_F->flag != 1 )
        {
          /* Suche eine benachbarte Flaeche mit einem um 1 niedrigeren Wert. */
          EN = mark_F->first;
          while ( EN->edge->face_a->flag != mark_F->flag -1  &&
		  EN->edge->face_b->flag != mark_F->flag -1     ) EN = EN->next;

          /* Kante in die Liste der gekreuzten Kanten einfuegen.*/
          /* Bestimme zusaetzlich, die Richtung der Kante.      */
          CE          = ( EDGE_LIST * ) psetmalloc ( sizeof( EDGE_LIST ) );
          CE->edge    = EN->edge;
          if ( ( EN->next != nil &&
                 ( EN->edge->x == EN->next->edge->x || 
                   EN->edge->x == EN->next->edge->y    ) )   ||
               ( EN->next == nil &&
                 ( EN->edge->x == mark_F->first->edge->x || 
                   EN->edge->x == mark_F->first->edge->y    ) ) )
               CE->rot = 0;
          else CE->rot = 1;

          if ( EN->edge->face_a->flag != 0  && 
               ( EN->edge->face_b->flag == 0 ||
                 EN->edge->face_a->flag < EN->edge->face_b->flag ) )
               mark_F = EN->edge->face_a;
          else mark_F = EN->edge->face_b;

          CE->face    = mark_F;
          CE->next    = cross_edges;
          cross_edges = CE;
        }

  return( cross_edges );

} /* end of GET_SHORTEST_PATH */




/* Es wurde eine Liste von Kanten berechnet, die durch
   den Kantenzug v->w geschnitten wird.
   Fuege nun die entsprechenden Aenderungen in den alten Graphen G ein.
*/

void CORRECT_DYN_GRAPH( G, cross_edges, list )

  DYN_GRAPH	*G;
  EDGE_LIST	*cross_edges;		/* Liste der zu kreuzenden Kanten     */
  POINT_LIST	*list;		   	/* Liste der Punkte des Kantenzuges   */
{
  AL		*x, *x1;
  AL		*Al;
  DYN_NODE	*DN, *DN1, *POSx, *POSw;
  E_NODE	*EN;
  EDGE_LIST	*EL;
  POINT_LIST	*li;

  /* Bestimme Kantenposition in AL(x). ( x = v ) */
  x  = list->Al;      
  EN = cross_edges->face->first;
  if ( (       EN->edge->x == x ||       EN->edge->y == x )  &&
       ( EN->next->edge->x == x || EN->next->edge->y == x )  )
     {
       EN = EN->next;
     }
  else
     {
       if ( EN->edge->x != x && EN->edge->y != x )
          {
            while ( EN->edge->x != x && EN->edge->y != x ) EN = EN->next;
            EN = EN->next;
          }
     }

  if ( EN->edge->x == x ) x1 = EN->edge->y;
  else                    x1 = EN->edge->x;

  POSx = x->first; 
  while ( POSx->name != x1 ) POSx = POSx->next;

  /* Falls der einzufuegende Kantenzug aus mehreren Kanten besteht, so
     fuege sie an den Punkt x an.
  */
  li = list; list = li->next; free( li );          /* Loesche v aus der Liste */
  while ( list->next != nil )
        {
          DN         = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
          DN->name   = list->Al;
          DN->next   = POSx->next;
          POSx->next = DN;

          DN         = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
          DN->name   = x;
          DN->next   = nil;
          x->first   = DN;
          POSx       = DN;
         
          x  = list->Al;
          li = list; list = li->next; free( li ); 
        }

  /* Fuege eine Kante von x zur ersten Schnittkante ein. */
  while ( cross_edges->next != nil )
        {
          /* Erzeuge einen neuen Knoten  (Schnittpunkt mit der Kante) */
          DN       = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
          DN->name = x;
          DN->next = nil;

          Al          = ( AL * ) psetmalloc ( sizeof( AL ) );
          Al->name    = G->new_V + 1;                    (G->new_V)++;
          Al->first   = DN;
          Al->next_AL = G->first_AL;
          G->first_AL = Al;

          /* Setze Kante von x zum neuen Knoten. */
          DN         = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
          DN->name   = Al;
          DN->next   = POSx->next;
          POSx->next = DN;
          POSx       = Al->first;

          /* 2 Kanten vom neuen Knoten zu den Endpunkten der Schnittkante */
          DN         = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
          DN1        = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
          DN1->name  = cross_edges->edge->x;
          DN->name   = cross_edges->edge->y;
          if ( cross_edges->rot == 0 ) 
             { DN1->next  = DN;
               DN->next   = nil;
               POSx->next = DN1;
               POSx       = DN1;
             }
          else
             { DN->next   = DN1;
               DN1->next  = nil;
               POSx->next = DN;
               POSx       = DN;
             }
          
          /* Schnittkante noch aufteilen. */
          DN = cross_edges->edge->x->first;
          while ( DN->name != cross_edges->edge->y ) DN = DN->next;
          DN->name = Al;
          DN = cross_edges->edge->y->first;
          while ( DN->name != cross_edges->edge->x ) DN = DN->next;
          DN->name = Al;
          
          /* Schnittkante bearbeitet. */
          x = Al;
          
          EL = cross_edges; cross_edges = EL->next;  free( EL );
        }

  /* Jetzt muss noch eine Kante von x nach w eingefuegt werden. */
  /* Bestimme erst die Position der Kante in AL(w).             */
  EN = cross_edges->face->first;
  if ( ( EN->edge->x == list->Al || EN->edge->y == list->Al )  &&
       ( EN->next->edge->x == list->Al || EN->next->edge->y == list->Al )  )
     {
       EN = EN->next;
     }
  else
     {
       if ( EN->edge->x != x && EN->edge->y != x )
          {
            while ( EN->edge->x != list->Al && EN->edge->y != list->Al ) 
                  EN=EN->next;
            EN = EN->next;
          }
     }

  if ( EN->edge->x == list->Al ) x1 = EN->edge->y; else x1 = EN->edge->x;

  POSw = list->Al->first; 
  while ( POSw->name != x1 ) POSw = POSw->next;

  /* Kante von x nach w eintragen. */
  DN         = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
  DN->name   = list->Al;
  DN->next   = POSx->next;
  POSx->next = DN;

  DN         = ( DYN_NODE * ) psetmalloc ( sizeof( DYN_NODE ) );
  DN->name   = x;
  DN->next   = POSw->next;
  POSw->next = DN;

  /* Letzten Eintrag aus der Punkteliste und aus der Kantenliste loeschen. */
  free( cross_edges );
  free( list        );

} /* end of CORRECT_DYN_GRAPH */




/* Fuege eine Kante (bzw. einen Kantenzug) in den Graphen G ein.
*/

void INSERT_PATH( DG, G, PG, help )

  DUAL_GRAPH	*DG;			/* Dualer Graph 		      */
  DYN_GRAPH	*G;			/* Dynamischer Graph                  */
  PLANAR_GRAPH  *PG;			/* Planarer Graph		      */
  HELP		help;			/* Zugriffstabelle 		      */
{
  EDGE_LIST	*GET_SHORTEST_PATH();
  void		CORRECT_DYN_GRAPH();

  int		v, v_old, w;
  POINT_LIST	*LN, *list;
  EDGE_LIST	*cross_edges;		/* Liste der zu kreuzenden Kanten     */

  /* Die geloeschte Frond kann einen ganzen Kantenzug beinhalten. */
  /* Es wird nun eine Liste erzeugt, die alle Punkte des Kanten-  */
  /* zuges enthaelt. (Also mindestens 2 Punkte.)                  */
  list = nil;

  w = PG->DEL_FROND->y;
  LN       = ( POINT_LIST * ) psetmalloc ( sizeof( POINT_LIST ) );
  LN->Al   = (*help)[ w ].Al;
  LN->next = list;
  list     = LN;

  /* Falls v->w eine Frond war, die beim Loeschen mehr als eine
     Kante mitgeloescht hat, muessen die restlichen Punkte des
     Kantenzuges auch noch bestimmt werden.
     ( Einfaches Kriterium : AL( v ) ist leer, dass heisst es
                             fuehrt noch keine Kante nach v, also
			     fehlt diese Kante noch.
     )
  */
  v = PG->DEL_FROND->x;
  do {
       LN       = ( POINT_LIST * ) psetmalloc ( sizeof( POINT_LIST ) );
       LN->Al   = (*help)[ v ].Al;
       LN->next = list;
       list     = LN;

       v_old = v;
       v     = (*PG->AL)[ v ].father;

     } while ( (*help)[ v_old ].Al->first == nil );

  w = (*PG->trans)[ PG->DEL_FROND->y ];
  v = (*PG->trans)[ v_old ];

  /* Die Kante (bzw. der Kantenzug) v -> w soll in G eingefuegt werden. */

  /* Bestimme den kuerzesten Weg im dualen Graphen. */
  cross_edges = GET_SHORTEST_PATH( DG, v, w );

  /* Adjazenlisten von G entsprechend veraendern. (neue Punkte + Kanten) */
  CORRECT_DYN_GRAPH( G, cross_edges, list );

} /* end of INSERT_PATH */





/* Loesche den Flaechengraphen im Speicher.
*/

void DELETE_DUAL_GRAPH( DG )

  DUAL_GRAPH	*DG;
{
  EDGE		*E;
  FACE		*F;
  E_NODE	*EN;

  /* Loesche die Kantenliste */
  E = DG->edge;
  while ( E != nil ) { DG->edge = E->next;  free( E );  E = DG->edge; }

  /* Loesche den Flaechengraphen. */
  F = DG->face;
  while ( F != nil )
        {
          EN = F->first;
          while ( EN != nil ) { F->first = EN->next; free(EN); EN = F->first; }

          DG->face = F->next;  free( F );  F = DG->face;
        }

  free( DG );

} /* end of DELETE_DUAL_GRAPH */




/* Ueberfuehre den planaren Graphen G in einen ungerichteten Graphen
   und versuche die geloeschten Kanten wieder in den Graphen einzufuegen.
   Dabei sollten so wenig wie moeglich Schnitte zwischen Kanten entstehen.
*/

DYN_GRAPH *ADD_CROSSINGS( PG )

  PLANAR_GRAPH	*PG;			/* Planarisierungsergebnis            */
{
  DYN_GRAPH	*CONVERT_PG_IN_G();	/* Konvertiere PG in einen ung. Gra.  */
  DUAL_GRAPH 	*CREATE_DUAL_GRAPH();	/* Erzeuge den Flaechengraphen        */
  void		INSERT_PATH();		/* Fuege eine Kante in G ein          */
  void		DELETE_DUAL_GRAPH();    /* Loesche den Flaechengraphen        */

  DYN_GRAPH	*G;
  DUAL_GRAPH	*DG;
  DELETED_FROND	*DF;			
  HELP		help;			/* Zugriffsstruktur		      */
  int           count;

  help = ( HELP ) psetmalloc ( (1+PG->V) * sizeof( HELP_COMP ) );

  /* Erzeuge aus dem PALM-TREE PG einen ungerichteten Graphen. */
  G = CONVERT_PG_IN_G( PG, help );
  count = 0;

  if ( PG->DEL_FROND != nil )
     {
       /* Fuege die geleoschten Kantenzuege ( mit Hilfe des Dualen Graphen )
          wieder in den Graphen G ein.
       */

       while ( PG->DEL_FROND != nil )
             {
               DG = CREATE_DUAL_GRAPH( G ); 
               INSERT_PATH( DG, G, PG, help );
               DELETE_DUAL_GRAPH( DG );
               
	       count = count + 1;

               /* Loesche den obersten Eintrag des DEL_FROND-Stack */
               DF = PG->DEL_FROND; PG->DEL_FROND = DF->next; free( DF );
             }
     }

  /* Loesche die Hilfsdatenstruktur. */
  free( help );

  /* OSapi_fprintf (stderr,"Number of the deleted edges : %d\n", count); */
  /* OSapi_fprintf (stderr,"Number of Crossing Points : %d\n", G->new_V - G-> old_V); */
  return( G );

} /* end of ADD_CROSSINGS */



/* end of ADDCROSS.C */


