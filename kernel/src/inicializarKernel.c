#include <inicializarKernel.h>
void inicializar_kernel(){
    printf("Kernel inicializado");
    inicializar_logs();
    inicializar_configs();
    conectarse_con_memoria();
    //fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    //log_info(kernel_logger, "Conexion con Memoria exitoso");
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
config_kernel = crear_config("/home/utnso/tp-2025-1c-Sinergia-SO-13/kernel/kernel.config");
IP_MEMORIA = config_get_string_value(config_kernel,"IP_MEMORIA");
PUERTO_MEMORIA = config_get_string_value(config_kernel,"PUERTO_MEMORIA");
PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config_kernel,"PUERTO_ESCUCHA_DISPATCH");
PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config_kernel,"PUERTO_ESCUCHA_INTERRUPT");
PUERTO_ESCUCHA_IO = config_get_string_value(config_kernel,"PUERTO_ESCUCHA_IO");
ALGORITMO_CORTO_PLAZO = config_get_string_value(config_kernel,"ALGORITMO_CORTO_PLAZO");
ALGORITMO_INGRESO_A_READY= config_get_string_value(config_kernel,"ALGORITMO_INGRESO_A_READY");
ALFA = config_get_string_value(config_kernel,"ALFA");
TIEMPO_SUSPENSION = config_get_string_value(config_kernel,"TIEMPO_SUSPENSION");
LOG_LEVEL = config_get_string_value(config_kernel,"LOG_LEVEL");
}

void imprimir_configs(){
    log_info(kernel_logger,"%s",TIEMPO_SUSPENSION); // el %s se pone para que no tire el warning
    // falta hacer lo mismo con las demas variables globales del archivo kernel.config
    //hay que estudiar como manejar enteros con logs
}

/*void iniciar_conexion_kernel_memoria(int identificador_cpu){
    int fd_conexion_kernel_dispatch = crear_conexion(PUERTO_KERNEL_DISPATCH, IP_MEMORIA);
    enviar_op_code(fd_conexion_kernel_dispatch, HANDSHAKE_CPU);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
    }
    else{
        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
}*/
