//Listado de metricas por proceso
#include <metricas.h>

t_list* lista_metricas_procesos;     // Lista que contiene las métricas de cada proceso

void inicializar_metricas_procesos() {
    lista_metricas_procesos = list_create();
}

void crear_metricas_proceso(int pid) {
    t_metricas* nuevas_metricas = malloc(sizeof(t_metricas));
    nuevas_metricas->pid = pid;
    nuevas_metricas->cant_acceso_tabla_pagina = 0;
    nuevas_metricas->instrucciones_solicitadas = 0;
    nuevas_metricas->bajadas_swap = 0;
    nuevas_metricas->cant_subidas_memoria_principal = 0;
    nuevas_metricas->cant_lecturas_memoria = 0;
    nuevas_metricas->cant_escrituras_memoria = 0;
    
    pthread_mutex_lock(&mutex_lista_metricas_procesos);
    list_add(lista_metricas_procesos, nuevas_metricas);
    pthread_mutex_unlock(&mutex_lista_metricas_procesos);
}

// Función para buscar las métricas de un proceso específico -> no se si tiene ucho sentido
t_metricas* buscar_metricas_proceso(int pid) {
    pthread_mutex_lock(&mutex_lista_metricas_procesos);
    t_metricas* resultado = NULL;
    for (int i = 0; i < list_size(lista_metricas_procesos); i++) {
        t_metricas* metricas = list_get(lista_metricas_procesos, i);
        if (metricas->pid == pid) {
            resultado = metricas;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_metricas_procesos);
    return resultado;
}

// Función para incrementar una métrica específica de un proceso
void incrementar_metrica_proceso(int pid, tipo_metrica metrica) {
    pthread_mutex_lock(&mutex_lista_metricas_procesos);
    t_metricas* metricas_proceso = NULL;
    
    // Buscar métricas dentro del mutex
    for (int i = 0; i < list_size(lista_metricas_procesos); i++) {
        t_metricas* metricas = list_get(lista_metricas_procesos, i);
        if (metricas->pid == pid) {
            metricas_proceso = metricas;
            break;
        }
    }
    
    if (metricas_proceso == NULL) {
        pthread_mutex_unlock(&mutex_lista_metricas_procesos);
        log_error(logger_memoria, "No se encontraron métricas para el proceso PID: %d", pid);
        return;
    }
    
    switch(metrica) {
        case ACCESO_TABLA:
            metricas_proceso->cant_acceso_tabla_pagina++;
            break;
        case INSTRUCCIONES_SOLICITADAS:
            metricas_proceso->instrucciones_solicitadas++;
            break;
        case BAJADAS_SWAP:
            metricas_proceso->bajadas_swap++;
            break;
        case SUBIDAS_MEMORIA:
            metricas_proceso->cant_subidas_memoria_principal++;
            break;
        case LECTURAS_MEMORIA:
            metricas_proceso->cant_lecturas_memoria++;
            break;
        case ESCRITURAS_MEMORIA:
            metricas_proceso->cant_escrituras_memoria++;
            break;
        default:
            log_warning(logger_memoria, "Tipo de métrica desconocido: %d", metrica);
            break;
    }
    pthread_mutex_unlock(&mutex_lista_metricas_procesos);
}

// Función para mostrar las métricas de un proceso (log obligatorio)
void mostrar_metricas_proceso(int pid) {
    t_metricas* metricas = buscar_metricas_proceso(pid);
    
    if (metricas == NULL) {
        log_error(logger_memoria, "No se encontraron métricas para el proceso PID: %d", pid);
        return;
    }
    
    //destruyo el proceso con las metricas actualizadas
    log_info(logger_memoria, "## PID: <%d> - Proceso Destruido - Metricas - Acc.T.Pag: <%d>; Inst.Sol.: <%d>; SWAP: <%d>; Mem.Prin.: <%d>; Lec.Mem.: <%d>; Esc.Mem.: <%d>",
        metricas->pid,
        metricas->cant_acceso_tabla_pagina,
        metricas->instrucciones_solicitadas,
        metricas->bajadas_swap,
        metricas->cant_subidas_memoria_principal,
        metricas->cant_lecturas_memoria,
        metricas->cant_escrituras_memoria
    );
}

// Función para destruir las métricas de un proceso
void destruir_metricas_proceso(int pid) {
    pthread_mutex_lock(&mutex_lista_metricas_procesos);
    for (int i = 0; i < list_size(lista_metricas_procesos); i++) {
        t_metricas* metricas = list_get(lista_metricas_procesos, i);
        if (metricas->pid == pid) {
            // Mostrar métricas antes de destruir
            log_info(logger_memoria, "## PID: <%d> - Proceso Destruido - Metricas - Acc.T.Pag: <%d>; Inst.Sol.: <%d>; SWAP: <%d>; Mem.Prin.: <%d>; Lec.Mem.: <%d>; Esc.Mem.: <%d>",
                metricas->pid,
                metricas->cant_acceso_tabla_pagina,
                metricas->instrucciones_solicitadas,
                metricas->bajadas_swap,
                metricas->cant_subidas_memoria_principal,
                metricas->cant_lecturas_memoria,
                metricas->cant_escrituras_memoria
            );
            
            // Remover de la lista y liberar memoria
            list_remove(lista_metricas_procesos, i);
            free(metricas);
            pthread_mutex_unlock(&mutex_lista_metricas_procesos);
            return;
        }
    }
    pthread_mutex_unlock(&mutex_lista_metricas_procesos);
    log_warning(logger_memoria, "No se encontraron métricas para destruir del proceso PID: %d", pid);
}

// Función auxiliar para actualizar métricas de acceso a tabla de páginas
void actualizar_metricas_acceso_tabla_paginas(int pid) {
    incrementar_metrica_proceso(pid, ACCESO_TABLA);
}