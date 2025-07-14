#ifndef MEMORIA_FISICA_H
#define MEMORIA_FISICA_H

#include <inicializar_memoria.h>
#include <bibliotecas.h>
#include <semaforos_memoria.h>
#include "paginacion.h"

// Estructura que representa un marco físico de memoria
typedef struct {
    int numero_marco;         // Índice del marco dentro del array
    int pid_propietario;      // PID del proceso dueño, -1 si está libre
    bool ocupado;             // true si está asignado, false si libre
    void* direccion_fisica;   // Puntero al inicio del marco en memoria_usuario
} t_marco;


extern void* memoria_usuario;   // Bloque de memoria física simulada
extern t_marco* marcos;         // Array de marcos físicos
extern int cantidad_marcos;     // Cantidad total de marcos
extern int tam_pagina;          // Tamaño de cada página/marco

bool inicializar_memoria_fisica(void);
int obtener_siguiente_marco_libre(void);
int contar_marcos_libres(void);
t_list* obtener_marcos_proceso(int pid);
void liberar_marco(int nro_marco);
int leer_memoria_fisica(int direccion_fisica, char* valor_leido, size_t tamanio);
void* leer_marco_memoria(int nro_marco);
int escribir_memoria_fisica(int direccion_fisica, char* buffer, size_t tamanio);
int escribir_marco_memoria(int nro_marco, void* datos_pagina);
#endif