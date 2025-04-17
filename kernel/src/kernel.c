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
    pthread_t hilo_kernel_dispatch;
    pthread_create(&hilo_kernel_dispatch,NULL,(void*)atender_kernel_dispatch(),NULL); //Creamos el hilo
    pthread_detach(hilo_kernel_dispatch);//El hilo se desacopla del hilo principal.
    atender_kernel_dispatch();

    //atender_kernel_interrupt();
    pthread_t hilo_kernel_interrupt;
    pthread_create(&hilo_kernel_interrupt,NULL,(void*)atender_kernel_interrupt(),NULL);
    pthread_detach(hilo_kernel_interrupt);//El hilo se desacopla del hilo principal.
    atender_kernel_interrupt();

    //atender_kernel_io
    pthread_t hilo_kernel_io;
    pthread_create(&hilo_kernel_io,NULL,(void*)atender_kernel_io(),NULL);
    atender_kernel_io(hilo_kernel_io);
    pthread_join(hilo_kernel_io,NULL); //Al ser el ultimo hilo, usamos JOIN.
    printf("\n");
    terminarPrograma(); // destruye los configs y los loggers
    printf("Programa Finalizado");
    
}

