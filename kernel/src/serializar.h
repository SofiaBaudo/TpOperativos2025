#include <k_vglobales.h>
#include <string.h>
#include <planificacion.h>


t_buffer_proceso *crear_buffer_proceso(struct pcb *proceso);
void crear_y_enviar_paquete(op_code codigo, struct pcb *proceso, int un_socket);