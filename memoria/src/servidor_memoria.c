#include "servidor_memoria.h"


void iniciar_servidor_memoria() {
    char puerto_escucha_str[10];
    snprintf(puerto_escucha_str, sizeof(puerto_escucha_str), "%d", memoria_config.PUERTO_ESCUCHA);
    int servidor_memoria = iniciar_servidor(puerto_escucha_str, logger_memoria, "Se ha iniciado el servidor de Memoria");

    if (servidor_memoria == -1){
        log_error(logger_memoria, "Error al iniciar el servidor de memoria");
        exit(EXIT_FAILURE);
    }
    
    log_debug(logger_memoria, "Servidor de memoria iniciado en el puerto %d", memoria_config.PUERTO_ESCUCHA);

    while (1) {
        int cliente = esperar_cliente(servidor_memoria, logger_memoria, ".........");
        if (cliente == -1) {
            log_error(logger_memoria, "Error al aceptar un cliente");
            continue;
        }
        int *cliente_ptr = malloc(sizeof(*cliente_ptr));
        *cliente_ptr = cliente;
        pthread_t hilo_cliente; 
        pthread_create(&hilo_cliente, NULL, manejar_cliente, (void*) cliente_ptr);
        pthread_detach(hilo_cliente);
    }
    close(servidor_memoria);
}
void* manejar_cliente(void *socketCliente) 
{   
    int cliente = *((int *)socketCliente); // Desreferencio el puntero para obtener el socket del cliente
    free(socketCliente);
    op_code cliente_id = recibir_op_code(cliente); 
    switch (cliente_id)
    {
        case HANDSHAKE_KERNEL:
            log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", cliente); // Log obligatorio
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            manejar_cliente_kernel(cliente);
            break;
        case HANDSHAKE_CPU:
            log_debug(logger_memoria, "Se conecto CPU");
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            t_buffer* buffer = crear_buffer_tamPag_entradasTabla_cantNiveles(
                memoria_config.TAM_PAGINA,
                memoria_config.ENTRADAS_POR_TABLA,
                memoria_config.CANTIDAD_NIVELES
            );
            crear_paquete(ENVIO_TAMPAG_ENTRADASTABLA_CANTIDADNIVELES, buffer, cliente);
            manejar_cliente_cpu(cliente);
            break;
        default:
            log_debug(logger_memoria,"CLIENTE DESCONOCIDO");
            break;
    }
    close(cliente);
    return NULL;
}

void manejar_cliente_kernel(int cliente) {
    while (1) {
        op_code peticion_kernel = recibir_op_code(cliente);
        if (peticion_kernel == -1) {
            log_error(logger_memoria, "Error al recibir peticion de Kernel");
            break; 
        }
        log_debug(logger_memoria, "Peticion recibida de KERNEL: %s", instruccion_a_string(peticion_kernel));
        switch (peticion_kernel){
            // Los OP_CODE que se envian a kernel quedan a eleccion de ellos, total es un int para memoria
            case INICIALIZAR_PROCESO_DESDE_NEW:
                // Recibir el paquete de proceso
                struct t_proceso_paquete *proceso_paquete = recibir_proceso(cliente);
                if(inicializar_proceso(proceso_paquete)){
                    enviar_op_code(cliente, ACEPTAR_PROCESO);
                    log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", proceso_paquete->pid, proceso_paquete->tamanio);
                } else {
                    enviar_op_code(cliente, RECHAZO_PROCESO);
                }
                break;
            case INICIALIZAR_PROCESO_SUSPENDIDO: {
                // Recibir el paquete de proceso
                struct t_proceso_paquete *proceso_paquete = recibir_proceso(cliente);
                if (!proceso_paquete) {
                    log_error(logger_memoria, "Error al recibir paquete de proceso suspendido");
                    enviar_op_code(cliente, RECHAZO_PROCESO);
                    break;
                }
                
                // Intentar reanudar el proceso desde SWAP
                reanudar_proceso_desde_kernel(proceso_paquete->pid, proceso_paquete->tamanio, cliente);
                
                // Liberar memoria del paquete
                free(proceso_paquete->path_pseudocodigo);
                free(proceso_paquete);
                break;
            }
            case FINALIZAR_PROCESO: {
                int pid = recibir_entero(cliente);
                finalizar_proceso(pid);
                log_info(logger_memoria, "## PID: %d - Proceso Destruido ", pid); // Log obligatorio de destrucción de proceso (faltan: metricas y swaps)
                enviar_op_code(cliente, FINALIZACION_CONFIRMADA);
                break;
            }
            case SUSPENDER_PROCESO: {
                // Recibir el PID del proceso a suspender
                int pid = recibir_entero(cliente);
                if (pid <= 0) {
                    log_error(logger_memoria, "PID inválido recibido para suspensión: %d", pid);
                    enviar_op_code(cliente, RECHAZO_PROCESO);
                    break;
                }
                
                // Suspender el proceso
                suspender_proceso_desde_kernel(pid, cliente);
                break;
            }
            case DUMP_MEMORY: {
                int pid = recibir_entero(cliente);
                bool dump_ok = dump_memoria_proceso(pid);
                if (dump_ok) {
                    enviar_op_code(cliente, ACEPTAR_PROCESO); // o DUMP_OK
                } else {
                    enviar_op_code(cliente, RECHAZO_PROCESO); // o DUMP_ERROR
                }
                break;
            }
            default:
                log_warning(logger_memoria, "Peticion desconocida de Kernel: %d", peticion_kernel);
                break;
        }
    }
}

void manejar_cliente_cpu(int cliente) {
    while (1) {
        op_code peticion = recibir_op_code(cliente);
        if (peticion == -1) {
            log_error(logger_memoria, "Error al recibir peticion de CPU");
            break; // Salimos del bucle si hay un error
        }
        log_debug(logger_memoria, "Peticion recibida de CPU: %s", instruccion_a_string(peticion));
        switch (peticion){
            case FETCH_INSTRUCCION: {
                // Recibir struct pedido de instrucción
                t_pedido_instruccion* pedido = recibir_pedido_instruccion(cliente);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir pedido de instrucción");
                    break;
                }

                // Obtener la instrucción correspondiente
                char* instruccion = obtener_instruccion_proceso(pedido->pid, pedido->pc);

                // Log obligatorio de obtención de instrucción
                if (instruccion != NULL) {
                    log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s", pedido->pid, pedido->pc, instruccion);
                } else {
                    log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: ", pedido->pid, pedido->pc);
                }

                // Enviar solo la instrucción a la CPU
                if (instruccion != NULL) {
                    enviar_instruccion(cliente, instruccion);
                    log_debug(logger_memoria, "Se envió instrucción a CPU: PID %d, PC %d, Instr: %s", pedido->pid, pedido->pc, instruccion);
                    free(instruccion);
                } else {
                    enviar_instruccion(cliente, "");
                    log_error(logger_memoria, "Se envió instrucción a CPU: PID %d, PC %d, Instr: ", pedido->pid, pedido->pc);
                }
                free(pedido);
                break;
            }
            case READ_MEMORIA: {
                /*
                 * Espera recibir un struct t_pedido_lectura_memoria con:
                 *   - pid: PID del proceso solicitante
                 *   - direccion_logica: dirección física absoluta (offset global en memoria de usuario, NO lógica ni relativa al proceso)
                 *   - tamanio: cantidad de bytes a leer (puede ser cualquier valor, incluso el tamaño de página)
                 *
                 * La dirección recibida debe ser el offset exacto desde el inicio de memoria_usuario,
                 * y si se solicita una página completa, debe coincidir con el byte 0 de la página.
                 *
                 * No se realiza traducción de direcciones ni paginación aquí: la CPU ya envía la dirección física lista.
                 */
                // Recibir struct pedido de lectura de memoria
                t_pedido_lectura_memoria* pedido = recibir_pedido_lectura_memoria(cliente);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir paquete de READ_MEMORIA");
                    break;
                }

                // Acceso real a memoria física: la dirección recibida es absoluta (offset global)
                char valor_leido[256] = {0};
                int resultado_lectura = leer_memoria_fisica(pedido->direccion_logica, valor_leido, pedido->tamanio);
                if (resultado_lectura != 0) {
                    log_error(logger_memoria, "Acceso fuera de rango en memoria física: offset %d, tamaño %d", pedido->direccion_logica, pedido->tamanio);
                    strcpy(valor_leido, "ERROR_OUT_OF_BOUNDS");
                } else {
                    valor_leido[pedido->tamanio] = '\0'; // Por si es string, para debug
                }

                // Log obligatorio: acceso a espacio de usuario
                log_info(logger_memoria, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d", pedido->pid, pedido->direccion_logica, pedido->tamanio);

                // Enviar valor leído a la CPU
                enviar_valor_leido(cliente, valor_leido, pedido->tamanio);

                free(pedido);
                break;
            }
            case WRITE_MEMORIA: {
                /*
                 * Espera recibir un struct t_pedido_escritura_memoria con:
                 *   - pid: PID del proceso solicitante
                 *   - direccion_logica: dirección física absoluta (offset global en memoria de usuario)
                 *   - tamanio: cantidad de bytes a escribir
                 *   - buffer: datos a escribir (tamanio bytes)
                 *
                 * La dirección recibida debe ser el offset exacto desde el inicio de memoria_usuario.
                 * No se realiza traducción de direcciones ni paginación aquí.
                 */
                t_pedido_escritura_memoria* pedido = recibir_pedido_escritura_memoria(cliente);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir paquete de WRITE_MEMORIA");
                    break;
                }

                // Log obligatorio: acceso a espacio de usuario
                log_info(logger_memoria, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d", pedido->pid, pedido->direccion_logica, pedido->tamanio);

                int resultado = escribir_memoria_fisica(pedido->direccion_logica, pedido->buffer, pedido->tamanio);
                if (resultado != 0) {
                    log_error(logger_memoria, "Acceso fuera de rango en memoria física (escritura): offset %d, tamaño %d", pedido->direccion_logica, pedido->tamanio);
                }
                destruir_pedido_escritura_memoria(pedido);
                break;
            } 
            default:
                log_warning(logger_memoria, "Peticion desconocida de CPU: %d", peticion);
                break;
        }
    }
}
