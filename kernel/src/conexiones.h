#include <k_vglobales.h>
#include <pthread.h>
#include <semaforos.h> // para poder hacer el sem_post en cpu


//ATENCION DE CLIENTES
void *atender_kernel_dispatch();
void atender_kernel_io();
void atender_kernel_interrupt();

//MANEJO
void *manejar_kernel_dispatch(void *socket_dispatch);
void *manejar_kernel_io(void *socket_io);
void* manejar_kernel_interrupt(void *socket_interrupt);

//SOLICITUDES
void solicitar_rafaga_de_io(int duracion);
bool solicitar_permiso_a_memoria(int socket,int tamanio);
int buscar_IO_solicitada(t_list *lista, char* nombre_io);

//CONEXION CON CLIENTES
int iniciar_conexion_kernel_memoria();
void cerrar_conexion(int socket);
struct pcb* obtener_primero(t_list *lista);

//MANEJO DE IOS
void sacar_primero_de_la_lista_de_bloqueados(t_list *lista);
void recorrer_lista_y_finalizar_procesos(t_list * lista);