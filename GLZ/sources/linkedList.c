

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"

lista_persone* creaListaPersone() {     // Viene inizializzata la lista che conterra le persone
	lista_persone* lista = (lista_persone*) malloc(sizeof(lista_persone));
	lista->testaLista = NULL;
	lista->elementoCorrente = NULL;
	return lista;
}

nodo_lista_persone* getTestaLista(lista_persone* lista) { // Restituisce la testa della lista
	return lista->testaLista;
}

nodo_lista_persone* creaTesta(lista_persone* lista, Persona* persona_da_aggiungere) {  // Aggiunge alla lista di persone la nuova persona, gestendo anche l'eventuale fallimento
	nodo_lista_persone* nodo = (nodo_lista_persone*) malloc(
			sizeof(nodo_lista_persone));
	if (NULL == nodo) {
		printf("\n Creazione del nodo della linked list fallita!\n");
		return NULL;
	}
	nodo->persona = persona_da_aggiungere;
	nodo->successivo = NULL;

	lista->testaLista = lista->elementoCorrente = nodo;

	return nodo;
}

nodo_lista_persone* aggiungiPersonaLista(lista_persone* lista,	Persona* persona_da_aggiungere) {  //Crea una lista se non c'è gia, altrimenti alloca in memoria spazio per nodo e puntatore
	if (NULL == lista->testaLista) {
		return (creaTesta(lista, persona_da_aggiungere));
	}
	nodo_lista_persone *nodo = (nodo_lista_persone*) malloc(
			sizeof(nodo_lista_persone));
	if (NULL == nodo) {
		printf("\n Creazione del nodo della linked list fallita! \n");
		return NULL;
	}

	nodo->persona = persona_da_aggiungere;

	nodo->successivo = NULL;

	lista->elementoCorrente->successivo = nodo;
	lista->elementoCorrente = nodo;
	return nodo;
}

nodo_lista_persone* ricercaPerTipo(lista_persone* lista, char *tipo,nodo_lista_persone **precedente) { //Restituisce il primo puntatore relativo al tipo di persona trovata, salvando il precedente nodo in precedente
	nodo_lista_persone *nodo = lista->testaLista;
	nodo_lista_persone *tmp = NULL;

	int found = 0;

	while (nodo != NULL) {
		if (strcmp(nodo->persona->categoriaPersona, tipo) == 0) {
			found = 1;
			break;
		} else {
			tmp = nodo;
			nodo = nodo->successivo;
		}
	}
	if (found) {
		if (precedente) {
			*precedente = tmp;
		}
		return nodo;
	} else {
		return NULL;
	}
}

void eliminaPerTipo(lista_persone* lista, char *tipo) { //Cancella del tutto la prima persona del tipo specificato, liberando anche la memoria
	nodo_lista_persone* precedente = NULL;
	nodo_lista_persone *del = NULL;

	del = ricercaPerTipo(lista, tipo, &precedente);

	if (del) {
		if (precedente != NULL) {
			precedente->successivo = del->successivo;
		}
		if (del == lista->elementoCorrente) {
			lista->elementoCorrente = precedente;
		}
		if (del == lista->testaLista) {
			lista->testaLista = del->successivo;
		}
		free(del->persona);
		free(del);
	} else {
		return ;
	}
}

nodo_lista_persone* ricercaPerDestinazione(lista_persone* lista, int destination, nodo_lista_persone** precedente) { //Restituisce la prima persona trovata con destinazione uguale a "destinazione", aggiorna anche "precedente"
	nodo_lista_persone *nodo = lista->testaLista;
	nodo_lista_persone *tmp = NULL;
	int found = 0;

	while (nodo != NULL) {
		if (nodo->persona->destinazione == destination) {
			found = 1;
			break;
		} else {
			tmp = nodo;
			nodo = nodo->successivo;
		}
	}

	if (found) {
		if (precedente) {
			*precedente = tmp;
		}
		return nodo;
	} else {
		return NULL;
	}
}
Persona* eliminaDiscesa(lista_persone* lista, int arrivo) { //Cancella la prima occorrenza che ha destinazione uguale ad "arrivo", restituisce il puntatore al nodo appena eliminato
	nodo_lista_persone *precedente = NULL;
	nodo_lista_persone *del = NULL;
	Persona* cancellata = NULL;

	del = ricercaPerDestinazione(lista, arrivo, &precedente);

	if (del) {
		if (precedente != NULL) {
			precedente->successivo = del->successivo;
		}
		if (del == lista->elementoCorrente) {
			lista->elementoCorrente = precedente;
		}
		if (del == lista->testaLista) {
			lista->testaLista = del->successivo;

		}
		cancellata = del->persona;
		free(del);
		return cancellata;
	}
	return cancellata;
}
