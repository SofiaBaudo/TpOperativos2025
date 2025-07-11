#include <SWAP.h>

FILE* swapfile = NULL;
t_list* paginas_en_swap = NULL;
t_list* lista_procesos = NULL;

void inicializar_swap(){
    swapfile = fopen(memoria_config.PATH_SWAPFILE, "rb+");
    if (!swapfile) {
        swapfile = fopen(memoria_config.PATH_SWAPFILE, "wb+");
    }
    if (!swapfile) {
        log_error(logger_memoria, "No se pudo abrir o crear el archivo swapfile.bin en: %s", memoria_config.PATH_SWAPFILE);
        exit(EXIT_FAILURE);
    }
    paginas_en_swap = list_create();
    log_info(logger_memoria, "SWAP inicializado correctamente en: %s", memoria_config.PATH_SWAPFILE);
}

void inicializar_lista_procesos() {
    if (lista_procesos == NULL) {
        lista_procesos = list_create();
    }
}
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido, int tamanio_proceso, int cliente){
    // Aplicar retardo de SWAP
    usleep(memoria_config.RETARDO_SWAP * 1000);
    
    // Obtener posición actual para el offset
    fseek(swapfile, 0, SEEK_END);
    int offset = ftell(swapfile);
    
    // Escribir el contenido de la página al archivo
    size_t bytes_escritos = fwrite(contenido, 1, memoria_config.TAM_PAGINA, swapfile);
    if (bytes_escritos != memoria_config.TAM_PAGINA) {
        log_error(logger_memoria, "Error al escribir página en SWAP para PID %d", pid);
        if (cliente != -1) {
            enviar_op_code(cliente, RECHAZO_PROCESO);
        }
        return;
    }
    fflush(swapfile);
    
    // Crear entrada para seguimeitno de la página en SWAP
    t_pagina_en_swap* entrada = malloc(sizeof(t_pagina_en_swap));
    entrada->pid = pid;
    entrada->nro_pagina = nro_pagina;
    entrada->offset_en_archivo = offset;
    list_add(paginas_en_swap, entrada);
    
    // Incrementar métrica de bajadas a SWAP
    incrementar_metrica_proceso(pid, BAJADAS_SWAP);
    
    log_debug(logger_memoria, "## PID: <%d> - Página <%d> guardada en SWAP en offset <%d>", pid, nro_pagina, offset);
}
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso, int cliente) {
    // Aplicar retardo de SWAP
    usleep(memoria_config.RETARDO_SWAP * 1000);
    
    // Buscar la página en la lista de páginas en SWAP
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid && entrada->nro_pagina == nro_pagina) {
            // Asignar buffer para leer la página
            void* buffer = malloc(memoria_config.TAM_PAGINA);
            if (!buffer) {
                log_error(logger_memoria, "Error al asignar memoria para leer página de SWAP");
                if (cliente != -1) {
                    enviar_op_code(cliente, RECHAZO_PROCESO);
                }
                return NULL;
            }
            
            // Leer la página desde el archivo SWAP
            fseek(swapfile, entrada->offset_en_archivo, SEEK_SET);
            size_t bytes_leidos = fread(buffer, 1, memoria_config.TAM_PAGINA, swapfile);
            
            if (bytes_leidos != memoria_config.TAM_PAGINA) {
                log_error(logger_memoria, "Error al leer página de SWAP para PID %d", pid);
                free(buffer);
                if (cliente != -1) {
                    enviar_op_code(cliente, RECHAZO_PROCESO);
                }
                return NULL;
            }
            
            // Incrementar métrica de subidas a memoria principal
            incrementar_metrica_proceso(pid, SUBIDAS_MEMORIA);
            
            // Remover la entrada de la lista de SWAP (ya no está en SWAP)
            list_remove(paginas_en_swap, i);
            free(entrada);
            
            log_debug(logger_memoria, "## PID: <%d> - Página <%d> cargada desde SWAP", pid, nro_pagina);
            return buffer;
        }
    }
    
    // No se encontró la página en SWAP
    log_warning(logger_memoria, "Página %d del proceso PID %d no encontrada en SWAP", nro_pagina, pid);
    if (cliente != -1) {
        enviar_op_code(cliente, RECHAZO_PROCESO);
    }
    return NULL;
}
//Función para eliminar páginas de SWAP
void eliminar_paginas_de_proceso(int pid){
    int paginas_eliminadas = 0;
    
    // Recorrer la lista desde el final para evitar problemas con los índices
    for (int i = list_size(paginas_en_swap) - 1; i >= 0; i--) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid) {
            // Remover la entrada de la lista y liberar memoria
            list_remove(paginas_en_swap, i);
            free(entrada);
            paginas_eliminadas++;
        }
    }
    
    if (paginas_eliminadas > 0) {
        log_info(logger_memoria, "## PID: <%d> - Se eliminaron <%d> páginas de SWAP", pid, paginas_eliminadas);
    } else {
        log_debug(logger_memoria, "No se encontraron páginas en SWAP para el proceso PID %d", pid);
    }
}

//Función para cerrar SWAP
void cerrar_swap(){
    if (swapfile != NULL) {
        fclose(swapfile);
        swapfile = NULL;
        log_debug(logger_memoria, "Archivo SWAP cerrado correctamente");
    }
    
    if (paginas_en_swap != NULL) {
        list_destroy_and_destroy_elements(paginas_en_swap, free);
        paginas_en_swap = NULL;
        log_debug(logger_memoria, "Lista de páginas en SWAP liberada");
    }
    
    if (lista_procesos != NULL) {
        list_destroy_and_destroy_elements(lista_procesos, free);
        lista_procesos = NULL;
        log_debug(logger_memoria, "Lista de procesos liberada");
    }
}

// Función para agregar un proceso a la lista de seguimiento
void agregar_proceso_a_lista(int pid, int tamanio, t_tabla_paginas* tabla_raiz) {
    t_tabla_proceso* proceso = malloc(sizeof(t_tabla_proceso));
    proceso->pid = pid;
    proceso->tamanio = tamanio;
    proceso->tabla_raiz = tabla_raiz;
    list_add(lista_procesos, proceso);
}

// Función para obtener tabla de proceso
t_tabla_proceso* obtener_tabla_proceso(int pid) {
    if (lista_procesos == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_tabla_proceso* proceso = list_get(lista_procesos, i);
        if (proceso->pid == pid) {
            return proceso;
        }
    }
    return NULL;
}

// Función para eliminar tabla de proceso
void eliminar_tabla_proceso(int pid) {
    if (lista_procesos == NULL) {
        return;
    }
    
    for (int i = 0; i < list_size(lista_procesos); i++) {
        t_tabla_proceso* proceso = list_get(lista_procesos, i);
        if (proceso->pid == pid) {
            // Liberar tabla de páginas
            if (proceso->tabla_raiz) {
                destruir_tabla_paginas_rec(proceso->tabla_raiz, 1);
            }
            // Remover de la lista y liberar memoria
            list_remove(lista_procesos, i);
            free(proceso);
            break;
        }
    }
}

// Función para obtener el número de marco de una página específica
int obtener_marco_de_pagina(t_tabla_paginas* tabla_raiz, int nro_pagina) {
    if (!tabla_raiz) {
        return -1;
    }
    
    int niveles = memoria_config.CANTIDAD_NIVELES;
    int entradas = memoria_config.ENTRADAS_POR_TABLA;
    t_tabla_paginas* actual = tabla_raiz;
    
    // Navegar hasta el último nivel
    for (int nivel = 1; nivel < niveles; nivel++) {
        int idx = (nro_pagina / (int)pow(entradas, niveles - nivel)) % entradas;
        if (idx >= actual->cantidad_entradas || !actual->entradas[idx].tabla_nivel_inferior) {
            return -1;
        }
        actual = actual->entradas[idx].tabla_nivel_inferior;
    }
    
    // Obtener el índice final y el marco
    int idx_final = nro_pagina % entradas;
    if (idx_final >= actual->cantidad_entradas) {
        return -1;
    }
    
    return actual->entradas[idx_final].nro_marco;
}

// Función para recorrer todas las páginas de un proceso
void recorrer_paginas_proceso(t_tabla_paginas* tabla_raiz, int nivel, int offset_pagina, void (*callback)(int, int, void*), void* contexto) {
    if (!tabla_raiz) {
        return;
    }
    
    if (nivel == memoria_config.CANTIDAD_NIVELES) {
        // Estamos en el último nivel, procesar las páginas
        for (int i = 0; i < tabla_raiz->cantidad_entradas; i++) {
            int nro_pagina = offset_pagina + i;
            int nro_marco = tabla_raiz->entradas[i].nro_marco;
            if (nro_marco != -1) {
                callback(nro_pagina, nro_marco, contexto);
            }
        }
        return;
    }
    
    // Recursivamente procesar los subárboles
    int entradas = memoria_config.ENTRADAS_POR_TABLA;
    int paginas_por_subarbol = pow(entradas, memoria_config.CANTIDAD_NIVELES - nivel);
    
    for (int i = 0; i < tabla_raiz->cantidad_entradas; i++) {
        if (tabla_raiz->entradas[i].tabla_nivel_inferior) {
            int nuevo_offset = offset_pagina + (i * paginas_por_subarbol);
            recorrer_paginas_proceso(tabla_raiz->entradas[i].tabla_nivel_inferior, 
                                   nivel + 1, nuevo_offset, callback, contexto);
        }
    }
}

// Callback para escribir páginas a SWAP durante suspensión
void escribir_pagina_a_swap_callback(int nro_pagina, int nro_marco, void* contexto) {
    t_contexto_suspension* ctx = (t_contexto_suspension*)contexto;
    
    if (ctx->error_encontrado) {
        return; // Ya hubo un error, no continuar
    }
    
    // Obtener la dirección física de la página
    void* contenido_pagina = marcos[nro_marco].direccion_fisica;
    
    // Escribir la página a SWAP
    escribir_pagina_en_swap(ctx->pid, nro_pagina, contenido_pagina, 0, ctx->cliente);
    
    // Liberar el marco
    marcos[nro_marco].ocupado = false;
    marcos[nro_marco].pid_propietario = -1;
}

// Función para suspender un proceso (llamada desde el servidor)
void suspender_proceso_desde_kernel(int pid, int cliente) {
    log_debug(logger_memoria, "## PID: <%d> - Iniciando suspensión del proceso", pid);
    
    // Obtener la tabla de páginas del proceso
    t_tabla_proceso* proceso = obtener_tabla_proceso(pid);
    if (!proceso) {
        log_warning(logger_memoria, "No se encontró el proceso PID %d para suspender", pid);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    
    // Configurar contexto para escribir páginas a SWAP
    t_contexto_suspension contexto;
    contexto.pid = pid;
    contexto.cliente = cliente;
    contexto.error_encontrado = false;
    
    // Recorrer todas las páginas del proceso y escribirlas a SWAP
    recorrer_paginas_proceso(proceso->tabla_raiz, 1, 0, escribir_pagina_a_swap_callback, &contexto);
    
    if (contexto.error_encontrado) {
        log_error(logger_memoria, "Error durante la suspensión del proceso PID %d", pid);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    
    // Eliminar las tablas de páginas del proceso (liberando memoria)
    eliminar_tabla_proceso(pid);
    
    log_debug(logger_memoria, "## PID: <%d> - Proceso suspendido exitosamente", pid);
    enviar_op_code(cliente, SUSPENSION_CONFIRMADA);
}

// Función para reanudar un proceso suspendido desde SWAP
void reanudar_proceso_desde_kernel(int pid, int tamanio, int cliente) {
    log_info(logger_memoria, "## PID: <%d> - Iniciando reanudación del proceso desde SWAP", pid);
    
    // Verificar si el proceso tiene páginas en SWAP
    bool tiene_paginas_en_swap = false;
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid) {
            tiene_paginas_en_swap = true;
            break;
        }
    }
    
    if (!tiene_paginas_en_swap) {
        log_warning(logger_memoria, "No se encontraron páginas en SWAP para el proceso PID %d", pid);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    
    // Calcular cuántas páginas necesita el proceso
    int paginas_necesarias = (tamanio + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;
    
    // Verificar si hay espacio suficiente en memoria principal
    int marcos_libres = contar_marcos_libres();
    if (marcos_libres < paginas_necesarias) {
        log_warning(logger_memoria, "No hay espacio suficiente en memoria para reanudar PID %d (necesita %d marcos, hay %d libres)", 
                    pid, paginas_necesarias, marcos_libres);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    
    // Recrear la estructura de tablas de páginas
    t_tabla_paginas* tabla_raiz = iniciar_proceso_paginacion(pid, tamanio);
    if (!tabla_raiz) {
        log_error(logger_memoria, "Error al recrear tablas de páginas para PID %d", pid);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    
    // Agregar el proceso a la lista de seguimiento
    agregar_proceso_a_lista(pid, tamanio, tabla_raiz);
    
    // Cargar todas las páginas desde SWAP
    bool error_carga = false;
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid) {
            // Obtener marco para esta página
            int nro_marco = obtener_marco_de_pagina(tabla_raiz, entrada->nro_pagina);
            if (nro_marco == -1) {
                log_error(logger_memoria, "No se pudo obtener marco para página %d del proceso %d", entrada->nro_pagina, pid);
                error_carga = true;
                break;
            }
            
            // Leer página desde SWAP
            fseek(swapfile, entrada->offset_en_archivo, SEEK_SET);
            void* direccion_fisica = marcos[nro_marco].direccion_fisica;
            size_t bytes_leidos = fread(direccion_fisica, 1, memoria_config.TAM_PAGINA, swapfile);
            
            if (bytes_leidos != memoria_config.TAM_PAGINA) {
                log_error(logger_memoria, "Error al leer página %d del proceso %d desde SWAP", entrada->nro_pagina, pid);
                error_carga = true;
                break;
            }
            
            // Marcar el marco como ocupado
            marcos[nro_marco].ocupado = true;
            marcos[nro_marco].pid_propietario = pid;
            
            // Incrementar métrica de subidas a memoria principal
            incrementar_metrica_proceso(pid, SUBIDAS_MEMORIA);
            
            log_debug(logger_memoria, "## PID: <%d> - Página <%d> cargada desde SWAP a marco <%d>", pid, entrada->nro_pagina, nro_marco);
        }
    }
    
    if (error_carga) {
        // Limpiar estructuras creadas en caso de error
        eliminar_tabla_proceso(pid);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    
    // Eliminar las páginas del proceso de la lista de SWAP
    eliminar_paginas_de_proceso(pid);
    
    log_info(logger_memoria, "## PID: <%d> - Proceso reanudado exitosamente desde SWAP", pid);
    enviar_op_code(cliente, ACEPTAR_PROCESO);
}

// Función para verificar si un proceso tiene páginas en SWAP
bool proceso_tiene_paginas_en_swap(int pid) {
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid) {
            return true;
        }
    }
    return false;
}