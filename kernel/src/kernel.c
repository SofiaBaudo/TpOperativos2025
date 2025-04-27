#include <kernel.h>
#include <conexiones.h>


int main(int argc, char* argv[]) { //argc es la cantidad de argumentos y argv que contiene cada argumento


   inicializar_kernel();
   //atender_kernel_dispatch();
   //atender_kernel_io();
  
  /*pcb = crear_proceso(2);
  procesos = list_create();
  list_add(procesos,pcb);
  struct pcb *devolver = list_get(procesos,0);
  printf("EL identificador del pcb en la lista es: %i",devolver->pid);
  PRUEBA DE LISTAS QUE ANDUVO BIEN
  */ 
   printf("Programa Finalizado");
}


//usar readline para lo de iniciar el planificador
