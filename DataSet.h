/**
 * Definició de la taula de clients.
 */


#define MAX_CLIENTS         200 /** Màxim de clients permesos */
#define MAX_LENGHT_DOMINI   200 /** Màxim de caràcters per el domini */

struct DataSet {

    struct Registre data[MAX_CLIENTS]; // Taula de clients que es poden connectar amb el servidor.
    char domini[MAX_LENGHT_DOMINI];
    int nClients; // Nombre de clients inscrits.


    // Inicialitza una taula de clients de màxim MAX_CLIENTS llargada
    void (*init) (struct DataSet *);
    // Mostra el dataset per pantalla (el contingut actual en memòria.)
    void (*showDataSet) (struct DataSet * ds);
    // Afegeix un registre a la taula.
    void (*insertRegistre) (struct DataSet * ds);
    // Esborra un registre de la taula.
    int (*deleteRegistre) (struct DataSet * ds, struct Registre * r);
    // Modifica un registre, usat per emplenar les adreces quan només es contenen els noms.
    void (*updateRegistre) (struct DataSet * ds, struct Registre * r);
    // Retorna la posició de la taula on es troba el registre r
    // o bé un registre n que té el mateix nom que r, o un registre n que té la mateixa ip i port que r.
    int (*getPosicio) (struct DataSet * ds, struct Registre * r);
};
