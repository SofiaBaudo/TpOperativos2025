#include <kernel.h>
#include <conexiones.h>



int main(int argc, char* argv[]){

 //argc es la cantidad de argumentos y argv que contiene cada argumento
  
   inicializar_kernel();
  
 

   //atender_kernel_io();
   //atender_kernel_dispatch();
   crear_proceso(5,"f1");
   //crear_proceso(2,"f3");
   //crear_proceso(1,"f4");
   //crear_proceso(23,"f");
  

   //crear_proceso(37,"f3");
  
   //pthread_t hilo_plani_largo_plazo;
   //pthread_create(&hilo_plani_largo_plazo,NULL,planificador_proceso_mas_chico_primero,NULL); //Creamos el hilo
   //pthread_join(hilo_plani_largo_plazo,NULL);
   

   //hilos para cpus e ios. Ios se pueden agregar durante la ejecucion y las cpus son fijas
   //pthread_t hilo_plani_largo_plazo;
   //pthread_create(&hilo_plani_largo_plazo,NULL,planificador_largo_plazo_fifo,NULL); //Creamos el hilo
   //pthread_join(hilo_plani_largo_plazo,NULL);
    
   //pthread_detach(hilo_plani_largo_plazo);//El hilo se desacopla del hilo principal.

// ¿wait y signal para que espere a que se cree un proceso?
   // aca hay que crear un proceso con los parametros ingresados en el json
   //hilo para el plani de largo plazo
   //aca hay que agregar el detach
   //hilo para el plani de corto plazo
   //hilo para el plani de mediano plazo
   
}  
  

 

