#ifndef SERVIDOR_MEMORIA
#define SERVIDOR_MEMORIA

#include "utils/utils.h"
#include "inicializar_memoria.h"

#include <pthread.h>

void iniciar_servidor_memoria();
void *manejar_cliente(void *socketCliente); //los hilos piden que la funcion sea void*, por eso puse el *


#endif
