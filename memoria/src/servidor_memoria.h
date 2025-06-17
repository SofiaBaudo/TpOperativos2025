#ifndef SERVIDOR_MEMORIA
#define SERVIDOR_MEMORIA

#include <instrucciones_memoria.h>
#include <variables_globales_memoria.h>
#include <utils/utils.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <espacio_de_usuario.h>


extern t_memoria_config memoria_config; // agrego el extern para que no de error de redefinicion
extern t_log* logger_memoria;

void leer_config();
void iniciar_logger_memoria();
void iniciar_servidor_memoria();
void *manejar_cliente(void *socketCliente); //los hilos piden que la funcion sea void*, por eso puse el *
void manejar_fetch_cpu(int socket_cpu);
void manejar_read_memoria(int socket_cpu);
void manejar_write_memoria(int socket_cpu);
op_code verificar_si_hay_espacio(int tamanio);
#endif
