//Incluir las librerias
#include <conexiones.h>
#include <inicializar_cpu.h>
#include <pthread.h>
#include "traduccion.h"
#include "cache.h"


//Funcion de Inicializacion de CPU

void inicializar_CPU(int id){
    inicializar_logs();
    inicializar_configs();
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;
    log_debug(cpu_log_debug, "Valor de tamPag: %d", 4096);
    //ejecutar_instrucciones(NULL);


    // Paso 1: Inicializar TLB vacía (todas las entradas con numPag = -1)
    //inicializarTLB();

// Paso 2: Agregar entradas hasta llenar la TLB
    /*agregarEntradaATLB(1, 101); // entra al índice 0
    agregarEntradaATLB(2, 102); // entra al índice 1
    agregarEntradaATLB(3, 103); // entra al índice 2

// Paso 3: Agregar una nueva entrada que debería reemplazar la más vieja (FIFO)
    agregarEntradaATLB(4, 104); // reemplaza la página 1

// Paso 4: Agregar otra que debería reemplazar la que estaba en índice 1 (página 2)
    agregarEntradaATLB(5, 105);
    agregarEntradaATLB(6, 106); // reemplaza página 2

// Paso 5: Imprimir el estado final de la TLB para verificar
    imprimirTLB();
*/

    log_debug(cpu_log_debug, "------------------------------");

    inicializarCache();
    char* contenido = "pipipi";

   
    usarCache(1, 1, "READ", contenido);
    imprimirCache();
    usarCache(1, 2, "WRITE", contenido);
    imprimirCache();
    usarCache(1, 3, "WRITE", contenido);
    imprimirCache();
    usarCache(1, 4, "READ", contenido);
    imprimirCache();
 //Accedés de nuevo a algunas
    usarCache(1, 3, "WRITE", contenido);
    imprimirCache();
    usarCache(1, 4, "READ", contenido);
    imprimirCache();
//Forzás reemplazo (debería eliminar 1 o 2)
    usarCache(1, 5, "READ", contenido);

    imprimirCache();

    //inicializar_kernel(valor_id);
    pthread_t hiloKernel;
    pthread_t hiloMemoria;
    pthread_create(&hiloKernel, NULL, inicializar_kernel, valor_id);
    pthread_create(&hiloMemoria, NULL, inicializar_memoria, valor_id);    
    pthread_join(hiloKernel, NULL);
    
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

    log_debug(cpu_log_debug, "llegue");
}