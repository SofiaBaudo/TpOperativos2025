#include <utils/hello.h>


void saludar(char* quien) {
    printf("Hola desde %s!!\n", quien);
}
// FUNCIONES PARA LOS CONFIGS
t_config *crear_config(char* direccion){
    t_config *nuevo_config = config_create(direccion); // se pasa la ruta del archivo .config
    if(!nuevo_config){
        perror("No se pudo crear el config");
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void destruir_config(t_config *config){
config_destroy(config);
}

// FUNCIONES PARA LOS LOGS

t_log *iniciar_logger(char* archivoLog, char* nombreLog){
    t_log *nuevo_logger = log_create(archivoLog,nombreLog,1,LOG_LEVEL_INFO);
    if(!nuevo_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
} //EN REALIDAD LOS LOGS ES MEJOR IR DEFINIENDOLOS A CONVENIENCIA PORQUE TIENEN DIFERENTES NIVELES

void destruir_logger(t_log *logger){
    log_destroy(logger);
}

//funcion del cliente

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket (server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);
connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);


	freeaddrinfo(server_info);

	return socket_cliente;
}

// FUNCIONES PARA EL SERVIDOR 
int iniciar_servidor(char *puerto,t_log *un_log, char *mensaje)
{
	struct addrinfo hints, *servinfo; //*p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	
 int socket_servidor = socket (servinfo->ai_family,
								servinfo->ai_socktype,
								servinfo->ai_protocol);
	
bind(socket_servidor,servinfo->ai_addr,servinfo->ai_addrlen);
	
listen(socket_servidor,SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(un_log, "Server: %s", mensaje);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log *un_log)
{
	
	int socket_cliente = accept(socket_servidor, NULL, NULL);;
	log_info(un_log, "Se conecto un cliente!");

	return socket_cliente;
}


int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}