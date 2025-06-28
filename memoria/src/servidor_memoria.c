#include <servidor_memoria.h>

t_memoria_config memoria_config;
t_log* logger_memoria;
void* contenido;
int tamanio_total;
int tamanio_disponible_en_memoria;

void leer_config(){ // Lee la config y guarda todos los values de las key (struct en el header)
    t_config* config = config_create("memoria.config");
    if (config == NULL) {
        printf("Error al leer el archivo de configuración\n");
        exit(EXIT_FAILURE);
    }
    memoria_config.PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
    memoria_config.TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    memoria_config.TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
    memoria_config.ENTRADAS_POR_TABLA = config_get_int_value(config, "ENTRADAS_POR_TABLA");
    memoria_config.CANTIDAD_NIVELES = config_get_int_value(config, "CANTIDAD_NIVELES");
    memoria_config.RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    memoria_config.PATH_SWAPFILE = strdup(config_get_string_value(config, "PATH_SWAPFILE"));
    memoria_config.RETARDO_SWAP = config_get_int_value(config, "RETARDO_SWAP");
    memoria_config.DUMP_PATH = strdup(config_get_string_value(config, "DUMP_PATH"));
    config_destroy(config); //destruye luego de guardarse los values
}
void iniciar_logger_memoria(){
    logger_memoria = log_create("memoria.log", "[Memoria]", true, LOG_LEVEL_TRACE);
    if (logger_memoria == NULL) {
        printf("Error al crear el logger de memoria\n");
        exit(EXIT_FAILURE);
    }
    //despues ver de cambiar el tipo 
}
void iniciar_servidor_memoria() { // Inicia el servidor multihilos para atender peticiones
    char puerto[6]; // Buffer para almacenar el puerto como cadena (maximo 5 digitos + '\0')
    sprintf(puerto, "%d", memoria_config.PUERTO_ESCUCHA); // Convierte el puerto a cadena.  A puerto le asigna el valor de memoria_config.puerto_escucha

    int servidor_memoria = iniciar_servidor(puerto, logger_memoria, "Se ha iniciado el servidor de Memoria");
    if (servidor_memoria == -1){
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
        pthread_detach(hilo_cliente); // Desvincula el hilo para que se libere automaticamente
    }
    close(servidor_memoria);
}
void *manejar_cliente(void *socketCliente) // Esta funcion maneja la conexion con el cliente dependiendo de que modulo venga
{   /*
    int tamPag = memoria_config.TAM_PAGINA;
    int entradasTabla = memoria_config.ENTRADAS_POR_TABLA;
    int cantNiveles = memoria_config.CANTIDAD_NIVELES;
    */
    int cliente = *((int *)socketCliente); // Desreferencio el puntero para obtener el socket del cliente
    free(socketCliente);
    op_code cliente_id = recibir_op_code(cliente); // !!!!!!!!!!DESPUES VER DE UNIFICAR LA FUNCION Q HIZO JERE EN EL UTILS DE RECIBIR CON UN CODE_OP PERO QUE SEA OP_CODE!!!!!!!!!!!!!!!!
    switch (cliente_id)
    {
        case HANDSHAKE_KERNEL:
            log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", cliente); //log oblig
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            //int tamanio = recibir_entero(cliente);
            //op_code respuesta = verificar_si_hay_espacio(tamanio);
            //enviar_op_code(cliente,respuesta);
            while (1) {
                op_code peticion_kernel = recibir_op_code(cliente);
                /*if (peticion_kernel == -1) {
                    log_warning(logger_memoria, "CPU desconectado");
                    break;
                }*/
                switch (peticion_kernel){
                    case INICIALIZAR_PROCESO_DESDE_NEW:
                        //Llamar Inicializacion Proceso
                        //inicializacion_proceso();
                        
                        break;
                    case INICIALIZAR_PROCESO_SUSPENDIDO:
                        //Llamar Iniciailizacion Proceso Suspendido
                        //leer_pagina_de_swap();
                        break;
                    case FINALIZAR_PROCESO:
                        //Llamar Finalizar Proceso
                        //destruir_proceso_instrucciones(pid);
                        break;
                    case SUSPENDER_PROCESO:
                        //Llamar Suspender Proceso
                        //escribir_pagina_en_swap();
                        break;    
                    default:
                        log_warning(logger_memoria, "Petición desconocida de Kernel: %d", peticion_kernel);
                        break;
                }
            }
            //manejar_cliente_kernel(cliente); <- HACER ESTA FUNCIONES EN LOS OTROS MODUELOS. 
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
            //peticiones
            while (1) {
                op_code peticion = recibir_op_code(cliente);
                /*if (peticion == -1) {
                    log_warning(logger_memoria, "CPU desconectado");
                    break;
                }*/
                switch (peticion){
                    case FETCH_INSTRUCCION:
                        manejar_fetch_cpu(cliente);
                        break;
                    case READ_MEMORIA:
                        manejar_read_memoria(cliente);
                        break;
                    case WRITE_MEMORIA:
                        manejar_write_memoria(cliente);
                        break;
                    case DUMP_MEMORY:
                        manejar_dump_memory(cliente, contenido);
                        break;    
                    default:
                        log_warning(logger_memoria, "Petición desconocida de CPU: %d", peticion);
                        break;
                }
            }
        default:
            log_debug(logger_memoria,"CLIENTE DESCONOCIDO");
            break;
}
    close(cliente);
    return NULL;
}
void manejar_fetch_cpu(int socket_cpu){
    //paquete con el PC y el PID
    t_paquete* paquete = recibir_paquete(socket_cpu);
    int pc = deserializar_pc(paquete);    // ya está en utils
    int pid = deserializar_pid(paquete);  // ya está en utils

    char* instruccion = obtener_instruccion(pid, pc);
    if (!instruccion) {
        log_error(logger_memoria, "Instrucción no encontrada para PID %d - PC %d", pid, pc);
        return;
    }
    log_info(logger_memoria, "## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s>", pid, pc, instruccion); //log oblig
    free(instruccion);
    //t_buffer* buffer = crear_buffer_instruccion(instruccion); -. HABLAR CON FEDE JERE
    //crear_paquete(ENVIO_INSTRUCCION, buffer, socket_cpu);
    //Sumo a la metrica de instrucciones solicitadas
    listado_metricas.instrucciones_solicitadas++;
}

void manejar_read_memoria(int socket_cpu) {
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int pid = deserializar_pid(paquete);
    int direccion_fisica = deserializar_entero_desde_stream(paquete);
    int tamanio = deserializar_entero_desde_stream(paquete);

    void* buffer = malloc(tamanio);
    leer_espacio_usuario(buffer, direccion_fisica, tamanio);  // ya lo tenés

    log_info(logger_memoria, "## PID: <%d> - Lectura - Dir. Física: <%d> - Tamaño: <%d>", pid, direccion_fisica, tamanio); //log oblig

    send(socket_cpu, buffer, tamanio, 0);
    free(buffer);
}
void manejar_write_memoria(int socket_cpu) {
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int pid = deserializar_pid(paquete);
    int direccion_fisica = deserializar_entero_desde_stream(paquete); // función auxiliar
    int tamanio = deserializar_entero_desde_stream(paquete);

    void* valor = malloc(tamanio);
    memcpy(valor, paquete->buffer->stream + paquete->buffer->offset, tamanio);
    escribir_espacio_usuario(direccion_fisica, valor, tamanio);

    log_info(logger_memoria, "## PID: <%d> - Escritura - Dir. Física: <%d> - Tamaño: <%d>", pid, direccion_fisica, tamanio); //log oblig

    free(valor);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}
void manejar_dump_memory(int socket_cpu, void* contenido){
    t_paquete* paquete = recibir_paquete(socket_cpu);

    int pid = deserializar_pid(paquete);
    log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", pid); //log oblig
    int direccion_fisica = deserializar_entero_desde_stream(paquete); // función auxiliar
    int tamanio = deserializar_entero_desde_stream(paquete);

    void* valor = malloc(tamanio);
    memcpy(valor, paquete->buffer->stream + paquete->buffer->offset, tamanio);

    if (direccion_fisica < 0 || tamanio <= 0) {
        log_error(logger_memoria, "Dump fallido: dirección o tamaño inválido");
        return;
    }
    // Obtener timestamp con formato YYYYMMDDHHMMSS
    char* timestamp = temporal_get_string_time("%Y%m%d%H%M%S");
    // Crear nombre del archivo: <DUMP_PATH>/<PID>-<TIMESTAMP>.dmp
    char* nombre_archivo = string_from_format("%s/%d-%s.dmp", memoria_config.DUMP_PATH, pid, timestamp);

    // Crear y abrir archivo
    FILE* archivo = fopen(nombre_archivo, "wb");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo crear el archivo dump: %s", nombre_archivo);
        free(timestamp);
        free(nombre_archivo);
        return;
    }
    // Escribir memoria del proceso en el archivo
    //fwrite(contenido, 1, tamanio_marco, archivo);
    fclose(archivo);

    log_debug(logger_memoria, "Dump de memoria generado para PID %d en %s", pid, nombre_archivo);

    free(timestamp);
    free(nombre_archivo);
}