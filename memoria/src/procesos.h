#ifndef PROCESOS_H
#define PROCESOS_H

#include "bibliotecas.h"
#include "variables_globales_memoria.h"
#include "comunicaciones_memoria.h"
#include "paginacion.h"
#include "memoria_fisica.h"
#include "inicializar_memoria.h"
#include "instrucciones_memoria.h"
#include "SWAP.h"

typedef struct t_proceso_memoria {
    int pid;
    int tamanio;
    t_list* instrucciones; // Lista de instrucciones (cada elemento es un char)
    void* tabla_paginacion_raiz;
} t_proceso_memoria;


bool inicializar_proceso(t_proceso_paquete* proceso_paquete);
bool finalizar_proceso(int pid);
struct t_proceso_memoria* buscar_proceso_en_memoria(int pid); // Devuelve el puntero al proceso en memoria, hay q usar mutex_procesos_en_memoria para despues tambien
bool dump_memoria_proceso(int pid);

#endif