#ifndef CACHE_H
#define CACHE_H

#include <variables_globales_cpu.h>

int usarCache(int pid, int numPag, char *instruccion, void* contenido);
void agregarPagCache(int nroPag, int pid, char* instruccion);
void* obtenerContenido(int nroPag, int pid);
bool estaHabilitadaCache();
bool estaEnCache(int numPag, int pid);
void inicializarCache();
void algoritmoClock(int pid, char *instruccion, int nroPag, void* contenido);
int bitModificado(char *instruccion);
NodosCache *retornarEspacioLibreCache();
bool hayEspacioLibreCache();
void agregarConAlgoritmos(int pid, char *instruccion, int nroPag, void* contenido);
int conseguirMarcoCache(int pid, int nroPag);
void desalojarProceso(int pid);
void imprimirCache();
void algoritmoClockM(int pid, char *instruccion, int nroPag, void*contenido);

#endif