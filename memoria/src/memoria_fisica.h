#ifndef MEMORIA_FISICA_H
#define MEMORIA_FISICA_H

#include "inicializar_memoria.h"
#include "bibliotecas.h"

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



// Inicializa la memoria física y los marcos. Devuelve true si tuvo éxito, false si hubo error
bool inicializar_memoria_fisica(void);
// Devuelve el número del siguiente marco libre, o -1 si no hay disponibles
int obtener_siguiente_marco_libre(void);
// Devuelve la cantidad de marcos libres actualmente en memoria
int contar_marcos_libres(void);
// Libera un marco físico, marcándolo como libre y reseteando sus datos
void liberar_marco(int nro_marco);

// Funciones de acceso seguro a memoria física
int leer_memoria_fisica(uint32_t direccion_fisica, void* buffer, size_t tamanio);
int escribir_memoria_fisica(uint32_t direccion_fisica, void* buffer, size_t tamanio);
#endif