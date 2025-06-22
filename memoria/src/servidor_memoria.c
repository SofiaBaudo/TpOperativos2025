#include <servidor_memoria.h>

//Declaracion de Variables Globales

int tamanio_disponible_en_memoria = 30;
t_memoria_config memoria_config;
t_log* logger_memoria;

//Funcion para Leer Configuraciones

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

//Funcion para Inicializar Loggers de Memoria

void iniciar_logger_memoria(){
    logger_memoria = log_create("memoria.log", "[Memoria]", true, LOG_LEVEL_TRACE);
    if (logger_memoria == NULL) {
        printf("Error al crear el logger de memoria\n");
        exit(EXIT_FAILURE);
    }
    //despues ver de cambiar el tipo 
}

//Funcion para Inicializar Servidor de Memoria

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

//Funcion para Manejar Cliente de Memoria(Depende del Modulo)

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
            int tamanio = recibir_entero(cliente);
            op_code respuesta = verificar_si_hay_espacio(tamanio);
            enviar_op_code(cliente,respuesta);
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
                switch (peticion) {
                    case FETCH_INSTRUCCION:
                        manejar_fetch_cpu(cliente);
                        break;
                    case READ_MEMORIA:
                        manejar_read_memoria(cliente);
                        break;
                    case WRITE_MEMORIA:
                        manejar_write_memoria(cliente);
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

//Funcion para Manejar la Instruccion de CPU (PC y el PID)

void manejar_fetch_cpu(int socket_cpu){
    //paquete con el PC y el PID
    t_paquete* paquete = recibir_paquete(socket_cpu);
    int pc = deserializar_pc(paquete);    // ya está en utils
    int pid = deserializar_pid(paquete);  // ya está en utils

    t_instruccion* instruccion = obtener_instruccion(pid, pc);
    if (!instruccion) {
        log_error(logger_memoria, "Instrucción no encontrada para PID %d - PC %d", pid, pc);
        return;
    }

    char* codigo_str = instruccion_a_string(instruccion->codigo); //hace q en vex de q diga instruccion 6 a q diga instruccion READ, ponele
    log_info(logger_memoria, "## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s>", pid, pc, codigo_str); //log oblig
    free(codigo_str);


    t_buffer* buffer = crear_buffer_instruccion(instruccion);
    crear_paquete(ENVIO_INSTRUCCION, buffer, socket_cpu);

    //Sumo a la metrica de instrucciones solicitadas
    listado_metricas.instrucciones_solicitadas++;
}

//Funcion para Manejar la Instruccion de Read de CPU

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

//Funcion para Manejar la Instruccion de Write de CPU

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

//Funcion de Verificar Espacio
//VER DONDE VA O SI DEBEMOS SACARLA(SI RESPONDEMOS EN OTRA PARTE)

op_code verificar_si_hay_espacio(int tamanio){
    if(tamanio>tamanio_disponible_en_memoria){
        return NO_HAY_ESPACIO_EN_MEMORIA;
    }
    else{
        tamanio_disponible_en_memoria-=tamanio;
        return HAY_ESPACIO_EN_MEMORIA;
    }
}