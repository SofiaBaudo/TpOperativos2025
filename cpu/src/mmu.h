#ifndef MMU_H
#define MMU_H

#include <variables_globales_cpu.h>

void creacionNivelesN(int n);
int traduccion(int direccion, int pid);
void enviarValoresMem(int entradaNivel, int pid);
int accederATp(int numPag, int pid);
int navegarNiveles(int numPag, int pid);
int conseguirMarco();

#endif