#include <conexiones.h>

void atender_kernel_dispatch(){
	
	int servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,kernel_logger,"El kernel se conecto y esta esperando al dispatch");
	if (servidor_kernel == -1) {
        log_error(kernel_logger, "Error al iniciar el servidor de kernel");
        exit(EXIT_FAILURE);
    }
    log_debug(kernel_logger, "Servidor de kernel para dispatch iniciado en el puerto %s", PUERTO_ESCUCHA_DISPATCH);// Log de inicio del servidor
    while (1) { // sigue ciclando mientras no se desconecte el cliente
		// Aca esperan al cliente (CPU)
 	int cliente_dispatch = esperar_cliente(servidor_kernel,kernel_logger,"CPU-DISPATCH");
 	if (cliente_dispatch == -1) {
            log_error(kernel_logger, "Error al aceptar un cliente");
            continue;
        }
		
 int *puntero_al_dispatch = malloc(sizeof(*puntero_al_dispatch));
        *puntero_al_dispatch = cliente_dispatch;
		 pthread_t hilo_kernel_dispatch;
    pthread_create(&hilo_kernel_dispatch,NULL,manejar_kernel_dispatch,(void*)puntero_al_dispatch); //Creamos el hilo
    pthread_detach(hilo_kernel_dispatch);//El hilo se desacopla del hilo principal.
		
	}
	close(servidor_kernel);
	}

void* manejar_kernel_dispatch(void *socket_dispatch){
	int dispatch = *((int *)socket_dispatch); // Desreferencio el puntero para obtener el socket del cliente
	free(socket_dispatch);
 op_code dispatch_id = recibir_op_code(dispatch); // !!!!!!!!!!DESPUES VER DE UNIFICAR LA FUNCION Q HIZO JERE EN EL UTILS DE RECIBIR CON UN CODE_OP PERO QUE SEA OP_CODE!!!!!!!!!!!!!!!!
   switch (dispatch_id)
    {
        case HANDSHAKE_CPU_DISPATCH:
            //LOG_INFO : ES EL LOG OBLIGATORIO
            log_info(kernel_logger, "## CPU-DISPATCH Conectado - FD del socket: %d", dispatch);
            enviar_op_code(dispatch, HANDSHAKE_ACCEPTED);
           
            break;
			default:
            log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", dispatch); //AVISA Q FUCNIONA MAL
            break;
    }
    close(dispatch);
    return NULL;
}

void atender_kernel_io(){
	
	int servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA_IO,kernel_logger,"El kernel se conecto y esta esperando al IO");
	if (servidor_kernel == -1) {
        log_error(kernel_logger, "Error al iniciar el servidor de kernel");
        exit(EXIT_FAILURE);
    }
    log_debug(kernel_logger, "Servidor de kernel para dispatch iniciado en el puerto %s", PUERTO_ESCUCHA_IO);// Log de inicio del servidor
    while (1) { // sigue ciclando mientras no se desconecte el cliente
		// Aca esperan al cliente (CPU)
 	int cliente_io = esperar_cliente(servidor_kernel,kernel_logger,"IO");
 	if (cliente_io == -1) {
            log_error(kernel_logger, "Error al aceptar un cliente");
            continue;
        }
		
 int *puntero_al_io= malloc(sizeof(*puntero_al_io));
        *puntero_al_io = cliente_io;
		pthread_t hilo_kernel_io;
    pthread_create(&hilo_kernel_io,NULL,manejar_kernel_io,(void*)puntero_al_io); //Creamos el hilo
    pthread_detach(hilo_kernel_io);//El hilo se desacopla del hilo principal.
		
	}
	close(servidor_kernel);
	}

void* manejar_kernel_io(void *socket_io){
	int io = *((int *)socket_io); // Desreferencio el puntero para obtener el socket del cliente
	free(socket_io);
    op_code io_id = recibir_op_code(io);
    // log_info(kernel_logger, "Valor recibido en io_id: %d", io_id); verificammos que el valor recibido sea el correcto
    switch (io_id)
    {        
        case HANDSHAKE_IO:
            //LOG_INFO : ES EL LOG OBLIGATORIO
            log_info(kernel_logger, "## IO Conectado - FD del socket: %d", io);
            enviar_op_code(io, HANDSHAKE_ACCEPTED);
            break;
		default:
            log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", io); //AVISA Q FUCNIONA MAL
            break;
    }
    close(io);
    return NULL;
}





