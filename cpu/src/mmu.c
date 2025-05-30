#include <mmu.h>

/*
Las direcciones logicas estarian compuestas de esta manera. 
[entrada_nivel_1 | entrada_nivel_2 | … | entrada_nivel_X | desplazamiento]
¿SE PODRIA PENSAR CON ARBOLES?
EL desplazameinto dice cuantos bits avanzar hasta la direccion que queres. 
*/

struct pagina{
    int numpag;
    //int tamPag; el tamPag lo tendriamos que pasar en el handshake con memoria. Entonces lo voy a poner ocmo una variable global y vemos
}

struct nivel{
    int numeroNivel;
    int tamanio;
    void* tablaDeNivel[400]; //es puntero generico porque hay veces que apunta a una pagina y veces que apunta a un prox nivel
}
//uint32_t es el tipo de dato que se suele hacer para la direccion de datos, es para 32 bits. 

void creacionNivelesN(int n){
   nivel* niveles[n]; 
   //es una tabla de n niveles donde cada nivel tiene el tipo de dato nivel. 
}
uint32_t traduccion(int desplazamiento, nivel niveles[], int cantNiveles, uint32_t dirLogica){
    pagina pagina;
    pagina.numpag = floor(dirLogica / tamPag);
    int entrada_nivel; 
    //no sabria como encararlo --> que tipo de dato seria? que es lo que tiene dentro.
    int desplazamiento = dirLogica % tamPag;
    //el desplazamiento se hace en cada nivel?
}