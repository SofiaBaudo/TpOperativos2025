#include "servidor_memoria.h"
#include "instrucciones.h"

//HACER LA CONEXION CON LOS OTROS MODULOS PARA QUE EL SERVIDOR NO SE QUEDE ESPERANDO

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
        int cliente = esperar_cliente(servidor_memoria, logger_memoria, "........."); // Espera a que un cliente se conecte+
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
        case GET_INSTRUCCION:  //(op_code) que indica:"Ey Memoria, pasame la próxima instrucción del proceso X en su Program Counter Y. Lo hace la CPU en su ciclo de instrucción (Fetch).
            int pid, pc;
            recv(cliente, &pid, sizeof(int), 0);
            recv(cliente, &pc, sizeof(int), 0);
            char* instruccion = obtener_instruccion(pid, pc);
            log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion); //ESTO ES LOG OBLIGATORIO
            enviar_mensaje(cliente, instruccion);
            break;
            
        case SOLICITAR_ESPACIO: 
            int tamanio;
            recv(cliente, &tamanio, sizeof(int), 0);
            bool hay_espacio = true;
            send(cliente, &hay_espacio, sizeof(bool), 0);
            log_info(logger_memoria, "## PID: <PID> - Solicitud de espacio de tamaño: %d", tamanio);
            break;
        case PROCESOS_CREADOS: //op_code que indica: "Ey Memoria, dame la lista de procesos creados"
            int pid;
            recv(cliente, &pid, sizeof(int), 0);
            int tamanio = tamanio_particion_por_pid(pid); // Obtener el tamaño de la partición
            log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", pid, tamanio); //ESTO ES LOG OBLIGATORIO
            enviar_mensaje(cliente, "OK");
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
/*void manejar_cliente_kernel(int cliente) {
    
    close(cliente);
}

//manejar_cliente_cpu(cliente);
void manejar_cliente_cpu(int cliente) {
    
    close(cliente);
}*/




/* void manejar_cliente_kernel(int socket)
{
    op_code operacion = recibir_operacion_from_paquete(socket);
    switch (operacion)
    {
    case CREATE_PROCESS:
        t_proceso_paquete *proceso_paquete = recibir_proceso(socket);
        log_obligatorio_proceso(CREACION, proceso_paquete->pid, proceso_paquete->tamanio);
        crear_proceso(socket, proceso_paquete);
        //en crear_proceso ya se esta enviando la respuesta
        break;
    case END_PROCESS:
        int pid = recibir_id(socket);
        log_obligatorio_proceso(DESTRUCCION, pid, tamanio_particion_por_pid(pid));
        finalizar_proceso(pid);
        enviar_estado_solicitud(socket, OK);
        break;
    case CREATE_THREAD:
        t_hilo_paquete *hilo_paquete = recibir_hilo(socket);
        log_obligatorio_hilo(CREACION, hilo_paquete->pid, hilo_paquete->tid);
        crear_contexto_hilo(hilo_paquete);
        enviar_estado_solicitud(socket, OK);
        break;
    case END_THREAD:
        t_thread_to_end_request *thread_to_end_request = recibir_thread_to_end(socket);
        log_obligatorio_hilo(DESTRUCCION, thread_to_end_request->pid, thread_to_end_request->tid);
        finalizar_hilo(thread_to_end_request->pid, thread_to_end_request->tid);
        enviar_estado_solicitud(socket, OK);
        free(thread_to_end_request);
        break;
    case MEMORY_DUMP:
        t_memory_dump_request *memory_dump_request = recibir_memory_dump_request(socket);
        log_obligatorio_memory_dump(memory_dump_request->pid, memory_dump_request->tid);
        memory_dump(memory_dump_request->pid, memory_dump_request->tid, socket);
        //en memory_dump ya se esta enviando la respuesta
        free(memory_dump_request);
        break;
    default:
        log_warning(memoria_logger, "Operación no manejable en kernel. op_code: %d", operacion);
        break;
    }
}

void manejar_cliente_cpu(int socket)
{
    int crashes = 0;
    while (1)
    {
        if(crashes >= 3){
            log_warning(memoria_logger, "Es probable que el servidor de CPU se haya cerrado. Se aborta el manejo de cliente");
            close(socket);
            exit(EXIT_FAILURE);
        }
        op_code operacion = recibir_operacion_from_paquete(socket);
        switch (operacion)
        {
        case GET_CONTEXTO_EJECUCION:
            t_get_contexto_ejecucion_request *get_contexto_ejecucion_request = recibir_get_contexto_ejecucion_request(socket);
            log_obligatorio_contexto(SOLICITADO, get_contexto_ejecucion_request->pid, get_contexto_ejecucion_request->tid);
            t_new_contexto_paquete *contexto_ejecucion = obtener_contexto_hilo(get_contexto_ejecucion_request->pid, get_contexto_ejecucion_request->tid);
            poblar_contexto_hilo_con_particion(contexto_ejecucion, get_contexto_ejecucion_request->pid);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_contexto_hilo(socket, OK, contexto_ejecucion);
            free(get_contexto_ejecucion_request);
            break;
        case UPD_CONTEXTO_EJECUCION:
            t_upd_contexto_paquete *upd_contexto_paquete = recibir_upd_contexto(socket);
            log_obligatorio_contexto(ACTUALIZADO, upd_contexto_paquete->pid, upd_contexto_paquete->tid);
            actualizar_contexto_hilo(upd_contexto_paquete);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_estado_solicitud(socket, OK);
            free(upd_contexto_paquete);
            break;
        case GET_INSTRUCCION:
            t_get_instruccion_request *get_instruccion_request = recibir_get_instruccion_request(socket);
            char *instruccion = obtener_instruccion_with_PC(get_instruccion_request->pid, get_instruccion_request->tid, get_instruccion_request->pc);
            log_obligatorio_instruccion(get_instruccion_request->pid, get_instruccion_request->tid, instruccion);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            if(string_contains(instruccion, "ERROR"))
            {
                enviar_estado_solicitud(socket, ERROR);
                break;
            }
            enviar_instruccion_a_cpu(instruccion, socket);
            free(get_instruccion_request);
            break;
        case READ_MEM:
            //CPU envia pid, tid y direccion fisica. Pero no valido la direccion fisica
            t_memory_read_request *read_mem_request = recibir_memory_read_request(socket);
            log_obligatorio_memoria_usuario(LECTURA, read_mem_request->pid, read_mem_request->tid,
                                            read_mem_request->direccion_fisica, BYTES_PER_OPERATION);
            void *buffer = malloc(BYTES_PER_OPERATION);
            leer_memoria(read_mem_request->direccion_fisica, buffer);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_respuesta_lectura_memoria_OK(socket, buffer);
            free(read_mem_request);
            break;
        case WRITE_MEM:
            //CPU envia pid, tid, direccion fisica y los bytes a escribir. Pero no valido la direccion fisica
            t_memory_write_request *write_mem_request = recibir_memory_write_request(socket);
            log_obligatorio_memoria_usuario(ESCRITURA, write_mem_request->pid, write_mem_request->tid,
                                            write_mem_request->direccion_fisica, BYTES_PER_OPERATION);
            escribir_memoria(write_mem_request->direccion_fisica, write_mem_request->buffer);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_estado_solicitud(socket, OK);
            free(write_mem_request->buffer);
            free(write_mem_request);
            break;
        default:
            log_warning(memoria_logger, "Operación no manejable en CPU. op_code: %d", operacion);
            crashes++;
            break;
        }
    }
}
     */

