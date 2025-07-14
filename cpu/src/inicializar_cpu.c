//Incluir las librerias
#include <conexiones.h>
#include <inicializar_cpu.h>
#include <pthread.h>
#include "traduccion.h"
#include "cache.h"
//Funcion de Inicializacion de CPU

void inicializar_CPU(int id){
    inicializar_logs(id);
    inicializar_configs();
    inicializarTLB();
    inicializarCache();
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;
    // INICIAR CONEXIONES => las dos son continuas, no necesito mantenerlas abiertas.
    // Estos hilos no se usan asi.
    iniciar_conexion_memoria_dispatch(id);
    iniciar_conexion_kernel_dispatch(id);
    iniciar_conexion_kernel_interrupt(id);
    while(1){
        ejecutar_instrucciones(NULL);
    }
}

//Funcion de Inicializacion de Logs

void inicializar_logs(int id){
    char archivo_log_cpu[50];
    sprintf(archivo_log_cpu, "cpu_%d.log", id);
    char nombre_cpu[200];
    sprintf(nombre_cpu, "cpu_%d", id);
    logger = log_create(archivo_log_cpu, nombre_cpu, true, LOG_LEVEL_INFO);
    if (logger == NULL) {
        printf("No se pudo crear el archivo de log para la CPU %d\n", id);
        exit(1);
    }
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