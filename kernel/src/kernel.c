#include <kernel.h>
#include <conexiones.h>



int main(int argc, char* argv[]){

 //argc es la cantidad de argumentos y argv que contiene cada argumento
  
   inicializar_kernel();
   struct pcb *pruebita1 = malloc(sizeof(struct pcb));
   pruebita1->pid = 1;
   pruebita1->proxima_estimacion = 1;
   pruebita1->duracion_ultima_rafaga = temporal_create();
   struct pcb *pruebita2 = malloc(sizeof(struct pcb));
   pruebita2->proxima_estimacion = 5;
   pruebita2->pid = 2;
   pruebita2->duracion_ultima_rafaga = temporal_create();
   //atender_kernel_io();
   //atender_kernel_dispatch();
  
   struct instancia_de_cpu *cpu_1 = malloc(sizeof(struct instancia_de_cpu));
   struct instancia_de_cpu *cpu_2 = malloc(sizeof(struct instancia_de_cpu));
   cpu_1->id_cpu = 1;
   cpu_1->puede_usarse = false;
   cpu_1->proceso_ejecutando = pruebita1;
   cpu_2->id_cpu = 2;
   cpu_2->puede_usarse = false;
   cpu_2->proceso_ejecutando = pruebita2;
   list_add(cpus_conectadas,cpu_1);
   sem_post(&CPUS_LIBRES);
   list_add(cpus_conectadas,cpu_2);
   sem_post(&CPUS_LIBRES);
   //atender_kernel_io();
   crear_proceso(1,"f4");
   pthread_t hilo_plani_largo_plazo;
   pthread_create(&hilo_plani_largo_plazo,NULL,planificador_largo_plazo_fifo,NULL); //Creamos el hilo
   //hilos para cpus e ios. Ios se pueden agregar durante la ejecucion y las cpus son fijas
   pthread_t hilo_plani_corto_plazo;
   //pthread_create(&hilo_plani_corto_plazo,NULL,planificador_corto_plazo_sjf_sin_desalojo,NULL); //Creamos el hilo
   pthread_create(&hilo_plani_corto_plazo,NULL,planificador_corto_plazo_sjf_con_desalojo,NULL); //Creamos el hilo
  
   pthread_join(hilo_plani_largo_plazo,NULL);
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
  

 

