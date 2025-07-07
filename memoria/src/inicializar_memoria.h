#ifndef INICIALIZA_MEMORIA_H
#define INICIALIZA_MEMORIA_H

#include "bibliotecas.h"
#include "memoria_fisica.h"
#include "procesos.h" // Para la lista global
#include <pthread.h>

// Estructura de configuración
typedef struct {
    int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_PAGINA;
    int ENTRADAS_POR_TABLA;
    int CANTIDAD_NIVELES;
    int RETARDO_MEMORIA;
    char* PATH_SWAPFILE;
    int RETARDO_SWAP;
    char* DUMP_PATH;
} t_memoria_config;

// Variables globales
extern t_log* logger_memoria;
extern t_memoria_config memoria_config;
extern pthread_mutex_t mutex_procesos_en_memoria;
extern pthread_mutex_t memoria_usuario_mutex;
extern t_list* procesos_en_memoria; 

// Funciones de inicialización
bool inicializar_memoria(void);


bool inicializar_configuracion(void);
bool inicializar_logger(void);
bool inicializar_mutex(void);
bool inicializar_listas_globales(void);

// Utilidades
void mostrar_resumen_sistema(void);

#endif