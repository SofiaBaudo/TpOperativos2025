#include <cache.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <traduccion.h>

void usarCache(int pid, int numPag, char *instruccion, char *contenido){
    NodosCache *aux = buscarSublistaPidCache(pid); //buscas cual es la cache que queres
    if(estaEnCache(numPag, pid)){
        for(int i = 0; i < ENTRADAS_CACHE; i++){
            if(aux->info.numPag == numPag){
                aux->info.bitdeUso = 1;
                aux->info.bitModificado = bitModificado(instruccion);
            }
        }
    }
    else{
        agregarPagCache(numPag, pid, instruccion);
    }
    
}

void agregarPagCache(int nroPag, int pid, char* instruccion){
    NodosCache *aux = buscarSublistaPidCache(pid);
    char* contenido = obtenerContenido(nroPag, pid);
    if(hayEspacioLibreCache(pid)){
        aux = retornarEspacioLibreCache(pid);
        aux->info.numPag = nroPag;
        aux->info.contenido = contenido;
        aux->info.bitdeUso = 1;
        aux->info.bitModificado = bitModificado(instruccion);
    }
    else{
        agregarConAlgoritmos(pid, instruccion, nroPag, contenido);
        log_info(cpu_logger, "PID: <%d> - Cache Add - Pagina: <%d>", pid, nroPag);
    }
}

void agregarConAlgoritmos(int pid, char *instruccion, int nroPag, char* contenido){

    if(strcmp(REEMPLAZO_CACHE, "CLOCK") == 0){
        algoritmoClock(pid, instruccion, nroPag, contenido);
    }
    else if(strcmp(REEMPLAZO_CACHE, "CLOCK-M")== 0){   
        algoritmoClockM(pid, instruccion, nroPag, contenido);
    }
}

char* obtenerContenido(int nroPag, int pid){
    t_buffer *buffer = crear_buffer_pid_numPag(pid, nroPag);
    crear_paquete(ENVIO_PID_NROPAG, buffer, fd_conexion_dispatch_memoria);
    char *contenido;
    recv(fd_conexion_dispatch_memoria, &contenido, sizeof(char*),0);
    return contenido;
}

bool estaHabilitada(int pid){
    NodosCache *aux = buscarSublistaPidCache(pid);
    if(ENTRADAS_CACHE == 0){
        return false;
    }
    if(aux == NULL){
        return false; //tiene mas de un frame
    }
    return (aux->sgte != NULL);
}

bool estaEnCache(int numPag, int pid){
    NodosCache *aux = buscarSublistaPidCache(pid);
    int contador = 0; 
    while(aux!= NULL && contador < ENTRADAS_CACHE){
        if(aux->info.numPag == numPag){
            log_info(cpu_logger, "PID: <%d> - Cache Hit - Pagina: <%d>", pid, numPag);
            return true;
        }
        aux = aux->sgte;
        contador++;
    }
    log_info(cpu_logger, "PID: <%d> - Cache Miss - Pagina: <%d>", pid, numPag);
    return false;    
}

NodosCache *buscarSublistaPidCache(int pid){
    NodosPidCache *aux = listaPidsCache;
    while(aux != NULL){
        if(aux->info.pid == pid){
            return aux->info.sublista;
        }
        aux = aux->sgte;
    }
    return NULL;
}

NodosCache *inicializarCache(int pid){
    NodosCache *aux = NULL; //porque lo estas inicializando => no esta en la lista de pids.
    NodosCache *ultimoNodo = NULL;
    for(int i = 0; i < ENTRADAS_CACHE; i++){
        NodosCache* nuevo = malloc(sizeof(NodoEntradasTLB));
        nuevo->info.bitdeUso = -1;
        nuevo->info.bitModificado =-1;
        nuevo->info.contenido = NULL;
        nuevo->info.numPag = -1;
        nuevo->sgte = NULL;

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
    return aux;
}

bool hayEspacioLibreCache(int pid){
    NodosCache *aux = buscarSublistaPidCache(pid);
    while(aux != NULL){
        if(aux->info.numPag == -1){
            return true;
        }
        aux = aux->sgte;
    }
    return false;
}

NodosCache *retornarEspacioLibreCache(int pid){
    NodosCache *aux = buscarSublistaPidCache(pid);
    while(aux != NULL){
        if(aux->info.numPag == -1){
            return aux;
        }
        aux = aux->sgte;
    }
    return NULL;    
}
void inicializarPidCache(int pid){
    NodosPidCache *nuevo = malloc(sizeof(EnlazadorTLBPID));
    nuevo->info.pid = pid;
    nuevo->info.sublista = inicializarCache(pid);
    nuevo->sgte = NULL;
    if (listaPidsCache == NULL) {
        listaPidsCache = nuevo;
    } else {
        NodosPidCache *aux = listaPidsCache;
        while (aux->sgte != NULL) {
            aux = aux->sgte;
        }
        aux->sgte = nuevo;  
    }
}
int bitModificado(char *instruccion){
    if(strcmp(instruccion, "WRITE") == 0){
        return 1;
    }
    else{
        return 0;
    }
}
void algoritmoClock(int pid, char *instruccion, int nroPag, char* contenido){
    NodosCache *aux = buscarSublistaPidCache(pid);
    while(1){
        if(aux->info.bitdeUso == 0){
            if(aux->info.bitModificado == 1){
            int marco = conseguirMarcoCache(pid, aux->info.numPag);
            t_buffer *buffer = crear_buffer_pid_numPag_contenido_marco(pid, aux->info.numPag, aux->info.contenido, marco);
            crear_paquete(ENVIO_PID_NROPAG_CONTENIDO_MARCO, buffer, fd_conexion_dispatch_memoria);
            log_info(cpu_logger,"PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pid, aux->info.numPag, marco);
            }
            aux->info.bitdeUso = 1;
            aux->info.bitModificado = bitModificado(instruccion);
            aux->info.contenido = contenido;
            aux->info.numPag = nroPag;
            break;
        }
        else{
            aux->info.bitdeUso = 0;
        }
        aux = aux->sgte;
    }   
}
void algoritmoClockM(int pid, char *instruccion, int nroPag, char*contenido){
    NodosCache *aux = buscarSublistaPidCache(pid);
    while(1){
        if(aux->info.bitdeUso == 0 && aux->info.bitModificado == 0){
            aux->info.bitdeUso =1;
            aux->info.bitModificado = bitModificado(instruccion);
            aux->info.contenido = contenido;
            aux->info.numPag = nroPag;
            break;
        }
        else if(aux->info.bitdeUso == 0 && aux->info.bitModificado == 1){
            int marco = conseguirMarcoCache(pid, aux->info.numPag);
            t_buffer *buffer = crear_buffer_pid_numPag_contenido_marco(pid, aux->info.numPag, aux->info.contenido, marco);
            crear_paquete(ENVIO_PID_NROPAG_CONTENIDO_MARCO, buffer, fd_conexion_dispatch_memoria);
            //se debe escribir esto a la memoria porque esta modificado
            log_info(cpu_logger, "PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pid, nroPag, marco);

            aux->info.bitdeUso = 1;
            aux->info.bitModificado = bitModificado(instruccion);
            aux->info.contenido = contenido;
            aux->info.numPag = nroPag;
            break;
        }
        else{
            if(aux->info.bitdeUso == 1){
                aux->info.bitdeUso = 0;
            }
        }
        aux = aux->sgte;
    }
}
void desalojarProceso(int pid){
    NodosCache *aux = buscarSublistaPidCache(pid);
    for(int i = 0; i < ENTRADAS_CACHE; i++){
        if(aux->info.bitModificado == 1){
            int marco = conseguirMarcoCache(pid, aux->info.numPag);
            t_buffer *buffer = crear_buffer_pid_numPag_contenido_marco(pid, aux->info.numPag, aux->info.contenido, marco);
            crear_paquete(ENVIO_PID_NROPAG_CONTENIDO_MARCO, buffer, fd_conexion_dispatch_memoria);
        }
        aux->info.bitdeUso =-1;
        aux->info.bitModificado = -1;
        aux->info.numPag = -1;
        aux->info.contenido = NULL;
    }
}

int conseguirMarcoCache(int pid, int nroPag){
    int tlbrespuesta = buscarTlb(nroPag, pid);
    int marco;
    if(tlbrespuesta == -1){
        marco = navegarNiveles(nroPag, pid);
        agregarEntradaATLB(nroPag, marco, pid);
    }
    else{
        marco = tlbrespuesta;
    }
    return marco;
}