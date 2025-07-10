#include <inicializar_memoria.h>

t_log* logger_memoria;
t_memoria_config memoria_config;
t_list* procesos_en_memoria; // Lista global de procesos en memoria

bool inicializar_memoria(void) {
    printf("=== INICIANDO SISTEMA DE MEMORIA ===\n");

    // Paso 1: Configuración
    if (!inicializar_configuracion()) {
        printf("Error: Configuración\n");
        return false;
    }

    // Paso 2: Logger
    if (!inicializar_logger()) {
        printf("Error: Logger\n");
        return false;
    }

    // Paso 3: Inicializar semáforos/mutex
    if (!inicializar_mutex()) {
        log_error(logger_memoria, "Error al inicializar semáforos/mutex");
        return false;
    }

    // Paso 4: Inicializar listas globales
    if (!inicializar_listas_globales()) {
        log_error(logger_memoria, "Error al inicializar listas globales");
        return false;
    }

    // Paso 4: Memoria física
    if (!inicializar_memoria_fisica()) {
        log_error(logger_memoria, "Error: Memoria física");
        return false;
    }
    //inicializo swap
    inicializar_swap(); //ver el bool q devuelve swap
    
    mostrar_resumen_sistema();
    log_debug(logger_memoria, "Sistema inicializado correctamente");

    return true;
}


// Inicializa todas las listas globales del sistema de memoria
bool inicializar_listas_globales(void) {
    procesos_en_memoria = list_create();
    inicializar_metricas_procesos();
    if (!procesos_en_memoria) {
        return false;
    }
    return true;
}

bool inicializar_configuracion() {
    t_config* config = config_create("memoria.config");
    if (config == NULL) {
        printf("Error al leer el archivo de configuración\n");
        return false; 
    }
    
    memoria_config.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    memoria_config.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    memoria_config.TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
    memoria_config.ENTRADAS_POR_TABLA = config_get_int_value(config, "ENTRADAS_POR_TABLA");
    memoria_config.CANTIDAD_NIVELES = config_get_int_value(config, "CANTIDAD_NIVELES");
    memoria_config.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    memoria_config.PATH_SWAPFILE = strdup(config_get_string_value(config, "PATH_SWAPFILE"));
    memoria_config.RETARDO_SWAP = config_get_int_value(config, "RETARDO_SWAP");
    memoria_config.DUMP_PATH = strdup(config_get_string_value(config, "DUMP_PATH"));
    
    config_destroy(config);
    printf("Configuración cargada correctamente\n");
    return true;
}

bool inicializar_logger() {
    logger_memoria = log_create("memoria.log", "[Memoria]", true, LOG_LEVEL_TRACE);
    if (logger_memoria == NULL) {
        printf("Error al crear el logger de memoria\n");
        return false;
    }
    
    log_debug(logger_memoria, "Logger inicializado correctamente");
    return true; 
}

void mostrar_resumen_sistema(void) {
    log_debug(logger_memoria, "=== RESUMEN DEL SISTEMA ===");
    log_debug(logger_memoria, "Memoria total: %d KB", memoria_config.TAM_MEMORIA / 1024);
    log_debug(logger_memoria, "Tamaño página: %d bytes", memoria_config.TAM_PAGINA);
    log_debug(logger_memoria, "Cantidad marcos: %d", memoria_config.TAM_MEMORIA / memoria_config.TAM_PAGINA);
    log_debug(logger_memoria, "Niveles paginación: %d", memoria_config.CANTIDAD_NIVELES);
    log_debug(logger_memoria, "Entradas por tabla: %d", memoria_config.ENTRADAS_POR_TABLA);
    log_debug(logger_memoria, "Puerto servidor: %d", memoria_config.PUERTO_ESCUCHA);
    log_debug(logger_memoria, "Retardo memoria: %d ms", memoria_config.RETARDO_MEMORIA);
    log_debug(logger_memoria, "Path SWAP: %s", memoria_config.PATH_SWAPFILE);
    log_debug(logger_memoria, "Retardo SWAP: %d ms", memoria_config.RETARDO_SWAP);
    log_debug(logger_memoria, "Path dump: %s", memoria_config.DUMP_PATH);
    log_debug(logger_memoria, "==========================");
}