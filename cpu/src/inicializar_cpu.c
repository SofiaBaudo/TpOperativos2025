//Incluir las librerias

#include <variables_globales_cpu.h>

//Funcion de Inicializacion de CPU

void inicializar_CPU(int identificador_cpu){ //el identificador es porque puede haber varias CPU.
    printf("CPU inicializado");
    inicializar_logs();
    inicializar_configs();
    iniciar_conexion_kernel_dispatch(identificador_cpu);
    iniciar_conexion_kernel_interrupt(identificador_cpu);
    //imprimir_configs();
    destruir_config(cpu_config);
 
}

void iniciar_conexion_kernel_dispatch(int identificador_cpu){
    int fd_conexion_kernel_dispatch = crear_conexion(PUERTO_KERNEL_DISPATCH, IP_MEMORIA);
    enviar_op_code(fd_conexion_kernel_dispatch, CPU_HANDSHAKE);//avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch); //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
    }
    else{
        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
}

// TODO> Hacer la funcion de conexion con el kernel interrupt

//Funcion de Inicializacion de Logs

void inicializar_logs(){
    cpu_logger = log_create("cpu.log" , "CL_LOG", 1 , LOG_LEVEL_INFO);
    if(!cpu_logger)
    {
        perror("No se pudo crear el logger.");
        exit(EXIT_FAILURE);
    }
    cpu_log_debug = log_create("cpu.log" , "CL_LOG", 1 , LOG_LEVEL_TRACE);
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
    //config_destroy(cpu_config);
    //exit(EXIT_SUCCESS);
}

//Funcion de Imprimir de Configs

void imprimir_configs(){
    log_info(cpu_logger, "CPU TLB: %s",REEMPLAZO_TLB);
}
