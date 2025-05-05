#include <k_vglobales.h>
#include <pthread.h>



void atender_kernel_dispatch();
void *manejar_kernel_dispatch(void *socket_dispatch);




void atender_kernel_io();
void *manejar_kernel_io(void *socket_io);
int iniciar_conexion_kernel_memoria();
//void atender_kernel_interrupt();
void cerrar_conexion(int socket);
void solicitar_rafaga_de_io(int duracion);
bool solicitar_permiso_a_memoria(int socket,int tamanio);