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
#include <sys/stat.h>
#include <fcntl.h>

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include "MIp2-lumi.h")                                           */

/* Definició de constants, p.e., #define MAX_LINIA 150                    */

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
 * ds ha d'estar buit i inicialitzat, domini ha d'estar buit i inicialitzat.
 * Llegeix tots els noms d'usuari del fitxer esmentat i els carrega al dataset en format de tuples.
 * Si quelcom ha anat malament retorna un valor menor a 0
 * Si tot ha anat bé retorna 0.
 */
int LUMI_llegirUsuaris(struct DataSet *ds, char * filename){

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
int LUMI_escriureUsuaris(struct DataSet *ds, char * filename){
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
	int lectura = LUMI_llegirUsuaris(d, filename);
	if(lectura < 0)
		return -1;

	// Inicialitzem el socket d'escolta del servidor.
	return UDP_CreaSock(ip, port);
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
	char ipRem[15] = "";
	int  portRem = 0;
	char missatge[MAX_MESSAGE_LENGHT];
	bzero(missatge, MAX_MESSAGE_LENGHT);
	int res = UDP_RepDe(sck, ipRem, &portRem, missatge, MAX_MESSAGE_LENGHT);

	if(res < 0){
		return -1;
	}
	else{
		printf("%s\n", missatge);
	}
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
	struct sockaddr_in adrrem;
	adrrem.sin_family=AF_INET;
	adrrem.sin_port=htons(portUDPrem);
	adrrem.sin_addr.s_addr= inet_addr(IPrem);
	int i;
	for(i=0;i<8;i++){adrrem.sin_zero[i]='\0';}

	//enviar el missatge
	int bescrit;
	if((bescrit=sendto(Sck,SeqBytes,LongSeqBytes,0,(struct sockaddr*)&adrrem,sizeof(adrrem)))==-1)
	{
		perror("Error en sendto");
		close(Sck);
		exit(-1);
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
		exit(-1);
	}

	//actualitzar IPrem i portUDPrem
	strcpy(IPrem,inet_ntoa(adrrem.sin_addr));
	*portUDPrem=ntohs(adrrem.sin_port);

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

}

/* Donat el nom DNS "NomDNS" obté la corresponent @IP i l'escriu a "IP*"  */
/* "NomDNS" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud qualsevol, i "IP*" és un "string" de C (vector de */
/* chars imprimibles acabat en '\0') d'una longitud màxima de 16 chars    */
/* (incloent '\0').                                                       */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé     */
int ResolDNSaIP(const char *NomDNS, char *IP)
{

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


/* Si ho creieu convenient, feu altres funcions...*/