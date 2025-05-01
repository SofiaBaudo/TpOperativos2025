#include <variables_globales_cpu.h>
#include <conexiones.h>

char* fetch(int pc);
void decode(char* instruccion_recibida);
void execute(instruccion instruccion_recibida);
void instruccion_noop(void);
void obtenerDelKernelPcPid(t_log* log, int pid, int pc);

bool estaContenido(char **array, char* valor, int tam);

void instruccion_write();
void instruccion_read();
void instruccion_goto();
