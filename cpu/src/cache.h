#ifndef CACHE_H
#define CACHE_H

#include <variables_globales_cpu.h>

void usarCache(int pid, int numPag, char *instruccion, char *contenido);
void agregarPagCache(int nroPag, int pid, char* instruccion);
char* obtenerContenido(int nroPag, int pid);
bool estaHabilitadaCache();
bool estaEnCache(int numPag, int pid);
void inicializarCache();
void algoritmoClock(int pid, char *instruccion, int nroPag, char* contenido);
int bitModificado(char *instruccion);
NodosCache *retornarEspacioLibreCache();
bool hayEspacioLibreCache();
void agregarConAlgoritmos(int pid, char *instruccion, int nroPag, char* contenido);
int conseguirMarcoCache(int pid, int nroPag);
void desalojarProceso(int pid);
void imprimirCache();
void algoritmoClockM(int pid, char *instruccion, int nroPag, char*contenido);

#endif