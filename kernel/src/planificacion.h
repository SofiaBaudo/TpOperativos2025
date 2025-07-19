
#include <commons/collections/list.h>
#include <commons/string.h>
#include <conexiones.h>

#include <stdio.h>
#include <stdlib.h>

//PLANIFICADORES
void *planificador_largo_plazo_fifo();
void *planificador_largo_plazo_proceso_mas_chico_primero();
void *planificador_corto_plazo_fifo();
void *planificador_corto_plazo_sjf_sin_desalojo();
void *planificador_corto_plazo_sjf_con_desalojo();
void *planificador_mediano_plazo();
void *planificador_mediano_plazo_fifo();
void *planificador_mediano_plazo_proceso_mas_chico_primero();

//CREACION, INICIALIZACION Y FINALIZACION DE PROCESOS
struct pcb* inicializar_un_proceso(struct pcb*pcb,int tamanio,char *ruta_archivo);
void incrementar_var_global_id_proceso();
void esperar_enter_por_pantalla();
void actualizar_proximo_a_consultar(Estado estadoInicial);
void *poner_a_ejecutar(void *argumentos);
void finalizar_proceso(struct pcb*proceso, Estado estadoInicial);
void liberar_proceso(struct pcb *proceso);
void gestionar_metrica_de_tiempo(struct pcb* proceso, Estado estadoInicial, Estado estadoNuevo);
//void mostrar_lista(t_list* lista);

//CAMBIO Y TRANSICION DE ESTADOS
struct pcb *obtener_copia_primer_proceso_de(Estado estado);
bool consultar_si_puede_entrar(struct pcb *proceso,op_code operacion);
struct pcb *sacar_primero_de_la_lista(Estado estado);
void insertar_ordenado_segun(t_list *lista, struct pcb *proceso, bool (*comparador)(void *, void *));
bool menor_por_tamanio(void* a, void* b);
char *cambiar_a_string(Estado estado);
void transicionar_a_new(struct pcb *proceso);
float calcular_proxima_estimacion(struct pcb *proceso);
void cambiarEstadoOrdenado(struct pcb* proceso,Estado estadoAnterior, Estado estadoNuevo,bool (*comparador)(void *, void *));
void intentar_iniciar();
void desalojar_proceso_de_cpu(struct pcb *proceso_desalojado);
void enviar_proceso_a_memoria(struct pcb* proceso);
void crear_hilo_de_ejecucion(struct pcb*proceso, struct instancia_de_cpu*cpu_aux);
struct pcb *buscar_proceso_a_suspender();
struct pcb* obtener_copia_primer_proceso_de_sin_mutex(Estado estado);
struct pcb *sacar_primero_de_la_lista_de_sin_mutex(Estado estado);
//CPU
int buscar_cpu_libre(t_list *lista);
struct instancia_de_cpu *obtener_cpu(int posicion);


//LISTAS
void ordenar_lista_segun(t_list *lista,bool (*comparador)(void *, void *));
//int buscar_en_lista(t_list *lista,int pid);

//SJF CON DESALOJO Y SIN DESALOJO
void frenar_y_restar_cronometros(t_list *lista);
void reanudar_cronometros(t_list *lista,int iterarciones);

//float calcular_proxima_estimacion(struct pcb *proceso);
bool menor_por_estimacion(void* a, void* b);
bool menor_por_estimacion_de_los_que_ya_estan_ejecutando(void* a, void* b);
bool ver_si_hay_que_desalojar(struct pcb*proceso);
struct instancia_de_cpu *buscar_cpu_con_proceso_con_mayor_estimacion();
bool recorrer_lista_de_cpus_y_ver_si_corresponde_desalojar(t_list *lista,struct pcb *proceso);

//MANEJO DE SYSCALLS
void mandar_paquete_a_cpu(struct pcb *proceso,struct instancia_de_cpu *cpu);
op_code manejar_dump(struct pcb *aux,struct instancia_de_cpu* cpu_en_la_que_ejecuta);
void liberar_cpu(struct instancia_de_cpu *cpu);
void listar_metricas_de_tiempo_y_estado(struct pcb *proceso);
char *cambiar_syscall_a_string(op_code syscall);
//PlANI MEDIANO PLAZO
void* funcion_para_bloqueados(void* arg);
void liberar_paquete(t_paquete *paquete);

