//Incluir las librerias

#include <inicializar_cpu.h>

//Funcion de Inicializacion de CPU

void* inicializar_CPU(int id){
    inicializar_logs();
    inicializar_configs();
    pthread_t hilosCPU[2]; // interrupt, para instrucciones
    pthread_create(&hilosCPU[0],NULL, iniciar_conexion_kernel_interrupt, id); //interrupt
    pthread_create(&hilosCPU[1],NULL, ejecutar_instrucciones, NULL);
    inicializar(id);
}

//Funcion de Inicializacion de Logs

void inicializar_logs(){
    cpu_logger = log_create("cpu.log" , "CPU", 1 , LOG_LEVEL_INFO);
    if(!cpu_logger)
    {
        perror("No se pudo crear el logger.");
        exit(EXIT_FAILURE);
    }
    cpu_log_debug = log_create("cpu.log" , "CPU", 1 , LOG_LEVEL_TRACE);
    if(cpu_log_debug == NULL){
        perror("No se pudo crear el logger.");
        exit(EXIT_FAILURE);
    }
    log_info(cpu_logger, "Logger de CPU iniciado correctamente");
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
}