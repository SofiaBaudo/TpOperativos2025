#include <inicializarIO.h>

void inicializar_IO(){
    printf("IO inicializado");
inicializar_logs();
inicializar_configs();

}

void inicializar_logs(){
    io_logger = log_create("IO.log","LOG_MODULO_IO",1,LOG_LEVEL_INFO);
    if(!io_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
     /* kernel_debug_log = log_create("kernel.log","LOG_MODULO_KERNEL",1,LOG_LEVEL_TRACE); // TRACE ES EL NIVEL MAS ALTO DE TODOS
    if(!kernel_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    */
}
void inicializar_configs(){
config_io = crear_config("/home/utnso/tp-2025-1c-Sinergia-SO-13/io/io.config");
IP_KERNEL = config_get_string_value(config_io,"IP_KERNEL");
PUERTO_KERNEL = config_get_string_value(config_io,"PUERTO_KERNEL");
LOG_LEVEL = config_get_string_value(config_io,"LOG_LEVEL");
}