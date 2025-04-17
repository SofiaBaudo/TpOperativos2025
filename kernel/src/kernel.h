#include <utils/utils.h>
#include <stdio.h>
#include <string.h>
#include <k_vglobales.h>
#include <finalizarKernel.h>
#include <inicializarKernel.h>

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

//VARIABLES PARA LAS CONFIGS

char *IP_MEMORIA;
int *PUERTO_MEMORIA;
int *PUERTO_ESCUCHA_DISPATCH;
int *PUERTO_ESCUCHA_INTERRUPT;
int *PUERTO_ESCUCHA_IO;
char *ALGORITMO_CORTO_PLAZO;
char *ALGORITMO_INGRESO_A_READY;
int *ALFA;
int *TIEMPO_SUSPENSION;
char *LOG_LEVEL;


/*void inicializar_kernel();
void inicializar_logs();
void inicializar_configs();
void imprimir_configs();
*/