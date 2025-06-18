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
#include <semaphore.h>

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
	IO_NOMBRE,
	//enums para conexion kernel_memoria
	FINALIZAR_PROCESO,
	FINALIZACION_CONFIRMADA,
	SUSPENDER_PROCESO,
	SUSPENSION_CONFIRMADA,
	SOLICITAR_ESPACIO_MEMORIA,
	HAY_ESPACIO_EN_MEMORIA,
	NO_HAY_ESPACIO_EN_MEMORIA,

	//enums memoria con cpu
	SOLCITAR_INSTRUCCION,
	RESPUESTA_INSTRUCCION,
	ENVIO_PID_Y_PC,
	ENVIO_PID_Y_ENTRADANIVEL,
	ENVIO_TAMPAG_ENTRADASTABLA_CANTIDADNIVELES,
	//enums instrucciones
	NOOP,
	WRITE,
	READ,
	GOTO,
	IO,
	INIT_PROC,
	DUMP_MEMORY,
	EXIT,

	//RESPUESTAS
	DUMP_ACEPTADO,
	DUMP_RECHAZADO,

	//INTERACCION ENTRE KERNEL E IO
	RAFAGA_DE_IO,
	FIN_DE_IO,

	//memoria
	FETCH_INSTRUCCION,
	READ_MEMORIA,
	WRITE_MEMORIA,
	ENVIO_INSTRUCCION,
	ENVIO_PID_DIRFIS_DAT
}op_code;
typedef struct
{
	int size;
	int offset;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

struct instancia_de_cpu{
    int id_cpu;
    bool puede_usarse;
    struct pcb *proceso_ejecutando;
    t_list *procesos_esperando;
	int cantInstancias;
};

typedef enum{
  NEW, //0
  READY,//1
  BLOCKED,//2
  EXEC,//3
  EXIT_ESTADO,//4
  READY_SUSPEND,//5
  BLOCKED_SUSPEND//6
} Estado;


struct instancia_de_io{
    char* nombre;
    bool puede_usarse;
    struct pcb *proceso_ejecutando;
    t_list *procesos_esperando;
	int cantInstancias;
	sem_t hay_procesos_esperando;
};
typedef enum{
  REC_CPU,
  REC_IO
}Recurso;
typedef struct t_instruccion {
    op_code codigo;
    char** parametros;
    int cantidad_parametros;
} t_instruccion;

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
int deserializar_cant_segundos(t_paquete *paquete);
void enviar_mensaje(int socket, char* mensaje);

t_buffer *crear_buffer();
t_buffer * crear_buffer_cpu(int pc, int pid);
t_buffer * crear_buffer_MarcoMem(int pid, int entradaNivel);
t_buffer *mandar_pid_a_memoria(int pid);
t_buffer * devolver_pid_a_kernel(int pid);
t_buffer *crear_buffer_vacio();
op_code obtener_codigo_de_operacion (t_paquete * paquete);
t_buffer *crear_buffer_io_nombre(char *nombre);
void crear_paquete(op_code codigo, t_buffer *buffer, int socket);
t_paquete* recibir_paquete(int socket);
char *deserializar_nombre_syscall_io(t_paquete *paquete);
char *deserializar_nombre_io(t_paquete *paquete);
int deserializar_pid(t_paquete *paquete);
int deserializar_pc(t_paquete *paquete);
int deserializar_tamanio(t_paquete *paquete);
char *deserializar_nombre_archivo(t_paquete *paquete);
t_buffer *crear_buffer_tamPag_entradasTabla_cantNiveles(int tamPag, int entradasTabla, int cantNiveles);
void deserializar_config_memoria(t_paquete *paquete, int* tamPag, int* entradasTabla, int* cantNiveles);
t_buffer *crear_buffer_pid_entradaNivel(int pid, int entradaNivel);
int deserializar_entradaNivel(t_paquete *paquete);
t_buffer * crear_buffer_para_ejecucion_de_io(int pid, int milisegundos);
t_buffer* crear_buffer_instruccion(t_instruccion* instr);
char* instruccion_a_string(op_code codigo);
int deserializar_entero_desde_stream(t_paquete* paquete);
t_buffer *crear_buffer_pid_dirFis_datos(int pid, int dirFis, char *datos);

#endif
