/**
 * Struct de registre de la taula que desa els usuaris.
 *
 */
struct Registre {

    // Camps que ha de tenir l'struct de registre
    char username[50]; // Nom d'usuari
    int  port;         // port UDP associat al nom d'usuari
    char ip[16];       // Ip associada a l'usuari.

    struct Registre (*fun) (char* _username, int _port, char* _ip);
    void (*show) (struct Registre * reg);
    int (*compare) (struct Registre * r1, struct Registre * r2);
};

/**
 * Implementació del constructor amb paràmetres.
 */
struct Registre  createRegistre(char* _username, int _port, char* _ip){
    struct Registre r;
    strcpy(r.username, _username);
    r.port = _port;
    strcpy(r.ip, _ip);
    return r;
}

/**
 * Mosta la tupla registre per pantalla.
 */
void show(struct Registre * reg){
    printf("%s\n", reg->username);
    printf("%i\n", reg->port);
    printf("%s\n", reg->ip);
}

/**
 * Compara dues tuples, retorna 0 si son iguals, difeerent de 0 altrament.
 * Dues tuples son iguals si tenen el mateix nom, o bé si tenen la mateixa ip i port.
 * Parlarem del mateix client.
 */
int equals(struct Registre * r1, struct Registre * r2){
    return (strcmp(r1->username, r2->username) || (strcmp(r1->ip, r2->ip) && r1->port == r2->port));
}
