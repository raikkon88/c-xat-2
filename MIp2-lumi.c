/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer lumi.c que implementa la capa d'aplicació de LUMI, sobre la     */
/* de transport UDP (fent crides a la interfície de la capa UDP           */
/* -sockets-).                                                            */
/* Autors: X, Y                                                           */
/*                                                                        */
/**************************************************************************/

#include "MIp2-lumi.h"


/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include "MIp2-lumi.h")                                           */

/* Definició de constants, p.e., #define MAX_LINIA 150                    */
#define MIDA_RESPOSTA_REGISTRE      3
#define MAX_LINIA                   200
#define SEPARADOR                   '#'


// #define AL0                  0
// #define AL1                  1
// #define AL2                  2
// #define AL3                  3
/* Declaració de funcions internes que es fan servir en aquest fitxer     */
/* (les seves definicions es troben més avall) per així fer-les conegudes */
/* des d'aqui fins al final de fitxer.                                    */
/* Com a mínim heu de fer les següents funcions internes:                 */

int UDP_CreaSock(const char *IPloc, int portUDPloc);
int UDP_EnviaA(int Sck, const char *IPrem, int portUDPrem, const char *SeqBytes, int LongSeqBytes);
int UDP_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes);
int UDP_TancaSock(int Sck);
int UDP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portUDPloc);
int UDP_DemanaConnexio(int Sck, const char *IPrem, int portUDPrem);
int UDP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes);
int UDP_Rep(int Sck, char *SeqBytes, int LongSeqBytes);
int UDP_TrobaAdrSockRem(int Sck, char *IPrem, int *portUDPrem);
int HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps);
int ResolDNSaIP(const char *NomDNS, char *IP);
void LUMI_UsuariIDnsDeMi(const char *direccio, char * dns, char * username);
void MontaAdrecaMi(char * direccio, const char * dns, const char * username);
int Log_CreaFitx(const char *NomFitxLog);
int Log_Escriu(int FitxLog, const char *MissLog);
int Log_TancaFitx(int FitxLog);

/* Definicio de funcions EXTERNES, és a dir, d'aquelles que en altres     */
/* fitxers externs es faran servir.                                       */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI.                                  */
/* Les funcions externes les heu de dissenyar vosaltres...                */

/* Descripció del que fa la funció...                                     */
/* Descripció dels arguments de la funció, què son, tipus, si es passen   */
/* per valor o per referència (la funció els omple)...                    */
/* Descripció dels valors de retorn de la funció...                       */


/**
 * Funció que prepara el servidor per al processament de peticions dels clents.
 * Reb un Dataset buit i un nom de fitxer i emplena el dataset amb les dades del
 * fitxer. Genera un socket d'escolta UDP sobre la ip i port passats per paràmetre.
 * RESPOSTA
 * --------------------------------------
 * ERROR -> -1
 * SUCCESS -> identificador del socket UDP connectat.
 */
int LUMI_inicialitza_servidor(struct DataSet * d, char * filename, char * ip, int port){
    init(d);
	int lectura = llegirUsuaris(d, filename);
	if(lectura < 0)
		return -1;

	// Inicialitzem el socket d'escolta del servidor.
	return UDP_CreaSock(ip, port);
}

void LUMI_crea_resposta_registre(char * resposta, char tipusResposta, int valorResposta){
    bzero(resposta, MIDA_RESPOSTA_REGISTRE);
    //sprintf(resposta, "%c%c%i", ACCEPTAT_MISSATGE, tipusResposta, valorResposta);
    sprintf(resposta, "%c%c%i", ACCEPTAT_MISSATGE, tipusResposta, valorResposta);
}

/**
 * Procediment de resolució de peticions del servidor. Serveix totes les peticions que li arrivin per els diferents sockets
 * Ja siguin sockets com el teclat o bé sockets UDP. El servidor no ha de servir peticions TCP.
 * socket està montat i obert, per tant es pot rebre per ell.
 * RESULTAT
 * ----------------------------
 * ERROR -> valor < 0
 * SUCCESS -> 0 (El prorgama ha finalitzat)
 */
int LUMI_start(int socket, struct DataSet * d){

	while(1){
		fd_set conjunt;
		FD_ZERO(&conjunt);
		FD_SET(socket,&conjunt);
		FD_SET(0,&conjunt);

		/* examinem lectura del teclat i del socket scon amb la llista conjunt */
		if(select(socket+1, &conjunt, NULL, NULL, NULL) == -1)
		{
			perror("Error en select");
			return (-1);
		}

		if(FD_ISSET(socket, &conjunt)){
			// Ha arribat quelcom per udp
			int res = LUMI_processa(socket, d);
			if(res < 0){
				return -1;
			}
		}
		else{
			// Ha arribat de teclat
			break;
		}
	}
    UDP_TancaSock(socket);
	return 0;
}

/**
 * Reb el missatge que ha arrivat per el socket sck, el processa i realitza l'acció que correspon
 * en funció del missatge que ha arrivat.
 * RESULTAT
 * -----------------------
 * ERROR -> -1
 * SUCCESS -> return 0
 */
int LUMI_processa(int sck, struct DataSet * d){
	char ipRem[MAX_IP_LENGTH] = "";
	int  portRem = 0;
	char missatge[MAX_MESSAGE_LENGHT];
	bzero(missatge, MAX_MESSAGE_LENGHT);
	int longitud = UDP_RepDe(sck, ipRem, &portRem, missatge, MAX_MESSAGE_LENGHT);

    int resultatAccio = 0;

	if(longitud < 0){
		return -1;
	}
	else{
		if(missatge[0] == REGISTRE){
			printf("%s -> %i bytes\n", "Petició de registre", longitud);
			int resultatRegistre = LUMI_registre(missatge, longitud, d, ipRem, portRem, 1);
            // int Sck, const char *IPrem, int portUDPrem, const char *SeqBytes, int LongSeqBytes
            char resposta[MIDA_RESPOSTA_REGISTRE] = "";
            LUMI_crea_resposta_registre(resposta, REGISTRE, resultatRegistre);
            //printf("Resposta : %s\n", resposta);
			resultatAccio = UDP_EnviaA(sck, ipRem, portRem, resposta, MIDA_RESPOSTA_REGISTRE);
            // TODO: Escriure log
            // if(resultatResposta < 0){
            //     // Escriure Log
            //     printf("%s\n", "Error al enviar la resposta del registre.");
            // }
		}
		else if(missatge[0] == DESREGISTRE){
			printf("%s -> %i bytes\n", "Petició de desregistre", longitud);
			int resultatRegistre = LUMI_registre(missatge, longitud, d, ipRem, portRem, 0);
            char resposta[MIDA_RESPOSTA_REGISTRE]="";
            LUMI_crea_resposta_registre(resposta, DESREGISTRE, resultatRegistre);
            //printf("Resposta : %s\n", resposta);
			resultatAccio = UDP_EnviaA(sck, ipRem, portRem, resposta, MIDA_RESPOSTA_REGISTRE);
            // TODO : Escriure Log
            // if(resultatResposta < 0){
            //     // Escriure Log
            //     printf("%s\n", "Error al enviar la resposta del desregistre.");
            // }
		}
		else if(missatge[0] == LOCALITZACIO){
			//printf("%s -> %i bytes\n","Petició de localització.", longitud);
            int resultatLocalitzacio = LUMI_localitza(sck, missatge, longitud, d);
            // TODO : Escriure Log.

		}
        else if(missatge[0] == ACCEPTAT_MISSATGE){
            if(missatge[1] == LOCALITZACIO){
                // En aquest cas ens està arrivant o bé una resposta d'un client o una resposta d'un servidor.
                int resultatResposta = LUMI_ProcessaRespostaLocalitzacio(sck, missatge, longitud, d);
                // TODO : Escriure Log.
            }
            else{
                // TODO :  Escriure Log,  El missatge rebut és incorrecte i s'ha descartat
            }
        }
        else {
            // TODO : Escriure log, el missatge rebut és incorrecte i s'ha descartat
        }
	}
}

int LUMI_ProcessaRespostaLocalitzacio(int sck, char * rebut, int longitud, struct DataSet * d){

    char direccio[MAX_LINIA];
    bzero(direccio, MAX_LINIA);
    // Treiem el codi de resposta i el missatge i ens quedem la direcció
    strncpy(direccio, rebut + 3, longitud);

    char nickTo[MAX_LINIA];
    char dnsTo[MAX_LINIA];
    char resta[MAX_LINIA];

    bzero(nickTo, MAX_LINIA);
    bzero(dnsTo, MAX_LINIA);
    bzero(resta, MAX_LINIA);

    // Extraiem els camps de les direccions
    if(rebut[2] == ONLINE_LLIURE){
        sscanf(direccio, "%[^'@']@%[^'#']#%s",nickTo, dnsTo, resta);
    }
    else{
        printf("%s\n", "Esta entrant per aqui");
        sscanf(direccio, "%[^'@']@%s",nickTo, dnsTo);
        printf("%s\n", nickTo);
        printf("%s\n", dnsTo);
    }

    int resultatAccio = 0;
    if(strcmp(dnsTo, d->domini) == 0){ // El missatge va dirigit a mi
        struct Registre desti = create(nickTo);
        existeixRegistre(d, &desti);
        if(desti.online != -1) { // El registre existeix
            resultatAccio = UDP_EnviaA(sck, desti.ip, desti.port, rebut, longitud);
            // TODO : Escriure Log
        }
        else {
            // TODO : Escriure log, el registre no existeix.
        }
    }
    else { // El missatge s'ha de reenviar a un altre servidor.
        // TODO S'ha de canviar el nickTo i treure'l, ha d'anar inclós a dins del missatge.
        //resultatAccio = LUMI_EnviaAMI(sck, nickTo, dnsTo, rebut);
        resultatAccio = LUMI_EnviaAMI(sck, dnsTo, rebut);
        // TODO : Escriure Log.
    }
}

/**
 * S'encarrega de deixar a un usuari en linia o fóra de línia dins del mateix servidor.
 * L'usuari que s'acaba de registrar queda amb l'estat online.
 * Parseja la cadena de caràcters rebut.
 * Actualitza el dataset amb la informació de l'usuari.
 * online estipula si l'acció és un registre o un desregistre.
 */
int LUMI_registre(char * rebut, int longitud, struct DataSet * d, char * ipRem, int portRem, int online){
	// S'extreuen els camps del missatge rebut.
	char * username = strncpy(rebut, rebut + 1, longitud);
	username[longitud]='\0';
	printf("%s\n",username);

	// Genero un registre i el marquem com online amb la informació que s'ha rebut.
	struct Registre user;
    ini(&user, username, portRem, ipRem, online);
	updateRegistre(d, &user);

    // TODO : S'ha canviar el mostrar dataset per el escriure log.
    showDataSet(d);
    return 0;
}

int LUMI_localitza(int sck, char * rebut, int longitud, struct DataSet * d){
    // S'extreuen els camps del missatge rebut.
    char direccions[MAX_LINIA];
    bzero(direccions, MAX_LINIA);
    strncpy(direccions, rebut + 1, longitud);

    char nickFrom[MAX_LINIA];
    char nickTo[MAX_LINIA];
    char dnsFrom[MAX_LINIA];
    char dnsTo[MAX_LINIA];

    // Extraiem els camps de les direccions
    sscanf(direccions, "%[^'@']@%[^'#']#%[^'@']@%s", nickFrom, dnsFrom, nickTo, dnsTo);
    int resultatAccio = 0;

    // Sóc el domini destí
    if(strcmp(dnsTo, d->domini) == 0){
        // Primer de tot comprovem que el client estigui al nostre domini (validació)
        struct Registre r = create(nickTo); // Genero un registre flag per fer la cerca
        existeixRegistre(d, &r); // Si no existeix r->online és -1

        if(r.online != -1){ // Si el client existeix :
            // Mirem si està on line o si no.
            if(r.online == 1){ // Si està onLine (Si està ocupat o no ho gestiona el client)
                // Li diem al client b que contesti al client a (BRIDGE)
                resultatAccio = UDP_EnviaA(sck, r.ip, r.port, rebut, longitud);
                // TODO : Escriure Log
            }
            else { // Si no està online :
                if(strcmp(dnsFrom, dnsTo) == 0){
                    // El servidor pot gestionar la resposta directa cap al client.
                    resultatAccio = LUMI_ContestaClientMateixDomini(sck, nickFrom, OFFLINE, d);
                    // TODO : Escriure Log
                }
                else{
                    // Contesta al servidor anterior
                    resultatAccio = LUMI_ContestaServidor(sck, nickFrom, dnsFrom, OFFLINE);
                    // TODO : Escriure Log
                }
            }
        }
        else { // Si el client no existeix
            if(strcmp(dnsFrom, dnsTo) == 0){ // (AL2)
                // El servidor pot gestionar la resposta directa cap al client.
                resultatAccio = LUMI_ContestaClientMateixDomini(sck, nickFrom, NO_EXISTEIX, d);
                // TODO : Escriure Log
            }
            else{
                // Contesta al servidor anterior
                resultatAccio = LUMI_ContestaServidor(sck, nickFrom, dnsFrom, NO_EXISTEIX);
                // TODO : Escriure Log
            }
        }
    }
    else { // No sóc el domini destí (Bridge)
        resultatAccio = LUMI_EnviaAMI(sck, dnsTo, rebut);
        // TODO : Escriure Log
    }
}

int LUMI_ContestaServidor(int sck, const char * nickFrom, const char * dnsFrom, int codi){
    char direccio[MAX_LINIA];
    MontaAdrecaMi(direccio, nickFrom, dnsFrom);
    char resposta[MAX_LINIA];
    LUMI_GeneraRespostaLocalitzacio(codi, direccio, resposta);
    return LUMI_EnviaAMI(sck, dnsFrom, resposta);
}

int LUMI_ContestaClientMateixDomini(int sck, char * nickFrom, int codiResposta, struct DataSet * d){
    // Cerco la ip del registre que m'ha contactat
    struct Registre origen = create(nickFrom);
    existeixRegistre(d, &origen);
    // Miro que no sigui un fake el nick
    if(origen.online != -1){
        // Contestem al client a que no es pot realitzar la connexió (AL3)
        char resposta[TOTAL_LENGHT_MESSAGE];
        LUMI_GeneraRespostaLocalitzacio(codiResposta, "", resposta);
        return UDP_EnviaA(sck, origen.ip, origen.port, resposta, strlen(resposta));
    }
    else{
        // TODO :  MISSATGE DESCARTAT... es pot escriure al log.
        // El que m'ha enviat el missatge no existeix i és un troll!
    }
}

int LUMI_GeneraRespostaLocalitzacio(int codi, char* contingut, char * resposta){
    char codiStr[3];
    sprintf(codiStr, "%c%c%c",ACCEPTAT_MISSATGE, LOCALITZACIO, codi);
    bzero(resposta, TOTAL_LENGHT_MESSAGE);
    strcat(resposta, codiStr);
    strcat(resposta, contingut);
    return 0;
}

int LUMI_getIpiPortDeSocket(int sck, char * ip, int * port){

    struct sockaddr_in sin;
    int addrlen = sizeof(sin);
    int local_port = 0;
    if(getsockname(sck, (struct sockaddr *)&sin, &addrlen) == 0 &&
       sin.sin_family == AF_INET && addrlen == sizeof(sin))
    {
        *port = (int)ntohs(sin.sin_port);
    }
    else{
        return -1;
    }

    //printf("%i\n", (int) portLocal);
    FILE *fd;
    char command[1024] = "ifconfig | head -n 2 | grep \"inet addr\" | sed -r 's/ +/:/g' | cut -d \":\" -f 4";
    //char ipLocal[16];
    fd = popen(command, "r");
    if(fd == NULL){
        return -1;
    }
    if(fgets(ip, 1024, fd) == NULL) return -1;
    ip[strlen(ip)-1]='\0';
    int status = pclose(fd);

    return 0;
}


void MontaAdrecaMi(char * direccio, const char * dns, const char * username){
    bzero(direccio, MAX_LINIA);
    strcpy(direccio, username);
    strcat(direccio, "@");
    strcat(direccio, dns);
}

int LUMI_EnviaAMI(int sck, const char * dns, const char * missatge){

    char ipServ[MAX_IP_LENGTH];
    bzero(ipServ, MAX_IP_LENGTH);
    ResolDNSaIP(dns, ipServ);

	int Byteenviats =  UDP_EnviaA(sck,ipServ,DEFAULT_PORT_SERVER,missatge,strlen(missatge));
	if(Byteenviats == -1 ){
		printf(" error de enviar peticio de localitzacio al server \n");
		return -1;
	}
    return 0;
}

/* Definicio de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer.                                  */

/* Crea un socket UDP a l’@IP “IPloc” i #port UDP “portUDPloc”            */
/* (si “IPloc” és “0.0.0.0” i/o “portUDPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port UDP, respectivament).      */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int UDP_CreaSock(const char *IPloc, int portUDPloc)
{
	int sock;
	/* Es crea el socket UDP sock del client (el socket "local"), que de moment no té       */
	/* adreça (@IP i #port UDP) assignada.     												*/
	if((sock=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("Error en socket");
		return (-1);
	}

	struct sockaddr_in adrloc;

	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portUDPloc);
	adrloc.sin_addr.s_addr=inet_addr(IPloc);    /* o bé: ...s_addr = INADDR_ANY */
	int i;
	for(i=0;i<8;i++){
		adrloc.sin_zero[i]='\0';
	}

	if((bind(sock,(struct sockaddr*)&adrloc,sizeof(adrloc)))==-1)
	{
		perror("Error en bind");
		close(sock);
		return (-1);
	}

	return sock;

}

/* Envia a través del socket UDP d’identificador “Sck” la seqüència de    */
/* bytes escrita a “SeqBytes” (de longitud “LongSeqBytes” bytes) cap al   */
/* socket remot que té @IP “IPrem” i #port UDP “portUDPrem”.              */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes       */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int UDP_EnviaA(int Sck, const char *IPrem, int portUDPrem, const char *SeqBytes, int LongSeqBytes)
{
    printf("S'esta enviant %s\n", SeqBytes);

	struct sockaddr_in adrrem;
	adrrem.sin_family=AF_INET;
	adrrem.sin_port=htons(portUDPrem);
	adrrem.sin_addr.s_addr= inet_addr(IPrem);
	int i;
	for(i=0;i<8;i++){adrrem.sin_zero[i]='\0';}

	//enviar el missatge
	int bescrit;
	if((bescrit=sendto(Sck,SeqBytes,strlen(SeqBytes),0,(struct sockaddr*)&adrrem,sizeof(adrrem)))==-1)
	{
		perror("Error en sendto");
		close(Sck);
		return -1;
	}

	return bescrit;

}

/* Rep a través del socket UDP d’identificador “Sck” una seqüència de     */
/* bytes que prové d'un socket remot i l’escriu a “SeqBytes*” (que té     */
/* una longitud de “LongSeqBytes” bytes).                                 */
/* Omple "IPrem*" i "portUDPrem*" amb respectivament, l'@IP i el #port    */
/* UDP del socket remot.                                                  */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* "SeqBytes*" és un vector de chars qualsevol (recordeu que en C, un     */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes       */
/* Retorna -1 si hi ha error; el nombre de bytes rebuts si tot va bé.     */
int UDP_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes)
{

	struct sockaddr_in adrrem;
	int ladrrem=sizeof(adrrem); // longitud del adrrem

	//rebre el missatge
	int bllegit;
	if((bllegit=recvfrom(Sck,SeqBytes,LongSeqBytes,0,(struct sockaddr*)&adrrem,&ladrrem))==-1)
	{
		perror("Error recvfrom\n");
		close(Sck);
		return -1;
	}

    printf("S'ha rebut -> %s\n", SeqBytes);


    bzero(IPrem, MAX_IP_LENGTH);
	strcpy(IPrem,inet_ntoa(adrrem.sin_addr));
	*portUDPrem=ntohs(adrrem.sin_port);
	return bllegit -1 ; // Se li resta el '\0'
}

/* S’allibera (s’esborra) el socket UDP d’identificador “Sck”.            */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TancaSock(int Sck)
{
	return close(Sck);

}

/* Donat el socket UDP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc*” i “portUDPloc*” amb respectivament, la seva   */
/* @IP i #port UDP.                                                       */
/* "IPloc*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0')                */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portUDPloc)
{
	struct sockaddr_in adrloc;
	int long_adrloc=sizeof(adrloc);
	if (getsockname(Sck, (struct sockaddr *)&adrloc, &long_adrloc) == -1)
	{
		close(Sck);
		return -1;
	}
	strcpy(IPloc, inet_ntoa(adrloc.sin_addr));
    *portUDPloc = (int)(intptr_t)ntohs(adrloc.sin_port);

	return 0;
}

/* El socket UDP d’identificador “Sck” es connecta al socket UDP d’@IP    */
/* “IPrem” i #port UDP “portUDPrem” (si tot va bé es diu que el socket    */
/* “Sck” passa a l’estat “connectat” o establert – established –).        */
/* Recordeu que a UDP no hi ha connexions com a TCP, i que això només     */
/* vol dir que es guarda localment l’adreça “remota” i així no cal        */
/* especificar-la cada cop per enviar i rebre. Llavors quan un socket     */
/* UDP està “connectat” es pot fer servir UDP_Envia() i UDP_Rep() (a més  */
/* de les anteriors UDP_EnviaA() i UDP_RepDe()) i UDP_TrobaAdrSockRem()). */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_DemanaConnexio(int Sck, const char *IPrem, int portUDPrem)
{

}

/* Envia a través del socket UDP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket UDP remot amb qui està connectat.                 */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int UDP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes)
{

}

/* Rep a través del socket UDP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes*” (que té una longitud de “LongSeqBytes” bytes).*/
/* "SeqBytes*" és un vector de chars qualsevol (recordeu que en C, un     */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; el nombre de bytes rebuts si tot va bé.     */
int UDP_Rep(int Sck, char *SeqBytes, int LongSeqBytes)
{

}

/* Donat el socket UDP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem*” i            */
/* “portUDPrem*” amb respectivament, la seva @IP i #port UDP.             */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockRem(int Sck, char *IPrem, int *portUDPrem)
{

	struct sockaddr_in adrrem;
	int long_adrrem=sizeof(adrrem);
	if (getsockname(Sck, (struct sockaddr *)&adrrem, &long_adrrem) == -1)
	{
		close(Sck);
		return -1;
	}
	strcpy(IPrem, inet_ntoa(adrrem.sin_addr));
    *portUDPrem = (int)(intptr_t)ntohs(adrrem.sin_port);

	return 0;

}

int LUMI_HaArribatAlgunaCosa(const int * socketsEscoltant, int nSockets){
    return HaArribatAlgunaCosaEnTemps(socketsEscoltant, nSockets, -1);
}

/* Examina simultàniament durant "Temps" (en [ms] els sockets (poden ser  */
/* TCP, UDP i stdin) amb identificadors en la llista “LlistaSck” (de      */
/* longitud “LongLlistaSck” sockets) per saber si hi ha arribat alguna    */
/* cosa per ser llegida. Si Temps és -1, s'espera indefinidament fins que */
/* arribi alguna cosa.                                                    */
/* "LlistaSck" és un vector d'enters d'una longitud >= LongLlistaSck      */
/* Retorna -1 si hi ha error; retorna -2 si passa "Temps" sense que       */
/* arribi res; si arriba alguna cosa per algun dels sockets, retorna      */
/* l’identificador d’aquest socket.                                       */
int HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps)
{
    fd_set conjunt;
	FD_ZERO(&conjunt);
	int i;
	int descmax = 0;
	for(i = 0; i < LongLlistaSck; i++){
		int fd = LlistaSck[i];
		FD_SET(fd,&conjunt);
		if(fd > descmax){
			descmax = fd;
		}
	}

	int selection;
	if(Temps == -1){
		if(select(descmax+1, &conjunt, NULL, NULL, NULL)==-1)
        {
            perror("Error en select");
		    return (-1);
        }
	}
    else{
        struct timeval timeout;
    	timeout.tv_sec = Temps;
      	timeout.tv_usec = 0;
        if(select(descmax+1, &conjunt, NULL, NULL, NULL)==-1)
        {
            perror("Error en select");
		    return (-1);
        }
	}
	for(i = 0;i < LongLlistaSck; i++){
		if (FD_ISSET(LlistaSck[i], &conjunt)) return LlistaSck[i];
	}

    return -2;
}

/* Donat el nom DNS "NomDNS" obté la corresponent @IP i l'escriu a "IP*"  */
/* "NomDNS" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud qualsevol, i "IP*" és un "string" de C (vector de */
/* chars imprimibles acabat en '\0') d'una longitud màxima de 16 chars    */
/* (incloent '\0').                                                       */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé     */
int ResolDNSaIP(const char *NomDNS, char *IP)
{
    // hostent serveix per desar informació de un host en concret
    // Paràmetres com host name, adreça ipv4...
    struct hostent *host;
	struct sockaddr_in adr;
	host = gethostbyname(NomDNS);
	if(host != NULL){
		memcpy((void *)&adr.sin_addr, host->h_addr_list[0], host->h_length);
		strcpy(IP,inet_ntoa(adr.sin_addr));
		return 0;
	}
	return -1;
}

/**
 * Donada una direcció MI extreu el nom dns.
 * dns conté la part del dns de la direcció MI de direcció.
 */
void LUMI_UsuariIDnsDeMi(const char *direccio, char * dns, char * username){
    sscanf(direccio, "%[^'@']@%s", username, dns);
}

/* Crea un fitxer de "log" de nom "NomFitxLog".                           */
/* "NomFitxLog" és un "string" de C (vector de chars imprimibles acabat   */
/* en '\0') d'una longitud qualsevol.                                     */
/* Retorna -1 si hi ha error; l'identificador del fitxer creat si tot va  */
/* bé.                                                                    */
int Log_CreaFitx(const char *NomFitxLog)
{
	return open(NomFitxLog,O_WRONLY|O_CREAT|O_TRUNC,0700);
}

/* Escriu al fitxer de "log" d'identificador "FitxLog" el missatge de     */
/* "log" "MissLog".                                                       */
/* "MissLog" és un "string" de C (vector de chars imprimibles acabat      */
/* en '\0') d'una longitud qualsevol.                                     */
/* Retorna -1 si hi ha error; el nombre de caràcters del missatge de      */
/* "log" (sense el '\0') si tot va bé                                     */
int Log_Escriu(int FitxLog, const char *MissLog)
{
	return write(FitxLog,MissLog,strlen(MissLog));
}

/* Tanca el fitxer de "log" d'identificador "FitxLog".                    */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int Log_TancaFitx(int FitxLog)
{
	return close(FitxLog);
}

int LUMI_CrearSocketClient(const char *IPloc, int portUDPloc)
{
	return UDP_CreaSock(IPloc,portUDPloc);
}

/* */
int LUMI_EnviaPeticio(const int * LlistaSck, int socketDeLlista, char * nickFrom, char * dnsFrom, char * nickTo, char * dnsTo, char * ipTCPRem, int * portTCPRem, char tipusPeticio, int timeout){
    int nombreReenviaments = 0;
    int socket = -2;
    int resultat = 0;
    while(nombreReenviaments < MAX_NOMBRE_RETRANSMISSIONS && socket == -2){
        if(tipusPeticio == REGISTRE){
            // Envia la petició de registre
            resultat = LUMI_PeticioRegistre(LlistaSck[socketDeLlista], nickFrom, dnsFrom);
        }
        else if(tipusPeticio == DESREGISTRE){
            // Envia la petició de desregitre
            resultat = LUMI_PeticioDesregistre(LlistaSck[socketDeLlista], nickFrom, dnsFrom);
        }
        else if(tipusPeticio == LOCALITZACIO){
            // Envia la petició de localització
            resultat = LUMI_PeticioLocalitzacio(LlistaSck[socketDeLlista], nickFrom, dnsFrom, nickTo, dnsTo);
        }
        else{
            resultat = -1;
        }

        if(resultat < 0){
            return resultat;
        }

        // Evalúa si ha arrivat alguna cosa dins del timeout.
        socket = HaArribatAlgunaCosaEnTemps(LlistaSck, sizeof(LlistaSck), timeout);
        nombreReenviaments++;
    }

    // Evaluació dels possibles errors de la funció.
    if(nombreReenviaments == MAX_NOMBRE_RETRANSMISSIONS){
        // Timeout expirat , escriure log
        // Mostrar per pantalla
        printf("S'ha sobrepassat el nombre de reenviaments\n");
    }
    else{
        if(socket >= 0){ // S'ha rebut la resposta
            printf("S'ha rebut resposta del servidor \n");
            char missatge[MAX_MESSAGE_LENGHT];
            bzero(missatge, MAX_MESSAGE_LENGHT);
            return LUMI_ProcessaClient(socket, missatge, "", "", ipTCPRem, portTCPRem);
            // Escriure log, registre ok
            // Mostrar per pantalla.
        }
        else { // Hi ha hagut un error en la resposta.
            printf("Hi ha hagut error en l'enviament \n");
            // Escriure Log, registre erroni.
            // Mostrar per pantalla
        }
    }

}

int LUMI_PeticioRegistre(int Sck, const char *usuari, char * domini){ //, const char *IPloc, int portUDPloc){

	//fem la peticio de registre
	char SeqBytes[TOTAL_LENGHT_MESSAGE];
    //char ipRemitent[MAX_IP_LENGTH];

    bzero(SeqBytes, TOTAL_LENGHT_MESSAGE);
    //bzero(ipRemitent, MAX_IP_LENGTH);

	strcpy(SeqBytes, "R");
	strcat(SeqBytes, usuari);

    return LUMI_EnviaAMI(Sck, domini, SeqBytes);
}


//int LUMI_PeticioDesregistre(int Sck, const char *usuari, const char *IPloc, int portUDPloc){
int LUMI_PeticioDesregistre(int Sck, const char *usuari, char * domini){

    char SeqBytes[TOTAL_LENGHT_MESSAGE];
    bzero(SeqBytes, TOTAL_LENGHT_MESSAGE);

	strcpy(SeqBytes, "D");
	strcat(SeqBytes, usuari);

    return LUMI_EnviaAMI(Sck, domini, SeqBytes);
}

int LUMI_PeticioLocalitzacio(int Sck, const char *nickFrom, const char * dnsFrom, const char * nickTo, const char *dnsTo){//, int portUDPloc){

	char SeqBytes[TOTAL_LENGHT_MESSAGE];

    char MI_preguntador[MAX_LINIA];
    char MI_preguntat[MAX_LINIA];

    bzero(SeqBytes, TOTAL_LENGHT_MESSAGE);
    bzero(MI_preguntador, MAX_LINIA);
    bzero(MI_preguntat, MAX_LINIA);

    MontaAdrecaMi(MI_preguntador, dnsFrom, nickFrom);
    MontaAdrecaMi(MI_preguntat, dnsTo, nickTo);

    SeqBytes[0] = LOCALITZACIO;
	strcat(SeqBytes, MI_preguntador);
    SeqBytes[strlen(SeqBytes)] = SEPARADOR;
	strcat(SeqBytes, MI_preguntat);

    LUMI_EnviaAMI(Sck, dnsTo, SeqBytes);
}

int LUMI_ResponLocalitzacio(int socket, int codi, const char * usuariPreguntador, const char * dnsPreguntador, char * ip, int portTCP){

    char missatge [TOTAL_LENGHT_MESSAGE];
    char direccio [TOTAL_LENGHT_MESSAGE];

    bzero(missatge, TOTAL_LENGHT_MESSAGE);
    bzero(direccio, TOTAL_LENGHT_MESSAGE);
    //printf("%s\n",missatge);
    //strcpy(missatge, "AL");
    missatge[0] = ACCEPTAT_MISSATGE;
    //printf("%s\n",missatge);
    missatge[1] = LOCALITZACIO;
    //printf("%s\n",missatge);
    missatge[2] = codi;
    //printf("%s\n",missatge);
    MontaAdrecaMi(direccio, dnsPreguntador, usuariPreguntador);

    if(codi == ONLINE_OCUPAT){
        strcat(missatge, direccio);
        //printf("%s\n",missatge);
    }
    else if(codi == ONLINE_LLIURE){
        strcat(missatge, direccio);
        //printf("%s\n",missatge);
        missatge[strlen(missatge)] = SEPARADOR;
        //printf("%s\n",missatge);
        strcat(missatge, ip);
        //printf("%s\n",missatge);
        missatge[strlen(missatge)] = SEPARADOR;
        //printf("%s\n",missatge);
        char portString[6];
        sprintf(portString, "%d", portTCP);
        strcat(missatge, portString);
        //printf("%s\n",missatge);
        //printf("%s\n","????????????????????????????????????????");
    }
    else{
        return -1;
    }

    return LUMI_EnviaAMI(socket, dnsPreguntador, missatge);
}

int LUMI_ProcessaClient(int sck, char * missatge, char * usuari, char * dns, char * ipTCPRem, int * portTCPrem){

    char ipRem[MAX_IP_LENGTH] = "";
	int  portRem = 0;

    int longitud = UDP_RepDe(sck, ipRem, &portRem, missatge, MAX_MESSAGE_LENGHT);
    //printf("##### HA ARRIBAT -> %s\n", missatge);
    int resultatAccio = 0;

    if(longitud < 0){
        return -1;
    }
    else{
        if(missatge[0] == ACCEPTAT_MISSATGE){
            if(missatge[1] == REGISTRE){ // Resposta de registre
                if(missatge[2] == CORRECTE){
                    // TODO :  Registrat correctament, escriure log
                    // Escriure per pantalla també.
                    return REGISTRE_CORRECTE;
                }
                else if(missatge[2] == INCORRECTE){
                    // TODO :  Registrat incorrectament, escriure log
                    // Escriure per pantalla també.
                    return REGISTRE_INCORRECTE;
                }
                else{
                    // TODO : Missatge arrivat erroni, escriure log
                    // Escriure per pantalla també.
                    return MISSATGE_ERRONI;
                }
            }
            else if(missatge[1] == DESREGISTRE){ // Resposta de desregistre
                if(missatge[2] == CORRECTE){
                    // TODO :  Desregistrat correctament, escriure log
                    // Escriure per pantalla també.
                    return DESREGISTRE_CORRECTE;
                }
                else if(missatge[2] == INCORRECTE){
                    // TODO :  Desregistrat incorrectament, escriure log
                    // Escriure per pantalla també.
                    return DESREGISTRE_INCORRECTE;
                }
                else{
                    // TODO : Missatge arrivat erroni, escriure log
                    // Escriure per pantalla també.
                    return MISSATGE_ERRONI;
                }
            }
            else if(missatge[1] == LOCALITZACIO){ // Resposta de localització
                if(missatge[2] == ONLINE_LLIURE){
                    // Client amb el que es vol parlar està en línia, ha arrivat un missatge com el següent :
                    //      AL0preguntador@dnsPreguntador#IP#PORT_TCP
                    // S'ha d'extreure IP i PORT_TCP
                    // S'ha de generar un socket TCP amb el programa MI i s'ha de connectar amb el client
                    char port[6];
                    bzero(port, 6);
                    char resta[MAX_MESSAGE_LENGHT];

                    sscanf(missatge, "%[^'#']#%[^'#']#%s",resta, ipTCPRem, port);
                    *portTCPrem = atoi(port);

                    return LOCALITZACIO_ONLINE_LLIURE;
                }
                else if(missatge[2] == OFFLINE){
                    // TODO : Localitzat incorrectament, escriure log
                    // i anunciar a programa que no es pot parlar per que el client està offline
                    return LOCALITZACIO_OFFLINE;
                }
                else if(missatge[2] == NO_EXISTEIX){
                    // TODO : Localitzat incorrectament, escriure log
                    // i anunciar a programa que no es pot parlar per que el client no existeix
                    return LOCALITZACIO_NO_EXISTEIX;
                }
                else if(missatge[2] == ONLINE_OCUPAT){
                    // TODO : Localitzat incorrectament, escriure log
                    // i anunciar a programa que no es pot parlar per que el client ja està ocupat
                    return LOCALITZACIO_ONLINE_OCUPAT;
                }
                else{
                    // TODO : Missatge arrivat erroni, escriure log
                    return MISSATGE_ERRONI;
                }
            }
        }
        else if(missatge[0] == LOCALITZACIO) { // Missatge de localització, pregunten per mi.

            char jo[MAX_MESSAGE_LENGHT];
            bzero(jo, MAX_MESSAGE_LENGHT);
            // S'emplenen els camps usuari i dns per que es pugui respondre.
            sscanf(missatge, "L%[^'@']@%[^'#']#%s", usuari, dns, jo);
            return LOCALITZACIO_PETICIO;
        }
        else{
            // TODO : Escriure log, s'ha de descartar el missatge
        }
    }

}
// FUNCTIONS REGISTRE

struct Registre create (char* _username){
    struct Registre r;
    strcpy(r.username, _username);
    r.port = -1;
    strcpy(r.ip, "");
    r.online = 0;
    return r;
}

/**
 * Implementació del constructor amb paràmetres.
 */
void ini(struct Registre * r, char* _username, int _port, char* _ip, int _online){
    //struct Registre r;
    strcpy(r->username, _username);
    r->port = _port;
    strcpy(r->ip, _ip);
    r->online = _online;
    //return r;
}

void copyRegistre(struct Registre * copy, struct Registre * original){
    strcpy(copy->username, original->username);
    copy->port = original->port;
    strcpy(copy->ip, original->ip);
    copy->online = original->online;
}

/**
 * Return true if the register is online, else return false.
 * True is expressed as a number 1 and false as number 0.
 */
int isOnline(struct Registre * r){
    return r->online;
}

/**
 * Assigne the online value.
 * If value is different thant 0 or 1 the assigned value is 0.
 * else value is assigned to online parameter.
 */
void setOnLine(struct Registre * r, int value){
    if(value == 0 || value == 1){
        r->online = value;
    }
}

/**
 * Mosta la tupla registre per pantalla.
 */
void show(struct Registre * reg){
    printf("Username  : %s\n", reg->username);
    printf("Port      : %i\n", reg->port);
    printf("IP        : %s\n", reg->ip);
    printf("Connected : %i\n", reg->online);
}

/**
 * Compara dues tuples, retorna 0 si son iguals, difeerent de 0 altrament.
 * Dues tuples son iguals si tenen el mateix nom, o bé si tenen la mateixa ip i port.
 * Parlarem del mateix client.
 */
int equals(struct Registre * r1, struct Registre * r2){
    return (strcmp(r1->username, r2->username) || (strcmp(r1->ip, r2->ip) && r1->port == r2->port));
}

// FUNCTIONS DataSet

/**
 * Implementacions per l'estructura DataSet
 */
void init(struct DataSet * ds){
    int i;
    for(i=0; i < MAX_CLIENTS; i++){
        struct Registre r;
        ini(&r, "", -1, "", 0);
        ds->data[i] = r;
    }

    bzero(ds->domini, MAX_LENGHT_DOMINI);

    ds->nClients = 0;
}

int getPosicio(struct DataSet * ds, struct Registre *r){
    int pos = 0;
    while(pos < ds->nClients && equals(&ds->data[pos], r) != 0){
        pos++;
    }
    if(pos == ds->nClients) return -1;
    else return pos;
}

void showDataSet(struct DataSet * ds){
    printf("%s", "#  ");
    printf("%s\n", ds->domini);
    printf("%s\n", "#####################################################################");

    int i;
    for(i = 0; i < ds->nClients; i++){
        show(&ds->data[i]);
        printf("%s\n","------------------------------------------------------------------");
    }
}

void insertRegistre(struct DataSet * ds, struct Registre *r){
    ds->data[ds->nClients] = *r;
    ds->nClients++;
}

int existeixRegistre(struct DataSet * ds, struct Registre * r){
    int posRegistre = getPosicio(ds, r);
    if(posRegistre >= 0){
        copyRegistre(r, &ds->data[posRegistre]);
    }
    else{
        r->online = -1;
    }
}

int deleteRegistre(struct DataSet * ds, struct Registre *r){
    int posRegistre = getPosicio(ds, r);

    if(posRegistre < 0){
        return -1;
    }
    else{
        while(posRegistre + 1 < ds->nClients){
            ds->data[posRegistre] = ds->data[posRegistre+1];
            posRegistre++;
        }
        ds->nClients--;
    }
    return 1;
}

int updateRegistre(struct DataSet * ds, struct Registre *r){
    int posRegistre = getPosicio(ds, r);
    if(posRegistre < 0){
        return -1;
    }
    else{
        copyRegistre(&ds->data[posRegistre], r);
    }
}

/**
 * ds ha d'estar buit i inicialitzat, domini ha d'estar buit i inicialitzat.
 * Llegeix tots els noms d'usuari del fitxer esmentat i els carrega al dataset en format de tuples.
 * Si quelcom ha anat malament retorna un valor menor a 0
 * Si tot ha anat bé retorna 0.
 */
int llegirUsuaris(struct DataSet *ds, char * filename){

	//open and get the file handle
	FILE* fh;

	//char * filename = DB_FILE;
	fh = fopen(filename , "r");

	//check if file exists
	if (fh == NULL){
	    printf("file does not exists %s\n", filename);
	    return 0;
	}

	const size_t line_size = 300;
	char* line = malloc(line_size);

	if(fgets(line, line_size, fh) != NULL){
		strncpy(ds->domini, line, strlen(line) - 1);
	}

	while (fgets(line, line_size, fh) != NULL)  {
		char tmp[line_size];
		bzero(tmp, line_size);
		strncpy(tmp, line, strlen(line) - 1);
		struct Registre usuari = create(tmp);
		insertRegistre(ds, &usuari);
	}
	free(line);    // Alliberar memòria reservada.
	return 0;
}

/**
 * Escriu el dataset lumi a un fitxer de text.
 * Aquest fitxer l'encapçala el nom del domini com a primera línia i tot seguit tots els usuaris que
 * formen part d'aquest dataset.
 * Si quelcom ha nat malament retorna un valor inferior a 0,
 * Si tot ha anat bé retorna 0.
 */
int escriureUsuaris(struct DataSet *ds, char * filename){
	FILE * fh;
	//char * filename = DB_FILE;
	fh = fopen(filename, "w+");

	if(fh == NULL){
		printf("file %s does not exists, Something has been wrong...\n", filename);
		return -1;
	}

	fputs(ds->domini, fh);
	fputs("\n", fh);
	int i;
	for(i = 0; i < ds->nClients; i++){
		fputs(ds->data[i].username, fh);
		fputs("\n", fh);
	}

	fclose(fh);
	return 0;
}
