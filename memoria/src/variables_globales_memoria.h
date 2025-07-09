#ifndef VARIABLES_GLOBALES_MEMORIA_H
#define VARIABLES_GLOBALES_MEMORIA_H

#include <bibliotecas.h>
#include <semaforos_memoria.h>

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

// Estructura de log
extern t_log* logger_memoria;
extern t_memoria_config memoria_config;

//Declaracion Enum de Metricas para los Procesos
typedef enum {
    ACCESO_TABLA,
    INSTRUCCIONES_SOLICITADAS,
    BAJADAS_SWAP,
    SUBIDAS_MEMORIA,
    LECTURAS_MEMORIA,
    ESCRITURAS_MEMORIA
} tipo_metrica;

//Declaracion Variables Globales(Extern para que no se incluyan multiples veces)
extern char* PATH_INSTRUCCIONES;
extern int tamanio_total;
extern int tamanio_disponible_en_memoria;

//Declaracion Struct Metricas por proceso
typedef struct t_metricas {
    int pid;
    int cant_acceso_tabla_pagina;
    int instrucciones_solicitadas;
    int bajadas_swap;
    int cant_subidas_memoria_principal;
    int cant_lecturas_memoria;
    int cant_escrituras_memoria;
} t_metricas;

//Declaracion de la lista de metricas por proceso
extern t_list* lista_metricas_procesos;

typedef struct t_tabla_paginas t_tabla_paginas;

#endif
