#include <kernel.h>
#include <conexiones.h>

int main(int argc, char* argv[]) {

    inicializar_kernel();
    //atender_kernel_dispatch();
    atender_kernel_io();
    enviar_entero(fd_io,3);
    
    printf("Programa Finalizado");
}

