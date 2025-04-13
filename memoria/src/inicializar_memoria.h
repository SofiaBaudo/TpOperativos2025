#ifndef INICIALIZAR_MEMORIA
#define INICIALIZAR_MEMORIA

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>

typedf struct memoria_config{
    int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_PAGINA;
    int ENTRADAS_POR_TABLA;
    int CANTIDAD_NIVELES;
    int RETARDO_MEMORIA;
    char* PATH_SWAPFILE;
    int RETARDO_SWAP;
    char* LOG_LEVEL;
    char* DUMP_PATH;
} memoria_config;

memoria_config* memoria_config;

t_log* logger_memoria;

#endif
