#include <kernel.h>
#include <conexiones.h>


int main(int argc, char* argv[]) { //argc es la cantidad de argumentos y argv que contiene cada argumento


   inicializar_kernel();
   //atender_kernel_dispatch();
   atender_kernel_io();
  
  
   printf("Programa Finalizado");
}


//usar readline para lo de iniciar el planificador
