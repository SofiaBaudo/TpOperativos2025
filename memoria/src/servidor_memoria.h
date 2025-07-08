#ifndef SERVIDOR_MEMORIA
#define SERVIDOR_MEMORIA

#include "bibliotecas.h"
#include "variables_globales_memoria.h"
#include "inicializar_memoria.h"
#include "comunicaciones_memoria.h"
#include "procesos.h"

void iniciar_servidor_memoria();
void* manejar_cliente(void* socketCliente);
void manejar_cliente_kernel(int cliente);
void manejar_cliente_cpu(int cliente);


#endif
