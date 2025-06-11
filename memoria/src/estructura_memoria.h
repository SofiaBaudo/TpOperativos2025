// estructura_memoria.h
#ifndef ESTRUCTURA_MEMORIA_H
#define ESTRUCTURA_MEMORIA_H

#include <stdbool.h>
#include "variables_globales_memoria.h"

// Estructura de tabla de páginas multinivel
typedef struct Tabla {
    struct Tabla** punteros; // Niveles intermedios
    int* valores;            // Último nivel: marcos
} Tabla;

extern Tabla* tabla_de_paginas_raiz;

// Funciones principales
Tabla* crear_tabla(int nivel_actual, int cantidad_niveles, int entradas_por_tabla);
void liberar_tabla(Tabla* tabla, int nivel_actual, int cantidad_niveles, int entradas_por_tabla);

// Inicialización de memoria de usuario
void inicializar_espacio_usuario();

// Funciones de acceso por direcciones físicas
char* funcion_lectura_fisica(unsigned int direccion_fisica, int tamanio);
char* funcion_escritura_fisica(unsigned int direccion_fisica, char* valor, int tamanio);

// Funciones de página completa
char* leer_pagina_completa(unsigned int direccion_fisica);
char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido);

/*
memoria usa esto para responder a la CPU cuando le pide la traducción de una dirección loguca
devuelve solo el número de marco (sin desplazamiento).
*/
int obtener_marco(int direccion_logica, int tam_pagina, int cantidad_niveles, int entradas_por_tabla, Tabla* tabla);

// Métricas por proceso
void* metricas_proceso(int pid, tipo_metrica metrica);

#endif
