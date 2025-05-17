#include <utils/utils.h>

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

int iniciar_servidor(char *puerto, t_log *un_log, char *mensaje)
{
	int socket_servidor;
	struct addrinfo hints, *servinfo; //*p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
 	socket_servidor = socket (servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	// Asociamos el socket a un puerto
		bind(socket_servidor,servinfo->ai_addr,servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor,SOMAXCONN);

	freeaddrinfo(servinfo);
	log_info(un_log, "SERVER: %s",mensaje);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log *un_log, char *mensaje)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(un_log,"Se conecto el cliente: %s",mensaje);
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

void enviar_entero(int socket_cliente, int numero)
{
	send(socket_cliente, &numero, sizeof(numero), MSG_WAITALL);
}

int recibir_entero(int fd_conexion){
	int entero;
	recv(fd_conexion, &entero, sizeof(entero), MSG_WAITALL);
	return entero;
}

void enviar_op_code(int socket_cliente, op_code codigo_operacion)
{
	enviar_entero(socket_cliente, codigo_operacion);
}

op_code recibir_op_code (int socket)
{
	op_code codigo_operacion;
	recv(socket, &codigo_operacion, sizeof(codigo_operacion), 0);
	return codigo_operacion;
}

void enviar_mensaje(int socket_cliente, char* mensaje) {
    int longitud = strlen(mensaje) + 1; //obtener log
    send(socket_cliente, &longitud, sizeof(int), 0); // no se si esta bien pero es para que el receptor sepa cuantos bytes debe leer la longitud del mensaje
    send(socket_cliente, mensaje, longitud, 0); 
}

t_buffer *crear_buffer(){
	t_buffer *buffer_aux = malloc (sizeof(t_buffer));
	return buffer_aux;
}

t_buffer * crear_buffer_cpu(int pc, int pid){ //esto se lo manda kernel a cpu
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pc, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer * devolver_pid_a_kernel(int pid){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer * crear_buffer_instruccion_init_proc(char* ruta_del_archivo, int tamanio_en_memoria){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = strlen(ruta_del_archivo);
	buffer_aux->size = sizeof(int) + longitud;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.

	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, ruta_del_archivo,longitud);
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &tamanio_en_memoria, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);

	buffer_aux -> stream = buffer_aux-> stream;

	return buffer_aux;
}
t_buffer * crear_buffer_instruccion_io (char* nombre, int milisegundos){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = strlen(nombre);
	buffer_aux->size = sizeof(int) + longitud;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.

	memcpy(buffer_aux->stream + buffer_aux->offset, &milisegundos, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, nombre,longitud);
	buffer_aux->offset += sizeof(int);
	

	buffer_aux -> stream = buffer_aux-> stream;

	return buffer_aux;
}


t_buffer *crear_buffer_vacio(){
	t_buffer *buffer_aux = crear_buffer();
	return buffer_aux;	
}


t_buffer *crear_buffer_io_nombre(char *nombre){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = strlen(nombre);
	buffer_aux->size = sizeof(int) + longitud;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.

	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, nombre,longitud);
	
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

void crear_paquete(op_code codigo, t_buffer *buffer, int socket){
	
	//llenamos el paquete con el buffer
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo; // Podemos usar una constante por operación
	paquete->buffer = buffer; // Nuestro buffer de antes.
	//armamos el stream a enviar
	void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code)+sizeof(int)); // el tamanio que hay que reservar segun el buffer + el tamanio del op_code + el tamanio del buffer
	paquete->buffer->offset = 0;
	
	memcpy((char *)a_enviar + paquete->buffer->offset, &(paquete->codigo_operacion), sizeof(op_code));
	paquete->buffer->offset += sizeof(op_code);
	memcpy((char *)a_enviar + paquete->buffer->offset, &(paquete->buffer->size), sizeof(int)); // el tamanio del buffer
	paquete->buffer->offset += sizeof(int);
	memcpy((char *)a_enviar + paquete->buffer->offset, paquete->buffer->stream, paquete->buffer->size);
	//enviamos con send()
	send(socket,(char*)a_enviar, buffer-> size + sizeof(op_code) + sizeof(int),0);
	
	//liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_paquete* recibir_paquete(int socket){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	// Primero recibimos el codigo de operacion
	recv(socket, &paquete->codigo_operacion, sizeof(paquete->codigo_operacion), 0);

	// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
	recv(socket, &(paquete->buffer->size), sizeof(int), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);
	return paquete;
}

char *deserializar_nombre_io(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
    int longitud;
    memcpy(&longitud,stream,sizeof(int));
        stream+=sizeof(int);
    char *nombre = malloc(longitud+1);
        memcpy(nombre,stream,longitud);
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return nombre;
}

int deserializar_pid(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
    int pid;
    memcpy(&pid,stream,sizeof(int));
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return pid;
}

int deserializar_pc(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+=sizeof(int);
    int pc;
    memcpy(&pc,stream,sizeof(int));
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return pc;
}

//para lo de deserializar init proc viene primero el tamanio en memoria