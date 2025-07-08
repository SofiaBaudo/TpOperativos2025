#ifndef COMUNICACIONES_MEMORIA_H
#define COMUNICACIONES_MEMORIA_H

#include "bibliotecas.h"
#include "inicializar_memoria.h"

struct t_proceso_paquete* recibir_proceso(int socket_cliente);

// Estructura para pedido de instrucción
typedef struct t_pedido_instruccion {
    int pid;
    int pc;
} t_pedido_instruccion;

struct t_pedido_instruccion* recibir_pedido_instruccion(int socket_cliente);

void enviar_instruccion(int socket_destino, const char* instruccion);

// Estructura para pedido de lectura de memoria
typedef struct t_pedido_lectura_memoria{
    int pid;
    int direccion_fisica;
    int tamanio;
    int direccion_logica;
} t_pedido_lectura_memoria;

// Recibe un pedido de lectura de memoria del socket y lo deserializa
struct t_pedido_lectura_memoria* recibir_pedido_lectura_memoria(int socket_cliente);

// Estructura para pedido de escritura de memoria
typedef struct t_pedido_escritura_memoria{
    int pid;
    int direccion_logica;
    void* buffer;
    int tamanio; // Se calcula al recibir el paquete, no se recibe explícitamente
} t_pedido_escritura_memoria;

void enviar_valor_leido(int socket_destino, void* buffer, size_t tamanio);
struct t_pedido_escritura_memoria* recibir_pedido_escritura_memoria(int socket_cliente);
void destruir_pedido_escritura_memoria(t_pedido_escritura_memoria* pedido);

#endif