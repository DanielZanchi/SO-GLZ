#ifndef PERSONA_H_
#define PERSONA_H_

typedef struct _persona {
    char* categoriaPersona;
    int peso;
    int destinazione;
} Persona;

static char *categoriePersone[3] = { "Adulto", "Bambino", "Addetto alla consegna" };

Persona creaPersona(char tipo, int destinazione);

#endif /* PERSONA_H_ */
