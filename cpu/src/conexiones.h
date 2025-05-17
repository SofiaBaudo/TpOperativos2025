//Incluimos las librerias.

#include <variables_globales_cpu.h>

//Inicializacion de las funciones.

void iniciar_conexion_memoria_dispatch(int identificador_cpu);
void cerrar_conexion(int socket);
void* iniciar_conexion_kernel_dispatch(int identificador_cpu, t_log* log);
void* iniciar_conexion_kernel_interrupt(void *arg);
void inicializar(int identificador_cpu);
void enviar_id(int fd_conexion, int identificador_cpu);