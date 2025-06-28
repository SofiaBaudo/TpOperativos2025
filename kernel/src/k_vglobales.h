#include <utils/utils.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>



// VARIABLES GLOBALES
// el extern informa que existe una variable global que alguien del modulo declaro pero solamente tiene que ser usada
extern t_log *kernel_logger; // para los obligatorios
extern t_log *kernel_debug_log; // para ir probando cosas
extern t_config *config_kernel;


//VARIABLES PARA LAS CONFIGS

extern char *IP_MEMORIA;
extern char *PUERTO_MEMORIA;
extern char *PUERTO_ESCUCHA_DISPATCH;
extern char *PUERTO_ESCUCHA_INTERRUPT;
extern char *PUERTO_ESCUCHA_IO;
extern char *ALGORITMO_CORTO_PLAZO;
extern char *ALGORITMO_INGRESO_A_READY;
extern char *ALFA;
extern char *TIEMPO_SUSPENSION;
extern char *LOG_LEVEL;
extern char *ESTIMACION_INICIAL;

// file descriptors

extern int fd_kernel;
extern int fd_memoria;
extern int fd_cpu_interrupt;
extern int fd_cpu_dispatch;
extern int fd_io;
//extern int cliente_interrupt;
//extern int cliente_io;
//extern int cliente_dispatch;
// procesos y listas
extern t_list *procesos; 
extern struct pcb *pcb;
extern t_list *colaEstados[7];
extern int identificador_del_proceso;

// lista de ios
extern t_list *ios_conectados;
extern t_list *cpus_conectadas;
extern struct pcb *ultimo_proceso_en_entrar;

//Mutex

extern pthread_mutex_t mx_identificador_del_proceso;
extern pthread_mutex_t mx_avisar_que_entro_el_primero;
extern pthread_mutex_t mx_proximo_a_consultar;
extern pthread_mutex_t mx_usar_cola_estado[7];
extern pthread_mutex_t mx_usar_recurso[2];

//SEMAFOROS
//extern sem_t INGRESO_DEL_PRIMERO;
extern sem_t CANTIDAD_DE_PROCESOS_EN_NEW;
extern sem_t CANTIDAD_DE_PROCESOS_EN_READY;
extern sem_t CANTIDAD_DE_PROCESOS_EN_BLOCKED;
extern sem_t INTENTAR_INICIAR_NEW;
extern sem_t INTENTAR_INICIAR_SUSP_READY;
extern sem_t INGRESO_DEL_PRIMERO_READY;
extern sem_t CPUS_LIBRES;
extern sem_t REPLANIFICAR;
extern sem_t CANTIDAD_DE_PROCESOS_EN[7];
extern sem_t UNO_A_LA_VEZ;
extern sem_t UNO_A_LA_VEZ_SUSP_READY;
extern sem_t SUSP_READY_SIN_PROCESOS;

//DECLARACIONES

int buscar_en_lista(t_list *lista,int pid);
void transicionar_a_ready(struct pcb *pcb,Estado estadoInicial);
void cambiarEstado (struct pcb *pcb, Estado estadoNuevo,Estado estadoAnterior);
void sacar_de_cola_de_estado(struct pcb *proceso,Estado estado);
float calcular_proxima_estimacion(struct pcb *proceso);
void finalizar_proceso(struct pcb*aux, Estado estadoInicial);
void transicionar_a_susp_ready(struct pcb *pcb);