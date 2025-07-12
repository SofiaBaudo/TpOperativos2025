#ifndef TRADUCCION_H
#define TRADUCCION_H

#include <variables_globales_cpu.h>

int traduccion(int direccion, int pid, char* instruccion, void *contenido);
void enviarValoresMem(int entradaNivel, int pid);
int navegarNiveles(int numPag, int pid);
int conseguirMarco(int pid);
int buscarTlb(int numPag, int pid);
void agregarEntradaATLB(int numPag, int numMarco);
void implementarAlgoritmoFIFO(int numPag, int numMarco);
void implementarAlgoritmoLRU(int numPag, int numMarco);
void inicializarTLB();
void imprimirTLB();
NodoEntradasTLB *retornarEspacioLibre();
bool hayEspacioLibre();
NodoEntradasTLB *encontrarNodoConMenosReferencia();
void modificarReferencia(int numPag);
bool estaYaEnTlb(int numPag);
void actualizarContadores(int numPag);
NodoEntradasTLB *dondeEstaenTLB(int numPag);
void desalojarProcesoTLB();

#endif