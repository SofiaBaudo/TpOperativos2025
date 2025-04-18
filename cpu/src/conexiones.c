//Incluir las librerias

#include <conexiones.h>

// Funcion Iniciar Conexion Kernel

void iniciar_conexion_kernel_dispatch(int identificador_cpu){
    int fd_conexion_kernel_dispatch = crear_conexion(IP_KERNEL,PUERTO_KERNEL_DISPATCH);
    enviar_op_code(fd_conexion_kernel_dispatch, HANDSHAKE_CPU);                    //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_kernel_dispatch);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
    }
    else{
        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
        exit(EXIT_FAILURE);
    }
}

//void iniciar_conexion_kernel_interrupt(int identificador_cpu){
//    int fd_conexion_kernel_interrupt = crear_conexion(IP_KERNEL,PUERTO_KERNEL_interrupt);
//    enviar_op_code(fd_conexion_kernel_interrupt, HANDSHAKE_CPU);                    //avisa que es CPU.
//    op_code respuesta = recibir_op_code(fd_conexion_kernel_interrupt);              //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
//    if (respuesta == HANDSHAKE_ACCEPTED){
//        log_info(cpu_logger, "Conexion con el kernel dispatch establecida correctamente");
//    }
//    else{
//        log_error(cpu_logger, "Error en la conexion con el kernel dispatch");
//        exit(EXIT_FAILURE);
//    }
//}


// Funcion Iniciar Conexion Memoria

void iniciar_conexion_memoria_dispatch(int identificador_cpu){
    int fd_conexion_dispatch_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    enviar_op_code(fd_conexion_dispatch_memoria, HANDSHAKE_CPU);                  //avisa que es CPU.
    op_code respuesta = recibir_op_code(fd_conexion_dispatch_memoria);            //recibe un entero que devuelve el kernel cuando la conexion esta hecha.
    if (respuesta == HANDSHAKE_ACCEPTED){
        log_info(cpu_logger, "Conexion con la memoria establecida correctamente");
    }
    else{
        log_error(cpu_logger, "Error en la conexion con memoria");
        exit(EXIT_FAILURE);
    }
}

//Hacer la funcion de conexion con el kernel interrupt

