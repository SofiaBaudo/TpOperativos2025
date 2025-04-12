#include <main.h>

int main(int argc, char* argv[]) {
     saludar("kernel\n");
kernel_logger = log_create("kernel.log","LOG_MODULO_KERNEL",1,LOG_LEVEL_INFO);
    if(!kernel_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
      kernel_debug_log = log_create("kernel.log","LOG_MODULO_KERNEL",1,LOG_LEVEL_TRACE); // TRACE ES EL NIVEL MAS ALTO DE TODOS
    if(!kernel_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    config_kernel = crear_config("/home/utnso/tp-2025-1c-Sinergia-SO-13/kernel/kernel.config");
    
    IP_MEMORIA = config_get_string_value(config_kernel,"IP_MEMORIA");
    TIEMPO_SUSPENSION = config_get_string_value(config_kernel,"TIEMPO_SUSPENSION");
    log_info(kernel_logger,"%s",TIEMPO_SUSPENSION); // el %s se pone para que no tire el warning
    // falta hacer lo mismo con las demas variables globales del archivo kernel.config
    //hay que estudiar como manejar enteros con logs

    destruir_logger(kernel_logger);
    destruir_logger(kernel_debug_log);
    destruir_config(config_kernel);
}

