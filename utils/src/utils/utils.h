#ifndef UTILS_utils_H_
#define UTILS_utils_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/config.h>
#include <string.h> // para usar, por ejemplo, la funcion memset en iniciar conexion
#include <unistd.h> // para funciones como por ejemplo close(linea 105 utils.c)

typedef enum op_code
{
	// Handshakes
	HANDSHAKE_KERNEL,
	HANDSHAKE_CPU,
	HANDSHAKE_MEMORIA,
	HANDSHAKE_IO,
	HANDSHAKE_CPU_DISPATCH,
	HANDSHAKE_CPU_INTERRUPT,
	HANDSHAKE_ACCEPTED,
	HANDSHAKE_DENIED,
	GET_INSTRUCCION,
    SOLICITAR_ESPACIO,
	MENSAJE,
	PAQUETE,
    HANDSHAKE,
    CREAR_PROCESO, // ESTE Y EL DE ABAJO SE PODRIAN CREAR PARA CADA CONEXION TIPO CREAR_PROCESO_KM (DE KERNEL A MEMORIA)
    RTA_CREAR_PROCESO, // RTA MK SERIA LA RTA QUE LA MEMORIA LE PASA AL KERNEL

	//enums para kernel
	EJECUTAR_RAFAGA_IO,
	RAFAGA_ACEPTADA,

	//enums para conexion kernel_memoria
	SOLICITAR_INICIO_DE_PROCESO,
	RESPUESTA_POSITIVA_INICIO_PROCESO,
	RESPUESTA_NEGATIVA_INICIO_PROCESO,
	FINALIZAR_PROCESO,
	FINALIZACION_CONFIRMADA,
	SUSPENDER_PROCESO,
	SUSPENSION_CONFIRMADA,

	//enums memoria con cpu
	SOLCITAR_INSTRUCCION,
	RESPUESTA_INSTRUCCION

	

}op_code;

// Enum de Instrucciones

/*typedef enum instruccion
{
	NOOP,
	WRITE,
	READ,
	GOTO,
	IO,
	INIT_PROC,
	DUMP_MEMORY,
	EXIT,
}instruccion;
*/

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


t_config *crear_config(char* direccion);
void destruir_config(t_config *config);
t_log *iniciar_logger(char* archivoLog, char* nombreLog);
void destruir_logger(t_log *logger);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char *puerto, t_log *un_log, char *mensaje);
int esperar_cliente(int socket_servidor, t_log *un_log, char *mensaje);
int recibir_operacion(int socket_cliente);

void enviar_entero(int socket_cliente, int numero);
int recibir_entero(int fd_conexion);

void enviar_op_code(int socket_cliente, op_code codigo_operacion);
op_code recibir_op_code (int socket);

void enviar_mensaje(int socket, char* mensaje);

#endif
