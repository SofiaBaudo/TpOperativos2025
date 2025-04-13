#include "inicializar_memoria.h"

void inicializar_memoria() {
    leer_config(); // Lee la configuración de memoria y la guarda en memoria_config
    iniciar_logger(); // Inicia el logger de memoria
}

void leer_config(){
    
}

t_log* iniciar_logger(){

    logger_memoria = log_create("memoria.log", "[Memoria]", true, memoria_config.LOG_LEVEL);
    if (logger_memoria == NULL) {
        printf("Error al crear el logger de memoria\n");
        exit(EXIT_FAILURE);
    }
    log_debug(logger_memoria, "Logger de memoria iniciado correctamente");
    return logger_memoria;
}