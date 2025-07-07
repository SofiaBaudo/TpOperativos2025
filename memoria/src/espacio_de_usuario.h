//Define de Espacio de Usuario
#ifndef ESPACIO_DE_USUARIO_H
#define ESPACIO_DE_USUARIO_H

#include <stdbool.h>
#include <variables_globales_memoria.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <servidor_memoria.h>
#include <math.h>
#include <commons/temporal.h>

// Estructura que representa un marco físico de memoria
typedef struct {
    int numero_marco;         // Índice del marco dentro del array
    int pid_propietario;      // PID del proceso dueño, -1 si está libre
    bool ocupado;             // true si está asignado, false si libre
    void* direccion_fisica;   // Puntero al inicio del marco en memoria_usuario
} t_marco;



// Variables globales del módulo de memoria física
extern void* memoria_usuario;   // Bloque de memoria física simulada
extern t_marco* marcos;         // Array de marcos físicos
extern int cantidad_marcos;     // Cantidad total de marcos
extern int tam_pagina;          // Tamaño de cada página/marco



// Inicializa la memoria física y los marcos
// Devuelve true si tuvo éxito, false si hubo error
bool inicializar_memoria_fisica(void);
void inicializar_espacio_usuario();
char* leer_pagina_completa(unsigned int direccion_fisica);
char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido);
void leer_espacio_usuario(void* destino, int direccion_fisica, int tamanio);
void escribir_espacio_usuario(int direccion_fisica, void* origen, int tamanio);

// Función para obtener marcos libres
int obtener_siguiente_marco_libre(void);

#endif