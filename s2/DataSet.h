/**
 * Definició de la taula de clients.
 */

#define MAX_CLIENTS  200 /** Màxim de clients permesos */

struct DataSet {
    struct Registre data[MAX_CLIENTS]; // Taula de clients que es poden connectar amb el servidor.
    int nClients; // Nombre de clients inscrits.

    /**
     * Inicialitza una taula de clients de màxim MAX_CLIENTS llargada
     */
    void (*init) (struct DataSet *);

    void (*showDataSet) (struct DataSet * ds);
    void (*insertRegistre) (struct DataSet * ds);
    int (*deleteRegistre) (struct DataSet * ds, struct Registre * r);
    void (*updateRegistre) (struct DataSet * ds, struct Registre * r);
    int (*getPosicio) (struct DataSet * ds, struct Registre * r);
};
