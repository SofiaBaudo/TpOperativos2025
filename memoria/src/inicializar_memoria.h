#ifndef INICIALIZAR_MEMORIA
#define INICIALIZAR_MEMORIA

//Incluir las librerias

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <variables_globales_memoria.h>
#include <commons/log.h>
#include <commons/config.h>

//Declaracion los Logs de Inicializar memoria

extern t_memoria_config memoria_config; // agrego el extern para que no de error de redefinicion
extern t_log* logger_memoria;

//Declaracion de Prototipos de Funciones

void leer_config();
void iniciar_logger_memoria();

#endif
