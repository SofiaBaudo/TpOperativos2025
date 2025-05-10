#include <kernel.h>
#include <conexiones.h>



int main(int argc, char* argv[]){

 //argc es la cantidad de argumentos y argv que contiene cada argumento
  
   inicializar_kernel();
   //atender_kernel_dispatch();
   crear_proceso(35,"f1");
   crear_proceso(2,"f");
   crear_proceso(2,"f3");
   pthread_t hilo_plani_largo_plazo;
   pthread_create(&hilo_plani_largo_plazo,NULL,planificador_largo_plazo_fifo,NULL); //Creamos el hilo
   pthread_join(hilo_plani_largo_plazo,NULL);
   
   //pthread_detach(hilo_plani_largo_plazo);//El hilo se desacopla del hilo principal.

// ¿wait y signal para que espere a que se cree un proceso?
   // aca hay que crear un proceso con los parametros ingresados en el json
   //hilo para el plani de largo plazo
   //aca hay que agregar el detach
   //hilo para el plani de corto plazo
   //hilo para el plani de mediano plazo
}  
  

 

