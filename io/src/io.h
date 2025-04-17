#include <utils/utils.h>
#include <stdio.h>
#include <io_vglobales.h>
#include <inicializarIO.h>

//VARIABLES GLOBALES
t_log *io_logger;
t_log *io_debug_log;
t_config *config_io;

// file descriptors
int fd_kernel;
int fd_memoria;
int fd_cpu_interrupt;
int fd_cpu_dispatch;
int fd_io;

//VARIABLES PARA LAS CONFIGS

char *IP_KERNEL;
char *PUERTO_KERNEL;
char *LOG_LEVEL;
