//Incluir las librerias

#include <variables_globales_cpu.h>

//Inicializacion de las funciones

char* fetch(int pc);
void decode(char* instruccion_recibida);
void execute(instruccion instruccion_recibida);
void instruccion_noop(void);
void instruccion_write();
void instruccion_read();
void instruccion_goto();
void check_interrupt();
int obtenerLongitud(char **obtenerInsPartes);
bool estaContenido(char **array, char* valor, int tam);
void obtenerDelKernelPcPid(t_log* log, int pid, int pc);

//Declaracion Variables Globales

char* instruccion_recibida;
char* obtenerInsPartes;
int longitudIns;
char* parametros;
char* traduccionNecesaria[] = {"WRITE", "READ"};
char* nombre_instruccion;
char* syscalls[] = {"IO","INIT_PROC","DUMP_MEMORY","EXIT"};
int contador;
int pid;
int pc;