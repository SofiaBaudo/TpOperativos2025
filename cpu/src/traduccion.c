#include <traduccion.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <cache.h>


int traduccion(int direccion, int pid, char *instruccion, char *contenido){ //te tendria que devolver la dir fisica
    int numPag = floor(direccion/tamPag);
    int desplazamiento = direccion % tamPag; 
    int marco;
    if(estaHabilitadaCache()){
        usarCache(pid, numPag, instruccion,contenido);
        return -1; //no hace falta delvoler algo porque se hizo en cache
    }
    int tlbrespuesta;
    tlbrespuesta = buscarTlb(numPag, pid);
    //actualizarContadores(numPag);
    if(tlbrespuesta == -1){
        //osea que no se encontro
        marco = navegarNiveles(numPag, pid);
        agregarEntradaATLB(numPag, marco);
    }
    else{
        marco = tlbrespuesta;
        //aca no actualizas la referencia porque ya llamas modificarRefenrecia en buscarEnTLB
        //aca solo actualizo --> si en el config esta lru
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
    }
    //el numero de marco que tenemos despues del if es el num de marco final(el ultimo de todos --> es el marco fisico)
    int marcoFinal = numMarco;
    return marcoFinal;
}
int conseguirMarco(int pid){
    int numMarco;
    recv(fd_conexion_dispatch_memoria, &numMarco, sizeof(int), 0);
    log_info(cpu_logger,"PID: <%d> - OBTENER MARCO - Página: <NUMERO_PAGINA> - Marco: <%d>", pid, numMarco);
    //se envia el paquete con la entradaNivel y el pid, memoria lo deseerializa y me envia el numero de marco unicamente, entonces no hace falta hacer un paquete. 
    return numMarco;
}
int buscarTlb(int numPag, int pid){
    NodoEntradasTLB *aux = listaTlb;
   for(int i = 0; i < ENTRADAS_TLB; i++){  
        if(aux->info.numPag == numPag ){
            log_info(cpu_logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, aux->info.numPag);
            log_info(cpu_logger,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", pid, aux->info.numPag, aux->info.numMarco);
            modificarReferencia(numPag);
            return aux->info.numMarco;
        }
        aux = aux->sgte;
   }
    log_info(cpu_logger,"PID: <%d> - TLB MISS - Pagina: <%d>", pid, numPag);
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
NodoEntradasTLB *punterosPos = NULL;
void implementarAlgoritmoFIFO(int numPag, int numMarco){
    if (punterosPos == NULL){
        return; 
    }    
    punterosPos->info.numPag = numPag;
    punterosPos->info.numMarco = numMarco;
    punterosPos->info.apareceEnTLB = true;
    punterosPos->info.tiempoSinReferencia = 0;

    punterosPos = punterosPos->sgte;
}

void implementarAlgoritmoLRU(int numPag, int numMarco){
    NodoEntradasTLB *aux = listaTlb;
    NodoEntradasTLB *nodoAReemplazar = aux;
   // if(estaYaEnTlb(numPag)->info.apareceEnTLB){
    // estaYaEnTlb(numPag)->info.tiempoSinReferencia = 0;
        if(hayEspacioLibre()){
            nodoAReemplazar = retornarEspacioLibre();
            nodoAReemplazar->info.numPag = numPag;
            nodoAReemplazar->info.numMarco = numMarco;
            nodoAReemplazar->info.tiempoSinReferencia = 0;
            nodoAReemplazar->info.apareceEnTLB = true;
        }   
        else{
        nodoAReemplazar = encontrarNodoConMenosReferencia();
        nodoAReemplazar->info.numMarco = numMarco;
        nodoAReemplazar->info.numPag = numPag;
        nodoAReemplazar->info.tiempoSinReferencia = 0;
         nodoAReemplazar->info.apareceEnTLB = true;
        }
    actualizarContadores(numPag);
}

//si ya fue referenciado entonces hubiera sido un tlb hit ?? no entienod porque usarias el lru, tipo funciona exactamente igual
void modificarReferencia(int numPag){
    NodoEntradasTLB *aux = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            aux->info.tiempoSinReferencia = 0;
            aux->info.apareceEnTLB = true;
        }
        else{
            aux->info.tiempoSinReferencia++;
        }
        aux = aux->sgte;
    }
}

void inicializarTLB(){
    NodoEntradasTLB *ultimoNodo = NULL;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        NodoEntradasTLB* nuevo = malloc(sizeof(NodoEntradasTLB));
        nuevo->info.numPag = -1;
        nuevo->info.numMarco = -1;
        nuevo->info.tiempoSinReferencia = 0;
        nuevo->info.apareceEnTLB = false;
        nuevo->sgte = NULL;
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
    
    punterosPos = listaTlb;
}


void imprimirTLB(){
    NodoEntradasTLB *aux = listaTlb;
    if (aux == NULL) {
        log_debug(cpu_log_debug, "TLB vacía.\n");
        return;
    }

    NodoEntradasTLB* actual = aux;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        log_debug(cpu_log_debug, "Entrada %d: Pagina = %d, Marco = %d, Rerencia = %d\n", i, actual->info.numPag, actual->info.numMarco, actual->info.tiempoSinReferencia);
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
bool estaYaEnTlb(int numPag){
    NodoEntradasTLB *aux = listaTlb;
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            return true;
        }
        aux = aux->sgte;
    }
    return false;
}
NodoEntradasTLB *dondeEstaenTLB(int numPag){
    NodoEntradasTLB *aux = listaTlb;
    if(estaYaEnTlb(numPag)){
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            return aux;
        }
        aux = aux->sgte;
    }
    }
    return NULL;
}
void actualizarContadores(int numPag){
    NodoEntradasTLB *aux = listaTlb;
    for (int i = 0; i < ENTRADAS_TLB; i++) {
        if (aux->info.numPag != -1 && aux->info.numPag != numPag) {
            aux->info.tiempoSinReferencia++;
        }
        aux = aux->sgte;
    }
}


