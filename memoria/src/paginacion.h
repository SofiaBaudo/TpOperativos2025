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

// Estructura de tabla de páginas multinivel
typedef struct Tabla {
    struct Tabla** punteros; // Niveles intermedios
    int* valores;            // Último nivel: marcos
} Tabla;

extern Tabla* tabla_de_paginas_raiz;

// Funciones principales
Tabla* crear_tabla(int nivel_actual, int cantidad_niveles, int entradas_por_tabla);
void liberar_tabla(Tabla* tabla, int nivel_actual, int cantidad_niveles, int entradas_por_tabla);
int obtener_marco(int direccion_logica, int tam_pagina, int cantidad_niveles, int entradas_por_tabla, Tabla* tabla);

#endif
