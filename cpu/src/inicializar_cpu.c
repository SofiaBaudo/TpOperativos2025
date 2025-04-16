//Incluir las librerias

#include <variables_globales_cpu.h>

//Funcion de Inicializacion de CPU

void inicializar_CPU(){
    inicializar_logs();
    inicializar_configs();
    imprimir_configs();
}

//Funcion de Inicializacion de Logs

void inicializar_logs(){
    cpu_logger = log_create("Cliente.log" , "CL_LOG", 1 , LOG_LEVEL_INFO);
    if(cpu_logger == NULL)
    {
        perror("No se pudo crear el logger.");
        exit(EXIT_FAILURE);
    }
    cpu_log_debug = log_create("Cliente.log" , "CL_LOG", 1 , LOG_LEVEL_TRACE);
    if(cpu_log_debug == NULL){
        perror("No se pudo crear el logger.");
        exit(EXIT_FAILURE);
    }
    log_debug(cpu_logger, "Logger de CPU iniciado correctamente");
    exit(EXIT_SUCCESS);
}

//Funcion de Inicializacion de Configs

void inicializar_configs(){
    cpu_config = crear_config("/home/utnso/tp-2025-1c-Sinergia-SO-13/cpu/cpu.config");
    if(cpu_config == NULL){
        perror("No se pudo crear la configuracion");
        exit(EXIT_FAILURE);
    }
    IP_MEMORIA = config_get_string_value(cpu_config,"IP MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(cpu_config,"PUERTO_MEMORIA");
    PUERTO_KERNEL_DISPATCH = config_get_string_value(cpu_config,"PUERTO_KERNEL_DISPATCH");
    PUERTO_KERNEL_INTERRUPT = config_get_string_value(cpu_config,"PUERTO_KERNEL_INTERRUPT");
    config_destroy(cpu_config);
    exit(EXIT_SUCCESS);
}

//Funcion de Imprimir de Configs

void imprimir_configs(){
    log_info(cpu_logger, "CPU TLB: %s",REEMPLAZO_TLB);
    
}