#include <variables_globales_cpu.h>
#include <conexiones.h>

void execute(instruccion instruccion_recibida);
void obtenerDelKernelPcPid(t_log* log, int pid, int pc);
void decode(char* instruccion_recibida);
bool estaContenido(char **array, char* valor, int tam);
char* fetch(int pc);
void instruccion_noop();
void instruccion_write();
void instruccion_read();
void instruccion_goto();
