#include <cache.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <traduccion.h>

NodosCache *punteroCache = NULL;
void usarCache(int pid, int numPag, char *instruccion, char *contenido){
   
    NodosCache *aux = cache;
    
    if(estaEnCache(numPag, pid)){
        for(int i = 0; i < ENTRADAS_CACHE; i++){
            if(aux->info.numPag == numPag){
                aux->info.bitdeUso = 1;
                aux->info.bitModificado = bitModificado(instruccion);
                break;
            }
            aux = aux->sgte;
        }
    }
    else{
        agregarPagCache(numPag, pid, instruccion);
    }    
}

    void agregarPagCache(int nroPag, int pid, char* instruccion){
        NodosCache *aux = cache;
        
        char* contenido = obtenerContenido(nroPag, pid);
        
        if(hayEspacioLibreCache()){
            aux = retornarEspacioLibreCache();
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

bool estaHabilitadaCache(){
    NodosCache *aux = cache;
    if(ENTRADAS_CACHE == 0){
        return false;
    }
    if(aux == NULL){
        return false; //tiene mas de un frame
    }
    return (aux->sgte != NULL);
}

bool estaEnCache(int numPag, int pid){
    NodosCache *aux = cache;
    int contador = 0; 
     if(cache == NULL){
        log_error(cpu_logger, "cache vacia");
        return false;
    }
    while(contador < ENTRADAS_CACHE){
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

void inicializarCache(){
    NodosCache *ultimoNodo = NULL;
    for(int i = 0; i < ENTRADAS_CACHE; i++){
        NodosCache* nuevo = malloc(sizeof(NodosCache));
        nuevo->info.bitdeUso = -1;
        nuevo->info.bitModificado =-1;
        nuevo->info.contenido = NULL;
        nuevo->info.numPag = -1;
        nuevo->sgte = NULL;

        if(ultimoNodo == NULL){ //entonces la lista esta vacia
            cache = nuevo; //apuntamos el primer nodo
            ultimoNodo = nuevo;
        }
        else{
            ultimoNodo->sgte = nuevo;
        }
        ultimoNodo = nuevo;           
    }
    if(ultimoNodo != NULL){
        ultimoNodo->sgte = cache;
    }
     punteroCache = cache;
}

bool hayEspacioLibreCache(){
    NodosCache *aux = cache;
    if(cache == NULL){
        return true;
    }
    int contador;
    while(contador < ENTRADAS_CACHE){
        if(aux->info.numPag == -1){
            return true;
        }
        aux = aux->sgte;
        contador++;
    }
    return false;
}

NodosCache *retornarEspacioLibreCache(){
    NodosCache *aux = cache;
    int contador = 0;
    while(contador < ENTRADAS_CACHE){
        if(aux->info.numPag == -1){
            return aux;
        }
        aux = aux->sgte;
        contador++;
    }
    return NULL;    
}
int bitModificado(char *instruccion){
      if(instruccion == NULL){
        return 0;
    }
    if(strcmp(instruccion, "WRITE") == 0){
        return 1;
    }
    else{
        return 0;
    }
}
void algoritmoClock(int pid, char *instruccion, int nroPag, char* contenido){
    while(1){
        if(punteroCache->info.bitdeUso == 0){
            if(punteroCache->info.bitModificado == 1){
            int marco = conseguirMarcoCache(pid, punteroCache->info.numPag);
            t_buffer *buffer = crear_buffer_pid_numPag_contenido_marco(pid, punteroCache->info.numPag, punteroCache->info.contenido, marco);
            crear_paquete(ENVIO_PID_NROPAG_CONTENIDO_MARCO, buffer, fd_conexion_dispatch_memoria);
            log_info(cpu_logger,"PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pid, punteroCache->info.numPag, marco);
            }
            punteroCache->info.bitdeUso = 1;
            punteroCache->info.bitModificado = bitModificado(instruccion);
            punteroCache->info.contenido = contenido;
            punteroCache->info.numPag = nroPag;
            break;
        }
        else{
            punteroCache->info.bitdeUso = 0;
        }
        punteroCache = punteroCache->sgte;
    }  
    punteroCache = punteroCache->sgte; 
}
void algoritmoClockM(int pid, char *instruccion, int nroPag, char*contenido){
    while(1){
        if(punteroCache->info.bitdeUso == 0 && punteroCache->info.bitModificado == 0){
            punteroCache->info.bitdeUso =1;
            punteroCache->info.bitModificado = bitModificado(instruccion);
            punteroCache->info.contenido = contenido;
            punteroCache->info.numPag = nroPag;
            break;
        }
        else if(punteroCache->info.bitdeUso == 0 && punteroCache->info.bitModificado == 1){
            int marco = conseguirMarcoCache(pid, punteroCache->info.numPag);
            t_buffer *buffer = crear_buffer_pid_numPag_contenido_marco(pid, punteroCache->info.numPag, punteroCache->info.contenido, marco);
            crear_paquete(ENVIO_PID_NROPAG_CONTENIDO_MARCO, buffer, fd_conexion_dispatch_memoria);
            //se debe escribir esto a la memoria porque esta modificado
            log_info(cpu_logger, "PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", pid, nroPag, marco);

            punteroCache->info.bitdeUso = 1;
            punteroCache->info.bitModificado = bitModificado(instruccion);
            punteroCache->info.contenido = contenido;
            punteroCache->info.numPag = nroPag;
            break;
        }
        else{
            if(punteroCache->info.bitdeUso == 1){
                punteroCache->info.bitdeUso = 0;
            }
        }
    }
    punteroCache = punteroCache->sgte;
}
void desalojarProceso(int pid){
    NodosCache *aux = cache;
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
        agregarEntradaATLB(nroPag, marco);
    }
    else{
        marco = tlbrespuesta;
    }
    return marco;
}

void imprimirCache() {
    NodosCache *aux = cache;
    log_info(cpu_logger, "Estado actual de la caché:");
    for (int i = 0; i < ENTRADAS_CACHE; i++) {
        log_info(cpu_logger, "Frame %d: Pag %d, Uso %d, Mod %d",
                 i, aux->info.numPag, aux->info.bitdeUso, aux->info.bitModificado);
        aux = aux->sgte;
    }
}

