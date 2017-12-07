#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Registre.c"
#include "DataSet.c"


/**
 * ds ha d'estar buit i inicialitzat.
 * Llegeix tots els noms d'usuari del fitxer esmentat i els carrega al dataset en format de tuples.
 */
void llegirUsuaris(struct DataSet *ds){


	
}


int main(int argc, char* argv[]) {

	struct DataSet d;

	init(&d);
	struct Registre r = createRegistre("marc", 2000, "8918912");
	struct Registre r2 = createRegistre("gil", 2000, "891892");
	insertRegistre(&d, &r);
	insertRegistre(&d, &r2);
	showDataSet(&d);
	printf("%i\n",getPosicio(&d, &r));
	deleteRegistre(&d, &r2);
	showDataSet(&d);


	/*
	int sock, i;
	int bllegit = 1, bescrit;
    char buff[200];
	int ladrrem;
	struct sockaddr_in adrloc, adrrem;
	char iploc[16];
	int portloc;

	for(i = 0; i < sizeof(buff); i++){
		buff[i] = '\0';
	}
	*/

	/* Es crea el socket UDP sock del servidor (el socket "local"), que de moment no té     */
	/* adreça (@IP i #port UDP) assignada.                                                  */
	/*
	if((sock=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		  perror("Error en socket");
		  exit(-1);
	}
	*/
	/* Per assignar adreça a sock (@IP i #port UDP) el programador ho pot fer de 2 maneres: */
	/* i) explícitament, és a dir, assignant uns valors concrets en el codi, fent bind()    */
	/* amb aquests valors; ii) implícitament, és a dir, deixant que els assigni el S.O.,    */
	/* que escull una @IP qualsevol de la màquina i un #port UDP que estigui lliure. Una    */
	/* assignació implicita es pot fer d'1 manera: fent bind() amb @IP 0.0.0.0 (que vol dir */
	/* qualsevol, és a dir, totes les @IP de la màquina) i #port UDP 0 (llavors, després    */
	/* del bind(), el S.O. haurà assignat un #port UDP). Assignar l'@IP 0.0.0.0 vol dir     */
	/* que el socket queda lligat a totes les @IP de la màquina i que rebrà paquets que     */
	/* arribin a qualsevol d'elles.                                                         */
	/* Aquí es fa bind amb @IP 0.0.0.0 i #port TCP 3000.
	*/

	/*strcpy(iploc,"0.0.0.0"); */   /* 0.0.0.0 correspon a INADDR_ANY */
	/*
	portloc = 3000;
	adrloc.sin_family=AF_INET;
	adrloc.sin_port=htons(portloc);
	*/
	//adrloc.sin_addr.s_addr=inet_addr(iploc);    /* o bé: ...s_addr = INADDR_ANY */
	/*
	for(i=0;i<8;i++){
		adrloc.sin_zero[i]='\0';
	}
	*/
	//bind port del server
	/*
	if((bind(sock,(struct sockaddr*)&adrloc,sizeof(adrloc)))==-1)
	{
		perror("Error en bind");
		close(sock);
		exit(-1);
	}
	*/


	/* Via sock es rep una línia del socket del client (el socket "remot").                 */
	/*
	ladrrem=sizeof(adrrem);
	*/ // longitud del adrrem
	/*
	while(bllegit > 0 && strcmp(buff, "$") != 0){
		memset(buff,0,strlen(buff));

		if((bllegit=recvfrom(sock,buff,sizeof(buff),0,(struct sockaddr*)&adrrem,&ladrrem))==-1)
		{
			perror("Error recvfrom\n");
			close(sock);
			exit(-1);
		}
		if(strcmp(buff, "$") != 0) printf("%s\n", buff);

	}
	*/

    /* Es tanca el socket sock.                                                             */
	/*
	close(sock);
	*/
    return 0;
}
