
/******************************************************************************/
/*									      */
/* Include-File		PLANAR.ST 					      */
/*									      */
/* Letzte Aenderung     17.02.88					      */
/*									      */
/* Generierung und Manipulation der Stacks.        			      */
/*									      */
/******************************************************************************/


/* Strukturbeschreibung des STACK's */


typedef
struct SPECIAL_FROND		/* Stack fuer Special-Fronds.                 */
{
  int			s;		/* Richtungsknoten                    */
  int			v;		/* Anfangspunkt der Frond-Edge        */
  int			w;		/* Endpunkt der Frond-Edge            */
  struct SPECIAL_FROND  *next;		/* Stack als verkettet Liste          */
} SPECIAL_FROND;


typedef
struct BLOCK_NODE		/* Block = Menge von Fronds.                  */
{
  int                   s;              /* Startknoten des ersten Pfades      */
  int                   ti;             /* Richtung (s->ti)  Innen            */
  int			to;		/* Richtung (s->to)  Aussen           */
  int			x[2];		/* x[0] Zeiger auf den linken Stack   */
					/* x[1] Zeiger auf den rechten Stack  */
  struct BLOCK_NODE	*next;         	/* Verkette Liste		      */
} BLOCK_NODE;


typedef
struct STACK_NODE       	/* Stack der offenen Segmente          	      */
{
  int			s, v, w;	/* Path  s -> ..-> v - - > w          */
  int			side;		/* Auf welcher Seite plaziert ?       */
					/* 0 = linke Seite,  1 = rechte Seite */
  struct STACK_NODE	*next;		/* Verkette Liste 		      */
} STACK_NODE;


typedef
struct STACK				/* Hilfsstruktur fuer EMBED           */
{
  SPECIAL_FROND *SF;			/* Liste der Special Fronds           */
  SPECIAL_FROND *last_SF;		/* Letztes Element des SF_Stack       */
  STACK_NODE  	*S;			/* Stack der offenen Segmente         */
  BLOCK_NODE    *B;			/* Bloecke                            */
  int           (*SPERRE)[];		/* 1 = Sperreintrag		      */
  int           (*STACK)[];             /* Implementierung des                */
  int		(*NEXT)[]; 		/* L- und R-Stack                     */
  int		FREE;			/* Erster freie Stelle auf dem STACK  */
} STACK;



/* Anlegen der Stacks (Hilfsstrukturen fuer EMBED).
*/

STACK *MAKE_EMBED_STACKS( E )
 
  int  		E;			/* Anzahl der Kanten 		      */
{
  STACK		*stack;
  SPECIAL_FROND *SF;
  STACK_NODE    *S;
  BLOCK_NODE    *B;

  /* Erzeuge den gesamten Stack */
  stack = ( STACK *)malloc( sizeof( STACK ) );

  /* Erzeuge den Special-Frond-Stack */
  /* Hier werden all Fronds von special Pathes abgespeichert, die
     waehrend Planarisation als einbettbar erkannt wurden.
     Diese Fronds werden nachtraeglich von CORRECT_PALM_TREE in den
     planaren Graphen eingetragen.
  */
  SF = ( SPECIAL_FROND * ) malloc ( sizeof( SPECIAL_FROND ) );
  SF->s     = 0;                                         /* Ein Dummy anlegen */
  SF->v     = 0;
  SF->w     = 0;
  SF->next  = nil;
  stack->SF = SF;
  stack->last_SF = SF;

  /* Segmente Stack  (S-Stack)  anlegen.
     Der Aufbau der Segmente bildet einen korrekten Klammerausdruck.
     D.h. Jedesmal, wenn ein Pfad ein neues Segment einleitet, wird
     der Pfad auf dem Stack abgespeichert.
     Wenn ein Segment geschlossen wird, wird der Pfad wieder
     vom Stack genommen (er liegt immer Oben) .
  */
  S = ( STACK_NODE *) malloc( sizeof( STACK_NODE ) );
  S->s     = 0;				/* Dummy 	      		      */
  S->v     = 0;
  S->w	   = 0;			
  S->side  = 0;
  S->next  = nil;
  stack->S = S;

  /* NEXT und STACK anlegen ( interne Realisierung des R- und L-Stack ).
     Der L- und der R-Stack sind am Anfang leer .
     NEXT( 1 ) zeigt auf den letzten Eintrag von R
     NEXT( 0 ) zeigt auf den letzten Eintrag von L
  */
  stack->STACK  = ( int (*)[] ) malloc( (E+2) * sizeof( int ) );
  stack->NEXT   = ( int (*)[] ) malloc( (E+2) * sizeof( int ) );

  (*stack->STACK)[ 1 ] = 0;
  (*stack->STACK)[ 0 ] = 0;
  (*stack->NEXT )[ 1 ] = 0;
  (*stack->NEXT )[ 0 ] = 0; 

  /* Das Feld SPERRE gibt an, ob es sich bei einem Frondeintrag auf dem
     L- bzw R-Stack um eine Frond oder um eine Sperre handelt.
  */
  stack->SPERRE = ( int (*)[] ) malloc( (E+2) * sizeof( int ) );

  /* Erste freie Stelle auf dem Stack : */
  stack->FREE = 2;

  /* Block-Stack  ( B-Stack )  anlegen.
  */
  B = ( BLOCK_NODE * ) malloc ( sizeof( BLOCK_NODE ) );
  (B->x)[0]   = 0;			/* Dummy                              */
  (B->x)[1]   = 0;
  B->s        = 0;
  B->ti       = 0;
  B->to	      = 0;
  B->next     = nil;
  stack->B    = B;

  return( stack );

} /* end of MAKE_EMBED_STACKS */




/* Loesche die alten Stacks.
*/

void DELETE_EMBED_STACKS( stack )

  STACK 	*stack;
{
  SPECIAL_FROND *SF;
  STACK_NODE    *S;
  BLOCK_NODE    *B;

  free( stack->SPERRE );
  free( stack->STACK  );
  free( stack->NEXT   );

  SF = stack->SF;
  while ( SF != nil ) { stack->SF = SF->next; free( SF ); SF = stack->SF; }

  S = stack->S;
  while ( S  != nil ) { stack->S  = S->next;  free( S );  S  = stack->S;  }

  B = stack->B;
  while ( B  != nil ) { stack->B  = B->next;  free( B );  B  = stack->B;  }

  free( stack );

} /* end of DELETE_EMBED_STACKS */






/* Lese das erste Element des L oder R-Stacks.
*/

int TOP_OF_LR( stack, side )

  STACK		*stack;
  int           side;			/* Welcher Keller ist gemeint         */
					/* 0 = linke Seite,  1 = rechte Seite */
{
  return( (*stack->STACK)[ (*stack->NEXT)[ side ] ] );

} /* end of TOP_OF_LR */



/* Lege einen Pfad auf den L-Stack (bzw. R-Stack).
   Aktualisiere dabei gleichzeitig die Blockstruktur.
*/

void PUSH_ON_LR( st, side, mode, s, v, w, ti, to )

  STACK		*st;
  int           side;			/* Auf welcher Seite plazieren ?      */
					/* 0 = linke Seite,  1 = rechte Seite */
  int           mode;			/* 1 = normaler Pfad                  */
 					/* 2 = Graph nicht mehr biconnected   */
                                        /* 3 = Special Path (beide Seiten)    */
        				/* 4 = Special Path (nur eine S.)     */
  int           s, v, w;		/* Pfad s->..v- ->w                   */
  int		ti, to;			/* Welche Kanten sind fest            */
{
  BLOCK_NODE    *B;
  int		not_side;
  int           help;

  not_side = ( side + 1 ) % 2;

  /* Lege einen neuen Block an.
     Es kann sein , dass der Block gleich mit anderen Bloecken
     veschmolzen werden muss.
  */
  B = ( BLOCK_NODE * )malloc( sizeof( BLOCK_NODE ) );
  (B->x)[0]   = 0;
  (B->x)[1]   = 0;
  B->s        = s;
  B->ti       = ti;
  B->to       = to;
  B->next     = st->B;
  st->B       = B;

  /* Wenn der Pfad keine einzelne Frond ist, dann wird eine SPERRE auf
     den  not_side-Stack gelegt.         
  */
  if ( s != v  &&  mode != 2 )
     {
       (*st->SPERRE)[ st->FREE ] = 1;/* Markiere den Eintrag als SPERRE */
       (*st->STACK )[ st->FREE ] = s;
       (*st->NEXT  )[ st->FREE ] = (*st->NEXT)[ not_side ];
       (*st->NEXT  )[ not_side ] = st->FREE;

       (B->x)[ not_side ] = st->FREE;
       ( st->FREE )++;
     }

  /* Wenn der Pfad kein special Path ist, dann muss er auf dem side-Stack
     abgespeichert werden.
  */
  if ( mode < 3 )
     {
       (*st->SPERRE)[ st->FREE ] = 0;
       (*st->STACK )[ st->FREE ] = w;
       (*st->NEXT  )[ st->FREE ] = (*st->NEXT)[ side ];
       (*st->NEXT  )[ side     ] = st->FREE;
     
       (B->x)[ side ] = st->FREE;
       (st->FREE)++;
     }

  /* Jetzt sind alle Eintraege auf dem L(R)-Stack eingetragen.
     Nun muss ueberprueft werden, welche Fronds sich gegenseitig
     behindern. ( D.h. Verschmelze bestimmte Bloecke )
  */
  /* Bei mode = 2 : So ein Pfad kann niemals mit einem anderen Pfad kollidieren.
         mode = 3 : Kann so oder so auf beiden Seiten plaziert werden.
  */
  if ( mode == 4 )
     {
       /* Verschmelze die letzten beiden Bloecke. */
       st->B = B->next;  free( B );  B = st->B;
     }
  else
  if ( mode == 1 )
     {
       /* Verschmelze die Bloecke, die sich gegenseitig behindern. */
       B = st->B;
       if ( (B->x)[ not_side ] == 0 ) help = not_side;
       else                           help = (B->x)[ not_side ];

       while (  (B->next->x)[ not_side ] != 0  &&
                (*st->STACK)[ (*st->NEXT)[ help ] ] > w  ) 
             {
               if ( (B->next->x)[ side ] == 0 )
                    (B->next->x)[ side ] = (B->x)[ side ];
               /* Loesche den obersten Block des B->Stack */
               st->B = B->next;  free( B );  B = st->B;
               help = (B->x)[ not_side ];
             }
     }

} /* end of PUSH_ON_LR */




/* Bestimme den ersten Pfad des aktuellen Segmentes.
*/

void TOP_OF_S( stack, s, v, w, side )

  STACK		*stack;
  int		*s, *v, *w;		/* Pfad                               */
  int           *side;			/* Seite, auf der plaziert wurde      */
					/* 0 = linke Seite,  1 = rechte Seite */
{
  *s    = stack->S->s;
  *v    = stack->S->v;
  *w    = stack->S->w;
  *side = stack->S->side;

} /* end of TOP_OF_S */



/* Setze den ersten Pfad eines Segments auf den S-Stack.
*/

void PUSH_ON_S( stack, side, s, v, w )

  STACK		*stack;
  int           side;			/* Seite, auf der plaziert wird       */
					/* 0 = linke Seite,  1 = rechte Seite */
  int		s, v, w;		/* Pfad				      */
{
  STACK_NODE	*S;

  S = (STACK_NODE *) malloc( sizeof( STACK_NODE ) );
  S->s     = s;
  S->v     = v;
  S->w     = w;
  S->side  = side;
  S->next  = stack->S;
  stack->S = S;

} /* end of PUSH_ON_S */



/* Loesche das erste Element des S-Stack.
*/

void POP_OF_S( stack )

  STACK		*stack;
{
  STACK_NODE	*S;

  S        = stack->S;
  stack->S = S->next;
  free( S );

} /* end of POP_OF_S */





/* Versuche durch Tauschen von Bloecken auf der Seite side
   Platz fuer den Pfad s-> - ->w zu schaffen.
   Dabei muss PG auch geaendert werden.
*/

int TAUSCHE( PG, st, inside, s, w )

  PLANAR_GRAPH	*PG;
  STACK		*st;
  int	        inside;
  int    	s,w; 			/* Pfad  s-> - ->w                    */
{
  int		test;			/* Konnte Platz geschaffen werden ?   */
  int           weiter;                 /* Muss weiter probiert werden ?      */
  int		outside;		/* Gegenteil von inside		      */
  int           save;
  int		u;
  int		ti, to;			/* Nichttauschbare Kanten             */
  P_NODE	*help1, *help2, *help3, *help4;
  BLOCK_NODE    *B, *B1;

  outside = ( inside + 1 ) %2;

  test = false;				/* Angenommen Tauschen bringt nichts. */

  B = st->B;

  /* Tausche Bloecke in Gedanken, bis ....  
     ( Jeder Block ist fuer sich tauschbar. (sonst waere es kein Block) )
     Wenn der Tauschtest true ergibt, dann zeigt B auf den
     tiefsten Block, der noch getauscht werden muss.
  */
  do { 
       if ( (B->x)[ outside ] != 0                           && 
            ( (*st->SPERRE)[ (B->x)[ outside ] ] != 0     ||
              (*st->SPERRE)[ (B->x)[  inside ] ] != 0     ||
              (*st->STACK )[ (*st->NEXT)[ outside ] ] > w   )   )
            weiter = false;
       else weiter = true;

       if (    weiter == true &&
            ( (*st->SPERRE)[ (*st->NEXT)[ (B->x)[ inside ] ] ] == 1  ||
              (*st->STACK )[ (*st->NEXT)[ (B->x)[ inside ] ] ] <= w     ) )
            test = true;
 
       if ( weiter == true  &&  test == false )  B = B->next;

     } while ( weiter == true  &&  test == false );

  if ( test == true )
     { 
       /* Verschmelze alle Bloecke, die getauscht werden muessen. */
       while ( B != st->B )
             { B1 = st->B;  st->B = B1->next;  free( B1 ); }

       /* Fuehre die Vertauschung auf dem Stack durch. */
       if ( (B->x)[ outside ] != 0 )
          {
            save                             = (*st->NEXT)[ (B->x)[ inside ] ];
            (*st->NEXT)[ (B->x)[  inside ] ] = (*st->NEXT)[ (B->x)[ outside ] ];
            (*st->NEXT)[ (B->x)[ outside ] ] = save;
            save                             = (*st->NEXT)[ outside ];
            (*st->NEXT)[ outside ]           = (*st->NEXT)[  inside ];
            (*st->NEXT)[  inside ]           = save; 
          }
       else
          {
            save                             = (*st->NEXT)[ (B->x)[ inside ] ];
            (*st->NEXT)[ (B->x)[  inside ] ] = (*st->NEXT)[ outside ];
            (*st->NEXT)[ outside ]           = (*st->NEXT)[  inside ];
            (*st->NEXT)[  inside ]           = save;
          }

       /* Vertausche den obersten Block */
       save              = (B->x)[ outside ];
       (B->x)[ outside ] = (B->x)[  inside ];
       (B->x)[  inside ] = save;

       /* Jetzt muss PG noch ueberarbeitet werden. */
       /* Vorgehen : Alle Baeume, die von dem Pfad s->..->(B->s) weggehen
                     muessen gespiegelt werden.
       */

       u  = B->s;
       ti = B->ti;
       to = B->to;

       while ( u >= s )
             {
               /* Vertausche Inside und Outside und markiere alle
                  Folgebaeume als getauscht.
               */
               help1 = (*PG->AL)[ u ].x[ outside ];
               (*PG->AL)[ u ].x[ outside ] = (*PG->AL)[ u ].x[ inside ];
               (*PG->AL)[ u ].x[  inside ] = help1;

               help1 = nil;
               help2 = (*PG->AL)[ u ].x[ outside ];
               while ( help2 != nil && help2->name != ti )
                     {
                       if ( help2->name > u )
                          (*PG->AL)[ help2->name ].swap =
                             ( (*PG->AL)[ help2->name ].swap + 1 ) % 2; 
                       help1 = help2;
                       help2 = help2->next;
                     }

               help3 = nil;
               help4 = (*PG->AL)[ u ].x[ inside ];
               while ( help4 != nil && help4->name != to )
                     {
                       if ( help4->name > u )
                          (*PG->AL)[ help4->name ].swap =
                             ( (*PG->AL)[ help4->name ].swap + 1 ) % 2; 
                       help3 = help4;
                       help4 = help4->next;
                     }

               if ( help1 != nil ) help1->next = help4;
               else                (*PG->AL)[ u ].x[ outside ] = help4;
               if ( help3 != nil ) help3->next = help2;
               else                (*PG->AL)[ u ].x[  inside ] = help2;

               ti = u;
               to = 0;
               u = (*PG->AL)[ u ].father;
             }

     }

  return( test );

} /* end of TAUSCHE */





/* Loesche ueberfluessige Eintraege in L und R.
*/

void UPDATE_LR( st, v )

  STACK		*st;	   		/* Stack			      */
  int		v;			/* Knotennummer                       */
{
  int           x,y;			/* Knotennummern		      */
  int		side;			/* 0 = linke Seite,  1 = rechte Seite */
  BLOCK_NODE	*B;                     /* Der erste Block auf dem B-Stack    */

  /* Loesche solange den ersten Block (x0,x1), bis
     x0 oder x1 kleiner als v ist (aber != 0).
  */
  x = (st->B->x)[0];
  y = (st->B->x)[1];
  while (    x + y > 0                           &&
          ( (*st->STACK)[ x ] >= v || x == 0 )  &&
          ( (*st->STACK)[ y ] >= v || y == 0 )     )
        {
           /* Loesche das Topelement des Block-Stack */
           B = st->B; st->B = B->next; free( B );
           x = (st->B->x)[0];
           y = (st->B->x)[1];
        }
 
  if ( (*st->STACK)[ x ] >= v ) (st->B->x)[0] = 0;
  if ( (*st->STACK)[ y ] >= v ) (st->B->x)[1] = 0;
     
  /* Loesche alle Knoten vom L und R-Stack, die >= v sind.
  */
  for ( side = 0; side <= 1; side++ )
     while( (*st->NEXT)[side] != 0 && (*st->STACK)[ (*st->NEXT)[side] ] >= v )
         (*st->NEXT)[ side ] = (*st->NEXT)[ (*st->NEXT)[ side ] ];

} /* end of UPDATE_LR */



/* end of Include-Filde   PLANAR.ST */

