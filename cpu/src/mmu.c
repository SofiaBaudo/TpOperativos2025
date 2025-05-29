#include <mmu.h>

/*
Las direcciones logicas estarian compuestas de esta manera. 
[entrada_nivel_1 | entrada_nivel_2 | … | entrada_nivel_X | desplazamiento]
¿SE PODRIA PENSAR CON ARBOLES?
EL desplazameinto dice cuantos bits avanzar hasta la direccion que queres. 
*/

struct pagina{
    int numpag;
    int tamPag;
}

struct nivel{
    int numeroNivel;
    int tamanio;
    void* tablaDeNivel[400]; //es puntero generico porque hay veces que apunta a una pagina y veces que apunta a un prox nivel
}

void creacionNivelesN(int n){
   nivel* niveles[n]; 
   //es una tabla de n niveles donde cada nivel tiene el tipo de dato nivel. 
}
