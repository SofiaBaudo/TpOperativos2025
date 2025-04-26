
#include <variables_globales_cpu.h>
#include <inicializar_cpu.h>

//Inicializacion Logs CPU
t_log *cpu_logger;
t_log *cpu_log_debug;
t_config *cpu_config; 

//Variables Globales CPU

char* IP_MEMORIA;
char* IP_KERNEL;
char* REEMPLAZO_TLB;
char* REEMPLAZO_CACHE;
char* LOG_LEVEL;
char* PUERTO_MEMORIA; //pensamos que era un numerico
char* PUERTO_KERNEL_DISPATCH;
char* PUERTO_KERNEL_INTERRUPT;
int ENTRADAS_TLB;
int ENTRADAS_CACHE;
int RETARDO_CACHE;