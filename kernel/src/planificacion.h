#include <k_vglobales.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <conexiones.h>



typedef enum{
  NEW, //0
  READY,//1
  BLOCKED,//2
  EXEC,//3
  EXIT_ESTADO,//4
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
  int tamanio;
  char *ruta_del_archivo_de_pseudocodigo; // a chequear
};


void crear_proceso(int tamanio, char *ruta_archivo);
int buscar_en_lista(t_list *lista,int pid);
void *planificador_largo_plazo_fifo();
struct pcb *agarrar_el_primer_proceso(t_list *lista);
char *cambiar_a_string(Estado estado);
bool menor_por_tamanio(void* a, void* b);
void insertar_ordenado_segun(t_list *lista, struct pcb *proceso, bool (*comparador)(void *, void *));
void cambiarEstado (struct pcb *pcb, Estado estadoNuevo,Estado estadoAnterior);
//struct pcb *seleccionar_proceso_segun_tamanio_mas_chico_en_memoria();

// podria armar algo parecido que devuelva la rafaga


