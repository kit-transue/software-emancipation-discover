#ifndef _ADTUGRAPH_TST
#define _ADTUGRAPH_TST


/******************************************************************************/
/*									      */
/* Include-File 	ADTUGRAPH.TST                                         */
/*									      */
/* Letzte Aenderung     08.02.88					      */
/*									      */
/* Prozeduren		AUSWERTUNG_UNDIR_GRAPH( *UNDIR_GRAPH )		      */
/*			Testausgabe eines ungerichteten Graphen.              */
/*									      */
/******************************************************************************/


void AUSWERTUNG_UNDIR_GRAPH ( G )

  UNDIR_GRAPH  *G;
{
  int GET_UGRAPH_TRANS_NO();   		/* i-te Nummer in de Transformationsta*/
  int GET_FIRST_UNODE();       		/* Bestimme den ersten Nachbarknoten  */
  int GET_NEXT_UNODE();          	/* Bestimme den naechsten Nachbarn    */

  NODE 	*help; 

  int  	i, j;

  if ( G == nil ) printf("\n\nDer Graph ist leer\n");
  else
    {
      printf("\n\nAdjazenzenlisten fuer %d Knoten.\n\n", G->n);

      for( i=1; i<=G->n; i++ )
         {
            printf("%d -> ", GET_UGRAPH_TRANS_NO( G,i ) ); 
            j = GET_FIRST_UNODE( G, i, &help );
            while ( j!=0 )
                  {
                    printf("%d, ", GET_UGRAPH_TRANS_NO( G,j )  ); 
                    j = GET_NEXT_UNODE( &help );
                  }
            printf("\n");
         }
    }

} /* end of AUSWERTUNG_UNDIR_GRAPH */



/* end of Include-File  ADTUGRAPH.TST */

#endif
