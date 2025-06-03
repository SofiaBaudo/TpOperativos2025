#ifndef VARIABLES_GLOBALES_MEMORIA_H
#define VARIABLES_GLOBALES_MEMORIA_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/utils.h"

extern int PUERTO_ESCUCHA;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern int ENTRADAS_POR_TABLA;
extern int CANTIDAD_NIVELES;
extern int RETARDO_MEMORIA;
extern char* PATH_SWAPHILE;
extern char* LOG_LEVEL;
extern char* DUMP_PATH;
extern char* PATH_INSTRUCCIONES;

typedef struct {
    int pid;
    int cant_acceso_tabla_pagina;
    int instrucciones_solicitadas;
    int bajadas_swap;
    int cant_subidas_memoria_principal;
    int cant_lecturas_memoria;
    int cant_escrituras_memoria;
} t_metricas;

extern t_metricas listado_metricas;

#endif // VARIABLES_GLOBALES_MEMORIA_H
