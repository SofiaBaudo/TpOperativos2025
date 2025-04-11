#include <utils/hello.h>


void saludar(char* quien) {
    printf("Hola desde %s!!\n", quien);
}
// FUNCIONES PARA LOS CONFIGS
t_config *crear_config(char* direccion){
    t_config *nuevo_config = config_create(direccion);
    if(!nuevo_config){
        perror("No se pudo crear el config");
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void destruir_config(t_config *config){
config_destroy(config);
}

// FUNCIONES PARA LOS LOGS

t_log *iniciar_logger(char* archivoLog, char* nombreLog){
    t_log *nuevo_logger = log_create(archivoLog,nombreLog,1,LOG_LEVEL_INFO);
    if(!nuevo_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}

void destruir_logger(t_log *logger){
    log_destroy(logger);
}
