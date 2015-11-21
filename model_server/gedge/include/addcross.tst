
/******************************************************************************/
/*									      */
/* Include-File 	ADDCROSS.TST                                          */
/*									      */
/* Letzte Aenderung     02.02.88					      */
/*									      */
/* Prozeduren           auswertung_Al( *DYN_GRAPH )                           */
/*			Testausgabe des Dynamischen Graphen.                  */
/*									      */
/*			asuwertung_DG( *DUAL_GRAPH )			      */
/*			Testausgabe des Dualen Graphen.			      */
/*									      */
/******************************************************************************/




/* Testausgabe fuer den DYN_GRAPH G.
*/

auswertung_Al( G )

  DYN_GRAPH	*G;			/* Dynamischer Graph                  */
{
  AL		*Al;			/* Eine Adjazenzliste		      */
  DYN_NODE	*DN; 			/* Ein Knoten einer Adjazenzliste     */

  printf("\nNeue geordnete Adjazenzliste :\n");

  Al = G->first_AL;
  while ( Al != nil )
        {
          printf( "%d --> ", Al->name );
          DN = Al->first;
          while ( DN != nil )
                {
                  printf( "%d, ", DN->name->name );
                  DN = DN->next;
                }
          printf("\n");
          Al = Al->next_AL;
        }

} /* end of auswertung_Al */




/* Testausgabe des Dualen Graphen.
   ( Der Duale Graph ist der Flaechengraph des Graphen G. )
*/

auswertung_DG( DG )

  DUAL_GRAPH	*DG;			/* Dualer Graph        	              */
{
  FACE		*face;			/* Eine Flaeche                       */
  E_NODE	*en;        		/* Eine Kante einer Flaeche           */
  int		i;   			/* Flaechennummer		      */

  printf("\nFlaechengraph : \n");
  face = DG->face;
  i = 0;
  while ( face != nil )
        {
          i++;
          printf( "Flaeche %d : ", i );
          en = face->first;
          while ( en != nil )
 	        {
                  printf("(%d,%d), ", en->edge->x->name, en->edge->y->name );
 		  en = en->next;
                }
          printf("\n");
          face = face->next;
        }

} /* end of auswertung_DG */



/* end of Include-File  ADDCROSS.TST */

