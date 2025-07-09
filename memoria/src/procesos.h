#ifndef PROCESOS_H
#define PROCESOS_H

#include "bibliotecas.h"
#include "comunicaciones_memoria.h"
#include "paginacion.h"
#include "memoria_fisica.h"
#include "inicializar_memoria.h"
#include "instrucciones_memoria.h"
#include "variables_globales_memoria.h"

typedef struct t_proceso_memoria {
    int pid;
    int tamanio;
    t_list* instrucciones; // Lista de instrucciones (cada elemento es un char)
    void* tabla_paginacion_raiz;
} t_proceso_memoria;

// Inicializa las estructuras de paginación y marcos para un nuevo proceso. Devuelve true si tuvo éxito, false si hubo error
bool inicializar_proceso(int pid, int tam_proceso, char *path_pseudocodigo);
// Libera y elimina un proceso de la lista global por PID
bool finalizar_proceso(int pid);
// Devuelve el puntero al proceso en memoria con el PID dado, o NULL si no existe
struct t_proceso_memoria* buscar_proceso_en_memoria(int pid); // Devuelve el puntero al proceso en memoria, hay q usar mutex_procesos_en_memoria para despues tambien
// Dump de memoria de un proceso. Devuelve true si fue exitoso, false si hubo error
bool dump_memoria_proceso(int pid);

#endif