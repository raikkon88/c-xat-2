/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer agelumi.c que implementa la interfície aplicació-usuari d'un    */
/* agent d'usuari de LUMI sol, no integrat a l'aplicació de MI, sobre la  */
/* capa d'aplicació de LUMI (fent crides a la interfície de la capa LUMI  */
/* -fitxers lumi.c i lumi.h- ).                                           */
/* Autors: Feng Lin, Marc Sànchez                                         */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
/* Incloem "MIp2-lumi.h" per poder fer crides a la interfície de LUMI     */
#include <stdio.h>
#include "MIp2-lumi.h"

/* Definició de constants, p.e., #define MAX_LINIA 150                    */
#define IP_DEFAULT      "0.0.0.0"       // Ip per defecte que utilitza el servidor per escoltar peticions UDP.
#define PORT_DEFAULT    0            // Port on el servidor escoltarà peticions UDP per defecte.

#define IP_SERVER      "192.168.1.103"
#define PORT_SERVER    8765

int main(int argc,char *argv[])
{

	char username[50];
	int resultatAccio;

	int SckLUMI = LUMI_CrearSocketClient(IP_DEFAULT,PORT_DEFAULT);
	if(SckLUMI == -1){
		printf("\n...error al crear el socket.\n");
	}

	printf("\n scoket client inicialitzat \n");

	//registre
	printf(" Entra el teu username: ");
	scanf("%s",username);

	//enviar peticio registre
	resultatAccio=LUMI_PeticioRegistre(SckLUMI,username,IP_SERVER,PORT_SERVER);

	if(resultatAccio==-1) printf("\n...error fent peticio registre \n");

	return 0;

 }
