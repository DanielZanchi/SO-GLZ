
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

enum direzione {
	ALTO, BASSO
} direzione;
enum piano {
	piano0, piano1, piano2, piano3
} piano;

enum terminazione {
	cinque_minuti, fine_file
} terminazione;

static const int CONNESSIONE_ASCENSORE = 0;

static const char* SOCKETS_PIANI[4] = { "piano0.sock", "piano1.sock",
		"piano2.sock", "piano3.sock" };

lista_persone* lista = NULL;
int carico = 0;
time_t tempo_avvio;
int num_bambini = 0;
int num_adulti = 0;
int num_addetti = 0;

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

void scriviNelSocket(int SocketFd, const void* buffer, size_t dim) { // Come in piani.c
	int scritto = write(SocketFd, buffer, dim);
	if (scritto < dim) {
		char* msg;
		asprintf(&msg, "Errore durante l'invio, piano %i, terminazione ascensore ",
				piano);
		perror(msg);
		exit(10);
	}
}

void leggiDalSocket(int SocketFd, void* nuovo_arrivo, size_t dim) {  // Come in piani.c
	int letto = read(SocketFd, nuovo_arrivo, dim);
	if (letto < 0) {
		char* msg;
		asprintf(&msg, "Errore durante la ricezione, piano %i, terminazione ascensore ",
				piano);
		perror(msg);
		exit(10);
	}
}

int salitaPersone(int SocketFd, FILE* logFp) {   // Si occupa di aspettare le persone e registrarne i dati nel file di LOG, aggiorna l'ascensore
	int presente = 0;

	leggiDalSocket(SocketFd, &presente, sizeof(int));
	if (!presente) {
		return 0;
	}

	Persona* nuovo_arrivo = (Persona*) malloc(sizeof(Persona));
	int numero_caricate = 0;
	while (presente) {
		nuovo_arrivo = (Persona*) malloc(sizeof(Persona));
		//printf("ricezione persona");
		leggiDalSocket(SocketFd, nuovo_arrivo, sizeof(Persona));
		aggiungiPersonaLista(lista, nuovo_arrivo);

		//legge dimensione stringa categoriaPersona
		long unsigned dimensione = 0;
		//printf("ricezione dimensione");
		leggiDalSocket(SocketFd, &dimensione, sizeof(dimensione));
		nuovo_arrivo->categoriaPersona = (char*) malloc(dimensione);

		//legge la stringa nome tipo
		//printf("ricezione tipo");
		leggiDalSocket(SocketFd, nuovo_arrivo->categoriaPersona, dimensione);

		int tempo_generazione = time(NULL) - tempo_avvio;
		time_t ora = time( NULL);
		printf(
				"[SALITA] %s al piano %i, con destinazione %i, tempo avvio %i, %s\n",
				nuovo_arrivo->categoriaPersona, piano, nuovo_arrivo->destinazione,
				tempo_generazione, ctime(&ora));
		fprintf(logFp,
				"[SALITA] %s al piano %i, con destinazione %i, tempo avvio %i, %s\n",
				nuovo_arrivo->categoriaPersona, piano, nuovo_arrivo->destinazione,
				tempo_generazione, ctime(&ora));

		carico = carico + nuovo_arrivo->peso;
		numero_caricate++;

		leggiDalSocket(SocketFd, &presente, sizeof(int));
	}
	return numero_caricate;
}

void discesaPersone(FILE* logFp) {   // Come carica persone, ma ne cancella i dati per simulare l'arrivo a destinazione
	Persona* scesa = NULL;
	scesa = eliminaDiscesa(lista, piano);
	while (scesa != NULL) {
		int tempo_scesa = time(NULL) - tempo_avvio;
		time_t ora = time( NULL);
		printf("[DISCESA] %s al piano %i, tempo avvio %i, %s\n",
				scesa->categoriaPersona, piano, tempo_scesa, ctime(&ora));
		fprintf(logFp, "[DISCESA] %s al piano %i, tempo avvio %i, %s\n",
				scesa->categoriaPersona, piano, tempo_scesa, ctime(&ora));

		switch(scesa->peso){
		case 80:
			num_adulti++;
			break;
		case 40:
			num_bambini++;
			break;
		case 90:
			num_addetti++;
			break;
		}
		carico = carico - scesa->peso;
		free(scesa);
		scesa = eliminaDiscesa(lista, piano);
	}
}

int main(int argc, char *argv[]) {    // Comunica con il Socket- si autentica con il Server - Carica e Scarica persone
	int peso_massimo_imbarcabile = PESO_MASSIMO;
	piano = 0;
	short piani_terminati[4] = { 0, 0, 0, 0 };
	int piani_attivi = 4;

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

	while (42) {
		spostamentoAscensore();
		sleep(TEMPO_SPOSTAMENTO);
		printf("Ascensore arrivato al piano %i\n", piano);
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
				if (piani_attivi == 0) {
					close(SocketFd);
					break;
				}
				continue;
			} else {
				piani_attivi--;
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
			num_bambini + num_adulti + num_addetti, num_bambini, num_adulti, num_addetti);
	printf("%s", msg);
	fprintf(logFp, "%s", msg);
	fclose(logFp);
	return 0;
}
