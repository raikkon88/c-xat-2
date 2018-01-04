/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer mi.c que implementa la capa d'aplicació de MI, sobre la capa de */
/* transport TCP (fent crides a la interfície de la capa TCP -sockets-).  */
/* Autors: Feng Lin, Marc Sànchez                                         */
/*                                                                        */
/**************************************************************************/
/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include "MIp1v4-mi.h")                                           */
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "MIp2-mi.h"

/* Definició de constants, p.e., #define MAX_LINIA 150                    */
#define ALL_INTERFACES 			"0.0.0.0"
#define MAX_LINE    			304
#define MAX_BUFFER  			300

/* Declaració de funcions internes que es fan servir en aquest fitxer     */
/* (les seves definicions es troben més avall) per així fer-les conegudes */
/* des d'aqui fins al final de fitxer.                                    */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc);
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc);
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem);
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem);
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes);
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes);
int TCP_TancaSock(int Sck);
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc);
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem);
int HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck);
void MostraError(const char *text);


/* Definicio de funcions EXTERNES, és a dir, d'aquelles que en altres     */
/* fitxers externs es faran servir.                                       */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa MI.                                    */

/* Inicia l’escolta de peticions remotes de conversa a través d’un nou    */
/* socket TCP en el #port “portTCPloc” i una @IP local qualsevol (és a    */
/* dir, crea un socket “servidor” o en estat d’escolta – listen –).       */
/* Retorna -1 si hi ha error; l’identificador del socket d’escolta de MI  */
/* creat si tot va bé.                                                    */
int MI_IniciaEscPetiRemConv(int portTCPloc)
{
    return TCP_CreaSockServidor(ALL_INTERFACES, portTCPloc);
}

/* Escolta indefinidament fins que arriba una petició local de conversa   */
/* a través del teclat o bé una petició remota de conversa a través del   */
/* socket d’escolta de MI d’identificador “SckEscMI” (un socket           */
/* “servidor”).                                                           */
/* Retorna -1 si hi ha error; 0 si arriba una petició local; SckEscMI si  */
/* arriba una petició remota.                                             */
int MI_HaArribatPetiConv(int SckEscMI)
{
	int socketsEscoltant[2];

	socketsEscoltant[0] = 0; // TECLAT
	socketsEscoltant[1] = SckEscMI;

	return HaArribatAlgunaCosa(socketsEscoltant,2);

}

/* Crea una conversa iniciada per una petició local que arriba a través   */
/* del teclat: crea un socket TCP “client” (en un #port i @IP local       */
/* qualsevol), a través del qual fa una petició de conversa a un procés   */
/* remot, el qual les escolta a través del socket TCP ("servidor") d'@IP  */
/* “IPrem” i #port “portTCPrem” (és a dir, crea un socket “connectat” o   */
/* en estat establert – established –). Aquest socket serà el que es farà */
/* servir durant la conversa.                                             */
/* Omple “IPloc*” i “portTCPloc*” amb, respectivament, l’@IP i el #port   */
/* TCP del socket del procés local.                                       */
/* El nickname local “NicLoc” i el nickname remot són intercanviats amb   */
/* el procés remot, i s’omple “NickRem*” amb el nickname remot. El procés */
/* local és qui inicia aquest intercanvi (és a dir, primer s’envia el     */
/* nickname local i després es rep el nickname remot).                    */
/* "IPrem" i "IPloc*" són "strings" de C (vectors de chars imprimibles    */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "NicLoc" i "NicRem*" són "strings" de C (vectors de chars imprimibles  */
/* acabats en '\0') d'una longitud màxima de 300 chars (incloent '\0').   */
/* Retorna -1 si hi ha error; l’identificador del socket de conversa de   */
/* MI creat si tot va bé.                                                 */
int MI_DemanaConv(const char *IPrem, int portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem)
{
	// crea socket Local
	int SocketLocal = TCP_CreaSockClient(IPloc, (int)portTCPloc);

	//connectar al server
	int sck= TCP_DemanaConnexio(SocketLocal,IPrem,portTCPrem);  // el socket de la conversa linia 108 esquelet.c
	// Omple “IPloc*” i “portTCPloc*”
	//TCP_TrobaAdrSockLoc(sck,IPloc,portTCPloc);

	// enviar i rebre nicknames falta fer... prob fallara
	char nick[MAX_LINE];
	//linia 469 esquelet.c
	sprintf(nick, "%s%.3d%s", "N", strlen(NicLoc), NicLoc);
	//envia nicklocal al server
	int nbyteNick = TCP_Envia(sck, nick, strlen(nick));

	char nickopponent[MAX_LINE];

	// linia 137 esquelet.c
	int resultatAccio= TCP_Rep(sck, nickopponent, MAX_LINE);
    if(resultatAccio < 0){
        return resultatAccio;
    }

    char tipus;
    MI_DesmontarProtocol(nickopponent, NicRem, &tipus, resultatAccio);

	return sck;

}

/* Crea una conversa iniciada per una petició remota que arriba a través  */
/* del socket d’escolta de MI d’identificador “SckEscMI” (un socket       */
/* “servidor”): accepta la petició i crea un socket (un socket            */
/* “connectat” o en estat establert – established –), que serà el que es  */
/* farà servir durant la conversa.                                        */
/* Omple “IPrem*”, “portTCPrem*”, “IPloc*” i “portTCPloc*” amb,           */
/* respectivament, l’@IP i el #port TCP del socket del procés remot i del */
/* socket del procés local.                                               */
/* El nickname local “NicLoc” i el nickname remot són intercanviats amb   */
/* el procés remot, i s’omple “NickRem*” amb el nickname remot. El procés */
/* remot és qui inicia aquest intercanvi (és a dir, primer es rep el      */
/* nickname remot i després s’envia el nickname local).                   */
/* "IPrem*" i "IPloc*" són "strings" de C (vectors de chars imprimibles   */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "NicLoc" i "NicRem*" són "strings" de C (vectors de chars imprimibles  */
/* acabats en '\0') d'una longitud màxima de 300 chars (incloent '\0').   */
/* Retorna -1 si hi ha error; l’identificador del socket de conversa      */
/* de MI creat si tot va bé.                                              */
int MI_AcceptaConv(int SckEscMI, char *IPrem, int *portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem)
{

    // acceptar connexio
    int portRemot;
	int socketActiu= TCP_AcceptaConnexio(SckEscMI,IPrem,&portRemot); // linia 119 esquelet.c
    if(socketActiu == -1){
        return -1;
    }
    *portTCPrem = portRemot;

    char nickopponent[MAX_LINE];

	int resultatAccio= TCP_Rep(socketActiu, nickopponent, MAX_LINE);
    if(resultatAccio <=0){
        return -1;
    }

    char tipus;
    int bytes;
    char nickNet[MAX_BUFFER];
    bzero(nickNet, MAX_BUFFER);
    resultatAccio = MI_DesmontarProtocol(nickopponent, &nickNet, &tipus, resultatAccio);
    if(resultatAccio < 0){
        return resultatAccio;
    }
    if(tipus == 'L'){
        return -1;
    }

    // Ja s'ha rebut el nick de l'altre.
    strcpy(NicRem, nickNet);
	// envia nicklocal al server
	int nbyteNick = MI_EnviaNick(socketActiu, NicLoc); //TCP_Envia(sck, nick, strlen(nick));
    if(nbyteNick < 0){
        return -1;
    }
	// if nbyteNick == ... return ...
	return socketActiu;
}

/**
 * Descobreix el port i la ip locals assignades dinàmicament
 * Retorna el port assignat o un número menor a 0 si va malament.
 * ipLocal tindrà la ip assignada en cas que tot vagi bé.
 */
int MI_getIpiPortDeSocket(int sck, char * ipLocal, int * port){

    struct sockaddr_in sin;
    int addrlen = sizeof(sin);
    int local_port = 0;
    if(getsockname(sck, (struct sockaddr *)&sin, &addrlen) == 0 &&
       sin.sin_family == AF_INET &&
       addrlen == sizeof(sin))
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
    if(fgets(ipLocal, 1024, fd) == NULL) return -1;
    ipLocal[strlen(ipLocal)-1]='\0';
    int status = pclose(fd);
    return 0;
}


/**
 * Extreu tots els camps del paquet que ha arrivat.
 * toParse conté el paquet, data tindrà el camp d'informació del paquet MI
 * tipus tindrà el camp 'N' o 'L' del paquet MI
 * bytes contindrà el nombre de bytes que conté el paquet MI
 */
int MI_DesmontarProtocol(char * toParse, char * data, char * tipus, int bytes){

    char nombreBytes[3];
    int i;
    for(i = 0; i < (int) bytes; i++){
        //printf("Positicio :%i , valor -> %i\n",i , toParse[i]);
        if(i == 0){
            char t = toParse[i];
            tipus[0] = t;
        }
        else if(i >0 && i < 4){
            nombreBytes[i-1] = toParse[i];
        }
        else{
            data[i-4]=toParse[i];
        }
    }
    return atoi(nombreBytes);
}


/* Escolta indefinidament fins que arriba una línia local de conversa a   */
/* través del teclat o bé una línia remota de conversa a través del       */
/* socket de conversa de MI d’identificador “SckConvMI” (un socket        */
/* "connectat”).                                                          */
/* Retorna -1 si hi ha error; 0 si arriba una línia local; SckConvMI si   */
/* arriba una línia remota.                                               */
int MI_HaArribatLinia(int SckConvMI)
{

	int socketsEscoltant[2];

	socketsEscoltant[0] = 0; // TECLAT
	socketsEscoltant[1] = SckConvMI;

	return HaArribatAlgunaCosa(socketsEscoltant,2);


}

/* Envia a través del socket de conversa de MI d’identificador            */
/* “SckConvMI” (un socket “connectat”) la línia “Linia” escrita per       */
/* l’usuari local.                                                        */
/* "Linia" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0'), no conté el caràcter fi de línia ('\n') i té una longitud       */
/* màxima de 300 chars (incloent '\0').                                   */
/* Retorna -1 si hi ha error; el nombre de caràcters n de la línia        */
/* enviada (sense el ‘\0’) si tot va bé (0 <= n <= 299).                  */
int MI_EnviaLinia(int SckConvMI, const char *Linia)
{
	// max_line  costant 204,pero demana 300 i el nom de la var local...
	char lineMI[MAX_LINE];
	//envia linia 463 esquelet.c
	sprintf(lineMI, "%s%.3d%s", "L", strlen(Linia), Linia);


	return TCP_Envia(SckConvMI, lineMI, strlen(lineMI));
}

/* Envia a través del socket de conversa de MI d’identificador            */
/* “SckConvMI” (un socket “connectat”) la línia “Linia” escrita per       */
/* l’usuari local.                                                        */
/* "Linia" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0'), no conté el caràcter fi de línia ('\n') i té una longitud       */
/* màxima de 300 chars (incloent '\0').                                   */
/* Retorna -1 si hi ha error; el nombre de caràcters n de la línia        */
/* enviada (sense el ‘\0’) si tot va bé (0 <= n <= 299).                  */
int MI_EnviaNick(int SckConvMI, const char *Linia)
{
	// max_line  costant 204,pero demana 300 i el nom de la var local...
	char nickMI[MAX_LINE];
	//envia linia 463 esquelet.c
	sprintf(nickMI, "%s%.3d%s", "N", strlen(Linia), Linia);
    //printf("%s\n", nickMI);
	return TCP_Envia(SckConvMI, nickMI, strlen(nickMI));
}

/* Rep a través del socket de conversa de MI d’identificador “SckConvMI”  */
/* (un socket “connectat”) una línia escrita per l’usuari remot, amb la   */
/* qual omple “Linia”, o bé detecta l’acabament de la conversa per part   */
/* de l’usuari remot.                                                     */
/* "Linia*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0'), no conté el caràcter fi de línia ('\n') i té una longitud       */
/* màxima de 300 chars (incloent '\0').                                   */
/* Retorna -1 si hi ha error; -2 si l’usuari remot acaba la conversa; el  */
/* nombre de caràcters n de la línia rebuda (sense el ‘\0’) si tot va bé  */
/* (0 <= n <= 299).                                                       */
int MI_RepLinia(int SckConvMI, char *Linia)
{
	// max_line  costant 204,pero demana 300 i el nom de la var local...
	char line[MAX_LINE];

	// linia 137 esquelet.c
	int resultatAccio= TCP_Rep(SckConvMI, line, MAX_LINE);
    if (resultatAccio==0) return -2; // acabar la conversa
	if (resultatAccio==-1) return -1; // error
    char tipus;
    int result = MI_DesmontarProtocol(line, Linia, &tipus, resultatAccio);
    if(tipus == 'N'){
        return -1;
    }

	return resultatAccio; // nombre de caracters n de la linia rebuda
}

/* Acaba la conversa associada al socket de conversa de MI                */
/* d’identificador “SckConvMI” (un socket “connectat”).                   */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int MI_AcabaConv(int SckConvMI)
{
	return TCP_TancaSock(SckConvMI);

}



/* Acaba l’escolta de peticions remotes de conversa que arriben a través  */
/* del socket d’escolta de MI d’identificador “SckEscMI” (un socket       */
/* “servidor”).                                                           */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int MI_AcabaEscPetiRemConv(int SckEscMI)
{
	return TCP_TancaSock(SckEscMI);
}

/* Definicio de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer.                                  */

/* Crea un socket TCP “client” a l’@IP “IPloc” i #port TCP “portTCPloc”   */
/* (si “IPloc” és “0.0.0.0” i/o “portTCPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port TCP, respectivament).      */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc)
{
    int fd;
	if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		close(fd);

		return (-1);
	}

	return fd;
}

/* Crea un socket TCP “servidor” (o en estat d’escolta – listen –) a      */
/* l’@IP “IPloc” i #port TCP “portTCPloc” (si “IPloc” és “0.0.0.0” i/o    */
/* “portTCPloc” és 0 es fa una assignació implícita de l’@IP i/o del      */
/* #port TCP, respectivament).                                            */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc)
{
    //printf("Anem a configurar %s al port %i\n", IPloc, portTCPloc);

	int fd;
	if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		close(fd);
		return -1;
	}

	int i;
	struct sockaddr_in adrloc;
	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portTCPloc);
	adrloc.sin_addr.s_addr=inet_addr(IPloc); /* o bé: ...s_addr = INADDR_ANY */
	for(i=0;i<8;i++){adrloc.sin_zero[i]='0';}

	if((bind(fd,(struct sockaddr*)&adrloc,sizeof(adrloc)))==-1)
	{
		close(fd);
		return (-1);
	}

	if((listen(fd,3))==-1)
	{
		close(fd);
		return (-1);
	}

	return fd;
}

/* El socket TCP “client” d’identificador “Sck” demana una connexió al    */
/* socket TCP “servidor” d’@IP “IPrem” i #port TCP “portTCPrem” (si tot   */
/* va bé es diu que el socket “Sck” passa a l’estat “connectat” o         */
/* establert – established –).                                            */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem)
{
    struct sockaddr_in adrConnexio;
	adrConnexio.sin_family=AF_INET;
	adrConnexio.sin_port=htons(portTCPrem);
	adrConnexio.sin_addr.s_addr=inet_addr(IPrem); /* o bé: ...s_addr = INADDR_ANY */
	int i;
	for(i=0;i<8;i++){adrConnexio.sin_zero[i]='0';}

	if((connect(Sck,(struct sockaddr*)&adrConnexio,sizeof(adrConnexio)))==-1)
	{
		close(Sck);
		return (-1);
	}
	return Sck; // El socket serà un valor positiu.
}

/* El socket TCP “servidor” d’identificador “Sck” accepta fer una         */
/* connexió amb un socket TCP “client” remot, i crea un “nou” socket,     */
/* que és el que es farà servir per enviar i rebre dades a través de la   */
/* connexió (es diu que aquest nou socket es troba en l’estat “connectat” */
/* o establert – established –; el nou socket té la mateixa adreça que    */
/* “Sck”).                                                                */
/* Omple “IPrem*” i “portTCPrem*” amb respectivament, l’@IP i el #port    */
/* TCP del socket remot amb qui s’ha establert la connexió.               */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; l’identificador del socket connectat creat  */
/* si tot va bé.                                                          */
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem)
{
    int socket;

	struct sockaddr_in adrConnexio;
	int long_adrrem=sizeof(adrConnexio);

	if((socket=accept(Sck,(struct sockaddr*)&adrConnexio, &long_adrrem))==-1)
	{
		close(Sck);
		return (-1);
	}

	// S'ha establert la connexió i s'emplenen les dades de qui s'ha connectat a les variables d'entrada.
    strcpy(IPrem, inet_ntoa(adrConnexio.sin_addr));
	//IPrem = inet_ntoa(adrConnexio.sin_addr);
	*portTCPrem = (int)(intptr_t)ntohs(adrConnexio.sin_port);
	return socket;
}

/* Envia a través del socket TCP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket TCP remot amb qui està connectat.                 */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes)
{
    return write(Sck,SeqBytes,strlen(SeqBytes));
}

/* Rep a través del socket TCP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes*” (que té una longitud de “LongSeqBytes” bytes),*/
/* o bé detecta que la connexió amb el socket remot ha estat tancada.     */
/* "SeqBytes*" és un vector de chars qualsevol (recordeu que en C, un     */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes.      */
/* Retorna -1 si hi ha error; 0 si la connexió està tancada; el nombre de */
/* bytes rebuts si tot va bé.                                             */
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes)
{
    return read(Sck, SeqBytes, LongSeqBytes);
}

/* S’allibera (s’esborra) el socket TCP d’identificador “Sck”; si “Sck”   */
/* està connectat es tanca la connexió TCP que té establerta.             */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TancaSock(int Sck)
{
    return close(Sck);
}

/* Donat el socket TCP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc*” i “portTCPloc*” amb respectivament, la seva   */
/* @IP i #port TCP.                                                       */
/* "IPloc*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc)
{
    struct sockaddr_in adrrem;
	int long_adrrem=sizeof(adrrem);
	if (getsockname(Sck, (struct sockaddr *)&adrrem, &long_adrrem) == -1)
	{
		close(Sck);
		return -1;
	}
	strcpy(IPloc, inet_ntoa(adrrem.sin_addr));
	//portTCPloc = (int*)ntohs(adrrem.sin_port);

	*portTCPloc = (int)(intptr_t)ntohs(adrrem.sin_port);
	//portTCPloc = res;
}

/* Donat el socket TCP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem*” i            */
/* “portTCPrem*” amb respectivament, la seva @IP i #port TCP.             */
/* "IPrem*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem)
{
    struct sockaddr_in adrrem;
	int long_adrrem=sizeof(adrrem);
	if (getpeername(Sck, (struct sockaddr *)&adrrem, &long_adrrem) == -1)
	{
		close(Sck);
		return -1;
	}
	strcpy(IPrem, inet_ntoa(adrrem.sin_addr));
    *portTCPrem = (int)(intptr_t)ntohs(adrrem.sin_port);

}

/* Examina simultàniament i sense límit de temps (una espera indefinida)  */
/* els sockets (poden ser TCP, UDP i stdin) amb identificadors en la      */
/* llista “LlistaSck” (de longitud “LongLlistaSck” sockets) per saber si  */
/* hi ha arribat alguna cosa per ser llegida.                             */
/* "LlistaSck" és un vector d'enters d'una longitud >= LongLlistaSck      */
/* Retorna -1 si hi ha error; si arriba alguna cosa per algun dels        */
/* sockets, retorna l’identificador d’aquest socket.                      */
int HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck)
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

	/* examinem lectura del teclat i del socket scon amb la llista conjunt */
	if(select(descmax+1, &conjunt, NULL, NULL, NULL) == -1)
	{
		perror("Error en select");
		return (-1);
	}
	// Ha d'haver arrivat algu per algun dels sockets
	int j;
	for(j = 0; j < LongLlistaSck; j++){
		if(FD_ISSET(LlistaSck[j], &conjunt)){
			//printf("FD seleccionat : %i \n", LlistaSck[j]);
			return LlistaSck[j];
		}
	}
	// Si surt del bucle vol dir que hi ha hagut un error ja que cap dels sockets ha saltat i no pot ser.
	return -1;
}

/**
 * Donada una direcció MI extreu el nom dns.
 * dns conté la part del dns de la direcció MI de direcció.
 */
void MI_UsuariIDnsDeMi(const char *direccio, char * dns, char * username){
    sscanf(direccio, "%[^'@']@%s", username, dns);
}

/* Escriu un missatge de text al flux d’error estàndard stderr, format    */
/* pel text “Text”, un “:”, un espai en blanc, un text que descriu el     */
/* darrer error produït en una crida de sockets, i un canvi de línia.     */
void MostraError(const char *text)
{
    fprintf(stderr, "%s: %s\n", text, strerror(errno));
}
