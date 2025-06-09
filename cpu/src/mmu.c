#include <mmu.h>
#include <math.h>

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
};
/*
void creacionNivelesN(int n){
   nivel* niveles[n]; 
   //es una tabla de n niveles donde cada nivel tiene el tipo de dato nivel. 
}
*/

int traduccion(int direccion, int pid){ //te tendria que devolver la dir fisica
    int numPag = floor(direccion/tamPag);
    int desplazamiento = direccion % tamPag; 
    int numeroMarco;
    navegarNiveles(numPag);
    numeroMarco = accederATp(numPag, pid);
    //SEGUIR
    return numeroMarco; //lo pongo esto para que no tire warning
}

void enviarValoresMem(int numPag, int pid){
    t_buffer *buffer = crear_buffer_paginaMem(pid,numPag);
    crear_paquete(ENVIO_PID_Y_NUMPAG, buffer,fd_conexion_dispatch_memoria);
}

void navegarNiveles(int numPag){
    int marcoDeNiveles[cantNiveles];
    for(int i = 0; i < cantNiveles; i++){
        double entradaNivel = floor(numPag/entradasTabla^(cantNiveles-i)) % entradasTabla;
        marcoDeNiveles[i] = entradaNivel;
        //guardo la entrada nivel por cada 
    }
}