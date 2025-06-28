#include <instrucciones_memoria.h>

lista_de_pcs *Lista_de_pcs = NULL;
lista_de_pids *Lista_de_pids = NULL;

void iniciar_lista_pids(){
    lista_de_pids* Lista_de_pids = list_create();
}
char* obtener_instruccion(int pid, int pc){
    lista_de_pcs *aux_pc = buscar_lista_pid(pid);
    //buscar pc dentro de la sublista
    while(aux_pc!=NULL){
        if(aux_pc->info_pc.pc == pc){
            return aux_pc->info_pc.instruccion_pc;
        }
        aux_pc = aux_pc->siguiente;
    }
    return NULL;
}
lista_de_pcs *buscar_lista_pid(int pid){
    lista_de_pids *aux = Lista_de_pids;
    if(aux == NULL){
        return NULL;
    }
    while(aux!=NULL && aux->info_pid.pid != pid){
        aux = aux->siguiente;
    }
    if(aux == NULL){
        return NULL; //no fue encontrado
    }
    if(aux->info_pid.pid == pid){
        return aux->info_pid.sublista; //enlace a lista de pcs
    }
    return NULL;
}
void enlazar_pid_nombre_archivo (int pid, char* PATH_INSTRUCCIONES){
    lista_de_pids *nuevo = malloc(sizeof(lista_de_pids));
    nuevo->info_pid.pid = pid;
    nuevo->info_pid.PATH_INSTRUCCIONES = PATH_INSTRUCCIONES;
    nuevo->siguiente = NULL;
    nuevo->info_pid.sublista = list_create();
    cargar_instrucciones_desde_archivo(pid, PATH_INSTRUCCIONES);
    
    list_add(Lista_de_pids, nuevo);
}
void cargar_instrucciones_desde_archivo(int pid, char* PATH_INSTRUCCIONES){
    FILE* archivo = fopen(PATH_INSTRUCCIONES, "rb");
    if (!archivo) {
        perror("No se pudo abrir el archivo de pseudocodigo");
        return;
    }
    lista_de_pcs *pcLista = buscar_lista_pid(pid);
    lista_de_pcs *nodoAnterior = NULL;
    char* faux;
    int contador_pcs = 1; //si es que el pc empieza desde 1
    fread(&faux, sizeof(char*), 1, archivo);
    while(!feof(archivo)){
        lista_de_pcs *nuevo = malloc(sizeof(lista_de_pcs)); //crea primer nodo
        if(pcLista == NULL){ //enlazando el nodo original con el nuevo del pc
            pcLista = nuevo;
            nodoAnterior = nuevo;
        }
        else{
            nodoAnterior->siguiente = nuevo;
        }// se puede reemplazar por list_add()
        nuevo->info_pc.pc = contador_pcs; //le estoy ponienndo el numero de pc
        nuevo->info_pc.instruccion_pc = faux;
        nuevo->siguiente = NULL;
        contador_pcs++;
        fread(&faux, sizeof(char*), 1, archivo);
    }
    fclose(archivo);
}
/*
//Funcion que Libera toda la memoria usada por una sola t_instruccion
//cuando se quiere limpiar memoria, cuando finaliza un proceso

void destruir_instruccion(char* instruccion_pc){
    for (int i = 0; i < instr->cantidad_parametros; i++) {
        free(instr->parametros[i]);
    }
    free(instr->parametros);
    free(instr);
}

//Funciin que Busca el proceso con ese pid en la lista global, libera cada instruccion de su lista, libera la estructura del proceso, lo saca de la lista global procesos_instrucciones
//Se usa al destruir un proceso (cuando pasa a EXIT creemos)

void destruir_proceso_instrucciones(int pid){
    for (int i = 0; i < list_size(lista_procesos_instrucciones); i++) {
        t_proceso_instrucciones* p = list_get(lista_procesos_instrucciones, i);
        if (p->pid == pid) {
           // list_destroy_and_destroy_elements(p->instrucciones, (void*)destruir_instruccion);
            list_remove(lista_procesos_instrucciones, i);
            log_info(logger_memoria, "## PID: <%d> - Proceso Destruido - Metricas - Acc.T.Pag: <%d>; Inst.Sol.: <%d>; SWAP: <%d>; Mem.Prin.: <%d>; Lec.Mem.: <%d>; Esc.Mem.: <%d>",
            pid,
            listado_metricas.cant_acceso_tabla_pagina,
            listado_metricas.instrucciones_solicitadas,
            listado_metricas.bajadas_swap,
            listado_metricas.cant_subidas_memoria_principal,
            listado_metricas.cant_lecturas_memoria,
            listado_metricas.cant_escrituras_memoria
            );
            free(p);
            return;
        }
    }
}
    */
