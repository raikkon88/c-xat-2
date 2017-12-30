#include "Registre.h"

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
    struct Registre r;
    strcpy(r->username, _username);
    r->port = _port;
    strcpy(r->ip, _ip);
    r->online = _online;
    return r;
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
