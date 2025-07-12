#include <utils/utils.h>
#include <stdio.h>
#include <signal.h> 
#include <unistd.h>   // Para close(), write(), etc.
#include <stdlib.h>   // Para exit()
#include <pthread.h>
//VARIABLES GLOBALES
extern t_log *io_logger;
extern t_log *io_debug_log;
extern t_config *config_io;

// file descriptors
extern int fd_kernel;
extern int fd_memoria;
extern int fd_cpu_interrupt;
extern int fd_cpu_dispatch;
extern int fd_io;

//VARIABLES PARA LAS CONFIGS

extern char *IP_KERNEL;
extern char *PUERTO_KERNEL;
extern char *LOG_LEVEL;

