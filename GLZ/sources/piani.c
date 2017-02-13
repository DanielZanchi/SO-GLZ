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
static const char* FILES_LOG[4] = { "piano0.log", "piano1.log",
"piano2.log", "piano3.log" };

time_t tempo_avvio;
time_t tempo_terminazione;

enum terminazione {
	cinque_minuti, fine_servizio
} terminazione = cinque_minuti;

int numero_piano;

void scriviNelSocket(int SocketFd, const void* buffer, size_t size) {
	int scritto = write(SocketFd, buffer, size);
	if (scritto < size) {
		char* msg;
		asprintf(&msg,
			"Errore invio messaggio socket \"%s\", terminazione al piano %i...\n",
			SOCKETS_PIANI[numero_piano], numero_piano);
			perror(msg);
			exit(10);
		}
	}

	void leggiDalSocket(int SocketFd, void* nuovo_arrivato, size_t size) {
		int letto = read(SocketFd, nuovo_arrivato, size);
		if (letto < 0) {
			char* msg;
			asprintf(&msg,
				"Errore ricezione messaggio socket \"%s\", terminazione al piano %i...\n",
				SOCKETS_PIANI[numero_piano], numero_piano);
				perror(msg);
				exit(10);
			}
		}

		void client() {
			char* categoria = NULL;

			char* tempo_generazione = NULL;
			int tempo_generazione_intero = 0;

			char* destinazione = NULL;
			int destinazione_intero = 0;

			FILE * input_file_piani = NULL;

			printf("Esecuzione del client, piano%i\n", numero_piano);

			input_file_piani = fopen(PIANI_FILE_INPUT[numero_piano], "r");

			if (input_file_piani == NULL) {
				char* msg;
				asprintf(&msg,
					"Impossibile aprire file di input \"%s\", terminazione client, piano %i ",
					PIANI_FILE_INPUT[numero_piano], numero_piano);
					perror(msg);
					exit(20);
				}

				while (1) {
					char* riga_corrente = NULL;
					int presente = 0;
					int tempo;
					size_t lunghezza = 0;

					//legge una riga dal file di input e genera la persona
					//se la riga e' vuota, termina

					int bytes_letti=getline(&riga_corrente, &lunghezza, input_file_piani);
					if(bytes_letti == -1){
						char* msg;
						asprintf(&msg,
							"Si e' verificato un errore di lettura \"%s\", terminazione client, piano %i\n",
							PIANI_FILE_INPUT[numero_piano], numero_piano);
							perror(msg);
							exit(20);
						}

						if (strcmp(riga_corrente, "\n") == 0) {
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
									exit(10);
								}

								SocketAddress.sun_family = AF_UNIX; /* Set domain type */
								strcpy(SocketAddress.sun_path, SOCKETS_PIANI[numero_piano]); /* Set name */

								int connesso = connect(SocketFd, SocketAddrPtr, SocketLenght);
								if (connesso == -1) {
									char* msg;
									asprintf(&msg, "Client al piano %i errore connessione", numero_piano);
									perror(msg);
									fclose(input_file_piani);
									exit(12);
								}
								char* riga_temp = riga_corrente;

								categoria = strsep(&riga_temp, " ");
								tempo_generazione = strsep(&riga_temp, " ");
								destinazione = strsep(&riga_temp, " ");

								tempo_generazione_intero = atoi(tempo_generazione);
								destinazione_intero = atoi(destinazione);

								Persona persona = creaPersona(categoria[0], destinazione_intero);
								free(riga_corrente);
								tempo = time(NULL);
								sleep(tempo_generazione_intero - (tempo - tempo_avvio));

								scriviNelSocket(SocketFd, &CONNESSIONE_PIANO_CLIENT,sizeof(CONNESSIONE_PIANO_CLIENT));

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

								fclose(input_file_piani);
							}

							void server() {
								time_t ora;
								lista_persone* coda = NULL;
								nodo_lista_persone* testa = NULL;
								FILE* log_file = NULL;
								Persona* nuovo_arrivato = NULL;
								int connessione = -1;

								coda = creaListaPersone();
								log_file = fopen(FILES_LOG[numero_piano], "w");

								printf("Start server, piano%i\n", numero_piano);
								if (log_file < 0) {
									char* msg;
									asprintf(&msg,
										"Impossibile aprire file di log \"%s\", terminazione server piano %i...",
										FILES_LOG[numero_piano], numero_piano);
										perror(msg);
										exit(20);
									}

									fprintf(log_file, "Avviato piano: %s (%i)\n", ctime(&tempo_avvio),
									(int) tempo_avvio);

									unlink(SOCKETS_PIANI[numero_piano]);
									int SocketFd;
									socklen_t SocketLenght;
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
											exit(10);
										}
										SocketAddress.sun_family = AF_UNIX; /* Set domain type */
										strcpy(SocketAddress.sun_path, SOCKETS_PIANI[numero_piano]); /* Set name */
										int indirizzo = bind(SocketFd, SocketAddrPtr, SocketLenght);
										if (indirizzo == -1) {
											printf(
												"Errore durante la bind del socket \"%s\", terminazione del server, piano %i...",
												SOCKETS_PIANI[numero_piano], numero_piano);
												perror("");
												exit(10);
											}
											indirizzo = listen(SocketFd, 2);
											if (indirizzo == -1) {
												printf(
													"Errore durante la listen del socket \"%s\", terminazione del server, piano %i...",
													SOCKETS_PIANI[numero_piano], numero_piano);
													perror("");
													exit(10);
												}

												while (1) {
													int clientFd = accept(SocketFd, SocketAddrPtr, &SocketLenght);
													if (clientFd == -1) {
														perror("Impossibile effettuare la connessione con i clients");
														printf("Terminazione del server, piano %i", numero_piano);
														exit(15);
													}
													//leggiDalSocket(clientFd, &connessione, sizeof(connessione));
													read(clientFd, &connessione, sizeof(connessione));

													if (connessione == CONNESSIONE_PIANO_CLIENT) {// si e' connesso un piano-client
													printf("Connessione server piano %i, con piano client\n", numero_piano);

													nuovo_arrivato = (Persona*) malloc(sizeof(Persona));
													if (nuovo_arrivato == NULL) {
														printf(
															"Errore allocazione memoria, terminazione server piano %i...",
															numero_piano);
															exit(26);
														}

														leggiDalSocket(clientFd, nuovo_arrivato, sizeof(Persona));

														//legge lunghezza striga categoriaPersona
														long unsigned dimensione = 0;
														leggiDalSocket(clientFd, &dimensione, sizeof(dimensione));

														//legge stringa categoriaPersona
														nuovo_arrivato->categoriaPersona = (char*) malloc(dimensione);
														leggiDalSocket(clientFd, nuovo_arrivato->categoriaPersona, dimensione);

														aggiungiPersonaLista(coda, nuovo_arrivato);

														ora = time( NULL);
														printf(
															"[GENERATO] %s al piano %i,con destinazione piano %i, %s\n",
															nuovo_arrivato->categoriaPersona, numero_piano,
															nuovo_arrivato->destinazione, ctime(&ora));
															fprintf(log_file,
																"[GENERATO] %s,con destinazione piano %i, %s\n",
																nuovo_arrivato->categoriaPersona, nuovo_arrivato->destinazione,
																ctime(&ora));

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
														fprintf(log_file, "Terminazione piano, %s (%i)\n", ctime(&ora), (int) ora);
														fclose(log_file);
													}

													void leggiArgomenti(int argc, char* argv[]) {
														if (argc == 1) {
															printf("Esecuzione con tempo limite di 5 minuti\n");
															return;
														}
														if (argc == 2 && strcmp(argv[1], "-termina") == 0) {
															terminazione = fine_servizio;
															printf(
																"Esecuzione senza tempo limite, fino alla fine del servizio\n");
															} else {
																printf(
																	"Uso:\n%s: Il programma termina dopo 5 minuti dall'avvio.\n"
																	"%s -termina: Il programma prosegue fino a quando non ha finito di servire i passeggeri.\n",
																	argv[0], argv[0]);
																	exit(1);
																}
															}

															int main(int argc, char * argv[]) {
																int status = 0;
																numero_piano = 0;
																int pidserver1 = 0;
																int pidserver2 = 0;
																int pidserver3 = 0;

																leggiArgomenti(argc, argv);

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
																// se il pid == 0, quindi è un nuovo server allora esso sarà un client, altrimenti server
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
