#ifndef INICIALIZA_MEMORIA_H
#define INICIALIZA_MEMORIA_H

#include "bibliotecas.h"
#include "memoria_fisica.h"
#include "procesos.h" // Para la lista global
#include "SWAP.h"
#include "variables_globales_memoria.h"

// Variables globales
extern pthread_mutex_t mutex_procesos_en_memoria;
extern pthread_mutex_t memoria_usuario_mutex;
extern t_list* procesos_en_memoria; 

// Funciones de inicialización
bool inicializar_memoria(void);
bool inicializar_configuracion(void);
bool inicializar_logger(void);
bool inicializar_mutex(void);
bool inicializar_listas_globales(void);
bool inicializar_swap_sistema(void);

// Utilidades q no hacen falta pero lo dejo xq queda lindo jeje
void mostrar_resumen_sistema(void);

#endif