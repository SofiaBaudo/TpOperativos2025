//Incluir las librerias

#include <variables_globales_cpu.h>



//Inicializacion de las funciones

void* ejecutar_instrucciones(void* arg);
char* fetch(int pc,int pid);
t_instruccion decode(char* instruccion_recibida);
void obtenerDelKernelPcPid(int pid, int pc);
void execute(t_instruccion instruccion_recibida);
void instruccion_noop(void);
void instruccion_write(int direccion, char* param2);
void instruccion_read(int direccion, char* param2);
void instruccion_goto(char *parametro);
void mandar_syscall(t_instruccion instruccion);
void check_interrupt(void);

