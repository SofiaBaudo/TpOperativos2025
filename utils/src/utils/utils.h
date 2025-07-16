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
#include <commons/temporal.h>

//OP_CODE DEL PROYECTO
typedef enum op_code
{
	// HANDSHAKE
	DESCONEXION_IO = -2,
	DESCONEXION_KERNEL = -1,
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
	ENVIO_DE_PID_Y_RUTA_ARCHIVO,
	//enums para kernel
	EJECUTAR_RAFAGA_IO,
	RAFAGA_ACEPTADA,
	IO_NOMBRE,
	//enums para conexion kernel_memoria
	INICIALIZAR_PROCESO_DESDE_NEW,
	INICIALIZAR_PROCESO_SUSPENDIDO,
	ACEPTAR_PROCESO,
	RECHAZO_PROCESO,
	FINALIZAR_PROCESO,
	FINALIZACION_CONFIRMADA,
	SUSPENDER_PROCESO,
	SUSPENSION_CONFIRMADA,
	MEMORIA_LISTA,
	SOLICITAR_ESPACIO_MEMORIA,
	HAY_ESPACIO_EN_MEMORIA,
	NO_HAY_ESPACIO_EN_MEMORIA,
	//enums memoria con cpu
	SOLICITAR_INSTRUCCION,
	RESPUESTA_INSTRUCCION,
	ENVIO_PID_Y_PC,
	ENVIO_PID_Y_ENTRADANIVEL,
	ENVIO_TAMPAG_ENTRADASTABLA_CANTIDADNIVELES,
	FUERA_DE_RANGO,
	//enums instrucciones
	NOOP,
	WRITE,
	READ,
	GOTO,
	IO,
	DESALOJO_ACEPTADO,
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
	MANDAR_INSTRUCCION,
	FETCH_INSTRUCCION,
	READ_MEMORIA,
	WRITE_MEMORIA,
	ENVIO_INSTRUCCION,
	ENVIO_PID_DIRFIS_DAT,
	ENVIO_PID_NROPAG,
	ENVIO_PID_NROPAG_CONTENIDO_MARCO,
	INSTRUCCION_TERMINADA,
	SYSCALL_EJECUTADA,
	ACTUALIZACION_EXITOSA,
	ACTUALIZACION_FALLIDA,
	
	//operaciones nuevas para TLB y cache de páginas desde memoria, funciones de memoria.
	ACCESO_TABLA_PAGINAS,
	RESPUESTA_ACCESO_TABLA_PAGINAS,
	LEER_PAGINA_COMPLETA,
	RESPUESTA_LEER_PAGINA_COMPLETA,
	RESPUESTA_ACTUALIZAR_PAGINA_COMPLETA,
	MANDAR_PID_Y_PC_FETCH,
	ENVIO_VALOR_LEIDO,
	CPU_LISTA,
	PROCESO_EJECUTANDO,
	SOLICITO_DESALOJO,
	DESALOJO_REALIZADO
}op_code;

//-------------ESTRUCTURAS ADMINISTRATIVAS--------

//ESTRUCTURA DEL BUFFER
typedef struct{
	int size;
	int offset;
	void* stream;
} t_buffer;

//ESTRUCTURA DEL PAQUETE
typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

// Estructura del paquete de proceso

//ESTADOS
typedef enum{
	NEW, 			//0
	READY,			//1
	BLOCKED,		//2
	EXEC,			//3
	EXIT_ESTADO,	//4
	SUSP_READY,		//5
	SUSP_BLOCKED	//6
} Estado;

//ESTRUCTURA DE UN PROCESO
struct pcb{
	int pid;
	int pc;
	int tamanio;
	int metricas_de_estado[7];
	t_temporal *metricas_de_tiempo[7];
	char *ruta_del_archivo_de_pseudocodigo;
	float proxima_estimacion; 
	float ultima_estimacion; 
	t_temporal *duracion_ultima_rafaga; 
	t_temporal *tiempo_bloqueado;
	pthread_t hilo_al_bloquearse;
	int proxima_rafaga_io;
	char* nombre_io_que_lo_bloqueo;
};

struct parametros_de_ejecucion{
	struct pcb* proceso;
	struct instancia_de_cpu *cpu_aux;
};

//ESTRUCTURA DE UNA IO
struct instancia_de_io{
	char* nombre;
    t_list *procesos_esperando;
	sem_t *hay_procesos_esperando;
	int socket_io_para_comunicarse;
	pthread_t hilo_instancia_de_io;
};

//ESTRUCTURA DE UNA CPU
struct instancia_de_cpu{
	int id_cpu;
	bool puede_usarse;
	struct pcb *proceso_ejecutando;
	//t_list *procesos_esperando;
	int socket_para_comunicarse;
	int socket_interrupt;
};
//RECURSOS
typedef enum{
  REC_CPU,
  REC_IO
}Recurso;

/*
typedef struct {
    char* opcode; //codigo de instruccion
} t_instruccion;
*/
//CONFIG
t_config *crear_config(char* direccion);
void destruir_config(t_config *config);

//LOGS
t_log *iniciar_logger(char* archivoLog, char* nombreLog);
void destruir_logger(t_log *logger);

//CONEXIONES
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char *puerto, t_log *un_log, char *mensaje);
int esperar_cliente(int socket_servidor, t_log *un_log, char *mensaje);

//ENVIO DE DATOS
void enviar_entero(int socket_cliente, int numero);
void enviar_mensaje(int socket, char* mensaje);
void enviar_op_code(int socket_cliente, op_code codigo_operacion);
void crear_paquete(op_code codigo, t_buffer *buffer, int socket);

//RECIBIR DATOS
int recibir_operacion(int socket_cliente);
int recibir_entero(int fd_conexion);
op_code recibir_op_code (int socket);
t_paquete* recibir_paquete(int socket);

//CREACION DE BUFFERS
t_buffer *crear_buffer();
t_buffer *crear_buffer_vacio();
t_buffer *crear_buffer_cpu(int pid, int pc);
//t_buffer* crear_buffer_instruccion(t_instruccion* instr);
t_buffer * crear_buffer_instruccion_io (char* nombre, int milisegundos, int *pid, int *pc);
t_buffer *crear_buffer_MarcoMem(int pid, int entradaNivel);
t_buffer *mandar_pid_a_memoria(int pid);
t_buffer *devolver_pid_a_kernel(int pid);
t_buffer *crear_buffer_pid_numPag(int pid, int nroPag);
t_buffer *crear_buffer_io_nombre(char *nombre);
t_buffer *crear_buffer_tamPag_entradasTabla_cantNiveles(int tamPag, int entradasTabla, int cantNiveles);
t_buffer *crear_buffer_pid_entradaNivel(int pid, int entradaNivel);
t_buffer * crear_buffer_instruccion_init_proc(char* ruta_del_archivo, int tamanio_en_memoria, int *pid, int *pc);
t_buffer *crear_buffer_pid_dirFis_datos(int pid, int dirFis, char *datos);
t_buffer *crear_buffer_para_ejecucion_de_io(int pid, int milisegundos);
t_buffer * crear_buffer_de_envio_de_proceso(int pid ,char *ruta_del_archivo, int tamanio);

//BUFFERS PARA NUEVAS OPERACIONES
t_buffer *crear_buffer_acceso_tabla_paginas(int pid, int pagina_logica);
t_buffer *crear_buffer_respuesta_acceso_tabla_paginas(int marco);
t_buffer *crear_buffer_leer_pagina_completa(int pid, int marco);
t_buffer *crear_buffer_respuesta_leer_pagina_completa(void* contenido, int tam_pagina);
t_buffer *crear_buffer_actualizar_pagina_completa(int pid, int marco, void* contenido, int tam_pagina);
t_buffer *crear_buffer_respuesta_actualizar_pagina_completa(bool exito);
t_buffer *buffer_nombre_de_instruccion(char *nombre);
//DESERIALIZACIONES
char *deserializar_nombre_syscall_io(t_paquete *paquete);
char *deserializar_nombre_io(t_paquete *paquete);
void deserializar_pid_y_pc(t_paquete *paquete, int *pid, int *pc);
int deserializar_pc(t_paquete *paquete);
int deserializar_pid(t_paquete *paquete);
int deserializar_tamanio(t_paquete *paquete);
char *deserializar_nombre_archivo(t_paquete *paquete);
int deserializar_cant_segundos(t_paquete *paquete);
void deserializar_config_memoria(t_paquete *paquete, int* tamPag, int* entradasTabla, int* cantNiveles);
int deserializar_entradaNivel(t_paquete *paquete);
int deserializar_entero_desde_stream(t_paquete* paquete);
int deserializar_nroPag(t_paquete *paquete);
void *deserializar_contenido(t_paquete *paquete);
char *deserializar_nombre_archivo_proceso(t_paquete *paquete);

//DESERIALIZACIONES PARA NUEVAS OPERACIONES
void deserializar_acceso_tabla_paginas(t_paquete *paquete, int *pid, int *pagina_logica);
int deserializar_respuesta_acceso_tabla_paginas(t_paquete *paquete);
void deserializar_leer_pagina_completa(t_paquete *paquete, int *pid, int *marco);
void* deserializar_respuesta_leer_pagina_completa(t_paquete *paquete, int *tam_pagina);
void deserializar_actualizar_pagina_completa(t_paquete *paquete, int *pid, int *marco, void** contenido, int *tam_pagina);
bool deserializar_respuesta_actualizar_pagina_completa(t_paquete *paquete);
int deserializar_pid_memoria(t_paquete *paquete);
int deserializar_tamanio_memoria(t_paquete *paquete);
char *deserializar_nombre_archivo_memoria(t_paquete *paquete);
char *deserializar_nombre_instruccion(t_paquete *paquete);
char *deserializar_dataIns(t_paquete *paquete);
int deserializar_dirFis(t_paquete *paquete);
int deserializar_marco(t_paquete *paquete);
int deserializar_tamPag(t_paquete *paquete);

//SIN CLASIFICACION
char* instruccion_a_string(op_code codigo);
int deserializar_pc_memoria(t_paquete *paquete);
op_code obtener_codigo_de_operacion (t_paquete * paquete);

t_buffer *crear_buffer_char_asterisco(char *nombre);
char *deserializar_char_asterisco(t_paquete *paquete);
char *deserializar_nombre_archivo_init_proc(t_paquete *paquete);

int deserializar_tamanio_escritura_memoria(t_paquete *paquete);
char *deserializar_datos_escritura_memoria(t_paquete *paquete);

t_buffer *crear_buffer_rafaga_de_io(int pid, int milisegundos);
int deserializar_milisegundos(t_paquete *paquete);
#endif