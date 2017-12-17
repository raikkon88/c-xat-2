/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer nodelumi.c que implementa la interfície aplicació-administrador */
/* d'un node de LUMI, sobre la capa d'aplicació de LUMI (fent crides a la */
/* interfície de la capa LUMI -fitxers lumi.c i lumi.h-).                 */
/* Autors: Feng Lin, Marc Sànchez                                         */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
/* Incloem "MIp2-lumi.h" per poder fer crides a la interfície de LUMI     */
#include "DataSet.c"
#include "MIp2-lumi.h"

#define DB_FILE "usuaris.txt"

/* Definició de constants, p.e., #define MAX_LINIA 150                    */


int main(int argc,char *argv[])
{
    struct DataSet d;
    init(&d);
	int lectura = LUMI_llegirUsuaris(&d, DB_FILE);
	showDataSet(&d);
	int escriptura = LUMI_escriureUsuaris(&d, DB_FILE);
	printf("%i\n", escriptura);
}
