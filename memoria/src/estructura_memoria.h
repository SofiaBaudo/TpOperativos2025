#ifndef ESTRUCTURA_MEMORIA_H
#define ESTRUCTURA_MEMORIA_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/utils.h"
#include <pthread.h>
#include <variables_globales_memoria.h>

void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura);
void* tabla_de_paginas(int ENTRADAS_POR_TABLA);
void* SWAP(int pid);
void* metricas_proceso(int pid, tipo_metrica metrica);
void* funcion_escritura(char* info_a_escribir);
void* funcion_lectura(char* info_a_leer);

#endif
