#include "Registre.c"
#include "DataSet.h"


/**
 * Implementacions per l'estructura DataSet
 */
void init(struct DataSet * ds){
    int i;
    for(i=0; i < MAX_CLIENTS; i++){
        struct Registre r = createRegistre("", -1, "");
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
        ds->data[posRegistre].port = ds->data[posRegistre].port;
    }
}
