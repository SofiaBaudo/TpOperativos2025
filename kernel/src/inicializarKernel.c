#include <inicializarKernel.h>


void inicializar_kernel(char *archivo, int tamanio){
  printf("Kernel inicializado");
    printf("\n");
    inicializar_logs();
    log_info(kernel_logger,"El tamanio es:%i",tamanio);
    inicializar_configs();
    inicializar_colas_de_estados();
    inicializar_listas_de_ios_y_cpus();
    inicializar_sincronizacion();
    crear_proceso(tamanio,archivo);
}

void inicializar_logs(){
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
}

void inicializar_configs(){
    config_kernel = crear_config("kernel.config");
    IP_MEMORIA = config_get_string_value(config_kernel, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    PUERTO_ESCUCHA_IO = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_IO");
    ALGORITMO_CORTO_PLAZO = config_get_string_value(config_kernel, "ALGORITMO_CORTO_PLAZO");
    ALGORITMO_INGRESO_A_READY = config_get_string_value(config_kernel, "ALGORITMO_INGRESO_A_READY");
    ALFA = config_get_string_value(config_kernel, "ALFA");
    TIEMPO_SUSPENSION = config_get_string_value(config_kernel, "TIEMPO_SUSPENSION");
    ESTIMACION_INICIAL = config_get_string_value(config_kernel, "ESTIMACION_INICIAL");
    LOG_LEVEL = config_get_string_value(config_kernel, "LOG_LEVEL");
}

void imprimir_configs(){
    log_info(kernel_logger,"%s",TIEMPO_SUSPENSION); // el %s se pone para que no tire el warning
    // falta hacer lo mismo con las demas variables globales del archivo kernel.config
    //hay que estudiar como manejar enteros con logs
}

void inicializar_colas_de_estados(){
    for(int i = 0; i < 7; i++) {
    colaEstados[i] = list_create(); // Crea una nueva lista para cada estado
}
}

void inicializar_listas_de_ios_y_cpus(){
    ios_conectados = list_create();
    cpus_conectadas = list_create();
}
