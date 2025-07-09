#include <comunicaciones_memoria.h>

/* CHEQUEAR COMO ENVIAN Y COMO RECIBEN EL RESTO DE MODULOS */

// Recibe un proceso del socket y lo deserializa en un t_proceso_paquete
struct t_proceso_paquete* recibir_proceso(int socket_cliente)
{
    t_buffer *buffer = crear_buffer_vacio();
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);
    t_proceso_paquete *proceso_paquete = malloc(sizeof(t_proceso_paquete));
    int offset = 0;

    memcpy(&(proceso_paquete->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&(proceso_paquete->tamanio), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    // Extraigo el path del pseudocódigo
    int pseudocodigo_len = buffer->size - offset;
    char *path_pseudocodigo = malloc(pseudocodigo_len + 1);
    memcpy(path_pseudocodigo, buffer->stream + offset, pseudocodigo_len);
    path_pseudocodigo[pseudocodigo_len] = '\0';
    
    // Guardar el path del pseudocódigo en el proceso_paquete
    proceso_paquete->path_pseudocodigo = path_pseudocodigo;

    if (buffer != NULL)
        free(buffer->stream);
    free(buffer);
    return proceso_paquete; 
}

// Recibe un pedido de instrucción como paquete (buffer serializado)
struct t_pedido_instruccion* recibir_pedido_instruccion(int socket_cliente) {
    t_buffer *buffer = crear_buffer_vacio();
    if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) != sizeof(int)) {
        free(buffer);
        return NULL;
    }
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) != buffer->size) {
        free(buffer->stream);
        free(buffer);
        return NULL;
    }
    t_pedido_instruccion* pedido = malloc(sizeof(t_pedido_instruccion));
    int offset = 0;
    memcpy(&(pedido->pc), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->pid), buffer->stream + offset, sizeof(int));
    // Liberar buffer
    free(buffer->stream);
    free(buffer);
    return pedido;
}

void enviar_instruccion(int socket_destino, const char* instruccion) {
    send(socket_destino, instruccion, strlen(instruccion) + 1, 0); // Enviar string con null terminator
    return;
}

struct t_pedido_lectura_memoria* recibir_pedido_lectura_memoria(int socket_cliente) {
    t_buffer *buffer = crear_buffer_vacio();
    // Recibir el tamaño del buffer
    if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) != sizeof(int)) {
        free(buffer);
        return NULL;
    }
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) != buffer->size) {
        free(buffer->stream);
        free(buffer);
        return NULL;
    }
    t_pedido_lectura_memoria* pedido = malloc(sizeof(t_pedido_lectura_memoria));
    int offset = 0;
    memcpy(&(pedido->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->direccion_fisica), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->tamanio), buffer->stream + offset, sizeof(int));
    // Liberar buffer
    free(buffer->stream);
    free(buffer);
    return pedido;
}

// Envía un buffer de memoria leído a la CPU como un solo paquete (tamaño + datos juntos)
void enviar_valor_leido(int socket_destino, void* buffer, size_t tamanio) {
    int tam = (int)tamanio;
    size_t paquete_size = sizeof(int) + tamanio;
    void* paquete = malloc(paquete_size);
    memcpy(paquete, &tam, sizeof(int));
    memcpy((char*)paquete + sizeof(int), buffer, tamanio);
    size_t total_enviado = 0;
    while (total_enviado < paquete_size) {
        ssize_t enviado = send(socket_destino, (char*)paquete + total_enviado, paquete_size - total_enviado, 0);
        if (enviado <= 0) break;
        total_enviado += enviado;
    }
    free(paquete);
    return;
}

// Recibe un pedido de escritura de memoria como paquete (buffer serializado)
struct t_pedido_escritura_memoria* recibir_pedido_escritura_memoria(int socket_cliente) {
    t_buffer *buffer = crear_buffer_vacio();
    // Recibir el tamaño del buffer
    if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) != sizeof(int)) {
        free(buffer);
        return NULL;
    }
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) != buffer->size) {
        free(buffer->stream);
        free(buffer);
        return NULL;
    }
    t_pedido_escritura_memoria* pedido = malloc(sizeof(t_pedido_escritura_memoria));
    int offset = 0;
    memcpy(&(pedido->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->direccion_logica), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    // El resto del buffer es el string/datos a escribir
    pedido->tamanio = buffer->size - offset;
    pedido->buffer = malloc(pedido->tamanio);
    memcpy(pedido->buffer, buffer->stream + offset, pedido->tamanio);
    // Liberar buffer
    free(buffer->stream);
    free(buffer);
    return pedido;
}

void destruir_pedido_escritura_memoria(t_pedido_escritura_memoria* pedido) {
    if (!pedido)
        return;
    if (pedido->buffer) 
        free(pedido->buffer);
    free(pedido);
}


///////////////////////////////////// Cami: ver con SOFI ///////////////////////////////////////
//IMPLEMENTACION DE FUNCIONES COMUNICACION CON CPU.
// Recibe un pedido de acceso a tabla de páginas
struct t_pedido_acceso_tabla_paginas* recibir_pedido_acceso_tabla_paginas(int socket_cliente) {
    t_buffer *buffer = crear_buffer_vacio();
    // Recibir el tamaño del buffer
    if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) != sizeof(int)) {
        free(buffer);
        return NULL;
    }
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) != buffer->size) {
        free(buffer->stream);
        free(buffer);
        return NULL;
    }
    t_pedido_acceso_tabla_paginas* pedido = malloc(sizeof(t_pedido_acceso_tabla_paginas));
    int offset = 0;
    memcpy(&(pedido->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->pagina_logica), buffer->stream + offset, sizeof(int));
    // Liberar buffer
    free(buffer->stream);
    free(buffer);
    return pedido;
}

// Recibe un pedido de leer página completa
struct t_pedido_leer_pagina_completa* recibir_pedido_leer_pagina_completa(int socket_cliente) {
    t_buffer *buffer = crear_buffer_vacio();
    // Recibir el tamaño del buffer
    if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) != sizeof(int)) {
        free(buffer);
        return NULL;
    }
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) != buffer->size) {
        free(buffer->stream);
        free(buffer);
        return NULL;
    }
    t_pedido_leer_pagina_completa* pedido = malloc(sizeof(t_pedido_leer_pagina_completa));
    int offset = 0;
    memcpy(&(pedido->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->marco), buffer->stream + offset, sizeof(int));
    // Liberar buffer
    free(buffer->stream);
    free(buffer);
    return pedido;
}

// Recibe un pedido de actualizar página completa
struct t_pedido_actualizar_pagina_completa* recibir_pedido_actualizar_pagina_completa(int socket_cliente) {
    t_buffer *buffer = crear_buffer_vacio();
    // Recibir el tamaño del buffer
    if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) != sizeof(int)) {
        free(buffer);
        return NULL;
    }
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) != buffer->size) {
        free(buffer->stream);
        free(buffer);
        return NULL;
    }
    t_pedido_actualizar_pagina_completa* pedido = malloc(sizeof(t_pedido_actualizar_pagina_completa));
    int offset = 0;
    memcpy(&(pedido->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->marco), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&(pedido->tam_pagina), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    // El resto del buffer es el contenido de la página
    pedido->contenido = malloc(pedido->tam_pagina);
    memcpy(pedido->contenido, buffer->stream + offset, pedido->tam_pagina);
    // Liberar buffer
    free(buffer->stream);
    free(buffer);
    return pedido;
}

// Envía el número de marco como respuesta al acceso a tabla de páginas
void enviar_numero_marco(int socket_destino, int marco) {
    size_t paquete_size = sizeof(int) + sizeof(int);
    void* paquete = malloc(paquete_size);
    int tam = sizeof(int);
    memcpy(paquete, &tam, sizeof(int));
    memcpy((char*)paquete + sizeof(int), &marco, sizeof(int));
    size_t total_enviado = 0;
    while (total_enviado < paquete_size) {
        ssize_t enviado = send(socket_destino, (char*)paquete + total_enviado, paquete_size - total_enviado, 0);
        if (enviado <= 0) break;
        total_enviado += enviado;
    }
    free(paquete);
}

// Envía el contenido de una página completa
void enviar_contenido_pagina(int socket_destino, void* contenido, int tam_pagina) {
    size_t paquete_size = sizeof(int) + tam_pagina;
    void* paquete = malloc(paquete_size);
    memcpy(paquete, &tam_pagina, sizeof(int));
    memcpy((char*)paquete + sizeof(int), contenido, tam_pagina);
    size_t total_enviado = 0;
    while (total_enviado < paquete_size) {
        ssize_t enviado = send(socket_destino, (char*)paquete + total_enviado, paquete_size - total_enviado, 0);
        if (enviado <= 0) break;
        total_enviado += enviado;
    }
    free(paquete);
}


// Destruye un pedido de actualizar página completa
void destruir_pedido_actualizar_pagina_completa(t_pedido_actualizar_pagina_completa* pedido) {
    if (!pedido)
        return;
    if (pedido->contenido) 
        free(pedido->contenido);
    free(pedido);
}

// Envía confirmación de actualización de página completa (OK o ERROR)
void enviar_confirmacion_actualizacion(int socket_destino, bool exito) {
    size_t paquete_size = sizeof(int) + sizeof(int);
    void* paquete = malloc(paquete_size);
    int tam = sizeof(int);
    int resultado = exito ? 1 : 0; // 1 para OK, 0 para ERROR
    
    memcpy(paquete, &tam, sizeof(int));
    memcpy((char*)paquete + sizeof(int), &resultado, sizeof(int));
    
    size_t total_enviado = 0;
    while (total_enviado < paquete_size) {
        ssize_t enviado = send(socket_destino, (char*)paquete + total_enviado, paquete_size - total_enviado, 0);
        if (enviado <= 0) break;
        total_enviado += enviado;
    }
    free(paquete);
}