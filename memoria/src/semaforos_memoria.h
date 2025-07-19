#ifndef SEMAFOROS_MEMORIA_H
#define SEMAFOROS_MEMORIA_H

#include "bibliotecas.h"

// Declaración de todos los mutex del sistema

extern pthread_mutex_t mutex_procesos_en_memoria;
extern pthread_mutex_t memoria_usuario_mutex;
extern pthread_mutex_t mutex_lista_metricas_procesos;
extern pthread_mutex_t mutex_paginas_en_swap;
extern pthread_mutex_t mutex_lista_procesos_swap;
extern pthread_mutex_t mutex_marcos;
extern pthread_mutex_t mutex_swap;

bool inicializar_mutex(void);

#endif