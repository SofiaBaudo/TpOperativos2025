#include <kernel.h>
#include <conexiones.h>


int main(int argc, char* argv[]){

 //argc es la cantidad de argumentos y argv que contiene cada argumento
  

   inicializar_kernel();

  crear_proceso(1,"proceso1");
  crear_proceso(2,"proceso2");
  crear_proceso(3,"proceso3");
  struct pcb *aux = seleccionar_proceso_segun_fifo();
  struct pcb *aux2 = seleccionar_proceso_segun_fifo();
  cambiarEstado(aux,NEW,READY);
  cambiarEstado(aux2,NEW,READY);

// ¿wait y signal para que espere a que se cree un proceso?
   // aca hay que crear un proceso con los parametros ingresados
   //hilo para el plani de largo plazo
   //aca hay que agregar el detach
   //hilo para el plani de corto plazo
   //hilo para el plani de mediano plazo
}  
  

 

