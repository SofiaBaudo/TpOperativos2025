#include <kernel.h>

int main(int argc, char* argv[]) {

    inicializar_kernel();
    //imprimir_configs();
    


// Iniciar kernel como servidor
//int fd_kernel = iniciar_servidor()
    //Conectarse con Memoria
    fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    log_info(kernel_logger, "Conexion con Memoria exitoso");
    log_destroy(kernel_logger);
    log_destroy(kernel_debug_log);
    config_destroy(config_kernel);
  // Conectarse con CPU

  // fd_cpu_dispatch = esperar_cliente(IP_CPU, )
}

