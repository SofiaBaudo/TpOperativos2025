#include <planificacion.h>


t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};


struct pcb *pcb;
struct pcb* crear_proceso(int pid) {
  struct pcb* pcb = malloc(sizeof(struct pcb));
  pcb->pid = pid;
  pcb->pc = 0;
  pcb -> estado = NEW;
  pcb->lista_de_rafagas = list_create(); // crea la lista como vacia
  return pcb; // devuelve un puntero al pcb
}



