#include <kernel.h>
#include <conexiones.h>

int main(int argc, char* argv[]) {

    inicializar_kernel();
 

    //atender_kernel_dispatch();

   
 printf("\n");
 atender_kernel_io();
   
    
    printf("\n");
    terminarPrograma(); // destruye los configs y los loggers
    printf("Programa Finalizado");
    
}

