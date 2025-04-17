//Incluir las librerias

#include <conexion_kernel.h>

// Funcion Iniciarl Conexion Kernel Dispacth

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

//TODO Hacer la funcion de conexion con el kernel interrupt