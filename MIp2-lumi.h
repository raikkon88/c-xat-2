/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de lumi.c                                             */
/*                                                                        */
/* Autors: Feng Lin, Marc Sànchez                                         */
/*                                                                        */
/**************************************************************************/

#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

/** Definició de constants */

#define MAX_MESSAGE_LENGHT          200
#define TOTAL_LENGHT_MESSAGE        204
#define MAX_IP_LENGTH               16
#define MAX_LINIA		             200
#define IP_AUTO                     "0.0.0.0"
#define PORT_AUTO                   0
#define DEFAULT_PORT_SERVER         8765
#define MAX_NOMBRE_RETRANSMISSIONS  3

// Tipus de peticions
#define ACCEPTAT_MISSATGE           'A'
#define REGISTRE                    'R'
#define DESREGISTRE                 'D'
#define LOCALITZACIO                'L'

// Estat de les respostes
#define CORRECTE                    '0'
#define INCORRECTE                  '1'

// Estat del client amb el que es vol parlar
#define ONLINE_LLIURE               '0'
#define OFFLINE                     '1'
#define NO_EXISTEIX                 '2'
#define ONLINE_OCUPAT               '3'

// Tipus de retorn de les peticions.
#define REGISTRE_CORRECTE           10
#define REGISTRE_INCORRECTE         11
#define DESREGISTRE_CORRECTE        12
#define DESREGISTRE_INCORRECTE      13
#define MISSATGE_ERRONI             14
#define LOCALITZACIO_ONLINE_LLIURE  15
#define LOCALITZACIO_OFFLINE        16
#define LOCALITZACIO_NO_EXISTEIX    17
#define LOCALITZACIO_ONLINE_OCUPAT  18
#define LOCALITZACIO_PETICIO        19


/**
 * Struct de registre de la taula que desa els usuaris. ************************************************************
 *
 */
struct Registre {

    // Camps que ha de tenir l'struct de registre
    char username[50];          // Nom d'usuari
    int  port;                  // port UDP associat al nom d'usuari
    char ip[MAX_IP_LENGTH];     // Ip associada a l'usuari.
    int  online;                // Estipula si el registre, és a dir l'usuari està online.
    int  peticionsAcumulades;   // contador de peticions acumulades que serveix per veure si el client està desconnectat

};

// DECLARACIÓ DE FUNCIONS DE L'STRUCT USUARIS
struct Registre create(char* _username);
void ini(struct Registre * r, char* _username, int _port, char* _ip, int _online, int _peticionsAcumulades);
void show(struct Registre * reg);
int compare(struct Registre * r1, struct Registre * r2);
int isOnline(struct Registre * r);
int desconnectat(struct Registre * r1, int maximPeticions);

/**
 * Definició de la taula de clients ************************************************************
 */
#define MAX_CLIENTS         200     /** Màxim de clients permesos */
#define MAX_LENGHT_DOMINI   200     /** Màxim de caràcters per el domini */

struct DataSet {

    struct Registre data[MAX_CLIENTS];  // Taula de clients que es poden connectar amb el servidor.
    char domini[MAX_LENGHT_DOMINI];     // Nom del domini
    int nClients;                       // Nombre de clients inscrits.
    int log;                            // File descriptor que utilitza el dataset per escriure en el fitxer de log.

};

// DECLARACIÓ DE FUNCIONS DE L'STRUCT DATASET

// Inicialitza una taula de clients de màxim MAX_CLIENTS llargada
void init(struct DataSet *);
// Mostra el dataset per pantalla (el contingut actual en memòria.)
void showDataSet(struct DataSet * ds);
// Afegeix un registre a la taula.
void insertRegistre(struct DataSet * ds, struct Registre *r);
// Esborra un registre de la taula.
int deleteRegistre(struct DataSet * ds, struct Registre * r);
// Modifica un registre, usat per emplenar les adreces quan només es contenen els noms.
int updateRegistre(struct DataSet * ds, struct Registre * r);
// Retorna la posició de la taula on es troba el registre r
// o bé un registre n que té el mateix nom que r, o un registre n que té la mateixa ip i port que r.
int getPosicio(struct DataSet * ds, struct Registre * r);
// Mira si un registre existeix, si existeix r és el registre
// Si no existeix r->online és -1
int existeixRegistre(struct DataSet * ds, struct Registre * r);
// copy és una còpia de original, copy s'espera que estigui buit i només inicialitzat.
void copyRegistre(struct Registre * copy, struct Registre * original);
// Retorna 1 si està connectat, 0 si està desconnectat
int isOnline(struct Registre * r);
// Carrega el dataset amb la informació que hi ha al fitxer filename
// Aquest fitxer ha de tenir el format :
//  domini
//  nickname
//  nickname
//  ...
int llegirUsuaris(struct DataSet * ds, char * filename);
// Desa els usuaris en un fitxer anomenat filename (DEPRECATED)
// Aquest fitxer ha de tenir el format :
//  domini
//  nickname
//  nickname
//  ...
int escriureUsuaris(struct DataSet * ds, char * filename);

/* Declaració de funcions externes de lumi.c, és a dir, d'aquelles que es */
/* faran servir en un altre fitxer extern, p.e., MIp2-p2p.c,              */
/* MIp2-nodelumi.c, o MIp2-agelumic.c. El fitxer extern farà un #include  */
/* del fitxer .h a l'inici, i així les funcions seran conegudes en ell.   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI.                                  */
/* Les funcions externes les heu de dissenyar vosaltres...                */

/** Funcions d'inicialització **/
int LUMI_inicialitza_servidor(struct DataSet * d, char * filename,  char * ip, int port);
int LUMI_start(int socket, struct DataSet * d);
/** Funcions de processament SERVIDOR **/
int LUMI_processa(int sck, struct DataSet * d);
int LUMI_registre(char * rebut, int longitud, struct DataSet * d, char * ipRem, int portRem, int online);
int LUMI_Localitza(int sck, char * rebut, int longitud, struct DataSet * d);
int LUMI_ProcessaRespostaLocalitzacio(int sck, char * rebut, int longitud, struct DataSet * d);
/** Funcions de processament CLIENT **/
int LUMI_CrearSocketClient(const char *IPloc, int portUDPloc);
int LUMI_EnviaPeticio(const int * LlistaSck, int socketDeLlista, char * nickFrom, char * dnsFrom, char * nickTo, char * dnsTo, char * ipTCPRem, int * portTCPRem, char tipusPeticio, int timeout, int logDescriptor);
int LUMI_PeticioRegistre(int Sck, const char *usuari, char * domini, int logDescriptor);
int LUMI_ResponLocalitzacio(int socket, int codi, const char * usuariPreguntador, const char * dnsPreguntador, char * ip, int portTCP, int logDescriptor);
int LUMI_PeticioDesregistre(int Sck, const char *usuari, char * domini, int logDescriptor);
int LUMI_PeticioLocalitzacio(int Sck, const char *nickFrom, const char * dnsFrom, const char * nickTo, const char *dnsTo, int logDescriptor);
int LUMI_ProcessaClient(int sck, char * missatge, char * usuari, char * dns, char * ipTCPRem, int * portTCPrem, int logDescriptor);
/** Funcions genèriques **/
int LUMI_EnviaAMI(int sck, const char * dns, const char * missatge);
int LUMI_getIpiPortDeSocket(int sck, char * ip, int * port);
int LUMI_HaArribatAlgunaCosa(const int * socketsEscoltant, int nSockets);
int LUMI_GeneraRespostaLocalitzacio(int codi, const char* contingut, char * resposta);
int LUMI_GeneraLog(char * NomFitxLog);
int LUMI_TancaLog(int logDescriptor);
/** Funcions de resposta **/
void LUMI_crea_resposta_registre(char * resposta, char tipusResposta, int valorResposta);
int LUMI_ContestaClientMateixDomini(int sck, char * nickFrom, int codiResposta, struct DataSet * d);
int LUMI_ContestaServidor(int sck, const char * nickFrom, const char * dnsFrom, int codi);
