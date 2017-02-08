
#include "persona.h"
#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

typedef struct _nodo_lista_persone nodo_lista_persone;

typedef struct _nodo_lista_persone {
	Persona* persona;
	nodo_lista_persone* successivo;
} nodo_lista_persone;

typedef struct _lista_persone {
	nodo_lista_persone* testaLista;
	nodo_lista_persone* elementoCorrente;
} lista_persone;


lista_persone* crea_lista_persone();

nodo_lista_persone* getHead(lista_persone* lista);

nodo_lista_persone* aggiungi_alla_lista(lista_persone* lista,
		Persona* persona_da_aggiungere);

nodo_lista_persone* cerca_per_tipo(lista_persone* lista, char *tipo,
		nodo_lista_persone **prev);

void cancella_per_tipo(lista_persone* lista, char *tipo);

Persona* cancella_scesa(lista_persone* lista, int arrivo);

#endif /* LINKEDLIST_H_ */
