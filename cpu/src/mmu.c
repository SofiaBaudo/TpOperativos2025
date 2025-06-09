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
    int marco;
    marco = navegarNiveles(numPag, pid);
    int direccionFisica = marco*tamPag + desplazamiento;

    return direccionFisica; 
}

void enviarValoresMem(int entradaNivel, int pid){
    t_buffer *buffer = crear_buffer_MarcoMem(pid,entradaNivel);
    crear_paquete(ENVIO_PID_Y_ENTRADANIVEL, buffer,fd_conexion_dispatch_memoria);
}

int navegarNiveles(int numPag, int pid){
    int numMarco;
    for(int i = 1; i < cantNiveles+1; i++){
        int elevado = pow(entradasTabla, cantNiveles-i);
        int entradaNivel = floor((numPag/elevado) % entradasTabla);
        enviarValoresMem(entradaNivel, pid);
        numMarco = conseguirMarco();
        //guardo la entrada nivel por cada 
    }
    //el numero de marco que tenemos despues del if es el num de marco final(el ultimo de todos --> es el marco fisico)
    int marcoFinal = numMarco;
    return marcoFinal;
}
int conseguirMarco(){
    int numMarco;
    recv(fd_conexion_dispatch_memoria, &numMarco, sizeof(int), 0);
    //se envia el paquete con la entradaNivel y el pid, memoria lo deseerializa y me envia el numero de marco unicamente, entonces no hace falta hacer un paquete. 
    return numMarco;
}