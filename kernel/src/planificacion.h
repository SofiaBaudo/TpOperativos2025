
#include <commons/collections/list.h>
#include <commons/string.h>
#include <conexiones.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/temporal.h>

//ESTADOS DEL PROCESO
typedef enum{
  NEW, //0
  READY,//1
  BLOCKED,//2
  EXEC,//3
  EXIT_ESTADO,//4
  READY_SUSPEND,//5
  BLOCKED_SUSPEND//6
} Estado;



//ESTRUCTURA DEL PROCESO
struct pcb{
  int pid;
  int pc;
  float proxima_estimacion; //deberia ser float
  float ultima_estimacion; //deberia ser float
  t_temporal *duracion_ultima_rafaga; //capaz deberia ser un long por el tema del gettime
  //despues terminar lo de las metricas de estado y la otra metrica
  //Estado estado;
  int tamanio;
  char *ruta_del_archivo_de_pseudocodigo; // a chequear
};

//PLANIFICADORES
void *planificador_largo_plazo_fifo();
void *planificador_largo_plazo_proceso_mas_chico_primero();
void *planificador_corto_plazo_fifo();
void *planificador_corto_plazo_sjf_sin_desalojo();
void *planificador_corto_plazo_sjf_con_desalojo();

//CREACION, INICIALIZACION Y FINALIZACION DE PROCESOS
void crear_proceso(int tamanio, char *ruta_archivo);
struct pcb* inicializar_un_proceso(struct pcb*pcb,int tamanio,char *ruta_archivo);
void incrementar_var_global_id_proceso();
void esperar_enter_por_pantalla();
void actualizar_proximo_a_consultar();
void poner_a_ejecutar(struct pcb* aux);
void finalizar_proceso(struct pcb*aux);


//CAMBIO Y TRANSICION DE ESTADOS
struct pcb *obtener_primer_proceso_de(Estado estado);
bool consultar_si_puede_entrar(struct pcb *proceso);
struct pcb *sacar_primero_de_la_lista(Estado estado);
void insertar_ordenado_segun(t_list *lista, struct pcb *proceso, bool (*comparador)(void *, void *));
bool menor_por_tamanio(void* a, void* b);
char *cambiar_a_string(Estado estado);
void transicionar_a_new(struct pcb *proceso);
void transicionar_a_ready(struct pcb *pcb);
void cambiarEstado (struct pcb *pcb, Estado estadoNuevo,Estado estadoAnterior);
void cambiarEstadoOrdenado(struct pcb* pcb,Estado estadoAnterior, Estado estadoNuevo,bool (*comparador)(void *, void *));

//CPU
int buscar_cpu_libre(t_list *lista);

//LISTA
int buscar_en_lista(t_list *lista,int pid);
void ordenar_lista_segun(t_list *lista,bool (*comparador)(void *, void *));

//SJF CON DESALOJO Y SIN DESALOJO
void frenar_y_restar_cronometros(t_list *lista);
float calcular_proxima_estimacion(struct pcb *proceso);
bool menor_por_estimacion(void* a, void* b);
bool menor_por_estimacion_de_los_que_ya_estan_ejecutando(void* a, void* b);
bool ver_si_hay_que_desalojar(struct pcb*proceso);
void desalojar_el_mas_grande();

//MANEJO DE SYSCALLS
void mandar_paquete_a_cpu(struct pcb *proceso);
int manejar_dump(struct pcb *aux);