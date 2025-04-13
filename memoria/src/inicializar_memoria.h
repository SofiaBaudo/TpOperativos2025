#ifndef INICIALIZAR_MEMORIA
#define INICIALIZAR_MEMORIA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/log.h>
#include <commons/config.h>

typedef struct t_memoria_config{ //le pongo el t adelante para saber y diferenciar que es un type (struct)
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

extern t_memoria_config memoria_config; // agrego el extern para que no de error de redefinicion

extern t_log* logger_memoria;

void inicializar_memoria();

#endif
