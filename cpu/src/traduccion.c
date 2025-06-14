#include <traduccion.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>//para LRU 

/*
Las direcciones logicas estarian compuestas de esta manera. 
[entrada_nivel_1 | entrada_nivel_2 | … | entrada_nivel_X | desplazamiento]
¿SE PODRIA PENSAR CON ARBOLES?
EL desplazameinto dice cuantos bits avanzar hasta la direccion que queres. 
*/
//LA DIRECCION NOS LA DA EL PARAMETRO DE LAS INSTRUCCIONES
//EL TAMAÑO DE LA PAG TE LA DA EL HANDSJAKE

/*
typedef struct{
    int numPag;
    int numMarco;
    int tiempoSinReferencia;
}infotlb;

typedef struct NodoEntradasTLB{
    infotlb info;
    struct NodoEntradasTLB *sgte;
}NodoEntradasTLB;
*/

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
   for(int i = 0; i < ENTRADAS_TLB; i++){  
        if(aux->info.numPag == numPag){
            log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, aux->info.numPag);
            log_info(logger,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", pid, aux->info.numPag, aux->info.numMarco);
            modificarReferencia(numPag);
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
        modificarReferencia(numPag);
    }
}
NodoEntradasTLB *punteroPos = NULL;
void implementarAlgoritmoFIFO(int numPag, int numMarco){
    if (punteroPos == NULL){
        return; //por las dudas para que no tire segmentation fault.
    }
    punteroPos->info.numPag = numPag;
    punteroPos->info.numMarco = numMarco;
    punteroPos = punteroPos->sgte; //hacemos que avance para la proxima que le agregues una 
}

void implementarAlgoritmoLRU(int numPag, int numMarco){
    NodoEntradasTLB *nodoAReemplazar = listaTlb;
    if(hayEspacioLibre()){
            nodoAReemplazar = retornarEspacioLibre();
            nodoAReemplazar->info.numPag = numPag;
            nodoAReemplazar->info.numMarco = numMarco;
            nodoAReemplazar->info.tiempoSinReferencia = 0;
        }   
        else{
        nodoAReemplazar = encontrarNodoConMenosReferencia();
        nodoAReemplazar->info.numMarco = numMarco;
        nodoAReemplazar->info.numPag = numPag;
        nodoAReemplazar->info.tiempoSinReferencia = 0;
        }
  
}

//si ya fue referenciado entonces hubiera sido un tlb hit ?? no entienod porque usarias el lru, tipo funciona exactamente igual
void modificarReferencia(int numPag){
    NodoEntradasTLB *aux = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            aux->info.tiempoSinReferencia = 0;
        }
        else{
            aux->info.tiempoSinReferencia = aux->info.tiempoSinReferencia + 1; //es un contador
        }
        aux = aux->sgte;
    }
}


void inicializarTLB(){
    listaTlb = NULL;
    NodoEntradasTLB *ultimoNodo = NULL;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        NodoEntradasTLB* nuevo = malloc(sizeof(NodoEntradasTLB));
        nuevo->info.numPag = -1;
        nuevo->info.numMarco = -1;
        nuevo->info.tiempoSinReferencia = 0;
        //mediante esta forma vemos cuales son los que estan vacios y cuales no
    
        if(ultimoNodo == NULL){ //entonces la lista esta vacia
            listaTlb = nuevo; //apuntamos el primer nodo
            ultimoNodo = nuevo;
        }
        else{
            ultimoNodo->sgte = nuevo;
        }
        ultimoNodo = nuevo;        
    }
    if(ultimoNodo != NULL){
        ultimoNodo->sgte = listaTlb;
    }
    punteroPos = listaTlb;
}
void imprimirTLB() {
    if (listaTlb == NULL) {
        log_debug(cpu_log_debug, "TLB vacía.\n");
        return;
    }

    NodoEntradasTLB* actual = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        log_debug(cpu_log_debug, "Entrada %d: Pagina = %d, Marco = %d\n", i, actual->info.numPag, actual->info.numMarco);
        actual = actual->sgte;
    }
}

NodoEntradasTLB *encontrarNodoConMenosReferencia(){
    NodoEntradasTLB *aux = listaTlb;
    NodoEntradasTLB *nodoAReemplazar = aux;
    int valorMaxSinRef = aux->info.tiempoSinReferencia;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(valorMaxSinRef < aux->info.tiempoSinReferencia){
            valorMaxSinRef = aux->info.tiempoSinReferencia;
            nodoAReemplazar = aux;
        }
        aux = aux->sgte; 
    }
    return nodoAReemplazar;
}
bool hayEspacioLibre(){
    NodoEntradasTLB *aux = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == -1){
            return true;
        }
        aux = aux->sgte;
    }
    return false;
}

NodoEntradasTLB *retornarEspacioLibre(){
    NodoEntradasTLB *aux = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == -1){
            return aux;
        }
        aux= aux->sgte;
    }
    return NULL;
}
bool noHay(int numPag){
    NodoEntradasTLB *aux = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            return false;
        }
        aux = aux->sgte;
    }
    return true;
}