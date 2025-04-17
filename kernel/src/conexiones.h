#include <k_vglobales.h>
#include <pthread.h>

void atender_kernel_dispatch();
void *manejar_kernel_dispatch(void *socket_dispatch);


void atender_kernel_io();
void *manejar_kernel_io(void *socket_io);
//void atender_kernel_interrupt();
//void atender_kernel_io();

