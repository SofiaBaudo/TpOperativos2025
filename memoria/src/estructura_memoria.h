#ifndef ESTRUCTURA_MEMORIA_H
#define ESTRUCTURA_MEMORIA_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include "utils/utils.h"
#include <pthread.h>
#include <variables_globales_memoria.h>

// Variables globales
extern int TAM_PAGINA;
extern int CANTIDAD_NIVELES;
extern int TAM_MEMORIA;
extern float tamanio_marco;

extern int pagina_proceso[64][2]; // sigue siendo fijo
extern int** tabla_dir_logica;
extern int** tabla_paginas;

void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura);
char* funcion_escritura(int pagina, char* info_a_escribir, int direccion);
char* funcion_lectura(int pagina, int tamanio, int direccion);
void inicializar_tabla_multinivel();
void inicializar_paginas();
bool buscar_en_pagina(int info_a_buscar);
int* leer_pagina();
bool actualizar_pagina(int pagina, int info);
void* metricas_proceso(int pid, tipo_metrica metrica);



#endif
