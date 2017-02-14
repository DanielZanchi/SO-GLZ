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

lista_persone* creaListaPersone();

nodo_lista_persone* getTestaLista(lista_persone* lista);

nodo_lista_persone* aggiungiPersonaLista(lista_persone* lista, Persona* persona_da_aggiungere);

nodo_lista_persone* ricercaPerTipo(lista_persone* lista, char *tipo, nodo_lista_persone **precedente);

void eliminaPerTipo(lista_persone* lista, char *tipo);

Persona* eliminaDiscesa(lista_persone* lista, int arrivo);

#endif /* LINKEDLIST_H_ */
