#include <utils/hello.h>
#include <stdio.h>

// VARIABLES GLOBALES

t_log *kernel_logger; // para los obligatorios
t_log *kernel_debug_log; // para ir probando cosas
t_config *config_kernel;

//VARIABLES PARA LAS CONFIGS

char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *PUERTO_ESCUCHA_DISPATCH;
char *PUERTO_ESCUCHA_INTERRUPT;
char *PUERTO_ESCUCHA_IO;
char *ALGORITMO_PLANIFICACION;
char *ALGORITMO_COLA_NEW;
char *ALFA;
char *TIEMPO_SUSPENSION;
char *LOG_LEVEL;