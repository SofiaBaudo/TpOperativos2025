#include <traduccion.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/*
Las direcciones logicas estarian compuestas de esta manera. 
[entrada_nivel_1 | entrada_nivel_2 | … | entrada_nivel_X | desplazamiento]
¿SE PODRIA PENSAR CON ARBOLES?
EL desplazameinto dice cuantos bits avanzar hasta la direccion que queres. 
*/
//LA DIRECCION NOS LA DA EL PARAMETRO DE LAS INSTRUCCIONES
//EL TAMAÑO DE LA PAG TE LA DA EL HANDSJAKE

typedef struct{
    int numPag;
    int numMarco;
}infotlb;

typedef struct NodoEntradasTLB{
    infotlb info;
    struct NodoEntradasTLB *sgte;
}NodoEntradasTLB;

NodoEntradasTLB *listaTlb = NULL; 

int traduccion(int direccion, int pid){ //te tendria que devolver la dir fisica
    int numPag = floor(direccion/tamPag);
    int desplazamiento = direccion % tamPag; 
    int marco;
    int tlbrespuesta;
    tlbrespuesta = buscarTlb(pid, numPag);
    if(tlbrespuesta == -1){
        //osea que no se encontro
        marco = navegarNiveles(numPag, pid);
    }
    else{
        marco = tlbrespuesta;
    }
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
        numMarco = conseguirMarco(pid);
        agregarEntradaATLB(numPag, numMarco);
        //guardo la entrada nivel por cada 
    }
    //el numero de marco que tenemos despues del if es el num de marco final(el ultimo de todos --> es el marco fisico)
    int marcoFinal = numMarco;
    return marcoFinal;
}
int conseguirMarco(int pid){
    int numMarco;
    recv(fd_conexion_dispatch_memoria, &numMarco, sizeof(int), 0);
    log_info(logger,"PID: <%d> - OBTENER MARCO - Página: <NUMERO_PAGINA> - Marco: <%d>", pid, numMarco);
    //se envia el paquete con la entradaNivel y el pid, memoria lo deseerializa y me envia el numero de marco unicamente, entonces no hace falta hacer un paquete. 
    return numMarco;
}
int buscarTlb(int numPag, int pid){
    NodoEntradasTLB *aux = listaTlb;
    while(aux != NULL && aux->sgte != NULL){   
        if(aux->info.numPag == numPag){
            log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, aux->info.numPag);
            log_info(logger,"PID: <%d> - OBTENER MARCO - Página: <NUMERO_PAGINA> - Marco: <%d>", pid, aux->info.numPag);
            return aux->info.numMarco;
        }
        aux = aux->sgte;
        
    }
    log_info(logger,"PID: <%d> - TLB MISS - Pagina: <%d>", pid, numPag);
    return -1;
}
void agregarEntradaATLB(int numPag, int numMarco){
    if(strcmp(REEMPLAZO_TLB, "FIFO") == 0){
        implementarAlgoritmoFIFO(numPag, numMarco);
    }
    else if(strcmp(REEMPLAZO_TLB, "LRU") == 0){
        implementarAlgoritmoLRU(numPag, numMarco);
    }
}

void implementarAlgoritmoFIFO(int numPag, int numMarco){
//TODO
}

void implementarAlgoritmoLRU(int numPag, int numMarco){
//TODO
}

void inicializarTLB(){
    listaTlb = NULL;
    NodoEntradasTLB *aux = listaTlb;
    NodoEntradasTLB *ant = NULL;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        NodoEntradasTLB* nuevo = malloc(sizeof(NodoEntradasTLB));
        nuevo->info.numPag = -1;
        nuevo->info.numMarco = -1;
        //mediante esta forma vemos cuales son los que estan vacios y cuales no
        nuevo->sgte = NULL;
    
        if(ant == NULL){
            listaTlb = nuevo;
        }
        else{
            ant->sgte = nuevo;
            ant = aux;
        }
        nuevo->sgte = aux;
    }
}
