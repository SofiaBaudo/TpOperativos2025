#include "servidor_memoria.h"

//HACER LA CONEXION CON LOS OTROS MODULOS PARA QUE EL SERVIDOR NO SE QUED ESPERANDO

void iniciar_servidor_memoria() { // Inicia el servidor multihilos para atender peticiones
    char puerto[6]; // Buffer para almacenar el puerto como cadena (máximo 5 dígitos + '\0')
    sprintf(puerto, "%d", memoria_config.PUERTO_ESCUCHA); // Convierte el puerto a cadena.  A puerto le asigna el valor de memoria_config.puerto_escucha

    int servidor_memoria = iniciar_servidor(puerto, logger_memoria, "Se ha iniciado el servidor de Memoria");
    if (servidor_memoria == -1) {
        log_error(logger_memoria, "Error al iniciar el servidor de memoria");
        exit(EXIT_FAILURE);
    }
    log_debug(logger_memoria, "Servidor de memoria iniciado en el puerto %d", memoria_config.PUERTO_ESCUCHA); // Log de inicio del servidor

    while (1) {
        int cliente = esperar_cliente(servidor_memoria, logger_memoria, "........."); // Espera a que un cliente se conecte
        if (cliente == -1) {
            log_error(logger_memoria, "Error al aceptar un cliente");
            continue;
        }
        int *cliente_ptr = malloc(sizeof(*cliente_ptr));
        *cliente_ptr = cliente;
        pthread_t hilo_cliente; // Crea un hilo para atender a varios clientes al mismo tiempo
        pthread_create(&hilo_cliente, NULL, manejar_cliente, (void*) cliente_ptr);
        pthread_detach(hilo_cliente); // Desvincula el hilo para que se libere automáticamente
    }
    close(servidor_memoria);
}
void *manejar_cliente(void *socketCliente) // Esta función maneja la conexión con el cliente dependiendo de que modulo venga
{
    int cliente = *((int *)socketCliente); // Desreferencio el puntero para obtener el socket del cliente
    free(socketCliente);
    op_code cliente_id = recibir_op_code(cliente); // !!!!!!!!!!DESPUES VER DE UNIFICAR LA FUNCION Q HIZO JERE EN EL UTILS DE RECIBIR CON UN CODE_OP PERO QUE SEA OP_CODE!!!!!!!!!!!!!!!!
    switch (cliente_id)
    {
        case HANDSHAKE_KERNEL:
            //LOG_INFO : ES EL LOG OBLIGATORIO
            log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", cliente);
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            // manejar_cliente_kernel(cliente); <- HACER ESTA FUNCIONES EN LOS OTROS MODUELOS. 
            break;
        case HANDSHAKE_CPU:
            log_debug(logger_memoria, "Se conecto CPU");
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            // manejar_cliente_cpu(cliente); <- HACER ESTA FUNCIONES EN LOS OTROS MODUELOS
            break;
        default:
            log_warning(logger_memoria, "No se pudo identificar al cliente; op_code: %d", cliente_id); //AVISA Q FUCNIONA MAL
            break;
    }
    close(cliente);
    return NULL;
}

//HACER FUNCIONES PARA MANEJAR LOS CLIENTES DE KERNEL Y CPU
//manejar_cliente_kernel(cliente);
void manejar_cliente_kernel(int cliente) {
    
    close(cliente);
}



//manejar_cliente_cpu(cliente);
void manejar_cliente_cpu(int cliente) {
    
    close(cliente);
}