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

#include <stdio.h>
#include "MIp2-lumi.h"

#define DB_FILE         "usuaris.txt"   // Nom del fitxer on es desen els usuaris en el servidor
#define IP_DEFAULT      "0.0.0.0"       // Ip per defecte que utilitza el servidor per escoltar peticions UDP.
#define PORT_DEFAULT    8765            // Port on el servidor escoltarà peticions UDP per defecte.

/* Definició de constants, p.e., #define MAX_LINIA 150                    */


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
    resultatAccio = LUMI_start(socket, &d);
    if(resultatAccio < 0){
        printf("ERROR -> %s\n","Error en el processament de peticions.");
    }

    showDataSet(&d);

}
