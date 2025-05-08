#include <serializar.h>



t_buffer_proceso *crear_buffer_proceso(struct pcb *proceso){
t_buffer_proceso* buffer = malloc(sizeof(t_buffer_proceso));
int longitud = strlen(proceso->ruta_del_archivo_de_pseudocodigo);
buffer->size = 4*sizeof(int)+ longitud; //pid,pc,tamanio en memoria + la longitud del archivo de pseudocodigo
buffer->offset = 0;
buffer->stream = malloc(buffer->size);

memcpy(buffer->stream + buffer->offset, &proceso->pid, sizeof(int));
buffer->offset += sizeof(int);

memcpy(buffer->stream + buffer->offset, &proceso->pc, sizeof(int));
buffer->offset += sizeof(int);

memcpy(buffer->stream + buffer->offset, &proceso->tamanio, sizeof(int));
buffer->offset += sizeof(int);

memcpy(buffer->stream + buffer->offset, &longitud, sizeof(int));
buffer->offset += sizeof(int);

memcpy(buffer->stream + buffer->offset, proceso->ruta_del_archivo_de_pseudocodigo, longitud);
//free(proceso->ruta_del_archivo_de_pseudocodigo);
return buffer;
}



void crear_y_enviar_paquete(op_code codigo, struct pcb *proceso, int un_socket){
t_paquete_proceso* paquete = malloc(sizeof(t_paquete_proceso));
paquete -> codigo_operacion = codigo;
paquete -> buffer = crear_buffer_proceso(proceso);

void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(int)); // el tamanio del buffer + el tamanio del op_code + el tamanio del contenido del buffer
paquete->buffer->offset = 0;

memcpy(a_enviar + paquete->buffer->offset, &(paquete->codigo_operacion), sizeof(op_code));
paquete->buffer->offset += sizeof(op_code);

memcpy(a_enviar + paquete->buffer->offset, &(paquete->buffer->size), sizeof(int)); // el tamanio del buffer
paquete->buffer->offset += sizeof(int);
memcpy(a_enviar + paquete->buffer->offset, paquete->buffer->stream, paquete->buffer->size);

send(un_socket, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(int), 0);

free(a_enviar);
free(paquete->buffer->stream);
free(paquete->buffer);
free(paquete);
return;
}