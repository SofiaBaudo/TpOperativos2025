#include <inicializarIO.h>

void inicializar_IO(char *nombre){
    printf("IO inicializado");
    inicializar_logs();
    inicializar_configs();
    fd_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL);
    enviar_op_code(fd_kernel, HANDSHAKE_IO);//avisa que es IO.
    int respuesta = recibir_op_code(fd_kernel);
    if(respuesta == HANDSHAKE_ACCEPTED){
        log_info(io_logger, "Conexion con Kernel exitosa");
        /*int longitud_nombre = strlen(nombre);
        send(fd_kernel, &longitud_nombre,sizeof(int),0); //le manda primero la longitud para que el kernel sepa cuanto espacio reservar
        send(fd_kernel,nombre,longitud_nombre,0); //sin el & porque ya es un puntero
        */
    t_buffer *buffer_aux = crear_buffer_io_nombre(nombre);
    crear_paquete(IO_NOMBRE,buffer_aux,fd_kernel);
    esperar_peticion();
    enviar_op_code(fd_kernel,DESCONEXION_IO);
    }
}

void inicializar_logs(){
    io_logger = log_create("IO.log","LOG_MODULO_IO",1,LOG_LEVEL_INFO);
    if(!io_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    /* kernel_debug_log = log_create("kernel.log","LOG_MODULO_KERNEL",1,LOG_LEVEL_TRACE); // TRACE ES EL NIVEL MAS ALTO DE TODOS
    if(!kernel_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    */
}

void inicializar_configs(){
config_io = crear_config("/home/utnso/tp-2025-1c-Sinergia-SO-13/io/io.config");
IP_KERNEL = config_get_string_value(config_io,"IP_KERNEL");
PUERTO_KERNEL = config_get_string_value(config_io,"PUERTO_KERNEL");
LOG_LEVEL = config_get_string_value(config_io,"LOG_LEVEL");
}