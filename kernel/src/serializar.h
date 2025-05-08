#include <k_vglobales.h>
#include <string.h>
#include <planificacion.h>

typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer_proceso;

typedef struct {
    op_code codigo_operacion;
    t_buffer_proceso* buffer;
} t_paquete_proceso;

t_buffer_proceso *crear_buffer_proceso(struct pcb *proceso);
t_paquete_proceso *crear_paquete(op_code codigo, struct pcb *proceso, int un_socket);