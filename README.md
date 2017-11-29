# Pràctica 2 - XAT

Hi ha penjats al moodle l'enunciat, la presentació de la primera sessió i un power point com a plantilla.

## Diferències de la primera

A la primera pràctica es demanava el port i la ip amb qui es volia connectar. A aquesta pràctica el que es farà és que els usuaris tinguin un nom fix.

Un nom fix que identifica a algú és una adreça.

Per tant hi haurà adreces de missatgeria instantània.

El que farem quan es vulgui parlar amb algú és escriure la seva adreça de missatgeria instantània. S'ha d'implementar la plataforma que permeti escoltar els diferents noms dns.

LUMI -> Localització d'usuaris de missatgeria instantània.

Donada una adreça LUMI ens retorna la ip i el port.

**S'han de mantenir les millores fetes a la app**

### Passos de la aplicació :

- Demana adreça de missatgeria instantània (ha de ser única)
- Demana el port d'escoltar
- Demana el nickname

Quan es vulgui parlar amb algú s'ha de demanar l'adreça de missatgeria instatània de l'altre.

## Implementació

### OPCIONS DE DISSENY - 1

Es proposa una capa LUMI definida per un protocol PLUMI. Es requereix un servidor amb una taula dels diferents clients que es volen connectar, els seus noms i les seves ip's i ports.

Es planteja cambiar de protocol de transmissió de TCP a UDP. Ja que tot i que TCP és fiable i UDP no amb UDP no ens fa falta realitzar connexions.

Tindrem un socket UDP al client i un socket UDP al servidor. Com que la capa de transport no té fiabilitat **L'ha de fer l'aplicació!! -> (Parlem de control d'error)**.

La situació d'error que es proposa és la possible pèrdua del paquet, per poder solucionar-ho s'ha de fer un timeout des del que pregunta, llavors tant si reb el paquet com si no tindrà el timeout per rebre la resposta que saltarà si no la reb.

Només s'ha d'estudiar la situació de paquets perduts!

### OPCIONS DE DISSENY - 2

En comptes de desar la direcció ip i port de MI es desa l'adreça ip i port udp de LUMI, llavors quan algu es vol connectar, pregunta al servidor i el servidor descobreix la ip i el port preguntant per udp a la persona a qui va dirigida. Llavors li retorna la comunicació al orígen de la conversa i es comuniquen entre ells.
