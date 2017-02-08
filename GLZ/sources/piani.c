
#include <unistd.h> /* write, lseek, close, exit */
#include <sys/stat.h> /*open */
#include <fcntl.h> /*open*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>   /*  Per socket AF_UNIX */
#include <time.h>
#include <string.h>
#include <errno.h>

#include "persona.h"
#include "linkedList.h"

#define DEFAULT_PROTOCOL 0
#define DURATA_MAX_MINUTI 5

static const int CONNESSIONE_PIANO_CLIENT = 1;
static const int CONNESSIONE_ASCENSORE = 0;

static const char* SOCKETS_PIANI[4] = { "piano0.sock", "piano1.sock",
		"piano2.sock", "piano3.sock" };
static const char* PIANI_FILE_INPUT[4] =
		{ "piano0", "piano1", "piano2", "piano3" };
static const char* NOME_FILE_LOG[4] = { "piano0.log", "piano1.log",
		"piano2.log", "piano3.log" };
time_t tempo_avvio;
time_t tempo_terminazione;
int numero_piano;

enum terminazione {
	cinque_minuti, fine_servizio
} terminazione = cinque_minuti;

void scriviNelSocket(int SocketFd, const void* buffer, size_t dim) {
	int scritto = write(SocketFd, buffer, dim);
	if (scritto < dim) {
		char* msg;
		asprintf(&msg,
				"Errore invio messaggio socket \"%s\", terminazione al piano %i...\n",
				SOCKETS_PIANI[numero_piano], numero_piano);
		perror(msg);
		exit(36);
	}
}

void leggiDalSocket(int SocketFd, void* nuovo_arrivo, size_t dim) {
	int letto = read(SocketFd, nuovo_arrivo, dim);
	if (letto < 0) {
		char* msg;
		asprintf(&msg,
				"Errore ricezione messaggio socket \"%s\", terminazione al piano %i...\n",
				SOCKETS_PIANI[numero_piano], numero_piano);
		perror(msg);
		exit(36);
	}
}

void client() {
	char* tipo = NULL;
	char* tempo_generazione = NULL;
	char* destinazione = NULL;
	int tempo_generazione_num = 0;
	int destinazione_num = 0;
	FILE * inputFp = NULL;

	printf("Esecuzione del client, piano%i\n", numero_piano);

	inputFp = fopen(PIANI_FILE_INPUT[numero_piano], "r");

	if (inputFp == NULL) {
		char* msg;
		asprintf(&msg,
				"Impossibile aprire file di input \"%s\", terminazione client, piano %i ",
				PIANI_FILE_INPUT[numero_piano], numero_piano);
		perror(msg);
		exit(-3);
	}
	long int posizione = 0;

	while (1) {
		char* riga = NULL;
		size_t len = 0;
		int presente = 0;
		int tempo;

		//legge una riga dal file di input e genera la persona
		//se la riga e' vuota, termina
		getline(&riga, &len, inputFp);

		if (strcmp(riga, "\n") == 0) {
			printf(
					"Ultima riga del file di input \"%s\", terminazione client, piano %i\n",
					PIANI_FILE_INPUT[numero_piano], numero_piano);
			break;
		}

		int SocketFd;
		int SocketLenght;
		struct sockaddr_un SocketAddress;
		struct sockaddr* SocketAddrPtr;
		SocketAddrPtr = (struct sockaddr*) &SocketAddress;
		SocketLenght = sizeof(SocketAddress);

		/* Create a UNIX socket, bidirectional, default protocol */
		SocketFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
		if (SocketFd == -1) {
			char* msg;
			asprintf(&msg,
					"Errore durante la creazione del socket \"%s\", terminazione client, piano %i ",
					SOCKETS_PIANI[numero_piano], numero_piano);
			perror(msg);
			exit(76);
		}

		SocketAddress.sun_family = AF_UNIX; /* Set domain type */
		strcpy(SocketAddress.sun_path, SOCKETS_PIANI[numero_piano]); /* Set name */

		int connesso = connect(SocketFd, SocketAddrPtr, SocketLenght);
		if (connesso == -1) {
			char* msg;
			asprintf(&msg, "Client al piano %i errore connessione", numero_piano);
			perror(msg);
			fclose(inputFp);
			exit(35);
		}
		char* rigaTmp = riga;

		tipo = strsep(&rigaTmp, " ");
		tempo_generazione = strsep(&rigaTmp, " ");
		destinazione = strsep(&rigaTmp, " ");

		tempo_generazione_num = atoi(tempo_generazione);
		destinazione_num = atoi(destinazione);

		Persona persona = creaPersona(tipo[0], destinazione_num);
		free(riga);
		tempo = time(NULL);
		sleep(tempo_generazione_num - (tempo - tempo_avvio));

		scriviNelSocket(SocketFd, &CONNESSIONE_PIANO_CLIENT,
				sizeof(CONNESSIONE_PIANO_CLIENT));

		long unsigned dimensione = sizeof(persona);
		//invia la persona
		scriviNelSocket(SocketFd, &persona, dimensione);

		//invia la lunghezza della stringa
		dimensione = strlen(persona.categoriaPersona) + 1;
		scriviNelSocket(SocketFd, &dimensione, sizeof(dimensione));

		//invia la stringa
		scriviNelSocket(SocketFd, persona.categoriaPersona, dimensione);

		close(SocketFd);
		if (tempo_terminazione <= time(NULL)) {
			printf("Durata programma raggiunta, terminazione client, piano %i\n",
					numero_piano);
			break;
		}
	}

	fclose(inputFp);
}

void server() {
	lista_persone* coda = NULL;
	nodo_lista_persone* testa = NULL;
	coda = creaListaPersone();

	FILE* logFp = NULL;
	Persona* nuovo_arrivo = NULL;
	int connessione = -1;
	time_t ora;

	logFp = fopen(NOME_FILE_LOG[numero_piano], "w");

	printf("Start server, piano%i\n", numero_piano);
	if (logFp < 0) {
		char* msg;
		asprintf(&msg,
				"Impossibile aprire file di log \"%s\", terminazione server piano %i...",
				NOME_FILE_LOG[numero_piano], numero_piano);
		perror(msg);
		exit(-3);
	}
	fprintf(logFp, "Avviato piano: %s (%i)\n", ctime(&tempo_avvio),
			(int) tempo_avvio);

	unlink(SOCKETS_PIANI[numero_piano]);
	int SocketFd;
	int SocketLenght;
	struct sockaddr_un SocketAddress;
	struct sockaddr* SocketAddrPtr;

	SocketAddrPtr = (struct sockaddr*) &SocketAddress;
	SocketLenght = sizeof(SocketAddress);

	/* Create a UNIX socket, bidirectional, default protocol */
	SocketFd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
	if (SocketFd == -1) {
		printf(
				"Errore durante la creazione del socket \"%s\", terminazione del server, piano %i...",
				SOCKETS_PIANI[numero_piano], numero_piano);
		exit(-76);
	}
	SocketAddress.sun_family = AF_UNIX; /* Set domain type */
	strcpy(SocketAddress.sun_path, SOCKETS_PIANI[numero_piano]); /* Set name */
	int result = bind(SocketFd, SocketAddrPtr, SocketLenght);
	if (SocketFd == -1) {
		printf(
				"Errore durante la bind del socket \"%s\", terminazione del server, piano %i...",
				SOCKETS_PIANI[numero_piano], numero_piano);
		perror("");
		exit(-76);
	}
	result = listen(SocketFd, 2);
	if (SocketFd == -1) {
		printf(
				"Errore durante la listen del socket \"%s\", terminazione del server, piano %i...",
				SOCKETS_PIANI[numero_piano], numero_piano);
		perror("");
		exit(-76);
	}

	while (1) {
		int clientFd = accept(SocketFd, SocketAddrPtr, &SocketLenght);
		if (clientFd == -1) {
			perror("Impossibile effettuare la connessione con i clients");
			printf("Terminazione del server, piano %i", numero_piano);
			exit(5);
		}
		//leggiDalSocket(clientFd, &connessione, sizeof(connessione));
		read(clientFd, &connessione, sizeof(connessione));

		if (connessione == CONNESSIONE_PIANO_CLIENT) {// si e' connesso un piano-client
			printf("Connessione server piano %i, con piano client\n", numero_piano);

			nuovo_arrivo = (Persona*) malloc(sizeof(Persona));
			if (nuovo_arrivo == NULL) {
				printf(
						"Errore allocazione memoria, terminazione server piano %i...",
						numero_piano);
				exit(37);
			}

			leggiDalSocket(clientFd, nuovo_arrivo, sizeof(Persona));

			//legge lunghezza striga categoriaPersona
			long unsigned dimensione = 0;
			leggiDalSocket(clientFd, &dimensione, sizeof(dimensione));

			//legge stringa categoriaPersona
			nuovo_arrivo->categoriaPersona = (char*) malloc(dimensione);
			leggiDalSocket(clientFd, nuovo_arrivo->categoriaPersona, dimensione);

			aggiungiPersonaLista(coda, nuovo_arrivo);

			ora = time( NULL);
			int tempo_generazione = ora - tempo_avvio;

			printf(
					"[GENERATO] %s al piano %i,con destinazione piano %i, ora avvio %i, %s\n",
					nuovo_arrivo->categoriaPersona, numero_piano,
					nuovo_arrivo->destinazione, tempo_generazione, ctime(&ora));
			fprintf(logFp,
					"[GENERATO] %s,con destinazione piano %i, ora avvio %i, %s\n",
					nuovo_arrivo->categoriaPersona, nuovo_arrivo->destinazione,
					tempo_generazione, ctime(&ora));

		} else if (connessione == CONNESSIONE_ASCENSORE) {// si e' connesso l'ascensore
			printf("Connessione del server piano %i, connessione ascensore\n", numero_piano);
			int peso = 0;
			int presente = 0;
			//riceve il peso massimo caricabile dall'ascensore
			leggiDalSocket(clientFd, &peso, sizeof(int));
			while (1) {
				testa = getTestaLista(coda);
				if (testa == NULL) {
					presente = 0;
					scriviNelSocket(clientFd, &presente, sizeof(int));
					close(clientFd);
					break;
				}
				peso = peso - testa->persona->peso;
				if (peso < 0) {
					presente = 0;
					scriviNelSocket(clientFd, &presente, sizeof(int));
					close(clientFd);
					break;
				}

				//comunica all'ascensore che ci sono persone da inviare
				presente = 1;
				scriviNelSocket(clientFd, &presente, sizeof(int));
				//write(clientFd, &presente, sizeof(int));

				//invia la persona
				scriviNelSocket(clientFd, testa->persona, sizeof(Persona));
				//write(clientFd, testa->persona, sizeof(Persona));

				//invia la dimensione della stringa categoriaPersona
				long unsigned dimensione = strlen(testa->persona->categoriaPersona) + 1;

				scriviNelSocket(clientFd, &dimensione, sizeof(dimensione));

				//invia la stringa categoriaPersona
				scriviNelSocket(clientFd, testa->persona->categoriaPersona,
						dimensione);

				eliminaPerTipo(coda, testa->persona->categoriaPersona);
			}
			close(clientFd);
		} else {
			printf("Errore durante la connessione");
			continue;
		}
		if (tempo_terminazione <= time(NULL)) {
			printf("Tempo limite raggiunto. ");
			break;
		}
	}
	close(SocketFd);
	printf("Server terminato, piano%i\n", numero_piano);
	ora = time(NULL);
	fprintf(logFp, "Terminazione piano, %s (%i)\n", ctime(&ora), (int) ora);
	fclose(logFp);
}

void leggi_parametri(int argc, char* argv[]) {
	if (argc == 1) {
		printf("Esecuzione con tempo limite di 5 minuti\n");
		return;
	}
	if (argc == 2 && strcmp(argv[1], "--fine-servizio") == 0) {
		terminazione = fine_servizio;
		printf(
				"Esecuzione senza tempo limite, fino alla fine del servizio\n");
	} else {
		printf(
				"Uso: %s [--fine-servizio]\nIl programma termina dopo 5 minuti dall'avvio.\n"
						"--fine-servizio: il programma termina solo quando ha terminato di servire i passeggeri\n",
				argv[0]);
		exit(1);
	}
}

int main(int argc, char * argv[]) {
	int status = 0;
	numero_piano = 0;
	int pidserver1 = 0;
	int pidserver2 = 0;
	int pidserver3 = 0;

	leggi_parametri(argc, argv);

	tempo_avvio = time(NULL) + 3;
	int prima_fork = 0;
	int pid = fork();
	if (pid) {
		prima_fork++;
	}
	pidserver2 = pid;

	pid = fork();
	//assegna i numeri dei piani differenziandoli in base ai risultati delle fork
	//e salva anche i pid dei figli (che saranno i piani server) per la terminazione
	if (pid) {
		if (prima_fork) {
			numero_piano = 2;
			pidserver3 = pid;
		} else {
			numero_piano = 0;
			pidserver1 = pid;
		}
	} else {
		if (prima_fork) {
			numero_piano = 3;
		} else {
			numero_piano = 1;
		}
	}

	pid = fork();
	time_t now = time(NULL);
	tempo_terminazione = now + (DURATA_MAX_MINUTI * 60);

	if (!pid) {
		sleep(tempo_avvio - now + 2);
		tempo_avvio = time(NULL);
		client();
	} else {
		sleep(tempo_avvio - now);
		tempo_avvio = time(NULL) + 2;
		server();
		//aspetta terminazione del piano client corrispondente
		waitpid(pid, &status, 0);
	}
	//per server piano 2 risulta pidserver3 != 0 e aspetta che server piano 3 termini
	if (pidserver3)
		waitpid(pidserver3, &status, 0);
	//per server piano 0 risulta pidserver1 != 0 e aspetta che server piano 1 termini
	if (pidserver1)
		waitpid(pidserver1, &status, 0);
	//per server piano 0 risulta pidserver2 != 0 e aspetta che server piano 2 termini
	if (pidserver2)
		waitpid(pidserver2, &status, 0);
	return status;
}
