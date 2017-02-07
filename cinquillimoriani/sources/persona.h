/*
 * persona.h
 *
 *  Created on: 06 giu 2016
 *      Author: cosimo
 */

#ifndef PERSONA_H_
#define PERSONA_H_

typedef struct _persona {
	char* nomeTipo;
	int peso;
	int destinazione;
} Persona;

static char *nomiTipi[3] = { "Adulto", "Bambino", "Addetto alla consegna" };

Persona creaPersona(char tipo, int destinazione);

#endif /* PERSONA_H_ */
