#ifndef _PLANAR_STC
#define _PLANAR_STC


/******************************************************************************/
/*									      */
/* Include-File 	PLANAR.TST                                            */
/*									      */
/* Letzte Aenderung     03.02.88					      */
/*									      */
/* Prozeduren		auswertung_STACK( *STACK )			      */
/*			Testausgabe des Stacks				      */
/*									      */
/*			auswertung_PG( *PLANAR_GRAPH )			      */
/*			Testausgabe des planaren Graphen                      */
/*									      */
/*									      */
/******************************************************************************/


/* Ausgabe des aktuellen Stacks. */

auswertung_STACK( st )

  STACK 	*st;
{
  STACK_NODE 	*S;
  BLOCK_NODE 	*B;
  int		i;

  printf("S-Keller   : ");
  S = st->S;
  while ( S != nil )
        { printf("(%d,%d,%d)} ",S->s, S->w, S->side ); S = S->next; }

  printf("\nL-Keller   : ");
  i = 0;
  while ( (*st->NEXT)[ i ] != 0 )
        { i = (*st->NEXT)[ i ]; printf("%d, ", (*st->STACK)[ i ] ); }

  printf("\nR-Keller   : ");
  i = 1;
  while ( (*st->NEXT)[ i ] != 0 )
        { i = (*st->NEXT)[ i ]; printf("%d, ", (*st->STACK)[ i ] ); }

  printf("\nB-Keller   : ");
  B = st->B;
  while ( B != nil )
        { printf("(%d,%d,(s=%d,ti=%d,to=%d)) ", 
          (*st->STACK)[ (B->x)[0] ], (*st->STACK)[ (B->x)[1] ] ,
          B->s, B->ti, B->to );
          B = B->next;
        }

  printf("\n\n");

} /* end of auswertung_STACK */




/* Ausgabe der Frondlisten FL( 1 ) .. FL( n ). */

auswertung_FL( FL, PG )

  FL_LISTE  	FL;
  PLANAR_GRAPH  *PG;
{
  int 		v,n;
  f_node 	*pf;
  w_node 	*pw;

  n = PG->V;

  for ( v=1; v<=n; v++ )
      {
        pf = (*FL)[v];
        printf( "FL(%d)= ",v);
        while ( pf != nil )
              {
                pw = pf->first_w;
                while ( pw != nil )
                      {
                        printf("(%d,%d(%d)) ",pw->w,pf->f,pw->n );
                        pw = pw->next_w;
                      }
                pf = pf->next_f;
             }
        printf("\n");
      }

printf("\n");

} /* end of auswertung_FL */



 
/* Testausgabe von PG.
*/

void auswertung_PG( PG )

  PLANAR_GRAPH	*PG;
{
  DELETED_FROND	*DF;
  int           n_del;
  int		i;
  P_NODE        *PN;

  for ( i=1; i<=PG->V; i++ )
      {
        printf("%d ",(*PG->AL)[i].swap);
        printf("%d(%d) -->  ", i, (*PG->trans)[ i ] );
        PN = (*PG->AL)[ i ].x[0];
        while ( PN != nil )
              {
                printf("%d(%d),", PN->name, (*PG->trans)[ PN->name ] );
                PN = PN->next;
              }
        printf("  #  ");
        PN = (*PG->AL)[ i ].x[1];
        while ( PN != nil )
              {
                printf("%d(%d),", PN->name, (*PG->trans)[ PN->name ] );
                PN = PN->next;
              }
        printf("\n");
      }

  if ( PG->DEL_FROND == nil ) 
       printf("Es wurden keine FRONDS geloescht !\n");
  else
     {
       n_del = 0;
       DF = PG->DEL_FROND;
       while ( DF != nil )
             { 
               n_del++;
               printf("Kante %d(%d) --> %d(%d)\n", 
               DF->x, (*PG->trans)[ DF->x ], DF->y, (*PG->trans)[ DF->y ] );
               DF = DF->next;
             }
       printf("Es wurden %d FRONDS geloescht !\n",n_del);
     }

} /* end of auswertung_PG */



/* end of Include-File  PLANAR.TST */


#endif
