#include <servidor_memoria.h>

int tamanio_disponible_en_memoria = 30;
t_memoria_config memoria_config;
t_log* logger_memoria;

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

//HACER LA CONEXION CON LOS OTROS MODULOS PARA QUE EL SERVIDOR NO SE QUEDE ESPERANDO

void iniciar_servidor_memoria() { // Inicia el servidor multihilos para atender peticiones
    char puerto[6]; // Buffer para almacenar el puerto como cadena (máximo 5 dígitos + '\0')
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
        pthread_detach(hilo_cliente); // Desvincula el hilo para que se libere automáticamente
    }
    close(servidor_memoria);
}

void *manejar_cliente(void *socketCliente) // Esta función maneja la conexión con el cliente dependiendo de que modulo venga
{   
    int tamPag = memoria_config.TAM_PAGINA;
    int entradasTabla = memoria_config.ENTRADAS_POR_TABLA;
    int cantNiveles = memoria_config.CANTIDAD_NIVELES;
    int cliente = *((int *)socketCliente); // Desreferencio el puntero para obtener el socket del cliente
    free(socketCliente);
    op_code cliente_id = recibir_op_code(cliente); // !!!!!!!!!!DESPUES VER DE UNIFICAR LA FUNCION Q HIZO JERE EN EL UTILS DE RECIBIR CON UN CODE_OP PERO QUE SEA OP_CODE!!!!!!!!!!!!!!!!
    switch (cliente_id)
    {
        case HANDSHAKE_KERNEL:
            //LOG_INFO : ES EL LOG OBLIGATORIO
            log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", cliente);
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            int tamanio = recibir_entero(cliente);
            op_code respuesta = verificar_si_hay_espacio(tamanio);
            enviar_op_code(cliente,respuesta);
            //manejar_cliente_kernel(cliente); <- HACER ESTA FUNCIONES EN LOS OTROS MODUELOS. 
            break;
        case HANDSHAKE_CPU:

            log_debug(logger_memoria, "Se conecto CPU");
            enviar_op_code(cliente, HANDSHAKE_ACCEPTED);
            t_buffer* buffer = crear_buffer_tamPag_entradasTabla_cantNiveles(tamPag, entradasTabla, cantNiveles);
            crear_paquete(ENVIO_TAMPAG_ENTRADASTABLA_CANTIDADNIVELES,buffer, cliente);
            //manejar_cpu(cpu);
            break;
        default:
            log_warning(logger_memoria, "No se pudo identificar al cliente; op_code: %d", cliente_id); //AVISA Q FUCNIONA MAL
            break;
    }
    close(cliente);
    return NULL;
}

op_code verificar_si_hay_espacio(int tamanio){
    if(tamanio>tamanio_disponible_en_memoria){
        return NO_HAY_ESPACIO_EN_MEMORIA;
    }
    else{
        tamanio_disponible_en_memoria-=tamanio;
        return HAY_ESPACIO_EN_MEMORIA;
    }
}