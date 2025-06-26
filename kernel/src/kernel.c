#include <kernel.h>
#include <conexiones.h>

void *funcion_que_duerme(){
   usleep(5000000);
   log_debug(kernel_debug_log,"Ya dormi");
   return NULL;
   }

int main(int argc, char* argv[]){

 //argc es la cantidad de argumentos y argv que contiene cada argumento
  
   inicializar_kernel();
   //PRUEBA PARA PLANI TMCP

 
   
   //PRUEBA PARA PLANIFICADOR DE CORTO PLAZO SJF SIN/CON DESALOJO

 
   //atender_kernel_io();
   //atender_kernel_dispatch();
  
 
   //crear_proceso(3,"f4");
   //crear_proceso(2,"f1");
   //atender_kernel_io();
   //crear_proceso(5,"f3");
   //crear_proceso(1,"f4");

   pthread_t hilo_dispatch;
   //pthread_t hilo_funcion_que_duerme;
   pthread_t hilo_plani_largo_plazo;
   pthread_create(&hilo_dispatch,NULL,atender_kernel_dispatch,NULL); //Creamos el hilo
   //pthread_create(&hilo_funcion_que_duerme,NULL,funcion_que_duerme,NULL);
   pthread_create(&hilo_plani_largo_plazo,NULL,planificador_largo_plazo_fifo,NULL); //Creamos el hilo

   //hilos para cpus e ios. Ios se pueden agregar durante la ejecucion y las cpus son fijas
   pthread_t hilo_plani_corto_plazo;
   pthread_create(&hilo_plani_corto_plazo,NULL,planificador_corto_plazo_sjf_con_desalojo,NULL); //Creamos el hilo
   //pthread_create(&hilo_plani_corto_plazo,NULL,planificador_corto_plazo_sjf_con_desalojo,NULL); //Creamos el hilo
   //pthread_join(hilo_dispatch,NULL);
   pthread_join(hilo_plani_largo_plazo,NULL);
   //pthread_join(hilo_funcion_que_duerme,NULL);
   pthread_join(hilo_plani_corto_plazo,NULL);
   
/*
   crear_proceso(5,"f1");
   crear_proceso(2,"f3");
   crear_proceso(1,"f4");
   crear_proceso(23,"f");
  

   //crear_proceso(37,"f3");
 
   pthread_t hilo_plani_largo_plazo;
   pthread_create(&hilo_plani_largo_plazo,NULL,planificador_largo_plazo_fifo,NULL); //Creamos el hilo
   //hilos para cpus e ios. Ios se pueden agregar durante la ejecucion y las cpus son fijas
   pthread_t hilo_plani_corto_plazo;
   //pthread_create(&hilo_plani_corto_plazo,NULL,planificador_corto_plazo_sjf_sin_desalojo,NULL); //Creamos el hilo
   pthread_create(&hilo_plani_corto_plazo,NULL,planificador_corto_plazo_sjf_sin_desalojo,NULL); //Creamos el hilo
  
   pthread_join(hilo_plani_largo_plazo,NULL);
   pthread_join(hilo_plani_corto_plazo,NULL);
   */
   //pthread_detach(hilo_plani_largo_plazo);//El hilo se desacopla del hilo principal.

// ¿wait y signal para que espere a que se cree un proceso?
   // aca hay que crear un proceso con los parametros ingresados en el json
   //hilo para el plani de largo plazo
   //aca hay que agregar el detach
   //hilo para el plani de corto plazo
   //hilo para el plani de mediano plazo
   
}  
  

 

