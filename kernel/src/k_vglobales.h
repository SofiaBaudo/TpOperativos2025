#include <utils/utils.h>
#include <stdio.h>
#include <pthread.h>


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


// file descriptors


extern int fd_kernel;
extern int fd_memoria;
extern int fd_cpu_interrupt;
extern int fd_cpu_dispatch;
extern int fd_io;
extern int cliente_io;

// procesos y listas
extern t_list *procesos; 
extern struct pcb *pcb;
extern t_list *colaEstados[7];
extern int identificador_del_proceso;

// lista de ios
extern t_list *ios_conectados;
extern t_list *cpus_conectadas;