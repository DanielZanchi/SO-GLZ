#include <sys/socket.h>
#include <sys/un.h>   /*   Per socket AF_UNIX */
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "persona.h"
#include "linkedList.h"

#define PESO_MASSIMO 300
#define DEFAULT_PROTOCOL 0
#define TEMPO_SOSTA 3
#define TEMPO_SPOSTAMENTO 3

static const int CONNESSIONE_ASCENSORE = 0;

enum terminazione {
    cinque_minuti, fine_file
} terminazione;

enum direzione {
    ALTO, BASSO
} direzione;

enum piano {
    piano0, piano1, piano2, piano3
} piano;


static const char* SOCKETS_PIANI[4] = { "piano0.sock", "piano1.sock",
    "piano2.sock", "piano3.sock" };

lista_persone* lista = NULL;
int carico = 0;
time_t tempo_avvio;
int contatore_bambini = 0;
int contatore_adulti = 0;
int contatore_addetti_consegne = 0;

void spostamentoAscensore() {  // L'ascensore si sposta di un piano in base alla direzione corrente
    if (direzione == ALTO) {
        if (piano == piano3) {
            direzione = BASSO;
            piano--;
        } else {
            piano++;
        }
    } else {
        if (piano == piano0) {
            direzione = ALTO;
            piano++;
        } else {
            piano--;
        }
    }
}

void scriviNelSocket(int SocketFd, const void* buffer, size_t size) { // Come in piani.c
    int scritto = write(SocketFd, buffer, size);
    if (scritto < size) {
        char* msg;
        asprintf(&msg, "Errore durante l'invio, piano %i, terminazione ascensore ",
                 piano);
        perror(msg);
        exit(10);
    }
}

void leggiDalSocket(int SocketFd, void* nuovo_arrivato, size_t size) {  // Come in piani.c
    int letto = read(SocketFd, nuovo_arrivato, size);
    if (letto < 0) {
        char* msg;
        asprintf(&msg, "Errore durante la ricezione, piano %i, terminazione ascensore ",
                 piano);
        perror(msg);
        exit(10);
    }
}

int salitaPersone(int SocketFd, FILE* logFp) {   // Si occupa di aspettare le persone e registrarne i dati nel file di LOG, aggiorna l'ascensore
    int persona_presente = 0;
    
    leggiDalSocket(SocketFd, &persona_presente, sizeof(int));
    if (!persona_presente) {
        return 0;
    }
    
    Persona* nuovo_arrivato = (Persona*) malloc(sizeof(Persona));
    int contatore_caricate = 0;
    while (persona_presente) {
        nuovo_arrivato = (Persona*) malloc(sizeof(Persona));
        //printf("ricezione persona");
        leggiDalSocket(SocketFd, nuovo_arrivato, sizeof(Persona));
        aggiungiPersonaLista(lista, nuovo_arrivato);
        
        //legge dimensione stringa categoriaPersona
        long unsigned dimensione = 0;
        //printf("ricezione dimensione");
        leggiDalSocket(SocketFd, &dimensione, sizeof(dimensione));
        nuovo_arrivato->categoriaPersona = (char*) malloc(dimensione);
        
        //legge la stringa nome tipo
        //printf("ricezione tipo");
        leggiDalSocket(SocketFd, nuovo_arrivato->categoriaPersona, dimensione);
        
        time_t ora = time( NULL);
        
        printf(
               "[SALITA] %s al piano %i, con destinazione %i, %s\n",
               nuovo_arrivato->categoriaPersona, piano, nuovo_arrivato->destinazione,
               ctime(&ora));
        fprintf(logFp,
                "[SALITA] %s al piano %i, con destinazione %i, %s\n",
                nuovo_arrivato->categoriaPersona, piano, nuovo_arrivato->destinazione,
                ctime(&ora));
        
        carico = carico + nuovo_arrivato->peso;
        contatore_caricate++;
        
        leggiDalSocket(SocketFd, &persona_presente, sizeof(int));
    }
    return contatore_caricate;
}

void discesaPersone(FILE* logFp) {   // Come carica persone, ma ne cancella i dati per simulare l'arrivo a destinazione
    Persona* scesa = NULL;
    scesa = eliminaDiscesa(lista, piano);
    while (scesa != NULL) {
        time_t ora = time( NULL);
        printf("[DISCESA] %s al piano %i, %s\n",
               scesa->categoriaPersona, piano, ctime(&ora));
        fprintf(logFp, "[DISCESA] %s al piano %i, %s\n",
                scesa->categoriaPersona, piano, ctime(&ora));
        
        switch(scesa->peso){
            case 80:
                contatore_adulti++;
                break;
            case 40:
                contatore_bambini++;
                break;
            case 90:
                contatore_addetti_consegne++;
                break;
        }
        carico = carico - scesa->peso;
        free(scesa);
        scesa = eliminaDiscesa(lista, piano);
    }
}

int main(int argc, char *argv[]) {    // Comunica con il Socket- si autentica con il Server - Carica e Scarica persone
    short piani_terminati[4] = { 0, 0, 0, 0 };
    int piani_non_terminati = 4;
    int peso_massimo_imbarcabile = PESO_MASSIMO;
    piano = 0;
    
    lista = creaListaPersone();
    
    sleep(4);
    tempo_avvio = time(NULL);
    
    FILE* logFp = NULL;
    logFp = fopen("ascensore.log", "w");
    fprintf(logFp, "Avvio ascensore: %s\n", ctime(&tempo_avvio));
    int persone_caricate = 0;
    do {
        int SocketFd, SocketLenght, tempo;
        struct sockaddr_un SocketAddress;
        struct sockaddr* SocketAddrPtr;
        
        SocketAddrPtr = (struct sockaddr*) &SocketAddress;
        SocketLenght = sizeof(SocketAddress);
        
        /* Create a UNIX socket, bidirectional, default protocol */
        SocketFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
        SocketAddress.sun_family = AF_UNIX; /* Set domain type */
        strcpy(SocketAddress.sun_path, SOCKETS_PIANI[piano]); /* Set name */
        int connesso = connect(SocketFd, SocketAddrPtr, SocketLenght);
        
        if (connesso == -1) {
            char* msg;
            asprintf(&msg, "Ascensore NON CONNESSO tramite socket \"%s\"\n",
                     SOCKETS_PIANI[piano]);
            perror(msg);
            close(SocketFd);
            exit(21);
        }
        
        scriviNelSocket(SocketFd, &CONNESSIONE_ASCENSORE,
                        sizeof(CONNESSIONE_ASCENSORE));
        
        peso_massimo_imbarcabile = PESO_MASSIMO - carico;
        scriviNelSocket(SocketFd, &peso_massimo_imbarcabile, sizeof(int));
        
        persone_caricate = salitaPersone(SocketFd, logFp);
        close(SocketFd);
        usleep(10000);
    } while (persone_caricate == 0);
    
    printf("Carico dell'ascensore = %i\n", carico);
    
    while (1) {
        spostamentoAscensore();
        sleep(TEMPO_SPOSTAMENTO);
        time_t ora = time(NULL);
        printf("[FERMATA] Piano %i, %s\n", piano, ctime(&ora));
        sleep(TEMPO_SOSTA);
        discesaPersone(logFp);
        int SocketFd, SocketLenght, tempo;
        struct sockaddr_un SocketAddress;
        struct sockaddr* SocketAddrPtr;
        
        SocketAddrPtr = (struct sockaddr*) &SocketAddress;
        SocketLenght = sizeof(SocketAddress);
        
        /* Create a UNIX socket, bidirectional, default protocol */
        SocketFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
        SocketAddress.sun_family = AF_UNIX; /* Set domain type */
        strcpy(SocketAddress.sun_path, SOCKETS_PIANI[piano]); /* Set name */
        int connesso = connect(SocketFd, SocketAddrPtr, SocketLenght);
        
        if (connesso != 0) {
            if (piani_terminati[piano]) {
                if (piani_non_terminati == 0) {
                    close(SocketFd);
                    break;
                }
                continue;
            } else {
                piani_non_terminati--;
                piani_terminati[piano] = 1;
                continue;
            }
        } else {
            printf("Connessione tramite \"%s\"\n", SOCKETS_PIANI[piano]);
        }
        
        scriviNelSocket(SocketFd, &CONNESSIONE_ASCENSORE,
                        sizeof(CONNESSIONE_ASCENSORE));
        
        peso_massimo_imbarcabile = PESO_MASSIMO - carico;
        scriviNelSocket(SocketFd, &peso_massimo_imbarcabile, sizeof(int));
        
        persone_caricate = salitaPersone(SocketFd, logFp);
        printf("Carico dell'ascensore = %i\n", carico);
        close(SocketFd);
    }
    printf("Terminazione ascensore...\n");
    time_t tempo_terminazione = time(NULL);
    fprintf(logFp, "Terminazione ascensore %s (%i)\n",
            ctime(&tempo_terminazione), (int) tempo_terminazione);
    char* msg;
    asprintf(&msg, "Resoconto attivita giornaliera:\nPersone servite %i\nBambini %i\nAdulti %i\nAddetti alle consegne %i\n",
             contatore_bambini + contatore_adulti + contatore_addetti_consegne, contatore_bambini, contatore_adulti, contatore_addetti_consegne);
    printf("%s", msg);
    fprintf(logFp, "%s", msg);
    fclose(logFp);
    return 0;
}
