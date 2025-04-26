#include <utils/utils.h>
#include <stdio.h>
#include <string.h>
#include <k_vglobales.h>
#include <finalizarKernel.h>
#include <inicializarKernel.h>
#include <conexiones.h>
// VARIABLES GLOBALES


t_log *kernel_logger; // para los obligatorios
t_log *kernel_debug_log; // para ir probando cosas
t_config *config_kernel;


// file descriptors
int fd_kernel;
int fd_memoria;
int fd_cpu_interrupt;
int fd_cpu_dispatch;
int fd_io;
int cliente_io;
//VARIABLES PARA LAS CONFIGS


char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *PUERTO_ESCUCHA_DISPATCH;
char *PUERTO_ESCUCHA_INTERRUPT;
char *PUERTO_ESCUCHA_IO;
char *ALGORITMO_CORTO_PLAZO;
char *ALGORITMO_INGRESO_A_READY;
char *ALFA;
char *TIEMPO_SUSPENSION;
char *LOG_LEVEL;




/*void inicializar_kernel();
void inicializar_logs();
void inicializar_configs();
void imprimir_configs();
*/


//aca copiar el typedef del estado
struct rafaga{
   int duracion;
   char modulo[4];
}


struct nodo_rafaga{
   rafaga info;
   nodo_rafaga -> sgte = NULL;
}
struct pcb{
   int pid=0;
   int pc=0;
   //despues terminar lo de las metricas de estado y la otra metrica
   //Estado estado;
   t_list *lista_de_rafagas;
}


t_list *procesos;


struct pcb* pcb = pcb_new(1);


list_add(procesos, pcb);


struct rafaga* rafaga = list_get(pcb->lista_de_rafagas, 0);






struct pcb* pcb_new(int pid) {
   struct pcb* pcb = malloc(sizeof(struct pcb));
   pcb->pid = pid;
   pcb->pc = 0;
   pcb->lista_de_rafagas = list_create(); // crea la lista como vacia
   return pcb; // devuelve un puntero al pcb 
}


// podria armar algo parecido que devuelva la rafaga
