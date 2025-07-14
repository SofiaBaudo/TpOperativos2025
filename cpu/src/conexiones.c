//Incluir las librerias        // Para printf, sprintf, etc.
#include <stdlib.h>       // Para malloc, free, exit
#include <string.h>       // (opcional, si usás funciones de string)
#include <conexiones.h>
#include <instrucciones.h>
// Funcion Iniciar Conexion Kernel

bool tengo_que_solicitar_pid_y_pc;

void iniciar_conexion_kernel_dispatch(int identificador_cpu){
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
}

void cerrar_conexion(int socket){
close(socket);
}
//Iniciar conexion Kernel

void iniciar_conexion_kernel_interrupt(int identificador_cpu){
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

void iniciar_conexion_memoria_dispatch(int id){
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
    log_info(logger, "CPU %d: Conexión con Memoria establecida", id);
    log_info(logger, "Finalizando hilo de Memoria");
}

//Inicializar el enviar id de la CPU

void enviar_id(int fd_conexion, int identificador_cpu){
    send(fd_conexion, &identificador_cpu, sizeof(identificador_cpu),0);
}