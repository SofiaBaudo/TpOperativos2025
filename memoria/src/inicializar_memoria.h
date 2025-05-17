#ifndef INICIALIZAR_MEMORIA
#define INICIALIZAR_MEMORIA

//Incluir las librerias

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <variables_globales_memoria.h>
#include <commons/log.h>
#include <commons/config.h>

//Declarar los Logs Globales

extern t_memoria_config memoria_config; // agrego el extern para que no de error de redefinicion
extern t_log* logger_memoria;

//Declaracion de Funciones

void inicializar_memoria();

#endif
