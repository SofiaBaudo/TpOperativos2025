#include <SWAP.h>


FILE* swapfile = NULL;
t_list* paginas_en_swap = NULL;

int tamanio_total;
int tamanio_disponible_en_memoria;

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
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido, int tamanio_proceso, int cliente){
    // Obtener posición actual para el offset
    fseek(swapfile, 0, SEEK_END);
    int offset = ftell(swapfile);
    
    // Escribir el contenido de la página al archivo
    size_t bytes_escritos = fwrite(contenido, 1, memoria_config.TAM_PAGINA, swapfile);
    if (bytes_escritos != memoria_config.TAM_PAGINA) {
        log_error(logger_memoria, "Error al escribir página en SWAP para PID %d", pid);
        enviar_op_code(cliente, RECHAZO_PROCESO);
        return;
    }
    fflush(swapfile);
    
    // Crear entrada para tracking de la página en SWAP
    t_pagina_en_swap* entrada = malloc(sizeof(t_pagina_en_swap));
    entrada->pid = pid;
    entrada->nro_pagina = nro_pagina;
    entrada->offset_en_archivo = offset;
    list_add(paginas_en_swap, entrada);
    
    // Incrementar métrica de bajadas a SWAP
    incrementar_metrica_proceso(pid, BAJADAS_SWAP);
    
    // Actualizar memoria disponible
    tamanio_disponible_en_memoria += memoria_config.TAM_PAGINA;
    
    log_debug(logger_memoria, "## PID: <%d> - Página <%d> guardada en SWAP en offset <%d>", pid, nro_pagina, offset);
    enviar_op_code(cliente, SUSPENSION_CONFIRMADA);
}
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso, int cliente) {
    // Verificar si hay espacio suficiente en memoria para cargar la página
    if (tamanio_disponible_en_memoria < memoria_config.TAM_PAGINA) {
        log_warning(logger_memoria, "No hay espacio suficiente en memoria para PID %d", pid);
        enviar_op_code(cliente, NO_HAY_ESPACIO_EN_MEMORIA);
        return NULL;
    }
    
    // Buscar la página en la lista de páginas en SWAP
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid && entrada->nro_pagina == nro_pagina) {
            // Asignar buffer para leer la página
            void* buffer = malloc(memoria_config.TAM_PAGINA);
            if (!buffer) {
                log_error(logger_memoria, "Error al asignar memoria para leer página de SWAP");
                enviar_op_code(cliente, RECHAZO_PROCESO);
                return NULL;
            }
            
            // Leer la página desde el archivo SWAP
            fseek(swapfile, entrada->offset_en_archivo, SEEK_SET);
            size_t bytes_leidos = fread(buffer, 1, memoria_config.TAM_PAGINA, swapfile);
            
            if (bytes_leidos != memoria_config.TAM_PAGINA) {
                log_error(logger_memoria, "Error al leer página de SWAP para PID %d", pid);
                free(buffer);
                enviar_op_code(cliente, RECHAZO_PROCESO);
                return NULL;
            }
            
            // Incrementar métrica de subidas a memoria principal
            incrementar_metrica_proceso(pid, SUBIDAS_MEMORIA);
            
            // Actualizar memoria disponible
            tamanio_disponible_en_memoria -= memoria_config.TAM_PAGINA;
            
            // Remover la entrada de la lista de SWAP (ya no está en SWAP)
            list_remove(paginas_en_swap, i);
            free(entrada);
            
            log_info(logger_memoria, "## PID: <%d> - Página <%d> cargada desde SWAP a memoria principal", pid, nro_pagina);
            enviar_op_code(cliente, ACEPTAR_PROCESO);
            return buffer;
        }
    }
    
    // No se encontró la página en SWAP
    log_warning(logger_memoria, "Página %d del proceso PID %d no encontrada en SWAP", nro_pagina, pid);
    enviar_op_code(cliente, RECHAZO_PROCESO);
    return NULL;
}
//Funcion Eliminar Pagina de SWAP
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
//Funcion Cerrar SWAP
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
}

// Función auxiliar para suspender un proceso completo
void suspender_proceso_completo(int pid) {
    log_debug(logger_memoria, "## PID: <%d> - Iniciando suspensión completa del proceso", pid);
    
    // Obtener la tabla de páginas del proceso
    t_tabla_proceso* proceso = obtener_tabla_proceso(pid);
    if (!proceso) {
        log_warning(logger_memoria, "No se encontró el proceso PID %d para suspender", pid);
        return;
    }
    
    // Configurar contexto para escribir páginas a SWAP
    t_contexto_suspension contexto;
    contexto.pid = pid;
    contexto.cliente = -1; // No hay cliente específico para suspensiones internas
    contexto.error_encontrado = false;
    
    // Recorrer todas las páginas del proceso y escribirlas a SWAP
    recorrer_paginas_proceso(proceso->tabla_raiz, 1, 0, escribir_pagina_a_swap_callback, &contexto);
    
    if (contexto.error_encontrado) {
        log_error(logger_memoria, "Error durante la suspensión del proceso PID %d", pid);
        return;
    }
    
    // Eliminar las tablas de páginas del proceso (liberando memoria)
    eliminar_tabla_proceso(pid);
    
    log_info(logger_memoria, "## PID: <%d> - Proceso suspendido exitosamente", pid);
}

// Función auxiliar para reanudar un proceso desde SWAP
bool reanudar_proceso_desde_swap(int pid, int tam_proceso) {
    log_debug(logger_memoria, "## PID: <%d> - Iniciando reanudación del proceso desde SWAP", pid);
    
    // Verificar si el proceso tiene páginas en SWAP
    t_list* paginas_proceso = list_create();
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_en_swap, i);
        if (entrada->pid == pid) {
            list_add(paginas_proceso, entrada);
        }
    }
    
    if (list_is_empty(paginas_proceso)) {
        log_warning(logger_memoria, "No se encontraron páginas en SWAP para el proceso PID %d", pid);
        list_destroy(paginas_proceso);
        return false;
    }
    
    // Calcular cuántas páginas necesita el proceso
    int paginas_necesarias = (tam_proceso + memoria_config.TAM_PAGINA - 1) / memoria_config.TAM_PAGINA;
    
    // Verificar si hay espacio suficiente en memoria principal
    int marcos_libres = 0;
    for (int i = 0; i < cantidad_marcos; i++) {
        if (!marcos[i].ocupado) {
            marcos_libres++;
        }
    }
    
    if (marcos_libres < paginas_necesarias) {
        log_warning(logger_memoria, "No hay espacio suficiente en memoria para reanudar PID %d (necesita %d marcos, hay %d libres)", 
                    pid, paginas_necesarias, marcos_libres);
        list_destroy(paginas_proceso);
        return false;
    }
    
    // Recrear la estructura de tablas de páginas
    if (!iniciar_proceso_paginacion(pid, tam_proceso)) {
        log_error(logger_memoria, "Error al recrear tablas de páginas para PID %d", pid);
        list_destroy(paginas_proceso);
        return false;
    }
    
    // Obtener la tabla de páginas recién creada
    t_tabla_proceso* proceso = obtener_tabla_proceso(pid);
    if (!proceso) {
        log_error(logger_memoria, "Error al obtener tabla de páginas para PID %d", pid);
        list_destroy(paginas_proceso);
        return false;
    }
    
    // Cargar cada página desde SWAP a memoria principal
    for (int i = 0; i < list_size(paginas_proceso); i++) {
        t_pagina_en_swap* entrada = list_get(paginas_proceso, i);
        
        // Obtener marco para esta página
        int nro_marco = obtener_marco_de_pagina(proceso->tabla_raiz, entrada->nro_pagina);
        if (nro_marco == -1) {
            log_error(logger_memoria, "No se pudo obtener marco para página %d del proceso %d", 
                      entrada->nro_pagina, pid);
            eliminar_tabla_proceso(pid);
            list_destroy(paginas_proceso);
            return false;
        }
        
        // Leer página desde SWAP
        fseek(swapfile, entrada->offset_en_archivo, SEEK_SET);
        void* direccion_fisica = marcos[nro_marco].direccion_fisica;
        size_t bytes_leidos = fread(direccion_fisica, 1, memoria_config.TAM_PAGINA, swapfile);
        
        if (bytes_leidos != memoria_config.TAM_PAGINA) {
            log_error(logger_memoria, "Error al leer página %d del proceso %d desde SWAP", 
                      entrada->nro_pagina, pid);
            eliminar_tabla_proceso(pid);
            list_destroy(paginas_proceso);
            return false;
        }
        
        // Marcar el marco como ocupado
        marcos[nro_marco].ocupado = true;
        marcos[nro_marco].pid_propietario = pid;
        
        // Incrementar métrica de subidas a memoria principal
        incrementar_metrica_proceso(pid, SUBIDAS_MEMORIA);
        
        log_debug(logger_memoria, "## PID: <%d> - Página <%d> cargada desde SWAP a marco <%d>", 
                  pid, entrada->nro_pagina, nro_marco);
    }
    
    // Actualizar memoria disponible
    tamanio_disponible_en_memoria -= (list_size(paginas_proceso) * memoria_config.TAM_PAGINA);
    
    // Eliminar las páginas del proceso de la lista de SWAP
    eliminar_paginas_de_proceso(pid);
    
    list_destroy(paginas_proceso);
    
    log_info(logger_memoria, "## PID: <%d> - Proceso reanudado exitosamente desde SWAP", pid);
    return true;
}