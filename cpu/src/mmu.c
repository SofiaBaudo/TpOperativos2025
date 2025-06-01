#include <mmu.h>

/*
Las direcciones logicas estarian compuestas de esta manera. 
[entrada_nivel_1 | entrada_nivel_2 | … | entrada_nivel_X | desplazamiento]
¿SE PODRIA PENSAR CON ARBOLES?
EL desplazameinto dice cuantos bits avanzar hasta la direccion que queres. 
*/
//LA DIRECCION NOS LA DA EL PARAMETRO DE LAS INSTRUCCIONES
//EL TAMAÑO DE LA PAG TE LA DA EL HANDSJAKE
struct nivel{
    int numeroNivel;
    int tamanio;
    void* tablaDeNivel[400]; //es puntero generico porque hay veces que apunta a una pagina y veces que apunta a un prox nivel
}

void creacionNivelesN(int n){
   nivel* niveles[n]; 
   //es una tabla de n niveles donde cada nivel tiene el tipo de dato nivel. 
}

int traduccion(int direccion){ //te tendria que devolver la dir fisica
    int numPag = floor(direccion/tamPag);
    int desplazamiento = direccion % tamPag; 
    int numeroMarco;
    numeroMarco = accederATP(numPag);
    //SEGUIR
}

int accederATp(int numPag){
    int numMarco = 0;
    send(fd_conexion_dispatch_memoria, &numPag, sizeof(int), 0);
    recv(fd_conexion_dispatch_memoria, &numMarco, sizeof(int), 0);
    return numMarco;
}