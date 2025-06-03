#ifndef SERVIDOR_MEMORIA
#define SERVIDOR_MEMORIA

//Incluir las librerias
Add commentMore actions
#include "utils/utils.h"
#include "inicializar_memoria.h"

#include <pthread.h>

//Incluir las Funciones

void iniciar_servidor_memoria();
//manejar_cpu(int cpu);
void *manejar_cliente(void *socketCliente); //los hilos piden que la funcion sea void*, por eso puse el *
op_code verificar_si_hay_espacio(int tamanio);

#endif
