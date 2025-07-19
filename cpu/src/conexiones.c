//Incluir las librerias        // Para printf, sprintf, etc.
#include <stdlib.h>       // Para malloc, free, exit
#include <string.h>       // (opcional, si usás funciones de string)
#include <conexiones.h>
#include <instrucciones.h>
// Funcion Iniciar Conexion Kernel

bool tengo_que_solicitar_pid_y_pc;
bool hayInterrupcion;
bool ultima_instruccion_fue_syscall_bloqueante;
pthread_mutex_t mx_interrupcion;
void* iniciar_conexion_kernel_dispatch(int identificador_cpu){
    ultima_instruccion_fue_syscall_bloqueante = false;
    log_debug(cpu_log_debug, "IP_KERNEL %i PUERTO_KERNEL_dispatch: %i", IP_KERNEL, PUERTO_KERNEL_DISPATCH);
    fd_conexion_kernel_dispatch = crear_conexion(IP_KERNEL,PUERTO_KERNEL_DISPATCH);
    enviar_op_code(fd_conexion_kernel_dispatch, HANDSHAKE_CPU_DISPATCH);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
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

void *iniciar_conexion_kernel_interrupt( int dentificador_cpu){
    log_debug(cpu_log_debug, "IP_KERNEL %i PUERTO_KERNEL_INTERRUPT: %i", IP_KERNEL, PUERTO_KERNEL_INTERRUPT);
    fd_conexion_kernel_interrupt = crear_conexion(IP_KERNEL,PUERTO_KERNEL_INTERRUPT);
    hayInterrupcion = false;
    enviar_op_code(fd_conexion_kernel_interrupt, HANDSHAKE_CPU_INTERRUPT);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_interrupt);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
        enviar_id(fd_conexion_kernel_interrupt, identificador_cpu);
        pthread_mutex_init(&mx_interrupcion,NULL);
        pthread_t hilo_interrupciones;
        pthread_create(&hilo_interrupciones,NULL,esperar_interrupcion,NULL);
        pthread_detach(hilo_interrupciones);
    }
   else{
        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

void* iniciar_conexion_memoria_dispatch(void* arg){
    free(arg);
    fd_conexion_dispatch_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    enviar_op_code(fd_conexion_dispatch_memoria, HANDSHAKE_CPU);                  //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_dispatch_memoria);            //recibe un entero que devuelve el kernel cuando la conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        t_paquete* paquete = recibir_paquete(fd_conexion_dispatch_memoria);
    if (!paquete || !paquete->buffer || !paquete->buffer->stream) {
        log_error(cpu_log_debug, "Error: paquete recibido inválido o buffer vacío");
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
void* inicializar_memoria(void* arg){
    int id = *(int *)arg;
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;
    tengo_que_solicitar_pid_y_pc = true;
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
    iniciar_conexion_memoria_dispatch(valor_id);
    log_info(logger, "CPU %d: Conexión con Memoria establecida", id);
    log_info(logger, "Finalizando hilo de Memoria");
    log_destroy(logger);
    return NULL;
}

void* inicializar_kernel(void* arg){   
    int id = *(int *)arg;
    int* valor_id = malloc(sizeof(int));
    *valor_id = id;
    tengo_que_solicitar_pid_y_pc = true;
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
    iniciar_conexion_kernel_dispatch(id);
    iniciar_conexion_kernel_interrupt(id);
    while(1){
       ejecutar_instrucciones(NULL);
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