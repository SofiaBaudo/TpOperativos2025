#ifndef COMUNICACIONES_MEMORIA_H
#define COMUNICACIONES_MEMORIA_H

#include <stdint.h>
#include "utils/utils.h"
#include "inicializar_memoria.h"

// Estructura del paquete de proceso
typedef struct {
    int pid;
    int tamanio;
    char* path_pseudocodigo;
} t_proceso_paquete;

t_proceso_paquete recibir_proceso(int socket_cliente);

// Estructura para pedido de instrucción
typedef struct {
    int pid;
    int pc;
} t_pedido_instruccion;

t_pedido_instruccion recibir_pedido_instruccion(int socket_cliente);

void enviar_instruccion(int socket_destino, const char* instruccion);

// Estructura para pedido de lectura de memoria
typedef struct {
    int pid;
    int direccion_fisica;
    int tamanio;
} t_pedido_lectura_memoria;

// Recibe un pedido de lectura de memoria del socket y lo deserializa
t_pedido_lectura_memoria* recibir_pedido_lectura_memoria(int socket_cliente);

// Estructura para pedido de escritura de memoria
typedef struct {
    int pid;
    int direccion_logica;
    void* buffer;
    int tamanio; // Se calcula al recibir el paquete, no se recibe explícitamente
} t_pedido_escritura_memoria;

t_pedido_escritura_memoria* recibir_pedido_escritura_memoria(int socket_cliente);
void destruir_pedido_escritura_memoria(t_pedido_escritura_memoria* pedido);

#endif