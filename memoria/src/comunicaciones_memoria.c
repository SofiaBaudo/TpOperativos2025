#include "comunicaciones_memoria.h"

/* CHEQUEAR COMO ENVIAN Y COMO RECIBEN EL RESTO DE MODULOS */

// Recibe un proceso del socket y lo deserializa en un t_proceso_paquete
struct t_proceso_paquete* recibir_proceso(int socket_cliente)
{
    t_buffer *buffer = crear_buffer_vacio();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Asignar memoria para el stream con el tamaño recibido
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Deserializar el contenido del stream en un t_proceso_paquete
    t_proceso_paquete *proceso_paquete = malloc(sizeof(t_proceso_paquete));
    int offset = 0;

    // Extraer el pid
    memcpy(&(proceso_paquete->pid), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);

    // Extraer el tamaño
    memcpy(&(proceso_paquete->tamanio), buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    
    // instrucciones
    // Extraer el path del pseudocódigo
    int pseudocodigo_len = buffer->size - offset;
    char *path_pseudocodigo = malloc(pseudocodigo_len + 1);
    memcpy(path_pseudocodigo, buffer->stream + offset, pseudocodigo_len);
    path_pseudocodigo[pseudocodigo_len] = '\0';
    
    // Guardar el path del pseudocódigo en el proceso_paquete
    proceso_paquete->path_pseudocodigo = path_pseudocodigo;

    // Liberar memoria del buffer
    if (buffer != NULL)
        free(buffer->stream);
    free(buffer);
    return proceso_paquete; 
}

// Recibe un pedido de instrucción como paquete (buffer serializado)
struct t_pedido_instruccion* recibir_pedido_instruccion(int socket_cliente) {
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

// Envía una instrucción (string) a la CPU usando enviar_mensaje de utils. 
void enviar_instruccion(int socket_destino, const char* instruccion) {
    send(socket_destino, instruccion, strlen(instruccion) + 1, 0); // Enviar string con null terminator
    return;
}

// Recibe un pedido de lectura de memoria como paquete (buffer serializado)
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