/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de lumi.c                                             */
/*                                                                        */
/* Autors: Feng Lin, Marc Sànchez                                         */
/*                                                                        */
/**************************************************************************/

#define MAX_MESSAGE_LENGHT  200

/* Declaració de funcions externes de lumi.c, és a dir, d'aquelles que es */
/* faran servir en un altre fitxer extern, p.e., MIp2-p2p.c,              */
/* MIp2-nodelumi.c, o MIp2-agelumic.c. El fitxer extern farà un #include  */
/* del fitxer .h a l'inici, i així les funcions seran conegudes en ell.   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI.                                  */
/* Les funcions externes les heu de dissenyar vosaltres...                */
//int LUMI_FuncioExterna(arg1, arg2...);

int LUMI_llegirUsuaris(struct DataSet *ds, char * filename);
int LUMI_escriureUsuaris(struct DataSet *ds, char * filename);
int LUMI_inicialitza_servidor(struct DataSet * d, char * filename, char * ip, int port);
int LUMI_start(int socket, struct DataSet * d);
int LUMI_processa(int sck, struct DataSet * d);
