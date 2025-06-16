#ifndef METRICAS_H
#define METRICAS_H

#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>
#include <servidor_memoria.h>
#include <math.h>

// Métricas por proceso
void* metricas_proceso(int pid, tipo_metrica metrica);

#endif