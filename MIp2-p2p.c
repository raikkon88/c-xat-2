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
#define TIMEOUT             3

/* Defineixo la posició del sockets a la taula de sockets */
#define SCK_TCP             1
#define SCK_UDP             2

#define DESCONNECTAT        0
#define CONNECTAT           1

#define DEIXAR_SOCKET_OBERT 0
#define TANCAR_SOCKET       1


#define FI_PROGRAMA         '0'

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
int fiPrograma(int * LlistaSck, int nSockets, char * missatge);
int conversa(int socketActiu, int * socketsEscoltant, const char * nickRemot, int tancarSocket);


int main(int argc,char *argv[])
{
    // CONSTANTS que es poden cambiar.
    //char ALL_IP[MAX_IP] = "0.0.0.0";

    // Creem les linies de dades del protocol.
    char ipRemota            [MAX_IP];      // Ip on es connectarà tant per la connexió de servidor com la de client.
    char ipServer            [MAX_IP];
    char adrecaMI            [MAX_BUFFER];
    char nicknameRemot       [MAX_BUFFER];
    char missatge            [MAX_BUFFER];
    char ipTcpLocal          [MAX_IP];
    char ipUdpLocal          [MAX_IP];
    char dnsPreguntador      [MAX_BUFFER];
    char usuariPreguntador   [MAX_BUFFER];
    char instruccio          [MAX_BUFFER];


    // Inicialitzem tots els strings.
    bzero(ipRemota,             MAX_IP);
    bzero(ipServer,             MAX_IP);
    bzero(adrecaMI,             MAX_BUFFER);
    bzero(nicknameRemot,        MAX_BUFFER);
    bzero(missatge,             MAX_BUFFER);
    bzero(ipTcpLocal,           MAX_IP);
    bzero(ipUdpLocal,           MAX_IP);
    bzero(dnsPreguntador,       MAX_BUFFER);
    bzero(usuariPreguntador,    MAX_BUFFER);



    int port; // Valor per defecte.
    int portTCPRem=0;
    int portTCPLocal=0;
    int portUDPLocal=0;
    char fi = '1';
    int nInstruccio = 0;

    int socketsEscoltant[N_SOCKETS];
    int socketEscoltador;
    int estat = DESCONNECTAT;

    int nBytes;
    int res;
    int socketActiu;

    printf("/*-------------------------------------------------------------------*/\n");
    printf("/* XAT DE XARXES -> Autors : Fent Lin i Marc Sánchez\n");
    printf("/*---------------------------------------------------\n");
    printf("/* Entra una adreça MI per comunicar-te\n");
    // S'obté el nickname.
    EvalResult(getMIAddress(adrecaMI), NULL, 0);

    char nickname[MAX_BUFFER];
    char domini[MAX_BUFFER];
    bzero(nickname, MAX_BUFFER);
    bzero(domini, MAX_BUFFER);
    MI_UsuariIDnsDeMi(adrecaMI, domini, nickname);

    printf("/*-------------------------------------------------------------------*/\n");
    printf("/* Informació del programa : \n");
    printf("/*---------------------------------------------------\n");

    // Es configura el socket de teclat.
    socketsEscoltant[TECLAT] = TECLAT;
    printf("/* SOCKET %i Entrada de teclat estàndard configurada. \n", socketsEscoltant[0]);
    printf("/* Ip configurada de manera local : %s\n",ALL_IP);

    // Es configura el socket UDP i s'emplenen els camps de ip i port locals.
    socketsEscoltant[SCK_UDP] = LUMI_CrearSocketClient(ALL_IP, PORT_DEFECTE);
    EvalResult(socketsEscoltant[SCK_UDP], socketsEscoltant, N_SOCKETS); // Evaluem el resultat de l'anterior instrucció
    EvalResult(LUMI_getIpiPortDeSocket(socketsEscoltant[SCK_UDP], ipUdpLocal, &portUDPLocal), socketsEscoltant, N_SOCKETS);
    printf("/* SOCKET %i Ip i Port UDP configurat de manera local : %s -> %i\n", socketsEscoltant[SCK_UDP], ipUdpLocal, portUDPLocal);

    // Es configura el socket TCP i s'emplenen els camps de ip i port locals.
    socketsEscoltant[SCK_TCP] = MI_IniciaEscPetiRemConv(PORT_DEFECTE);
    socketEscoltador = socketsEscoltant[SCK_TCP];
    EvalResult(socketsEscoltant[SCK_TCP], socketsEscoltant, N_SOCKETS); // Evaluem el resultat de l'anterior instrucció
    EvalResult(MI_getIpiPortDeSocket(socketsEscoltant[SCK_TCP], ipTcpLocal, &portTCPLocal), socketsEscoltant, N_SOCKETS);
    printf("/* SOCKET %i Ip i Port TCP configurat de manera local : %s -> %i\n", socketsEscoltant[SCK_TCP], ipTcpLocal, portTCPLocal);

    int resultat = LUMI_EnviaPeticio((int *)&socketsEscoltant, SCK_UDP, nickname, domini, "", "", "", 0, REGISTRE, TIMEOUT);
    if(resultat == REGISTRE_CORRECTE){
        // S'ha registrat correctament, escriure per pantalla.
        printf("/* Registrat contra el domini : %s\n", domini);
    }
    else{
        // Error en el registre, s'ha d'escriure el log, acabar la app. o tornar a demanar el nickname.
        fiPrograma(socketsEscoltant, N_SOCKETS, "Error al registrar contra el domini");
        fi = FI_PROGRAMA;
    }

    // EN aquest punt estic registrat!
    while(fi != FI_PROGRAMA){

        // Purguem variables que s'han de tornar a fer servir.
        bzero(instruccio, MAX_BUFFER);
        bzero(missatge, MAX_BUFFER);
        bzero(usuariPreguntador, MAX_BUFFER);
        bzero(dnsPreguntador, MAX_BUFFER);

        printf("/*-------------------------------------------------------------------*/\n");
        printf("/* Funcionament del programa : \n");
        printf("/* Opció 1 -> Espera que es connectin a la ip i port anteriors\n");
        printf("/* Opció 2 -> Entra @MI amb format nickname@dom.ini per connectar-te\n");
        printf("/* Opció 3 -> Efectúa el desregistre i finalitza el programa entrant un 0\n");
        printf("/*-------------------------------------------------------------------*/\n");

        //socketActiu=MI_HaArribatPetiConv(socketsEscoltant[SCK_TCP]);
        socketActiu = LUMI_HaArribatAlgunaCosa(socketsEscoltant, N_SOCKETS);
        //printf("%s -> %i\n", "CUIDADO!", socketActiu);
        EvalResult(socketActiu, socketsEscoltant, N_SOCKETS);
        if(socketActiu == socketsEscoltant[TECLAT]){
            nInstruccio = readFromKeyboard(instruccio, MAX_BUFFER);
            //printf("%d\n", nInstruccio);
            if(nInstruccio == 1 && instruccio[0] == FI_PROGRAMA){
                int resultat = LUMI_EnviaPeticio((int *)&socketsEscoltant, SCK_UDP, nickname, domini, "", "", "", 0, DESREGISTRE, TIMEOUT);
                if(resultat == DESREGISTRE_CORRECTE){
                    // S'ha registrat correctament, escriure per pantalla.
                    printf("/* Desregistrat contra el domini : %s\n", domini);
                }
                else {
                    printf("/- Hi ha hagut un problema al desregistrar amb el domini : %s\n", domini);
                }
                fi = FI_PROGRAMA;
                fiPrograma(socketsEscoltant, N_SOCKETS, "Has finalitzat el programa, bye bye!!");
            }
            else {
                // Extraiem les dades amb qui es vol connectar
                sscanf(instruccio, "%[^'@']@%s", usuariPreguntador, dnsPreguntador);
                int resultat = LUMI_EnviaPeticio((int *)&socketsEscoltant, SCK_UDP, nickname, domini, usuariPreguntador, dnsPreguntador, ipRemota, &portTCPRem, LOCALITZACIO, TIMEOUT);
                if(resultat == LOCALITZACIO_ONLINE_OCUPAT){
                    printf("/* El client %s del domini %s està online peró conversant amb un altre. \n", nickname, domini);
                }
                else if(resultat == LOCALITZACIO_ONLINE_LLIURE){
                    printf("/* Connectant amb %s@%s a la ip %s, port %i. \n", usuariPreguntador, dnsPreguntador, ipRemota, portTCPRem);
                    estat = CONNECTAT;
                    socketEscoltador = socketsEscoltant[SCK_TCP];
                    socketsEscoltant[SCK_TCP] = MI_DemanaConv(ipRemota, portTCPRem, ipTcpLocal, &portTCPLocal, usuariPreguntador, nickname);
                    EvalResult(socketsEscoltant[SCK_TCP], socketsEscoltant, N_SOCKETS);
                    printf("%s\n", "s'inicia la conversa... crec que no hi arriva...");
                    conversa(socketsEscoltant[SCK_TCP], socketsEscoltant, nickname, TANCAR_SOCKET);
                }
                else if(resultat == LOCALITZACIO_NO_EXISTEIX){
                    printf("/* El client %s del domini %s no existeix!. \n", nickname, domini);
                }
                else if(resultat == LOCALITZACIO_OFFLINE){
                    printf("/* El client %s del domini %s està desconnectat. \n", nickname, domini);
                }
                else{
                    // TODO : El missatge format és incorrecte
                }
            }
        }
        // Si el socket actiu és el socket udp com que no estem conversant acceptem la conversa.
        else if(socketActiu == socketsEscoltant[SCK_UDP]) {
            int peticio = LUMI_ProcessaClient(socketActiu, missatge, usuariPreguntador, dnsPreguntador, "", 0);
            if(peticio == LOCALITZACIO_PETICIO){
                // S'ha de retornar el missatge : AL0preguntador@dnsPreguntador#IP#PORT_TCP
                resultat = LUMI_ResponLocalitzacio(socketsEscoltant[SCK_UDP], ONLINE_LLIURE, usuariPreguntador, dnsPreguntador, ipTcpLocal, portTCPLocal);
            }
        }
        // Si el socket actiu no és un teclat i no és una petició TCP fem un accept.
        else {
            printf("Esta fent una petició TCP\n");
            socketEscoltador = socketsEscoltant[SCK_TCP];
            socketsEscoltant[SCK_TCP] = MI_AcceptaConv(socketActiu, ipRemota, &port, ALL_IP, &portTCPLocal, adrecaMI, nicknameRemot);
            estat = CONNECTAT;
            EvalResult(socketsEscoltant[SCK_TCP], socketsEscoltant, N_SOCKETS);
            conversa(socketsEscoltant[SCK_TCP], socketsEscoltant, nicknameRemot, DEIXAR_SOCKET_OBERT);
            socketsEscoltant[SCK_TCP] = socketEscoltador;
        }
        // -------------------------
        estat = DESCONNECTAT;
        //
        //mostraDadesRemotes(nicknameRemot, port, ipRemota);
        //MI_AcabaConv(socketsEscoltant[SCK_TCP]);
        // Intent de treure els prompts local i remot
        // S'ha intentat accedir al buffer de la pantalla per extreure el prompt si arriva algu i sinó es pinta l'altre.
        //---------------------------------------------------------------------------------------------------------------
        // char promptLocal[strlen(nickname)+1];
        // creaPrompt(promptLocal, nickname);
        //
        // char promptRemot[strlen(nicknameRemot)+1];
        // creaPrompt(promptRemot, nicknameRemot);

        //socketsEscoltant[SCK_TCP] = socketEscoltador;
        // while(fi == '1' || fi != '0'){
        //
        //     nInstruccio = readFromKeyboard(instruccio, MAX_BUFFER);
        //     if(nInstruccio == 1){
        //         fi = instruccio[0];
        //     }
        // }
    }

    return (0);

 }

int conversa(int socketActiu, int * socketsEscoltant, const char * nickRemot, int tancarSocket){
    printf("/*-------------------------------------------------------------------*/\n");
    printf("/* S'HA ESTABLERT UNA CONVERSA TCP AMB %s\n", nickRemot);
    printf("/* Parla o espera que et parlin...\n");
    printf("/*-------------------------------------------------------------------*/\n");

    int resultatAccio = 1;
    while(resultatAccio > 0){
        char missatge[MAX_BUFFER];
        bzero(missatge, MAX_BUFFER);
        socketActiu = MI_HaArribatLinia(socketsEscoltant[SCK_TCP]);
        if(socketActiu == TECLAT){
            EvalResult(readFromKeyboard(missatge, MAX_BUFFER), socketsEscoltant, N_SOCKETS);
            if(strcmp(missatge,"$")!=1) break;
            resultatAccio = MI_EnviaLinia(socketsEscoltant[SCK_TCP], missatge);
            EvalResult(resultatAccio, socketsEscoltant, N_SOCKETS);
        }
        else{
            resultatAccio = MI_RepLinia(socketActiu, missatge);
            if(resultatAccio != 0){
                printf("%s\n", missatge);
            }
       }
        // Estem conversant, per tant hem de contestar amb un codi en concret.
        // else if(socketActiu == SCK_UDP){
        //     int peticio = LUMI_ProcessaClient(socketsEscoltant[SCK_UDP], missatge, usuariPreguntador, dnsPreguntador, "", 0);
        //     if(peticio == LOCALITZACIO_PETICIO){
        //         // S'ha de retornar el missatge : AL3preguntador@dnsPreguntador
        //         resultat = LUMI_ResponLocalitzacio(socketsEscoltant[SCK_UDP], ONLINE_OCUPAT, usuariPreguntador, dnsPreguntador, "", 0);
        //     }
        // }
    }
    //En cas que el resultat sigui -2 o -1 es tenquen tots els sockets.
    EvalResult(resultatAccio, socketsEscoltant, N_SOCKETS);
    printf("/*-------------------------------------------------------------------*/\n");
    printf("/* CONVERSA ACABADA Seguim amb l'execució del programa... \n");
    printf("/*-------------------------------------------------------------------*/\n");
    if(tancarSocket == TANCAR_SOCKET){
        MI_AcabaConv(socketsEscoltant[SCK_TCP]);
    }
    return 0;
}

/***************** FUNCIONS DEL PROGRAMA ***********************************/
int fiPrograma(int * LlistaSck, int nSockets, char * missatge){

    printf("/* %s\n",missatge);
    printf("/* Fi de l'execució...\n");
    printf("/*-------------------------------------------------------------------*/\n");
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
	inData[bytes_llegits-1] = '\0';

    return bytes_llegits - 1;
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
int getMIAddress(char * adrecaMI){
	return readFromKeyboard(adrecaMI, MAX_BUFFER);
}

/**
 * Obté la ip per teclat
 * ip conté el valor obtingut si tot va bé,
 * retorna el nombe de bytes llegits si tot és correcte, un valor negatiu si no.
 */
int getIPAddress(char * ip){
	return readFromKeyboard(ip, MAX_IP);
}
