//Incluir las librerias

#include <variables_globales_cpu.h>

//Inicializacion de los Loggers

t_log *logger;

//Inicializacion de las funciones

void ejecutar_instrucciones(void);
char* fetch(int pc,int pid);
t_instruccion decode(char* instruccion_recibida);
void obtenerDelKernelPcPid(int pid, int pc);
void execute(t_instruccion instruccion_recibida);
void instruccion_noop(void);
void instruccion_write(char* param1, char* param2);
void instruccion_read(char* param1, char* param2);
void instruccion_goto(char *parametro);
void mandar_syscall(t_instruccion instruccion);
void check_interrupt(void);

//Declaracion Variables Globales

char* instruccion_recibida;
char** obtenerInsPartes;
char* parametros;
char* nombre_instruccion;
int pid;
int pc;