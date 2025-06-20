#include <cache.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void usarCache(int pid){
    NodosCache *aux = buscarSublistaPidCache(pid); //buscas cual es la cache que queres
    
}

void agregarPagCache(int nroPag, int pid){

}

void* obtenerContenido(int nroPag, int pid){
    t_buffer *buffer = crear_buffer_pid_numPag(pid, nroPag);
    crear_paquete(ENVIO_PID_NROPAG, buffer, fd_conexion_dispatch_memoria);
    void *contenido;
    recv(fd_conexion_dispatch_memoria, &contenido, sizeof(void*),0);
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
            return true;
        }
        aux = aux->sgte;
        contador++;
    }
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

void inicializarCache(int pid){
    NodosCache *aux = buscarSublistaPidCache(pid);
    for(int i = 0; i < ENTRADAS_CACHE; i++){
        aux->info.bitdeUso = -1;
        aux->info.bitModificado =-1;
        aux->info.contenido = NULL;
        aux->info.numPag = -1;

        aux= aux->sgte;
    }
}