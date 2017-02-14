#include "persona.h"

Persona creaPersona(char categoria, int destinazione) {
    Persona nuova_persona;
    nuova_persona.destinazione = destinazione;
    switch (categoria) {
        case 'A':
            nuova_persona.categoriaPersona = categoriePersone[0];
            nuova_persona.peso = 80;
            break;
        case 'B':
            nuova_persona.categoriaPersona = categoriePersone[1];
            nuova_persona.peso = 40;
            break;
        case 'C':
            nuova_persona.categoriaPersona = categoriePersone[2];
            nuova_persona.peso = 90;
            break;
    }
    return nuova_persona;
}
