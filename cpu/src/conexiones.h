//Incluimos las librerias.
#include <variables_globales_cpu.h>

//Inicializacion de las funciones.

void cerrar_conexion(int socket);
void* iniciar_conexion_kernel_dispatch(void *arg);
void* iniciar_conexion_kernel_interrupt(void *arg);
void* inicializar_kernel(int id);
void enviar_id(int fd_conexion, int identificador_cpu);
void* iniciar_conexion_memoria_dispatch(void *arg);
void* inicializar_memoria(int id);

