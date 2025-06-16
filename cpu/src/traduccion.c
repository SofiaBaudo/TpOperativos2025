#include <traduccion.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

int traduccion(int direccion, int pid){ //te tendria que devolver la dir fisica
    int numPag = floor(direccion/tamPag);
    int desplazamiento = direccion % tamPag; 
    int marco;
    int tlbrespuesta;
    tlbrespuesta = buscarTlb(numPag, pid);
    //actualizarContadores(numPag);
    if(tlbrespuesta == -1){
        //osea que no se encontro
        marco = navegarNiveles(numPag, pid);
        agregarEntradaATLB(numPag, marco, pid);
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
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
   for(int i = 0; i < ENTRADAS_TLB; i++){  
        if(aux->info.numPag == numPag ){
            log_info(cpu_logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, aux->info.numPag);
            log_info(cpu_logger,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", pid, aux->info.numPag, aux->info.numMarco);
            modificarReferencia(numPag,pid);
            return aux->info.numMarco;
        }
        aux = aux->sgte;
   }
    log_info(cpu_logger,"PID: <%d> - TLB MISS - Pagina: <%d>", pid, numPag);
    return -1;
}
void agregarEntradaATLB(int numPag, int numMarco, int pid){
    NodoEntradasTLB *nodoexistente = dondeEstaenTLB(numPag,pid);
    if(nodoexistente != NULL){
        nodoexistente->info.numMarco = numMarco;
        modificarReferencia(numPag,pid);
        actualizarContadores(numPag,pid);
        return;
    }
    else{
    if(strcmp(REEMPLAZO_TLB, "FIFO") == 0){
        implementarAlgoritmoFIFO(numPag, numMarco, pid);
    }
    else if(strcmp(REEMPLAZO_TLB, "LRU") == 0){
        implementarAlgoritmoLRU(numPag, numMarco, pid);
        modificarReferencia(numPag,pid);
    }
    }
}

void implementarAlgoritmoFIFO(int numPag, int numMarco, int pid){
    NodoEntradasTLB *punteroPos = buscarPunteroFIFO(pid);
    if (punteroPos == NULL){
        return; //por las dudas para que no tire segmentation fault.
    }
    punteroPos->info.numPag = numPag;
    punteroPos->info.numMarco = numMarco;
    punteroPos = punteroPos->sgte; //hacemos que avance para la proxima que le agregues una 
}

void implementarAlgoritmoLRU(int numPag, int numMarco, int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
    NodoEntradasTLB *nodoAReemplazar = aux;
   // if(estaYaEnTlb(numPag)->info.apareceEnTLB){
    // estaYaEnTlb(numPag)->info.tiempoSinReferencia = 0;
        if(hayEspacioLibre(pid)){
            nodoAReemplazar = retornarEspacioLibre(pid);
            nodoAReemplazar->info.numPag = numPag;
            nodoAReemplazar->info.numMarco = numMarco;
            nodoAReemplazar->info.tiempoSinReferencia = 0;
            nodoAReemplazar->info.apareceEnTLB = true;
        }   
        else{
        nodoAReemplazar = encontrarNodoConMenosReferencia(pid);
        nodoAReemplazar->info.numMarco = numMarco;
        nodoAReemplazar->info.numPag = numPag;
        nodoAReemplazar->info.tiempoSinReferencia = 0;
         nodoAReemplazar->info.apareceEnTLB = true;
        }
    actualizarContadores(numPag, pid);
}

//si ya fue referenciado entonces hubiera sido un tlb hit ?? no entienod porque usarias el lru, tipo funciona exactamente igual
void modificarReferencia(int numPag, int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
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

NodoEntradasTLB *inicializarTLB(int pid){
    NodoEntradasTLB *aux = NULL;
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
            aux = nuevo; //apuntamos el primer nodo
            ultimoNodo = nuevo;
        }
        else{
            ultimoNodo->sgte = nuevo;
        }
        ultimoNodo = nuevo; 
          
    }
    if(ultimoNodo != NULL){
        ultimoNodo->sgte = aux;
    }
    NodoEntradasTLB* temp = aux;
    for (int i = 0; i < ENTRADAS_TLB; i++) {
    log_debug(cpu_log_debug, "Nodo %d: %p, sgte = %p", i, temp, temp->sgte);
    temp = temp->sgte;
}
   return aux;
}
void imprimirTLB(int pid) {
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
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

NodoEntradasTLB *encontrarNodoConMenosReferencia(int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
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
bool hayEspacioLibre(int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == -1){
            return true;
        }
        aux = aux->sgte;
    }
    return false;
}

NodoEntradasTLB *retornarEspacioLibre(int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == -1){
            return aux;
        }
        aux= aux->sgte;
    }
    return NULL;
}
bool estaYaEnTlb(int numPag, int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            return true;
        }
        aux = aux->sgte;
    }
    return false;
}
NodoEntradasTLB *dondeEstaenTLB(int numPag, int pid){
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
    if(estaYaEnTlb(numPag, pid)){
    for(int i = 0; i < ENTRADAS_TLB; i++){
        if(aux->info.numPag == numPag){
            return aux;
        }
        aux = aux->sgte;
    }
    }
    return NULL;
}
void actualizarContadores(int numPag, int pid) {
    NodoEntradasTLB *aux = buscarSublistaPid(pid);
    for (int i = 0; i < ENTRADAS_TLB; i++) {
        if (aux->info.numPag != -1 && aux->info.numPag != numPag) {
            aux->info.tiempoSinReferencia++;
        }
        aux = aux->sgte;
    }
}
NodoEntradasTLB *buscarSublistaPid(int pid){
    EnlazadorTLBPID *aux = listaPids;
    while(aux!= NULL){
        if(aux->info.pid == pid){
            return aux->info.sublista;
        }
        aux= aux->sgte;
    }
    return NULL;
}
void agregarAEnlazador(int pid){
    EnlazadorTLBPID *nuevo = malloc(sizeof(EnlazadorTLBPID));
    nuevo->info.pid = pid;
    nuevo->info.sublista = inicializarTLB(pid);
    nuevo->info.punteroPos = nuevo->info.sublista;
    nuevo->sgte = NULL;
        if (listaPids == NULL) {
        listaPids = nuevo;
    } else {
        EnlazadorTLBPID *aux = listaPids;
        while (aux->sgte != NULL) {
            aux = aux->sgte;
        }
        aux->sgte = nuevo;
    }
}
NodoEntradasTLB *buscarPunteroFIFO(int pid){
    EnlazadorTLBPID *aux = listaPids;
    while(aux != NULL){
        if(aux->info.pid == pid){
            return aux->info.punteroPos;
        }
        aux = aux->sgte;
    }
    return NULL;
}
void actualizarPunteroPos(int pid){
    EnlazadorTLBPID *aux = listaPids;
    while(aux!= NULL){
        if(aux->info.pid == pid){
            aux->info.punteroPos = aux->info.punteroPos->sgte;
            return;
        }
        aux= aux->sgte;
    }
}