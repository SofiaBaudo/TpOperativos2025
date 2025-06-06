#ifndef ESTRUCTURA_MEMORIA_H
#define ESTRUCTURA_MEMORIA_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/utils.h"
#include <pthread.h>
#include <variables_globales_memoria.h>

// Declaracion de Prototipos de Funciones
void inicializar_tabla_dir_logica();
void liberar_tabla_dir_logica();
void memory_dump();
void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura);
void* tabla_de_paginas(int ENTRADAS_POR_TABLA);
void* SWAP(int pid);
void* metricas_proceso(int pid, tipo_metrica metrica);
char* funcion_lectura(int pagina, int tamanio, int direccion);
char* funcion_escritura(int pagina, char* info_a_escribir, int direccion);
bool buscar_en_pagina(int info_a_buscar);
bool actualizar_pagina(int pagina_usuario, int informacion);

#endif
