#ifndef COMUNICACIONES_MEMORIA_H
#define COMUNICACIONES_MEMORIA_H

#include <bibliotecas.h>
#include <inicializar_memoria.h>

// Estructura para pedido de instrucción
typedef struct t_pedido_instruccion {
    int pid;
    int pc;
} t_pedido_instruccion;

// Estructura para pedido de lectura de memoria
typedef struct t_pedido_lectura_memoria{
    int pid;
    int direccion_fisica;
    int tamanio;
    int direccion_logica;
} t_pedido_lectura_memoria;

// Estructura para pedido de escritura de memoria
typedef struct t_pedido_escritura_memoria{
    int pid;
    int direccion_logica;
    void* buffer;
    int tamanio; // Se calcula al recibir el paquete, no se recibe explícitamente
} t_pedido_escritura_memoria;

// Estructuras para las nuevas operaciones
typedef struct t_pedido_acceso_tabla_paginas {
    int pid;
    int pagina_logica;
} t_pedido_acceso_tabla_paginas;

typedef struct t_pedido_leer_pagina_completa {
    int pid;
    int marco;
} t_pedido_leer_pagina_completa;

typedef struct t_pedido_actualizar_pagina_completa {
    int pid;
    int marco;
    void* contenido;
    int tam_pagina;
} t_pedido_actualizar_pagina_completa;


struct t_pedido_instruccion* recibir_pedido_instruccion(int socket_cliente);
void enviar_instruccion(int socket_destino,char* instruccion);
// Recibe un pedido de lectura de memoria del socket y lo deserializa
void enviar_valor_leido(int socket_destino, void* buffer, size_t tamanio);
//void destruir_pedido_escritura_memoria(t_pedido_escritura_memoria* pedido);
// Funciones para la comunicacion con cpu
struct t_pedido_acceso_tabla_paginas* recibir_pedido_acceso_tabla_paginas(int socket_cliente);
struct t_pedido_leer_pagina_completa* recibir_pedido_leer_pagina_completa(int socket_cliente);
struct t_pedido_actualizar_pagina_completa* recibir_pedido_actualizar_pagina_completa(int socket_cliente);
void enviar_numero_marco(int socket_destino, int marco);
void enviar_contenido_pagina(int socket_destino, void* contenido, int tam_pagina);
void enviar_confirmacion_actualizacion(int socket_destino, bool exito);
void destruir_pedido_actualizar_pagina_completa(t_paquete* pedido);

#endif