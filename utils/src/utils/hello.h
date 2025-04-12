#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

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
#include <unistd.h> // para funciones como por ejemplo close(linea 105 hello.c)
/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
typedef enum
{
	MENSAJE,
	PAQUETE,
    HANDSHAKE,
    CREAR_PROCESO, // ESTE Y EL DE ABAJO SE PODRIAN CREAR PARA CADA CONEXION TIPO CREAR_PROCESO_KM (DE KERNEL A MEMORIA)
    RTA,CREAR_PROCESO // RTA MK SERIA LA RTA QUE LA MEMORIA LE PASA AL KERNEL
}op_code;

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


void saludar(char* quien);
t_config *crear_config(char* direccion);
void destruir_config(t_config *config);
t_log *iniciar_logger(char* archivoLog, char* nombreLog);
void destruir_logger(t_log *logger);
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char *puerto,t_log *un_log, char *mensaje);
int esperar_cliente(int socket_servidor, t_log *un_log);
int recibir_operacion(int socket_cliente);
#endif
