//Incluir las librerias

#include <conexiones.h>

// Funcion Iniciar Conexion Kernel

void* iniciar_conexion_kernel_dispatch(int identificador_cpu, t_log* log){
    fd_conexion_kernel_dispatch = crear_conexion(IP_KERNEL,PUERTO_KERNEL_DISPATCH);
    enviar_op_code(fd_conexion_kernel_dispatch, HANDSHAKE_CPU_DISPATCH);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(log, "Conexion con el kernel dispatch establecida correctamente");
        enviar_id(fd_conexion_kernel_dispatch, identificador_cpu);
    }
    else{
        log_error(log, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
   
    return NULL;
}

void cerrar_conexion(int socket){
close(socket);
}
//Iniciar conexion Kernel

void* iniciar_conexion_kernel_interrupt(void *arg){
    int identificador_cpu = *(int *)arg;
    fd_conexion_kernel_interrupt = crear_conexion(IP_KERNEL,PUERTO_KERNEL_INTERRUPT);
    enviar_op_code(fd_conexion_kernel_interrupt, HANDSHAKE_CPU_INTERRUPT);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_interrupt);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
        enviar_id(fd_conexion_kernel_dispatch, identificador_cpu);
    }
   else{
        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

//Iniciar conexion Memoria

void iniciar_conexion_memoria_dispatch(int identificador_cpu){
    fd_conexion_dispatch_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    enviar_op_code(fd_conexion_dispatch_memoria, HANDSHAKE_CPU);                  //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_dispatch_memoria);            //recibe un entero que devuelve el kernel cuando la conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger ,"Conexion con la memoria establecida correctamente");
    }
    else{
        log_error(cpu_logger, "Error en la conexion con memoria");
        exit(EXIT_FAILURE);
    }
}

//Inicializar las CPUs pedidas por el Config

void inicializar(int id){    
    t_log* logger;
    char archivo_log_cpu[50];
    sprintf(archivo_log_cpu, "cpu_%d.log", id);
    char nombre_cpu[200];
    sprintf(nombre_cpu, "cpu_%d", id);
    logger = log_create(archivo_log_cpu, nombre_cpu, true, LOG_LEVEL_INFO);
    if (logger == NULL) {
        printf("No se pudo crear el archivo de log para la CPU %d\n", id);
        exit(1);
    }
    log_info(logger, "Iniciando CPU %d", id);
    iniciar_conexion_kernel_dispatch(id, logger);
    log_destroy(logger);
}

//Inicializar el enviar id de la CPU

void enviar_id(int fd_conexion, int identificador_cpu){
    send(fd_conexion, &identificador_cpu, sizeof(identificador_cpu),0);
}