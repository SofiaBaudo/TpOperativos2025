#ifndef INICIALIZA_MEMORIA_H
#define INICIALIZA_MEMORIA_H

#include <bibliotecas.h>
#include <semaforos_memoria.h>
#include <memoria_fisica.h>
#include <procesos.h> // Para la lista global
#include <SWAP.h>
#include <variables_globales_memoria.h>

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
    char* PATH_INSTRUCCIONES;
} t_memoria_config;

// Estructura de log
extern t_log* logger_memoria;
extern t_memoria_config memoria_config;

// Variables globales
extern t_list* procesos_en_memoria; 

// Funciones de inicialización
bool inicializar_memoria(void);
bool inicializar_configuracion(void);
bool inicializar_logger(void);
bool inicializar_listas_globales(void);
bool inicializar_swap_sistema(void);

#endif