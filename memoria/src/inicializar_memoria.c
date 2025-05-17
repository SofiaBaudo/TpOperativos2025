//Incluir las librerias

#include "inicializar_memoria.h"

//Declaracion los Logs Globales

t_memoria_config memoria_config;

t_log* logger_memoria;

void leer_config(){ // Lee la config y guarda todos los values de las key (struct en el header)
    t_config* config = config_create("memoria.config");
    if (config == NULL) {
        printf("Error al leer el archivo de configuración\n");
        exit(EXIT_FAILURE);
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
    config_destroy(config); //destruye luego de guardarse los values
}

t_log* iniciar_logger_memoria(){

    logger_memoria = log_create("memoria.log", "[Memoria]", true, LOG_LEVEL_TRACE);
    if (logger_memoria == NULL) {
        printf("Error al crear el logger de memoria\n");
        exit(EXIT_FAILURE);
    }
    log_debug(logger_memoria, "Logger de memoria iniciado correctamente");
    return logger_memoria;
}

void inicializar_memoria() {
    leer_config(); // Lee la configuración de memoria y la guarda en memoria_config
    iniciar_logger_memoria(); // Inicia el logger de memoria

    log_debug(logger_memoria, "Memoria inicializada correctamente"); //Utilizo log_debug en vez de log_info para q no se confundan con los logs obligatorios del tp.
}
