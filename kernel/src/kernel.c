#include <kernel.h>

int main(int argc, char* argv[]) {

    inicializar_kernel();
    //imprimir_configs();
    
    // Iniciar kernel como servidor
    //fd_kernel = iniciar_servidor()
    //Conectarse con Memoria
    //fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    //log_info(kernel_logger, "Conexion con Memoria exitoso");

   //Atender mensajes y que se quede escuchando
  
    atender_kernel_dispatch();
    atender_kernel_interrupt();
    terminarPrograma(); // destruye los configs y los loggers
    printf("\n");
    printf("Programa Finalizado");
  
}

