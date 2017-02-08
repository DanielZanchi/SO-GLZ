/*
 * Persona.c
 *
 *  Created on: 07 giu 2016
 *      Author: cosimo
 */

#include "persona.h"

Persona creaPersona(char tipo, int destinazione) {
	Persona nuova_persona;
	switch (tipo) {
	case 'A':
		nuova_persona.peso = 80;
		nuova_persona.categoriaPersona = categoriePersone[0];
		break;
	case 'B':
		nuova_persona.peso = 40;
		nuova_persona.categoriaPersona = categoriePersone[1];
		break;
	case 'C':
		nuova_persona.peso = 90;
		nuova_persona.categoriaPersona = categoriePersone[2];
		break;
	}
	nuova_persona.destinazione = destinazione;
	return nuova_persona;
}
