#ifndef TRADUCCION_H
#define TRADUCCION_H

#include <variables_globales_cpu.h>



int traduccion(int direccion, int pid);
void enviarValoresMem(int entradaNivel, int pid);
int navegarNiveles(int numPag, int pid);
int conseguirMarco(int pid);
int buscarTlb(int numPag, int pid);
void agregarEntradaATLB(int numPag, int numMarco, int pid);
void implementarAlgoritmoFIFO(int numPag, int numMarco, int pid);
void implementarAlgoritmoLRU(int numPag, int numMarco, int pid);
NodoEntradasTLB *inicializarTLB(int pid);
void imprimirTLB(int pid);
NodoEntradasTLB *retornarEspacioLibre(int pid);
bool hayEspacioLibre(int pid);
NodoEntradasTLB *encontrarNodoConMenosReferencia(int pid);
void modificarReferencia(int numPag, int pid);
bool estaYaEnTlb(int numPag, int pid);
void actualizarContadores(int numPag, int pid);
NodoEntradasTLB *dondeEstaenTLB(int numPag, int  pid);
NodoEntradasTLB *buscarSublistaPid(int pid);
void agregarAEnlazador(int pid);
void actualizarPunteroPos(int pid);
NodoEntradasTLB *buscarPunteroFIFO(int pid);
#endif