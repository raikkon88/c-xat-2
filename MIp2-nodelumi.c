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

/** INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MIp2-lumi.h"

/* Definició de constants, p.e., #define MAX_LINIA 150                    */
#define DB_FILE         "MIp2-nodelumi.cfg"       // Nom del fitxer on es desen els usuaris en el servidor
#define IP_DEFAULT      "0.0.0.0"           // Ip per defecte que utilitza el servidor per escoltar peticions UDP.
#define PORT_DEFAULT    8765                // Port on el servidor escoltarà peticions UDP per defecte.

int mostrarMenu(){
	printf("\n##############################################################################\n");
	printf("| Gestio del node:                                                             |\n");
	printf("################################################################################\n");
	printf("| alta   : Donar d'alta un usuari                                              |\n");
	printf("| baixa  : Donar de baixa un usuari                                            |\n");
	printf("| llista : Llistar els usuaris                                                 |\n");
	printf("| menu   : Mostrar menú                                                        |\n");
	printf("| sortir : Sortir del node                                                     |\n");
	printf("################################################################################\n");
	return 0;
}




int main(int argc,char *argv[])
{
    // Declaració de variables.
    int resultatAccio; // Control intern d'error de l'aplicació
    struct DataSet d;  // Estructura de dades per el programa servidor
    // Inicialització del servidor.
    resultatAccio = LUMI_inicialitza_servidor(&d, DB_FILE, IP_DEFAULT, PORT_DEFAULT);
    if(resultatAccio < 0){
        printf("ERROR -> %s\n","Error en la inicialització del servidor.");
    }

    printf("%s\n", "Initialization success ... ");
    int socket = resultatAccio;
    int i;
    /*
    resultatAccio = LUMI_start(socket, &d);
    if( resultatAccio < 0){
        printf("ERROR -> %s\n","Error en el processament de peticions.");
    }
    */
    char buff[100];
    int run = 1;
    mostrarMenu();
    while(run == 1){
		resultatAccio = LUMI_start(socket, &d);
		if( resultatAccio < 0){
			printf("ERROR -> %s\n","Error en el processament de peticions.");
		}
		else if (resultatAccio==0){ //entrat per teclat
			fgets(buff,100, stdin);
			if(strcmp(buff,"sortir\n") == 0){
				run = 0;
			}else if(strcmp(buff,"menu\n") == 0){
				mostrarMenu();
			}else if(strcmp(buff,"llista\n") == 0){
			/*********** llistar clients ******************/
				printf("################################################################################\n");
				printf("| Llistat d'usuaris                                                            |\n");
				printf("################################################################################\n");
				
				for(i=0;i<d.nClients;i++){
					printf("| %s - estat: %d - ip: %s - port: %d\n", d.data[i].username,d.data[i].online,d.data[i].ip, d.data[i].port);
				}
				
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
			/************** llistar clients ***************/
			}else if(strcmp(buff,"baixa\n") == 0){
			/*********** donar baixa client ****************/
				char usuari[50];
				printf("################################################################################\n");
				printf("| Donar de baixa un usuari                                                     |\n");
				printf("################################################################################\n");
				printf("| Entra nom d'usuari: ");
				scanf("%s",usuari);
				int trobat = 0;
				i = 0;
				while(trobat == 0 && i < d.nClients){
					if(strcmp(d.data[i].username,usuari) == 0){
						trobat = 1;
					}
					i++;
				}
				if(trobat == 1){
					int n = i-1;
					for(n; n < d.nClients; n++){
						strcpy(d.data[n].username,d.data[n+1].username);
						strcpy(d.data[n].ip,d.data[n+1].ip);
						d.data[n].port = d.data[n+1].port;
						d.data[n].online = d.data[n+1].online;
						d.data[n].peticionsAcumulades = d.data[n+1].peticionsAcumulades;
						
					}
					d.nClients = d.nClients - 1;
					printf("usuari donat de baixa correctament\n");
				}
				else{
					printf("l'usuari %s no existeix.\n", usuari);
				}
				
			/************* donar baixa client **************/
			}else if(strcmp(buff,"alta\n") == 0){
			/************* donar alta client **************/
				char usuari[50];
				printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				printf("| Donar d'alta un usuari                                                       |\n");
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				printf("| Entra nom d'usuari: ");
				scanf("%s",usuari);
				int trobat = 0;
				i = 0;
				while(trobat == 0 && i < d.nClients){
					if(strcmp(d.data[i].username,usuari) == 0){
						trobat = 1;
					}
					i++;
				}
				if(trobat == 0){
					
					strcpy(d.data[d.nClients].username,usuari);
					strcpy(d.data[d.nClients].ip,"");
					d.data[d.nClients].port=-1;
					d.data[d.nClients].online=0;
					d.data[d.nClients].peticionsAcumulades=0;
					d.nClients = d.nClients + 1;
					printf("usuari donat d'alta correctament\n");
				}
				else{ //Si exisrteix
					printf("usuari %s ja existeix.\n", usuari);
				}
			/************* donar alta client ***************/
			}
			
		}
		
	}
    
    
    //TODO : Falta buidar el buffer de teclat si s'ha acabat.
    /*************** actualitzar el fitxer .cfg abans d'acabar el proces ****************/
    
    FILE *fp = fopen(DB_FILE,"w"); // el segon parametre "w" fa buidar tot el contingut del fitxer
    
    if(fp == NULL ) {
			perror("error amb el fitxer MIp2-nodelumi.cfg");
			return -1;
	}
	char domini[MAX_LENGHT_DOMINI];
	sprintf(domini,"%s",d.domini);
	fprintf(fp,"%s",domini);
	fprintf(fp,"\n");
	
	for(i=0;i<d.nClients;i++){
			fprintf(fp,"%s",d.data[i].username);
			fprintf(fp,"\n");
	}    
    

	return 0;
}
