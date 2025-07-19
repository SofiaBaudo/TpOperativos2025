#ifndef SWAP_H
#define SWAP_H

#include <bibliotecas.h>
#include <variables_globales_memoria.h>
#include <inicializar_memoria.h>
#include <semaforos_memoria.h>
#include <procesos.h>


typedef struct {
    int pid;
    size_t offset_swap;      // Donde empieza el bloque del proceso en swapfile.bin
    size_t tamanio;          // Tamaño total de los datos guardados (en bytes)
    char* path_pseudocodigo; // Ruta al pseudocódigo del proceso
} ProcesoSwap;

typedef struct {
    size_t offset;
    size_t tamanio;
} HuecoSwap;

extern t_list* procesos_swap; // Lista de procesos que están en SWAP
extern t_list* huecos_swap; // Lista global de huecos libres

void inicializar_swap(); 
int suspender_proceso(int pid);
size_t buscar_espacio_libre_swap(size_t tamanio);
int escribir_en_swap(void* buffer, size_t tamanio, size_t offset);
int desuspender_proceso(int pid);
int leer_de_swap(void* buffer, size_t tamanio, size_t offset);

#endif