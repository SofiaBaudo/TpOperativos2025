//Define de Metricas
#ifndef METRICAS_H
#define METRICAS_H

#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>
#include <servidor_memoria.h>
#include <math.h>

// Funciones para manejo de métricas por proceso
void inicializar_metricas_procesos();
void crear_metricas_proceso(int pid);
void incrementar_metrica_proceso(int pid, tipo_metrica metrica);
void mostrar_metricas_proceso(int pid);
void destruir_metricas_proceso(int pid);
t_metricas* buscar_metricas_proceso(int pid);

#endif