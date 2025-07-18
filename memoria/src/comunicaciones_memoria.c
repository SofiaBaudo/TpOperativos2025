#include <comunicaciones_memoria.h>

void enviar_instruccion(int socket_destino, char* instruccion) {
    t_buffer *buffer = buffer_nombre_de_instruccion(instruccion);
    crear_paquete(MANDAR_INSTRUCCION,buffer,socket_destino);
    return;
}

// Envía un buffer de memoria leído a la CPU como un solo paquete (tamaño + datos juntos)

void enviar_valor_leido(int socket_destino, void* buffer, size_t tamanio) {
    enviar_entero(socket_destino,555);
    return;
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
// Recibe un pedido de acceso a tabla de páginas.

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
void destruir_pedido_actualizar_pagina_completa(t_paquete* pedido){
    if (!pedido)
        return;
    void* contenido = deserializar_contenido(pedido);
    if (contenido) 
    log_debug(logger_memoria, "entre al destruccion y estoy x hcer los frees");
    free(contenido);
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
        size_t enviado = send(socket_destino, (char*)paquete + total_enviado, paquete_size - total_enviado, 0);
        if (enviado <= 0) break;
        total_enviado += enviado;
    }
    free(paquete);
}