#include <instrucciones_memoria.h>

t_list* lista_instrucciones;
t_list* lista_procesos_instrucciones;

//Funcion Inicializar Lista de Procesos con Instrucciones
void iniciar_lista_procesos_instrucciones(){
    lista_instrucciones = list_create();
}

//Funcion que Convierte la linea de Codigo
//Convierte una linea completa del pseudocodigo, como "READ 10 20", en una estructura t_instruccion con: el enum correspondiente (por ejemplo: READ) una lista de los parámetros como strings (["10", "20"])la cantidad de parámetros (2)

t_instruccion* analizar_linea(char* linea){
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    char* linea_copia = strdup(linea);
    char* token = strtok(linea_copia, " ");
    //instr->codigo = string_a_instruccion(token);
    instr->parametros = malloc(sizeof(char*) * 10);
    instr->cantidad_parametros = 0;

    while ((token = strtok(NULL, " ")) != NULL) {
        instr->parametros[instr->cantidad_parametros++] = strdup(token);
    }
    free(linea_copia);
    return instr;
}

//Funcion que Busca la instruccion numero pc del proceso con pid.
//Devuelve el puntero a la instruccion que necesita la CPU ejecutar.

t_instruccion* obtener_instruccion(int pid, int pc){
    for (int i = 0; i < list_size(lista_procesos_instrucciones); i++) {
        t_proceso_instrucciones* p = list_get(lista_procesos_instrucciones, i);
        if (p->pid == pid) {
            return list_get(p->instrucciones, pc);
        }
    }
    return NULL;
}

//Funcion que Lee linea por linea un archivo de pseudocodigo (ej: "proceso1") y convierte cada linea en una t_instruccion, agregandolas a una lista.
//Devuelve un t_list* con todas las instrucciones del proceso, ya parseadas.

t_list* cargar_instrucciones_desde_archivo(char* PATH_INSTRUCCIONES){
    FILE* archivo = fopen(PATH_INSTRUCCIONES, "r");
    if (!archivo) {
        perror("No se pudo abrir el archivo de pseudocodigo");
        return NULL;
    }

    t_list* instrucciones = list_create();
    char* linea = NULL;
    size_t len = 0;

    while (getline(&linea, &len, archivo) != -1) {
        linea[strcspn(linea, "\n")] = 0;
        list_add(instrucciones, analizar_linea(linea));
    }

    free(linea);
    fclose(archivo);
    return instrucciones;
}

//Funcion que Guarda la lista de instrucciones de un proceso en una estructura junto con su PID, y lo agrega a una lista global (procesos_instrucciones).
//se usa cuando la Memoria crea un nuevo proceso.
//agrega el pid q necesitamos

void registrar_instrucciones_proceso(int pid, char* PATH_INSTRUCCIONES){
    t_proceso_instrucciones* nuevo = malloc(sizeof(t_proceso_instrucciones));
    nuevo->pid = pid;
    nuevo->instrucciones = cargar_instrucciones_desde_archivo(PATH_INSTRUCCIONES);
    list_add(lista_procesos_instrucciones, nuevo);
}

//Funcion que Libera toda la memoria usada por una sola t_instruccion
//cuando se quiere limpiar memoria, cuando finaliza un proceso

void destruir_instruccion(t_instruccion* instr){
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
void manejar_read_memoria(int socket_cpu) {
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int pid = deserializar_pid(paquete);
    int direccion_fisica = deserializar_entero_desde_stream(paquete);
    int tamanio = deserializar_entero_desde_stream(paquete);

    void* buffer = malloc(tamanio);
    leer_espacio_usuario(buffer, direccion_fisica, tamanio);  // ya lo tenés

    log_info(logger_memoria, "## PID: <%d> - Lectura - Dir. Física: <%d> - Tamaño: <%d>", pid, direccion_fisica, tamanio); //log oblig

    send(socket_cpu, buffer, tamanio, 0);
    free(buffer);
}
void manejar_write_memoria(int socket_cpu) {
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int pid = deserializar_pid(paquete);
    int direccion_fisica = deserializar_entero_desde_stream(paquete); // función auxiliar
    int tamanio = deserializar_entero_desde_stream(paquete);

    void* valor = malloc(tamanio);
    memcpy(valor, paquete->buffer->stream + paquete->buffer->offset, tamanio);
    escribir_espacio_usuario(direccion_fisica, valor, tamanio);

    log_info(logger_memoria, "## PID: <%d> - Escritura - Dir. Física: <%d> - Tamaño: <%d>", pid, direccion_fisica, tamanio); //log oblig

    free(valor);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}
void manejar_dump_memory(int socket_cpu, void* contenido){
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int pid = deserializar_pid(paquete);
    log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", pid); //log oblig
    int direccion_fisica = deserializar_entero_desde_stream(paquete); // función auxiliar
    int tamanio = deserializar_entero_desde_stream(paquete);

    void* valor = malloc(tamanio);
    memcpy(valor, paquete->buffer->stream + paquete->buffer->offset, tamanio);

    if (direccion_fisica < 0 || tamanio <= 0) {
        log_error(logger_memoria, "Dump fallido: dirección o tamaño inválido");
        return;
    }
    // Obtener timestamp con formato YYYYMMDDHHMMSS
    char* timestamp = temporal_get_string_time("%Y%m%d%H%M%S");
    // Crear nombre del archivo: <DUMP_PATH>/<PID>-<TIMESTAMP>.dmp
    char* nombre_archivo = string_from_format("%s/%d-%s.dmp", memoria_config.DUMP_PATH, pid, timestamp);

    // Crear y abrir archivo
    FILE* archivo = fopen(nombre_archivo, "wb");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo crear el archivo dump: %s", nombre_archivo);
        free(timestamp);
        free(nombre_archivo);
        return;
    }
    // Escribir memoria del proceso en el archivo
    //fwrite(contenido, 1, tamanio_marco, archivo);
    fclose(archivo);

    log_debug(logger_memoria, "Dump de memoria generado para PID %d en %s", pid, nombre_archivo);

    free(timestamp);
    free(nombre_archivo);
}