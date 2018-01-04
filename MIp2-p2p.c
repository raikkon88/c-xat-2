/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer p2p.c que implementa la interfície aplicació-usuari de          */
/* l'aplicació de MI amb l'agent de LUMI integrat, sobre les capes        */
/* d'aplicació de MI i LUMI (fent crides a les interfícies de les capes   */
/* MI -fitxers mi.c i mi.h- i LUMI -lumi.c i lumi.h- ).                   */
/* Autors: Feng Lin, Marc Sànchez                                         */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
/* Incloem "MIp2-mi.h" per poder fer crides a la interfície de MI i       */
/* "MIp2-lumi.h" per poder fer crides a la interfície de LUMI.            */

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */
/* Incloem "MIp1v4-mi.h" per poder fer crides a la interfície de MI       */
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "MIp2-mi.h"
#include "MIp2-lumi.h"

/* Definició de constants, p.e., #define MAX_LINIA 150                      */
#define TECLAT 				0
#define PORT_DEFECTE        0
#define ALL_IP              "0.0.0.0"

/* Defineixo la posició del sockets a la taula de sockets */
#define SCK_TCP             1
#define SCK_UDP             2

/* Definició de màxims de longitud dels diferents tipus d'string            */
#define MAX_BUFFER 			300
#define MAX_TYPE 			2
#define MAX_IP 				16
#define MAX_LINE			304
#define MAX_PORT			6
#define N_SOCKETS           3

/* Definició de funcions per el programa principal                          */
void EvalResult(int res, const int *sockets, int nSockets);
int getPort();


int main(int argc,char *argv[])
{
    // CONSTANTS que es poden cambiar.
   //char ALL_IP[MAX_IP] = "0.0.0.0";

   // Creem les linies de dades del protocol.
   char ipRemota        [MAX_IP];      // Ip on es connectarà tant per la connexió de servidor com la de client.
   char nickname        [MAX_BUFFER];
   char nicknameRemot   [MAX_BUFFER];
   char missatge        [MAX_BUFFER];

   bzero(ipRemota,      MAX_IP);
   bzero(nickname,      MAX_BUFFER);
   bzero(nicknameRemot, MAX_BUFFER);
   bzero(missatge,      MAX_BUFFER);

   int port; // Valor per defecte.
   int portLocal=0;
   int fi = 1;

   int socketsEscoltant[N_SOCKETS];
   int socketEscoltador;
   //int nSockets = 3;
   int nBytes;
   int res;
   int socketActiu;

   printf("/*-------------------------------------------------------------------*/\n");
   printf("/* XAT DE XARXES -> Autors : Fent Lin i Marc Sánchez\n");
   printf("/*---------------------------------------------------\n");
   printf("/* Entra un nick per comunicar-te\n");
   // S'obté el nickname.
   EvalResult(getNickname(nickname), NULL, 0);
   // Es configura el socket de teclat.
   socketsEscoltant[TECLAT] = TECLAT;

   // Es configura el socket UDP.
   socketsEscoltant[SCK_UDP] = LUMI_CrearSocketClient(IP_DEFAULT, PORT_DEFECTE);

   while(fi != 0){
       // Estic escoltant per TCP
       socketsEscoltant[SCK_TCP] = MI_IniciaEscPetiRemConv(PORT_DEFECTE);
//= socketEscoltador;
       EvalResult(socketEscoltador, socketsEscoltant, N_SOCKETS); // Evaluem el resultat de l'anterior instrucció
       // Obtenim el port i la ip locals assignades dinàmicament.
       portLocal = MI_DescobreixIpIPortDinamic(socketsEscoltant[SCK_TCP], ALL_IP);
       EvalResult(portLocal, socketsEscoltant, 2);
       printf("/*-------------------------------------------------------------------*/\n");
       printf("/* Informació del programa : */\n");
       printf("/*---------------------------------------------------\n");
       printf("/* Ip configurada de manera local : %s\n",ALL_IP);
       printf("/* Port configurat de manera local : %i\n", portLocal);

       // printf("Espera que es connectin o configura un port i una ip per aquest ordre per realitzar la connexió.\n");

       printf("/*-------------------------------------------------------------------*/\n");
       printf("/* Funcionament del programa : \n");
       printf("/* Opció 1 -> Espera que es connectin a la ip i port anteriors\n");
       printf("/* Opció 2 -> Entra port i ip per aquest ordre\n");
       printf("/*-------------------------------------------------------------------*/\n");

       socketActiu=MI_HaArribatPetiConv(socketsEscoltant[SCK_TCP]);
       EvalResult(socketActiu, socketsEscoltant, N_SOCKETS);
       if(socketActiu == TECLAT){
           port = getNumber();
           printf("/* Has entrat el port %i, ara entra la ip : \n", port);
           int ipLong = getIPAddress(ipRemota);
           EvalResult(ipLong, socketsEscoltant, N_SOCKETS);
           socketActiu = MI_DemanaConv(ipRemota, port, ALL_IP, &portLocal, nickname, nicknameRemot);
           EvalResult(socketActiu, socketsEscoltant, N_SOCKETS);
           socketsEscoltant[SCK_TCP] = (int)socketActiu;
       }
       // Si el socket actiu és el socket udp com que no estem conversant acceptem la conversa.
       else if(socketActiu == SCK_UDP) {

       }
       // Si el socket actiu no és un teclat i no és una petició TCP fem un accept.
       else {
           socketActiu = MI_AcceptaConv(socketActiu, ipRemota, &port, ALL_IP, &portLocal, nickname, nicknameRemot);
           EvalResult(socketActiu, socketsEscoltant, N_SOCKETS);
           socketsEscoltant[SCK_TCP] = socketActiu;

           int resultatAccio = 1;
           while(resultatAccio > 0){
               bzero(missatge, MAX_BUFFER);
               socketActiu = MI_HaArribatLinia(socketsEscoltant[SCK_TCP]);
               if(socketActiu == TECLAT){
                   EvalResult(readFromKeyboard(missatge, MAX_BUFFER), socketsEscoltant, N_SOCKETS);
                   if(strcmp(missatge,"$")!=1) break;
                   resultatAccio = MI_EnviaLinia(socketsEscoltant[SCK_TCP], missatge);
                   EvalResult(resultatAccio, socketsEscoltant, N_SOCKETS);
               }
               // Estem conversant, per tant hem de contestar amb un codi en concret.
               else if(socketActiu == SCK_UDP){

               }
               else{
                   resultatAccio = MI_RepLinia(socketActiu, missatge);
                   if(resultatAccio != 0){
                       printf("%s\n", missatge);
                   }
               }
           }
       }
       // -------------------------

       mostraDadesRemotes(nicknameRemot, port, ipRemota);

       // Intent de treure els prompts local i remot
       // S'ha intentat accedir al buffer de la pantalla per extreure el prompt si arriva algu i sinó es pinta l'altre.
       //---------------------------------------------------------------------------------------------------------------
       // char promptLocal[strlen(nickname)+1];
       // creaPrompt(promptLocal, nickname);
       //
       // char promptRemot[strlen(nicknameRemot)+1];
       // creaPrompt(promptRemot, nicknameRemot);



       // En cas que el resultat sigui -2 o -1 es tenquen tots els sockets.
       EvalResult(resultatAccio, socketsEscoltant, N_SOCKETS);

       int i;
       close(socketsEscoltant[SCK_TCP]);
       socketsEscoltant[SCK_TCP] = socketEscoltador;
       printf("/*-------------------------------------------------------------------*/\n");
       printf("/* CONVERSA ACABADA, vols seguir amb el programa??\n");
       printf("/* Entre '1' per continuar '0' per acabar.\n");
       printf("/*-------------------------------------------------------------------*/\n");
       fi = getNumber();
   }


   return (0);

 }

/***************** FUNCIONS D'EVALUACIÓ ************************************/

/**
* Evalua res, si és negatiu tenca els sockets i acaba l'apliacació, altrament no fa res.
* - res : és el resultat que s'ha d'evaluar
* - sockets : son tots els sockets que l'aplicació té oberts
* - nSockets : és el nombre de sockets que l'aplicació té oberts
* En cas que hagi tencat l'aplicació haurà mostrat per pantalla l'error que s'ha produït.
*/
void EvalResult(int res, const int *sockets, int nSockets){
	if(res < 0 && res != -2){
		int i = 0;
		// Faltaria buidar el buffer del TECLAT que sempre el poso a la posició 0 de sockets.
		while(sockets != NULL && i<nSockets){
				close(sockets[i]);
				i++;
		}
		MostraError("Error ");
		exit (-1);
	}
}

/***************** FUNCIONS DE LECTURA ************************************/

/**
 * Llegeix de teclat numberBytesToRead caràcters i els posa a inData
 * Retorna un -1 si quelcom ha anat malament.
 * Retorna el nombre de bytes llegits si tot ha anat bé, inData conté la cadena de caràcters llegida.
 */
int readFromKeyboard(char * inData, int numberBytesToRead){
	int bytes_llegits;
	if((bytes_llegits=read(0, inData, numberBytesToRead))==-1)
	{
    return (-1);
	}
	//printf("%i\n", bytes_llegits);
	inData[bytes_llegits-1] = '\0';
	//printf("%s\n",inData);

    return bytes_llegits;
}

/**
 * Crea el prompt donat un nickname i el desa al paràmetre prompt passat per referència.
 * prompt conté el nick + el caràcter '>'
 */
int creaPrompt(char * prompt, char * nick){
    bzero(prompt, strlen(nick)+1);
    strcpy(prompt, nick);
    prompt[strlen(nick)] = '>';
}

/**
 * Llegeix un número de teclat que fa referència al port.
 * Retorna aquest número com un enter positiu > 0.
 * Si hi ha error retorna -1.
 */
int getNumber(){
	char port[MAX_BUFFER];
	bzero(port, '\0');
	int res = readFromKeyboard(port, MAX_PORT);
	EvalResult(res, NULL, 0); // Indiquem que no hi ha sockets amb un 0 i així no petarà.
	int portNumber;
	EvalResult(portNumber = atoi(port), NULL, 0); // indiquem que no hi ha sockets amb un 0 i així no petarà.
	return portNumber;
}

/**
 * Mostra per pantalla els valors nicknameR, portR i ipR
 * Aquests valors son els valors de l'altre integrant de la connexió remota.
 */
int mostraDadesRemotes(char * nicknameR, int portR, char * ipR){
    printf("/*-------------------------------------------------------------------*/\n");
    printf("/* Establerta connexió amb %s a la ip %s i el port %i\n", nicknameR, ipR, portR);
    printf("/*-------------------------------------------------------------------*/\n");
}

/**
 * Obté el nickname per teclat
 * nickname conté el nick obtingut si tot va bé,
 * retorna el nombe de bytes llegits si tot és correcte, un valor negatiu si no.
 */
int getNickname(char * nickname){
	return readFromKeyboard(nickname, MAX_BUFFER);
}

/**
 * Obté la ip per teclat
 * ip conté el valor obtingut si tot va bé,
 * retorna el nombe de bytes llegits si tot és correcte, un valor negatiu si no.
 */
int getIPAddress(char * ip){
	return readFromKeyboard(ip, MAX_IP);
}
