#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char *argv[]) {


	int sock, i;
	int bllegit, bescrit;
	char buff[200];
	char buffreal[200];
	struct sockaddr_in adrrem;
	char iprem[16];
	int portrem;

	//strcpy(iprem,"192.168.1.109");
	strcpy(iprem,argv[1]);

	//portrem = 3000;
	portrem = atoi(argv[2]);

	/* Es crea el socket UDP sock del client (el socket "local"), que de moment no té       */
	/* adreça (@IP i #port UDP) assignada.                                                  */
	if((sock=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("Error en socket");
		exit(-1);
	}

	/* Per assignar adreça a sock (@IP i #port UDP) el programador ho pot fer de 2 maneres: */
	/* i) explícitament, és a dir, assignant uns valors concrets en el codi, fent bind()    */
	/* amb aquests valors; ii) implícitament, és a dir, deixant que els assigni el S.O.,    */
	/* que escull una @IP qualsevol de la màquina i un #port UDP que estigui lliure. Una    */
	/* assignació implicita es pot fer de 2 maneres: i) fent bind() amb @IP 0.0.0.0 (que    */
	/* vol dir qualsevol, és a dir, totes les @IP de la màquina) i #port UDP 0 (llavors,    */
	/* després del bind() el S.O. haurà assignat un #port UDP; si es fa connect(), que és   */
	/* opcional, després d'ell el S.O. haurà assignat una @IP; si en canvi es fa sendto(),  */
	/* el S.O. no assigna cap @IP de manera fixa i l'@IP origen del paquet enviat és la de  */
	/* la interfície de sortida); ii) no fent bind() (llavors, si es fa connect(), que és   */
	/* opcional, després d'ell el S.O. haurà assignat un #port UDP i una @IP; si en canvi   */
	/* es fa sendto(), el S.O. haurà assignat un #port UDP però no assigna cap @IP de       */
	/* manera fixa i l'@IP origen del paquet enviat és la de la interfície de sortida).     */
	/* Aquí no es fa bind() ni connect().                                                   */
	/* Es llegeix una línia del teclat                                                      */

	while(strcmp(buffreal, "$") != 0){

		bzero(buffreal, 200);
		bzero(buff,  200);
		//rebre el missatge que vols enviar pel teclat
		if((bllegit=read(0,buff,sizeof(buff)))==-1)
		{
			perror("Error en read");
			close(sock);
			exit(-1);
		}
		strncpy(buffreal, buff, bllegit -1);
		/* Via sock s'envia la línia al socket del servidor (el socket "remot"). Primer s’omple */
		/* adrrem amb l’adreça del socket remot (@IP i #port UDP) i després es fa sendto(). A   */
		/* més com que abans no s'ha fet bind(), ara sendto() farà que s’assigni un #port UDP   */
		/* de manera implícita (l'@IP origen del paquet enviat serà la de la interfície de      */
		/* sortida).                                                                            */
		/* L'adreca del socket remot és @IP 10.0.0.23 i #port UDP 3000.                         */

		adrrem.sin_family=AF_INET;
		adrrem.sin_port=htons(portrem);
		adrrem.sin_addr.s_addr= inet_addr(iprem);
		for(i=0;i<8;i++){adrrem.sin_zero[i]='\0';}

		//el enviar el missatge
		if((bescrit=sendto(sock,buffreal,bllegit,0,(struct sockaddr*)&adrrem,sizeof(adrrem)))==-1)
		{
			perror("Error en sendto");
			close(sock);
			exit(-1);
		}

	}


	/* Es tanca el socket sock.                                                             */
	close(sock);

    return 0;
}
