#include <stdlib.h>
#include <stdio.h>
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
    nodo_lista_persone* nodo = (nodo_lista_persone*) malloc(sizeof(nodo_lista_persone));
    if (nodo == NULL) {
        printf("\n Creazione del nodo della linked list fallita!\n");
        return NULL;
    }
    nodo->persona = persona_da_aggiungere;
    nodo->successivo = NULL;
    
    lista->testaLista = lista->elementoCorrente = nodo;
    
    return nodo;
}

nodo_lista_persone* aggiungiPersonaLista(lista_persone* lista,	Persona* persona_da_aggiungere) {  //Crea una lista se non c'è gia, altrimenti alloca in memoria spazio per nodo e puntatore
    if (lista->testaLista == NULL) {
        return (creaTesta(lista, persona_da_aggiungere));
    }
    nodo_lista_persone *nodo = (nodo_lista_persone*) malloc(sizeof(nodo_lista_persone));
    if (nodo == NULL) {
        printf("\n Creazione del nodo della linked list fallita! \n");
        return NULL;
    }
    
    nodo->persona = persona_da_aggiungere;
    nodo->successivo = NULL;
    lista->elementoCorrente->successivo = nodo;
    lista->elementoCorrente = nodo;
    
    return nodo;
}

nodo_lista_persone* ricercaPerTipo(lista_persone* lista, char *tipo, nodo_lista_persone **precedente) { //Restituisce il primo puntatore relativo al tipo di persona trovata, salvando il precedente nodo in precedente
    nodo_lista_persone *nodo_temp = NULL;
    nodo_lista_persone *nodo = lista->testaLista;
    int found = 0;
    
    while (nodo != NULL) {
        if (strcmp(nodo->persona->categoriaPersona, tipo) == 0) {
            found = 1;
            break;
        } else {
            nodo_temp = nodo;
            nodo = nodo->successivo;
        }
    }
    if (found) {
        if (precedente) {
            *precedente = nodo_temp;
        }
        return nodo;
    } else {
        return NULL;
    }
}

void eliminaPerTipo(lista_persone* lista, char *tipo) { //Cancella del tutto la prima persona del tipo specificato, liberando anche la memoria
    nodo_lista_persone * eliminata = NULL;
    nodo_lista_persone* precedente = NULL;
    
    eliminata = ricercaPerTipo(lista, tipo, &precedente);
    
    if (eliminata) {
        if (precedente != NULL) {
            precedente->successivo = eliminata->successivo;
        }
        if (eliminata == lista->elementoCorrente) {
            lista->elementoCorrente = precedente;
        }
        if (eliminata == lista->testaLista) {
            lista->testaLista = eliminata->successivo;
        }
        free(eliminata->persona);
        free(eliminata);
    } else {
        return ;
    }
}

nodo_lista_persone* ricercaPerDestinazione(lista_persone* lista, int destination, nodo_lista_persone** precedente) { //Restituisce la prima persona trovata con destinazione uguale a "destinazione", aggiorna anche "precedente"
    nodo_lista_persone *nodo = lista->testaLista;
    nodo_lista_persone *nodo_temp = NULL;
    int found = 0;
    
    while (nodo != NULL) {
        if (nodo->persona->destinazione == destination) {
            found = 1;
            break;
        } else {
            nodo_temp = nodo;
            nodo = nodo->successivo;
        }
    }
    
    if (found) {
        if (precedente) {
            *precedente = nodo_temp;
        }
        return nodo;
    } else {
        return NULL;
    }
}

Persona* eliminaDiscesa(lista_persone* lista, int arrivo) { //Cancella la prima occorrenza che ha destinazione uguale ad "arrivo", restituisce il puntatore al nodo appena eliminato
    nodo_lista_persone *precedente = NULL;
    nodo_lista_persone * eliminata = NULL;
    Persona* cancellata = NULL;
    
    eliminata = ricercaPerDestinazione(lista, arrivo, &precedente);
    
    if (eliminata) {
        if (precedente != NULL) {
            precedente->successivo = eliminata->successivo;
        }
        if (eliminata == lista->elementoCorrente) {
            lista->elementoCorrente = precedente;
        }
        if (eliminata == lista->testaLista) {
            lista->testaLista = eliminata->successivo;
            
        }
        cancellata = eliminata->persona;
        free(eliminata);
        return cancellata;
    }
    return cancellata;
}
