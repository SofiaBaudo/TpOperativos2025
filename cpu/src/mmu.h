#ifndef MMU_H
#define MMU_H

#include <variables_globales_cpu.h>

void creacionNivelesN(int n);
int traduccion(int direccion, int pid);
void enviarValoresMem(int numPag, int pid);
int accederATp(int numPag, int pid);
void navegarNiveles(int numPag);

#endif