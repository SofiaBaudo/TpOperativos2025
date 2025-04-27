#include <k_vglobales.h>
#include <commons/collections/list.h>


typedef enum{
  NEW, //0
  READY,//1
  BLOCKED,//2
  EXEC,//3
  EXIT,//4
  READY_SUSPEND,//5
  BLOCKED_SUSPEND//6
} Estado;


struct rafaga{
  int duracion;
  char modulo[4];
};
/*struct nodo_rafaga{
  struct rafaga info;
  struct nodo_rafaga *sgte = NULL;
};*/


struct pcb{
  int pid;
  int pc;
  //despues terminar lo de las metricas de estado y la otra metrica
  Estado estado;
  t_list *lista_de_rafagas;
};


//struct rafaga* rafaga = list_get(pcb->lista_de_rafagas, 0);


//extern struct pcb *pcb;
//extern t_list *procesos;


struct pcb* crear_proceso(int pid);






// podria armar algo parecido que devuelva la rafaga


