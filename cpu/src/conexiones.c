//Incluir las librerias        // Para printf, sprintf, etc.
#include <stdlib.h>       // Para malloc, free, exit
#include <string.h>       // (opcional, si usás funciones de string)
#include <conexiones.h>

// Funcion Iniciar Conexion Kernel

void* iniciar_conexion_kernel_dispatch(void *arg){
    int identificador_cpu = *((int*) arg);
    log_debug(cpu_log_debug,"El id es: %i",identificador_cpu);
    free(arg);
    fd_conexion_kernel_dispatch = crear_conexion(IP_KERNEL,PUERTO_KERNEL_DISPATCH);
    enviar_op_code(fd_conexion_kernel_dispatch, HANDSHAKE_CPU_DISPATCH);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    log_debug(cpu_log_debug,"La respuesta recibida es: %i",respuesta);
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
//deserializar tamPag, entradasTabla,cantNiveles

void* iniciar_conexion_memoria_dispatch(void* arg){
    free(arg);
    fd_conexion_dispatch_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    enviar_op_code(fd_conexion_dispatch_memoria, HANDSHAKE_CPU);                  //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_dispatch_memoria);            //recibe un entero que devuelve el kernel cuando la conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger ,"Conexion con la memoria establecida correctamente");
        t_paquete* paquete = recibir_paquete(fd_conexion_dispatch_memoria);
    if (!paquete || !paquete->buffer || !paquete->buffer->stream) {
        log_error(cpu_logger, "Error: paquete recibido inválido o buffer vacío");
        exit(1);
    }
    deserializar_config_memoria(paquete, &tamPag, &entradasTabla, &cantNiveles);
    }
    else{
        log_error(cpu_logger, "Error en la conexion con memoria");
        exit(EXIT_FAILURE);
    }
    return NULL;
}
//Inicializar las CPUs pedidas por el Config

// me parece que primero tendria que hacer el handshake y despues inicializar memoria. Y lo mismo con kernel. 

void* inicializar_memoria(void* arg){
    int id = *(int *)arg;
  
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
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;
    pthread_t hilo_cliente_mem;
    pthread_create(&hilo_cliente_mem, NULL, iniciar_conexion_memoria_dispatch,valor_id);
    pthread_join(hilo_cliente_mem, NULL); //esto hace que espere a que termine el hilo hijo para terminar el programa. 
    log_info(logger, "CPU %d: Conexión con Memoria establecida", id);
    while(1){
        op_code terminar = recibir_op_code(fd_conexion_dispatch_memoria);
        if(terminar == -1){
            log_info(logger, "se cerro la conexion con memoria");
            break;
        }
    }
    log_info(logger, "Finalizando hilo de Memoria");
    log_destroy(logger);
    free(valor_id);
    return NULL;
}

void* inicializar_kernel(void* arg){   
    int id = *(int *)arg;
    log_debug(cpu_log_debug, "entre a kernel");
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
    pthread_t hilo_cliente_kernel;
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;
    pthread_create(&hilo_cliente_kernel, NULL, iniciar_conexion_kernel_dispatch, valor_id);
    pthread_join(hilo_cliente_kernel, NULL);
    log_debug(cpu_log_debug,"Por entrar al while");
    while(1){
        op_code terminar = recibir_op_code(fd_conexion_kernel_dispatch);
        if(terminar == -1){
            log_info(logger, "se cerro la conexion con kernel");
            break;
        }
    }
    log_info(logger, "Finalizando hilo de Kernel");
    log_destroy(logger);
    free(valor_id);
    return NULL;
}

//Inicializar el enviar id de la CPU

void enviar_id(int fd_conexion, int identificador_cpu){
    send(fd_conexion, &identificador_cpu, sizeof(identificador_cpu),0);
}