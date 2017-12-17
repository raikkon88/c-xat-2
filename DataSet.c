#include "DataSet.h"

/**
 * Implementacions per l'estructura DataSet
 */
void init(struct DataSet * ds){
    int i;
    for(i=0; i < MAX_CLIENTS; i++){
        struct Registre r = createRegistre("", -1, "", 0);
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
        strcpy(ds->data[posRegistre].ip, r->ip);
        ds->data[posRegistre].port = r->port;
        ds->data[posRegistre].online = r->online;
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
