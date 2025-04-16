//Incluir las librerias

#include <utils/utils.h>
#include <stdio.h>
#include <inicializar_cpu.h>
#include <commons/config.h>
#include <commons/log.h>

//Inicializacion Logs CPU

extern t_log* cpu_logger;
extern t_log* cpu_log_debug;
extern t_config* cpu_config; 

//Variables Globales CPU

extern char* IP_MEMORIA;
extern char* IP_KERNEL;
extern char* REEMPLAZO_TLB;
extern char* REEMPLAZO_CACHE;
extern char* LOG_LEVEL;
extern char* PUERTO_MEMORIA; //pensamos que era un numerico
extern char* PUERTO_KERNEL_DISPATCH;
extern char* PUERTO_KERNEL_INTERRUPT;
extern int ENTRADAS_TLB;
extern int ENTRADAS_CACHE;
extern int RETARDO_CACHE;