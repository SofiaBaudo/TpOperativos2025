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
            log_debug(logger_memoria,"Ya volvi de la funcion");
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
        //nos mande un opcode
        op_code peticion_kernel = -2;
        enviar_op_code(cliente, MEMORIA_LISTA);
        while (peticion_kernel != DESCONEXION_KERNEL){
            peticion_kernel = recibir_op_code(cliente);
            log_debug(logger_memoria, "Peticion recibida de KERNEL: %s", instruccion_a_string(peticion_kernel));
            switch (peticion_kernel){
                case INICIALIZAR_PROCESO_DESDE_NEW:{
                    t_paquete *proceso_paquete = recibir_paquete(cliente);
                    log_debug(logger_memoria, "Termine de recibir proceso");
                    int pid = deserializar_pid_memoria(proceso_paquete);
                    int tam_proceso = deserializar_tamanio_memoria(proceso_paquete);
                    char *path_pseudocodigo = deserializar_nombre_archivo_memoria(proceso_paquete);
                    log_debug(logger_memoria, "el pseudocodigo es: %s", path_pseudocodigo);
                    if(inicializar_proceso(pid,tam_proceso,path_pseudocodigo)){
                        log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", pid, tam_proceso);
                        enviar_op_code(cliente, ACEPTAR_PROCESO);
                        break;
                    } 
                    else {
                        enviar_op_code(cliente, RECHAZO_PROCESO);
                        break;
                    }
                }
                case INICIALIZAR_PROCESO_SUSPENDIDO: {
                    t_paquete *proceso_paquete = recibir_paquete(cliente);
                    int pid = deserializar_pid_memoria(proceso_paquete);
                    int tam_proceso = deserializar_tamanio_memoria(proceso_paquete);
                    //char *path_pseudocodigo = deserializar_nombre_archivo_memoria(proceso_paquete);
                    if (!proceso_paquete) {
                        log_error(logger_memoria, "Error al recibir paquete de proceso suspendido");
                        enviar_op_code(cliente, RECHAZO_PROCESO);
                        break;
                    }
                    // Intentar reanudar el proceso desde SWAP
                    reanudar_proceso_desde_kernel(pid, tam_proceso,cliente);
                    // Liberar memoria del paquete
                    //free(path_pseudocodigo);
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
                    log_warning(logger_memoria, "Peticion desconocida: %d", peticion_kernel);
                    log_debug(logger_memoria,"Se desconecto Kernel");
                    break;
            }
        }
    log_debug(logger_memoria,"Ya sali del switch y estoy por cerrar la conexion");
    close(cliente);
    return;
}

void manejar_cliente_cpu(int cliente){
    while (1) {
        //op_code peticion = recibir_op_code(cliente);
        t_paquete *pedido = recibir_paquete(cliente);
        op_code peticion = obtener_codigo_de_operacion(pedido);
        log_debug(logger_memoria, "Peticion recibida de CPU: %s", instruccion_a_string(peticion));
        switch (peticion){
            case FETCH_INSTRUCCION: {
             // Recibir struct pedido de instrucción
               // enviar_op_code(cliente,MANDAR_PID_Y_PC_FETCH);
                //t_paquete *pedido = recibir_paquete(cliente);
                int pid;
                int pc;
                deserializar_pid_y_pc(pedido,&pid,&pc);
                log_debug(logger_memoria,"EL pid es %i", pid);
                log_debug(logger_memoria, "El pc es %i", pc);
                //int pid = deserializar_pid_memoria(pedido);
               //int pc = deserializar_pc_memoria(pedido);
                //log_debug(logger_memoria,"EL pid es %d", pid);
                //log_debug(logger_memoria, "El pc es %d", pc);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir pedido de instrucción");
                    break;
                }
                // Obtener la instrucción correspondiente
                char* instruccion = obtener_instruccion_proceso(pid, pc);

                // Log obligatorio de obtención de instrucción
                if (instruccion != NULL) {
                    log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion);
                    usleep(3000000);
                } else {
                    log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: ", pid, pc);
                }

                // Enviar solo la instrucción a la CPU
                if (instruccion != NULL) {
                    enviar_instruccion(cliente, instruccion);
                    log_debug(logger_memoria, "Se envió instrucción a CPU: PID %d, PC %d, Instr: %s", pid, pc, instruccion);
                    free(instruccion);
                }else{
                    enviar_instruccion(cliente, "");
                    log_error(logger_memoria, "Se envió instrucción a CPU: PID %d, PC %d, Instr: ", pid, pc);
                }
                //free(pedido);
                break;
            }
            case READ_MEMORIA: {
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
            case ACCESO_TABLA_PAGINAS: {
                // Recibir pedido de acceso a tabla de páginas
                t_pedido_acceso_tabla_paginas* pedido = recibir_pedido_acceso_tabla_paginas(cliente);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir pedido de acceso a tabla de páginas");
                    break;
                }

                // Obtener el número de marco navegando la tabla multinivel
                int marco = obtener_marco_de_pagina_logica(pedido->pid, pedido->pagina_logica);
                
                // Enviar el número de marco a la CPU
                enviar_numero_marco(cliente, marco);

                free(pedido);
                break;
            }
            case LEER_PAGINA_COMPLETA: {
                // Recibir pedido de lectura de página completa
                t_pedido_leer_pagina_completa* pedido = recibir_pedido_leer_pagina_completa(cliente);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir pedido de lectura de página completa");
                    break;
                }

                // Obtener el contenido completo de la página
                void* contenido = obtener_contenido_pagina_completa(pedido->marco, memoria_config.TAM_PAGINA);
                
                if (contenido != NULL) {
                    // Enviar el contenido a la CPU
                    enviar_contenido_pagina(cliente, contenido, memoria_config.TAM_PAGINA);
                    free(contenido);
                } else {
                    log_error(logger_memoria, "Error al obtener contenido de página completa - Marco: %d", pedido->marco);
                    // Enviar una página vacía en caso de error
                    void* pagina_vacia = calloc(1, memoria_config.TAM_PAGINA);
                    enviar_contenido_pagina(cliente, pagina_vacia, memoria_config.TAM_PAGINA);
                    free(pagina_vacia);
                }

                free(pedido);
                break;
            }
            case ACTUALIZAR_PAGINA_COMPLETA: {
                // Recibir pedido de actualización de página completa
                t_pedido_actualizar_pagina_completa* pedido = recibir_pedido_actualizar_pagina_completa(cliente);
                if (pedido == NULL) {
                    log_error(logger_memoria, "Error al recibir pedido de actualización de página completa");
                    break;
                }

                // Actualizar el contenido de la página
                bool exito = actualizar_contenido_pagina_completa(pedido->marco, pedido->contenido, pedido->tam_pagina);
                
                // Enviar confirmación a la CPU
                enviar_confirmacion_actualizacion(cliente, exito);

                destruir_pedido_actualizar_pagina_completa(pedido);
                break;
            }
            case ENVIO_PID_Y_ENTRADANIVEL: {//hablar de cambiar el nombre
                // Recibir pedido con PID y entrada de nivel para navegación de tabla de páginas
                // Deserializar PID y entrada de nivel
                int offset = 0;
                int pid = 
                
                // NOTA: CPU está enviando entrada de nivel, pero necesitamos página lógica
                // Por compatibilidad, interpretamos entradaNivel como número de página lógica
                int marco = obtener_marco_de_pagina_logica(pid, entradaNivel);
                
                // Enviar el marco de vuelta a CPU
                send(cliente, &marco, sizeof(int), 0);
                
                log_debug(logger_memoria, "PID: %d - Enviado marco: %d para página: %d", pid, marco, entradaNivel);
                
                free(buffer->stream);
                free(buffer);
                break;
            }
            case ENVIO_PID_NROPAG: { //hablar de cambiar el nombre
                // Recibir pedido con PID y número de página para obtener contenido
                log_debug(logger_memoria,"Entre a envio pid y nropag");
                usleep(2000000);
                int pid = deserializar_pid_memoria(pedido);
                log_debug(logger_memoria, "el numero de pid es %i", pid);
                int nroPag = deserializar_nroPag(pedido);
                log_debug(logger_memoria, "el numero de pagina es %i", nroPag);
            
                int marco = obtener_marco_de_pagina_logica(pid, nroPag);
                log_debug(logger_memoria, "el marco es: %i", marco);
                if (marco != -1) {
                    log_debug(logger_memoria, "entre al if del marco");
                    usleep(2000000);
                    // Obtener el contenido completo de la página
                    void* contenido = obtener_contenido_pagina_completa(marco, memoria_config.TAM_PAGINA);
                    log_debug(logger_memoria, "contenido obtenido");
                    if (contenido != NULL) {
                        log_debug(logger_memoria, "entre al if del if porque el contenido no es NULL");
                        usleep(2000000);
                        // NOTA: CPU espera un puntero, pero esto no funciona entre procesos
                        // Por compatibilidad, enviamos el puntero local pero esto necesita ser corregido en CPU
                        send(cliente, &contenido, sizeof(void*), 0);
                        
                        log_debug(logger_memoria, "PID: %d - Enviado puntero contenido para página: %d, marco: %d", pid, nroPag, marco);
                        
                        // Liberar el contenido que obtuvimos
                        free(contenido);
                    } else {
                        log_debug(logger_memoria, "entre al else del if");
                        // Enviar puntero nulo en caso de error
                        void* contenido_nulo = NULL;
                        send(cliente, &contenido_nulo, sizeof(void*), 0);
                        log_error(logger_memoria, "Error al obtener contenido - PID: %d, Página: %d", pid, nroPag);
                    }
                } else {
                    log_debug(logger_memoria, "entre al else del if grande");
                    // Enviar puntero nulo si no se encuentra el marco
                    void* contenido_nulo = NULL;
                    send(cliente, &contenido_nulo, sizeof(void*), 0);
                    log_error(logger_memoria, "Marco no encontrado - PID: %d, Página: %d", pid, nroPag);
                }
                break;
            } 
            default:
                log_warning(logger_memoria, "Peticion desconocida de CPU: %d", peticion);
                break;
        }
    }
}
