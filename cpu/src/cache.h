#ifndef CACHE_H
#define CACHE_H

#include <variables_globales_cpu.h>

void usarCache(int pid);
void agregarPagCache(int nroPag, int pid);
void* obtenerContenido(int nroPag, int pid);
bool estaHabilitada(int pid);
bool estaEnCache(int numPag, int pid);
NodosCache *buscarSublistaPidCache(int pid);
void inicializarCache(int pid);

#endif