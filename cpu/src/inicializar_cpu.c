//Incluir las librerias
#include <conexiones.h>
#include <inicializar_cpu.h>
#include <pthread.h>
#include "traduccion.h"
#include "cache.h"


//Funcion de Inicializacion de CPU

void inicializar_CPU(int id){
    inicializar_logs();
    log_debug(cpu_log_debug, "jsjfdslkfdslfds");
    inicializar_configs();
    inicializarTLB();
    inicializarCache();
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;

    pthread_t hiloKernel;
    pthread_t hiloMemoria;
    pthread_create(&hiloKernel, NULL, inicializar_kernel, valor_id);
    pthread_create(&hiloMemoria, NULL, inicializar_memoria, valor_id);    
    pthread_join(hiloKernel, NULL);
    pthread_join(hiloMemoria, NULL);  
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
    cpu_config = crear_config("cpu.config");
    if(cpu_config == NULL){
        perror("No se pudo crear la configuracion");
        exit(EXIT_FAILURE);
    }
    IP_MEMORIA = config_get_string_value(cpu_config,"IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(cpu_config,"PUERTO_MEMORIA");
    PUERTO_KERNEL_DISPATCH = config_get_string_value(cpu_config,"PUERTO_KERNEL_DISPATCH");
    PUERTO_KERNEL_INTERRUPT = config_get_string_value(cpu_config,"PUERTO_KERNEL_INTERRUPT");
    ENTRADAS_TLB= config_get_int_value(cpu_config, "ENTRADAS_TLB");
    REEMPLAZO_TLB = config_get_string_value(cpu_config, "REEMPLAZO_TLB");
    ENTRADAS_CACHE = config_get_int_value(cpu_config, "ENTRADAS_CACHE");
    REEMPLAZO_CACHE = config_get_string_value(cpu_config, "REEMPLAZO_CACHE");
    RETARDO_CACHE = config_get_int_value(cpu_config, "RETARDO_CACHE");
    LOG_LEVEL = config_get_string_value(cpu_config, "LOG_LEVEL");

}