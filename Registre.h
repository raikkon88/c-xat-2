/**
 * Struct de registre de la taula que desa els usuaris.
 *
 */
struct Registre {

    // Camps que ha de tenir l'struct de registre
    char username[50]; // Nom d'usuari
    int  port;         // port UDP associat al nom d'usuari
    char ip[16];       // Ip associada a l'usuari.
    int  online;       // Estipula si el registre, és a dir l'usuari està online.

    struct Registre (*fun) (char* _username, int _port, char* _ip);
    struct Registre (*create) (char* _username);
    void (*show) (struct Registre * reg);
    int (*compare) (struct Registre * r1, struct Registre * r2);
    int (*isOnline) (struct Registre * r);
};
