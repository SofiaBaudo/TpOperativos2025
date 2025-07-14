//Incluimos las librerias.
#include <variables_globales_cpu.h>

//Inicializacion de las funciones.

void cerrar_conexion(int socket);
void iniciar_conexion_kernel_dispatch(int valor_id);
void iniciar_conexion_kernel_interrupt(int identificador_cpu);
void enviar_id(int fd_conexion, int identificador_cpu);
void iniciar_conexion_memoria_dispatch(int id);

