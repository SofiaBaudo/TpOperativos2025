#ifndef SERVIDOR_MEMORIA
#define SERVIDOR_MEMORIA

#include "bibliotecas.h"
#include "inicializar_memoria.h"
#include "comunicaciones_memoria.h"
#include "procesos.h"

extern t_memoria_config memoria_config; // agrego el extern para que no de error de redefinicion
extern t_log* logger_memoria;

void iniciar_servidor_memoria();
void* manejar_cliente(void* socketCliente);
void manejar_cliente_kernel(int cliente);
void manejar_cliente_cpu(int cliente);


#endif
