#ifndef CACHE_H
#define CACHE_H

#include <variables_globales_cpu.h>

void usarCache(int pid, int numPag, char *instruccion, char *contenido);
void agregarPagCache(int nroPag, int pid, char* instruccion);
char* obtenerContenido(int nroPag, int pid);
bool estaHabilitada(int pid);
bool estaEnCache(int numPag, int pid);
NodosCache *buscarSublistaPidCache(int pid);
NodosCache *inicializarCache(int pid);
void algoritmoClock(int pid, char *instruccion, int nroPag, char* contenido);
int bitModificado(char *instruccion);
void inicializarPidCache(int pid);
NodosCache *retornarEspacioLibreCache(int pid);
bool hayEspacioLibreCache(int pid);
void agregarConAlgoritmos(int pid, char *instruccion, int nroPag, char* contenido);
int conseguirMarcoCache(int pid, int nroPag);
void desalojarProceso(int pid);
void algoritmoClockM(int pid, char *instruccion, int nroPag, char*contenido);

#endif