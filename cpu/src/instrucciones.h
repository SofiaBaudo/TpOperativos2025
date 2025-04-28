#include <variables_globales_cpu.h>
#include <conexiones.h>

void execute(instruccion_recibida);
void obtenerDelKernelPcPid(t_log* log, int pid, int pc);
instruccion fetch(int pc);
void instruccion_noop();
void instruccion_write();
void instruccion_read();
void instruccion_goto();
void instruccion_io();
void instruccion_initproc();
void instruccion_dumpmemory();
void instruccion_exit();