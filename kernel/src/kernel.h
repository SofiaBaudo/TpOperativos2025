#include <utils/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <k_vglobales.h>
#include <finalizarKernel.h>
#include <inicializarKernel.h>
#include <conexiones.h>
#include <commons/temporal.h>
#include <planificacion.h>

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
//int cliente_io;

//VARIABLES PARA LAS CONFIGS
char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *PUERTO_ESCUCHA_DISPATCH;
char *PUERTO_ESCUCHA_INTERRUPT;
char *PUERTO_ESCUCHA_IO;
char *ALGORITMO_CORTO_PLAZO;
char *ALGORITMO_INGRESO_A_READY;
char *ALFA;
char *ESTIMACION_INICIAL;
char *TIEMPO_SUSPENSION;
char *LOG_LEVEL;





