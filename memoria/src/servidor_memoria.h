#ifndef SERVIDOR_MEMORIA
#define SERVIDOR_MEMORIA

#include <instrucciones.h>
#include <variables_globales_memoria.h>
#include <utils/utils.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>

//Declaracion de Prototipos de Funciones
void leer_config();
void iniciar_logger_memoria();
void iniciar_servidor_memoria();
//manejar_cpu(int cpu);
void *manejar_cliente(void *socketCliente); //los hilos piden que la funcion sea void*, por eso puse el *
op_code verificar_si_hay_espacio(int tamanio);

//Declaracion los Logs de Inicializar memoria
extern t_memoria_config memoria_config; // agrego el extern para que no de error de redefinicion
extern t_log* logger_memoria;

//Declaracion de Prototipos de Funciones

#endif
