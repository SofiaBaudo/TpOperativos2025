#include "procesos.h"

bool inicializar_proceso(t_proceso_paquete* proceso_paquete) {
    int pid = proceso_paquete->pid;
    int tam_proceso = proceso_paquete->tamanio;
    char* path_pseudocodigo = proceso_paquete->path_pseudocodigo;
    // 1. Validar que el tamaño del proceso no sea mayor que la memoria total
    if (tam_proceso > memoria_config.TAM_MEMORIA) {
        log_warning(logger_memoria, "El proceso %d solicita %d bytes, mayor que la memoria total (%d)", pid, tam_proceso, memoria_config.TAM_MEMORIA);
        return false;
    }

    // 2. Calcular cuántos marcos necesita
    int marcos_necesarios = (tam_proceso + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;

    // 3. Contar marcos libres usando función de memoria_fisica
    int libres = contar_marcos_libres();
    if (libres < marcos_necesarios) {
        log_warning(logger_memoria, "No hay marcos libres suficientes para el proceso %d: necesita %d, hay %d", pid, marcos_necesarios, libres);
        return false;
    }

    // 4. Si hay espacio, crear estructuras administrativas de paginación
    t_tabla_paginas* tabla_raiz = iniciar_proceso_paginacion(pid, tam_proceso);

    // 5. Generar instrucciones segun path para proceso
    t_list* instrucciones = generar_instrucciones_proceso(pid, path_pseudocodigo);

    // 6. Si todo fue exitoso, armar y agregar el proceso a la lista global
    if (tabla_raiz != NULL && instrucciones != NULL) {
        t_proceso_memoria* nuevo_proceso = malloc(sizeof(t_proceso_memoria));
        nuevo_proceso->pid = pid;
        nuevo_proceso->tamanio = tam_proceso;
        nuevo_proceso->instrucciones = instrucciones;
        nuevo_proceso->tabla_paginacion_raiz = tabla_raiz;

        pthread_mutex_lock(&mutex_procesos_en_memoria);
        list_add(procesos_en_memoria, nuevo_proceso);
        pthread_mutex_unlock(&mutex_procesos_en_memoria);

        log_info(logger_memoria, "Proceso %d inicializado correctamente con %d marcos. Path pseudocódigo: %s", pid, marcos_necesarios, path_pseudocodigo);
        return true;
    } else {
        if (tabla_raiz)
            destruir_tabla_paginas_rec(tabla_raiz, 1);
        if (instrucciones)
            list_destroy_and_destroy_elements(instrucciones, free);
        log_error(logger_memoria, "Fallo la inicialización del proceso %d", pid);
        return false;
    }
}


// Libera y elimina un proceso de la lista global por PID
bool finalizar_proceso(int pid) {
    // 1. Buscar y remover el proceso de la lista global (sección crítica)
    bool _match(void* elem) {
        t_proceso_memoria* proc = (t_proceso_memoria*)elem;
        return proc->pid == pid;
    }

    pthread_mutex_lock(&mutex_procesos_en_memoria);
    t_proceso_memoria* proc = list_remove_by_condition(procesos_en_memoria, _match);
    pthread_mutex_unlock(&mutex_procesos_en_memoria);

    // 2. Validar si se encontró el proceso
    if (!proc) {
        log_warning(logger_memoria, "No se encontró el proceso %d para finalizar", pid);
        return false;
    }

    // 3. Liberar la lista de instrucciones
    list_destroy_and_destroy_elements(proc->instrucciones, free);

    // 4. Liberar la tabla de paginación
    destruir_tabla_paginas_rec(proc->tabla_paginacion_raiz, 1);

    // 5. Liberar el struct del proceso
    free(proc);

    // 6. Loguear el resultado
    log_info(logger_memoria, "Proceso %d finalizado y recursos liberados", pid);
    return true;
}

// Devuelve el puntero al proceso en memoria con el PID dado, o NULL si no existe
t_proceso_memoria* buscar_proceso_en_memoria(int pid) {
    t_proceso_memoria* encontrado = NULL;
    pthread_mutex_lock(&mutex_procesos_en_memoria);
    for (int i = 0; i < list_size(procesos_en_memoria); i++) {
        t_proceso_memoria* proc = list_get(procesos_en_memoria, i);
        if (proc->pid == pid) {
            encontrado = proc;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_procesos_en_memoria);
    return encontrado;
}

// Dump de memoria de un proceso. Devuelve true si fue exitoso, false si hubo error
bool dump_memoria_proceso(int pid) {
    t_proceso_memoria* proceso = buscar_proceso_en_memoria(pid);
    if (!proceso) {
        log_error(logger_memoria, "DUMP_MEMORY: Proceso %d no encontrado", pid);
        return false;
    }

    int tamanio = proceso->tamanio;
    if (tamanio <= 0) {
        log_error(logger_memoria, "Dump fallido: tamaño inválido para PID %d", pid);
        return false;
    }

    void* buffer = malloc(tamanio);
    if (!buffer) {
        log_error(logger_memoria, "DUMP_MEMORY: No se pudo reservar memoria para el dump de PID %d", pid);
        return false;
    }

    pthread_mutex_lock(&memoria_usuario_mutex);
    // Copiar la memoria real del proceso (asume contiguo, adaptar si es necesario)
    memcpy(buffer, (char*)memoria_usuario + (pid * tamanio), tamanio);
    pthread_mutex_unlock(&memoria_usuario_mutex);

    // Obtener timestamp con formato YYYYMMDD%H%M%S
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", &tm_info);
    // Crear nombre del archivo: <DUMP_PATH>/<PID>-<TIMESTAMP>.dmp
    char filename[256];
    snprintf(filename, sizeof(filename), "%s%d-%s.dmp", memoria_config.DUMP_PATH, pid, timestamp);

    FILE* archivo = fopen(filename, "wb");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo crear el archivo dump: %s", filename);
        free(buffer);
        return false;
    }
    size_t written = fwrite(buffer, 1, tamanio, archivo);
    fclose(archivo);
    free(buffer);
    if (written != (size_t)tamanio) {
        log_error(logger_memoria, "DUMP_MEMORY: Error al escribir archivo %s (escribió %zu de %d bytes)", filename, written, tamanio);
        return false;
    }

    log_info(logger_memoria, "## PID: %d - Dump de memoria exitoso en %s", pid, filename);
    return true;
}