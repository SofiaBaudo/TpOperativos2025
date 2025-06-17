#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdbool.h>
#include <variables_globales_memoria.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <servidor_memoria.h>
#include <math.h>

typedef struct {
    int nro_entrada;             
    int nro_marco;               
    void* siguiente_nivel;       
} entrada_tabla_pagina;

typedef struct {
    int nivel;
    entrada_tabla_pagina* entradas;
} tabla_pagina_nivel;

typedef struct {
    int pid;
    tabla_pagina_nivel* tabla_raiz;
} t_tabla_proceso;

extern t_log* logger;

tabla_pagina_nivel* crear_tablas_proceso();
int obtener_marco_final(tabla_pagina_nivel* tabla_raiz, int* indices_niveles);

#endif
