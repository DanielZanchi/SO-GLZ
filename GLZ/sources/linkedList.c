/*
 * linkedList.c
 *
 *  Created on: 07 giu 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"

lista_persone* crea_lista_persone() {     // Viene inizializzata la lista che conterra le persone
	lista_persone* lista = (lista_persone*) malloc(sizeof(lista_persone));
	lista->head = NULL;
	lista->current = NULL;
	return lista;
}

nodo_lista_persone* getHead(lista_persone* lista) { // Restituisce la testa della lista
	return lista->head;
}

nodo_lista_persone* crea_testa(lista_persone* lista, Persona* persona_da_aggiungere) {  // Aggiunge alla lista di persone la nuova persona, gestendo anche l'eventuale fallimento
	nodo_lista_persone* nodo = (nodo_lista_persone*) malloc(
			sizeof(nodo_lista_persone));
	if (NULL == nodo) {
		printf("\n Creazione del nodo della linked list fallita!\n");
		return NULL;
	}
	nodo->persona = persona_da_aggiungere;
	nodo->next = NULL;

	lista->head = lista->current = nodo;

	return nodo;
}

nodo_lista_persone* aggiungi_alla_lista(lista_persone* lista,	Persona* persona_da_aggiungere) {  //Crea una lista se non c'è gia, altrimenti alloca in memoria spazio per nodo e puntatore
	if (NULL == lista->head) {
		return (crea_testa(lista, persona_da_aggiungere));
	}
	nodo_lista_persone *nodo = (nodo_lista_persone*) malloc(
			sizeof(nodo_lista_persone));
	if (NULL == nodo) {
		printf("\n Creazione del nodo della linked list fallita! \n");
		return NULL;
	}

	nodo->persona = persona_da_aggiungere;

	nodo->next = NULL;

	lista->current->next = nodo;
	lista->current = nodo;
	return nodo;
}

nodo_lista_persone* cerca_per_tipo(lista_persone* lista, char *tipo,nodo_lista_persone **prev) { //Restituisce il primo puntatore relativo al tipo di persona trovata, salvando il precedente nodo in prev
	nodo_lista_persone *nodo = lista->head;
	nodo_lista_persone *tmp = NULL;

	int found = 0;

	while (nodo != NULL) {
		if (strcmp(nodo->persona->nomeTipo, tipo) == 0) {
			found = 1;
			break;
		} else {
			tmp = nodo;
			nodo = nodo->next;
		}
	}
	if (found) {
		if (prev) {
			*prev = tmp;
		}
		return nodo;
	} else {
		return NULL;
	}
}

int cancella_per_tipo(lista_persone* lista, char *tipo) { //Cancella del tutto la prima persona del tipo specificato, liberando anche la memoria
	nodo_lista_persone* prev = NULL;
	nodo_lista_persone *del = NULL;

	del = cerca_per_tipo(lista, tipo, &prev);

	if (del) {
		if (prev != NULL) {
			prev->next = del->next;
		}
		if (del == lista->current) {
			lista->current = prev;
		}
		if (del == lista->head) {
			lista->head = del->next;
		}
		free(del->persona);
		free(del);
	} else {
		return -1;
	}
}

nodo_lista_persone* cerca_per_destinazione(lista_persone* lista, int destination, nodo_lista_persone** prev) { //Restituisce la prima persona trovata con destinazione uguale a "destination", aggiorna anche "prev"
	nodo_lista_persone *nodo = lista->head;
	nodo_lista_persone *tmp = NULL;
	int found = 0;

	while (nodo != NULL) {
		if (nodo->persona->destinazione == destination) {
			found = 1;
			break;
		} else {
			tmp = nodo;
			nodo = nodo->next;
		}
	}

	if (found) {
		if (prev) {
			*prev = tmp;
		}
		return nodo;
	} else {
		return NULL;
	}
}
Persona* cancella_scesa(lista_persone* lista, int arrivo) { //Cancella la prima occorrenza che ha destinazione uguale ad "arrivo", restituisce il puntatore al nodo appena eliminato
	nodo_lista_persone *prev = NULL;
	nodo_lista_persone *del = NULL;
	Persona* cancellata = NULL;

	del = cerca_per_destinazione(lista, arrivo, &prev);

	if (del) {
		if (prev != NULL) {
			prev->next = del->next;
		}
		if (del == lista->current) {
			lista->current = prev;
		}
		if (del == lista->head) {
			lista->head = del->next;

		}
		cancellata = del->persona;
		free(del);
		return cancellata;
	}
	return cancellata;
}
