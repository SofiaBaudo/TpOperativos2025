#ifndef VARIABLES_GLOBALES_CPU_H
#define VARIABLES_GLOBALES_CPU_H

// Incluir las librerías

#include <utils/utils.h>
#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <pthread.h>

// Inicialización Logs CPU

extern t_log *cpu_logger;
extern t_log *cpu_log_debug;
extern t_config *cpu_config; 
extern t_log *logger;
extern t_log* logs;

// Declaración Variables Globales CPU

extern int fd_conexion_dispatch_memoria;
extern int fd_conexion_kernel_dispatch;
extern int fd_conexion_kernel_interrupt;
extern char* IP_MEMORIA;
extern char* IP_KERNEL;
extern char* REEMPLAZO_TLB;
extern char* REEMPLAZO_CACHE;
extern char* LOG_LEVEL;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_KERNEL_DISPATCH;
extern char* PUERTO_KERNEL_INTERRUPT;
extern int ENTRADAS_TLB;
extern int ENTRADAS_CACHE;
extern int RETARDO_CACHE;
extern int tamPag;
extern int cantNiveles;
extern int entradasTabla;

//Declaracion Variables Globales

extern char* instruccion_recibida;
extern char** obtenerInsPartes; 
extern char* parametros;
extern char* nombre_instruccion;
extern int pid;
extern int pc;

typedef struct {
    char* opcode; //codigo de instruccion
    char* param1; 
    char* param2; 
} t_instruccion;

#endif