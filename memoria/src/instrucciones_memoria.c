#include <instrucciones_memoria.h>

t_list *Lista_de_pcs;
t_list *Lista_de_pids;

void iniciar_lista_pids(){
    Lista_de_pids = list_create();
}
char* obtener_instruccion(int pid, int pc){
    infoPid *info = buscar_info_pid(pid); //me devuelve la info del nodo que tiene el mismo pid que el que buscamos. La info seria --> pid, el path y el puntero a la sublista
    //buscar pc dentro de la sublista
    for(int i = 0; i < list_size(info->sublista); i++){
        pc_con_instruccion *instruccion = list_get(info->sublista, i); //esto se encarga de ir moviendo los punteros
        if(instruccion->pc == pc){
            return instruccion->instruccion_pc;
        }
    }
    return NULL;
}
infoPid* buscar_info_pid(int pid) { //aca tuvimos que pasar todo a t_list porque list_create() crea la lista pero en formato t_list*
    for (int i = 0; i < list_size(Lista_de_pids); i++) { //recorro hasta el final de la lista
        infoPid* info = list_get(Lista_de_pids, i);//agarro el nodo en la posicion i list_get
        if (info->pid == pid) { //si el pid del nodo que agarramos es el mismo que el del parametro que retorne
            return info;
        }
    }
    return NULL; //si llego aca es porque nunca entro al if => no esta.
}
void enlazar_pid_nombre_archivo (int pid, char* PATH_INSTRUCCIONES){
    infoPid *nuevo = malloc(sizeof(infoPid));
   
    nuevo->pid = pid;
    nuevo->PATH_INSTRUCCIONES = PATH_INSTRUCCIONES;
     
    nuevo->sublista = list_create();
    list_add(Lista_de_pids, nuevo);
    cargar_instrucciones_desde_archivo(pid, nuevo->PATH_INSTRUCCIONES);
}
void cargar_instrucciones_desde_archivo(int pid, char* PATH_INSTRUCCIONES){
    FILE* archivo = fopen(PATH_INSTRUCCIONES, "r");
    if (!archivo) {
        perror("No se pudo abrir el archivo de pseudocodigo");
        return;
    }
    infoPid *infoPid = buscar_info_pid(pid);
    char* faux = NULL;
    size_t len = 0;
    int contador_pcs = 1; //si es que el pc empieza desde 1
    while(getline(&faux, &len, archivo) != -1){
        pc_con_instruccion *nuevo = malloc(sizeof(pc_con_instruccion)); //crea primer nodo
        nuevo->pc = contador_pcs; //le estoy ponienndo el numero de pc
        nuevo->instruccion_pc = strdup(faux);
        contador_pcs++;
        list_add(infoPid->sublista, nuevo);
    }
    free(faux);
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
